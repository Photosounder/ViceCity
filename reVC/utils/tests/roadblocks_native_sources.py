"""Check actual roadblock field types and packed save contracts, then compile every changed consumer."""
from pathlib import Path
import os,json,sys,shutil,subprocess,re
import roadblocks_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/roadblocks-c-tests';overlay=out/'native-overlay';production='--production' in sys.argv
probeDir=out/'native-probes';probeDir.mkdir(exist_ok=True);shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
changes=json.loads((out/'changes.json').read_text());old=(out/'before/src/control/RoadBlocks.cpp').read_text();header=(out/'before/src/control/RoadBlocks.h').read_text()
for path in changes:(overlay/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['RoadBlockSystem.c','RoadBlocks_backend.h']:(overlay/'src/control'/name).write_text((out/'preview'/name).read_text(),newline='\n')
dirs=[root/'src']+[p for p in (root/'src').rglob('*') if p.is_dir()];deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
for phase,base in [('before',root),('after',root if production else overlay)]:
    text='#include "'+(base/'src/core/common.h').as_posix()+'"\n#include <type_traits>\n#include <stddef.h>\n'
    text+=header.replace('#pragma once','').replace('#include "common.h"','')+'\n' if phase=='before' else '#include "'+(base/'src/control/RoadBlocks.h').as_posix()+'"\n'
    prefix='CRoadBlocks::' if phase=='before' else 'RoadBlocks_'
    for name,kind in [('NumRoadBlocks','int16_t'),('RoadBlockNodes','int16_t[NUMROADBLOCKS]'),('InOrOut','bool[NUMROADBLOCKS]'),('aScriptRoadBlocks','CScriptRoadblock[NUM_SCRIPT_ROADBLOCKS]')]:text+='static_assert(std::is_same<decltype('+prefix+name+'),'+kind+'>::value,"Original '+name+' storage type");\n'
    text+='static_assert(sizeof(CScriptRoadblock)==28,"Script record extent");\nstatic_assert(alignof(CScriptRoadblock)==4,"Script record alignment");\nstatic_assert(offsetof(CScriptRoadblock,m_vInf)==0 && offsetof(CScriptRoadblock,m_vSup)==12 && offsetof(CScriptRoadblock,m_bInUse)==24,"Script field order");\n'
    if phase=='after':text+='static_assert(sizeof(RoadBlockVector)==12,"Vector extent");\n'
    (probeDir/('types-'+phase+'.cpp')).write_text(text,newline='\n')
results=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    options='/nologo /c /O2 /EHsc /std:c++14 /Zc:sizedDealloc- /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX '
    native=options+' '.join('/I"'+str(p)+'"' for p in dirs+deps);translated=options+' '.join('/I"'+str(p)+'"' for p in [overlay/p.relative_to(root) for p in dirs]+deps)
    current=root if production else overlay;flags=native if production else translated
    units=[('types-before',probeDir/'types-before.cpp',native),('types-after',probeDir/'types-after.cpp',flags)]
    if '--types-only' not in sys.argv:
        units += [(Path(path).stem,current/path,flags) for path in changes if path.endswith('.cpp')]
        units += [('RoadBlockSystem',current/'src/control/RoadBlockSystem.c',flags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))]
    for name,source,compilerOptions in units:
        prefix=f'{"production" if production else "native"}{bits}-{name}';obj=out/(prefix+'.obj');driver=out/(prefix+'.cmd')
        driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+compilerOptions+' /Fo"'+str(obj)+'" "'+str(source)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,str(log),run.stdout[-3500:])
        results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-types.json' if '--types-only' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
