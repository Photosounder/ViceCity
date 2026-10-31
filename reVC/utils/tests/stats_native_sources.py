"""Check actual statistics field types and packed save contracts, then compile every changed consumer."""
from pathlib import Path
import os,json,sys,shutil,subprocess,re
import stats_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stats-c-tests';overlay=out/'native-overlay';production='--production' in sys.argv
probeDir=out/'native-probes';probeDir.mkdir(exist_ok=True);shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
changes=json.loads((out/'changes.json').read_text());old=(out/'before/src/core/Stats.cpp').read_text();header=(out/'before/src/core/Stats.h').read_text()
for path in changes:(overlay/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['StatisticsSystem.c','Stats_backend.h']:(overlay/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
dirs=[root/'src']+[p for p in (root/'src').rglob('*') if p.is_dir()];deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
save=re.findall(r'(?m)^\s*CopyToBuf\(buf, (\w+)\);',m.operation(old,'SaveStats'));load=re.findall(r'(?m)^\s*CopyFromBuf\(buf, (\w+)\);',m.operation(old,'LoadStats'));assert save==load
for phase,base in [('before',root),('after',root if production else overlay)]:
    text='#include "'+(base/'src/core/common.h').as_posix()+'"\n#include <type_traits>\n#include <stdint.h>\n'
    if phase=='before':text+=header.replace('#pragma once','').replace('"../audio/MusicSavedData.h"','"'+(root/'src/audio/MusicSavedData.h').as_posix()+'"')+'\n'
    else:text+='#include "'+(base/'src/core/Stats.h').as_posix()+'"\n'
    prefix='CStats::' if phase=='before' else 'Stats_'
    for kind,name,array in m.fields(header):
        expected=kind+' '+re.sub(r'\b(TOTAL_\w+)\b',prefix+r'\1',array)
        text+='static_assert(std::is_same<decltype('+prefix+name+'),'+expected+'>::value,"Original canonical type '+name+'");\n'
    for name in ['TOTAL_FASTEST_TIMES','TOTAL_HIGHEST_SCORES','TOTAL_BEST_POSITIONS','TOTAL_PROPERTIES']:
        value=re.search(name+r'\s*=\s*(\d+)',header)[1];text+='static_assert('+prefix+name+'=='+value+',"Original extent '+name+'");\n'
    for kind,name,args in m.signatures(old,header):text+='static_assert(std::is_same<decltype(&'+prefix+name+'),'+kind+' (*)('+args+')>::value,"Original operation signature '+name+'");\n'
    total='+'.join('sizeof('+('MusicSavedData_FavoriteRadioStationList' if name.startswith('MusicSavedData_') else prefix+name)+')' for name in save)
    text+='enum { TestStatsPackedSaveSize='+total+'};\n'
    if '--probe-save-size' in sys.argv:text+='template<unsigned Size> struct NativeStatsPackedSize;NativeStatsPackedSize<TestStatsPackedSaveSize> observedPackedSize;\n'
    else:text+='static_assert(TestStatsPackedSaveSize==595,"Actual original packed statistics payload");\n'
    (probeDir/('types-'+phase+'.cpp')).write_text(text,newline='\n')
(out/'save-fields.json').write_text(json.dumps({'save':save,'load':load},indent=2),newline='\n')
results=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    options='/nologo /c /O2 /EHsc /std:c++14 /Zc:sizedDealloc- /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX '
    native=options+' '.join('/I"'+str(p)+'"' for p in dirs+deps);translated=options+' '.join('/I"'+str(p)+'"' for p in [overlay/p.relative_to(root) for p in dirs]+deps)
    current=root if production else overlay;flags=native if production else translated
    units=[('types-before',probeDir/'types-before.cpp',native),('types-after',probeDir/'types-after.cpp',flags)]
    if '--types-only' not in sys.argv:
        units += [(Path(path).stem,current/path,flags) for path in changes if path.endswith('.cpp')]
        units += [('StatisticsSystem',current/'src/core/StatisticsSystem.c',flags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))]
    for name,source,compilerOptions in units:
        prefix=f'{"production" if production else "native"}{bits}-{name}';obj=out/(prefix+'.obj');driver=out/(prefix+'.cmd')
        driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+compilerOptions+' /Fo"'+str(obj)+'" "'+str(source)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,str(log),run.stdout[-3500:])
        results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-types.json' if '--types-only' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
