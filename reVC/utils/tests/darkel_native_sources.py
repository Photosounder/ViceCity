"""Compile the complete rampage projection and prove all original canonical field types on x86/x64."""
from pathlib import Path
import os,sys,json,subprocess,shutil
import darkel_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/darkel-c-tests';overlay=out/'native-overlay';production='--production' in sys.argv
probeDir=out/'native-probes';probeDir.mkdir(exist_ok=True)
shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
changes=json.loads((out/'changes.json').read_text())
for path in changes:(overlay/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['RampageSystem.c','Darkel_backend.h']:(overlay/'src/control'/name).write_text((out/'preview'/name).read_text(),newline='\n')
dirs=[root/'src']+[p for p in (root/'src').rglob('*') if p.is_dir()]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
original=(out/'before/src/control/Darkel.h').read_text()
for phase,base in [('before',root),('after',root if production else overlay)]:
    text='#include "'+(base/'src/core/common.h').as_posix()+'"\n#include <type_traits>\n#include <stdint.h>\n'
    if phase=='before':text+=original.replace('#pragma once','').replace('private:','public:')+'\n'
    else:text+='#include "'+(base/'src/control/Darkel.h').as_posix()+'"\n'
    for kind,star,name,array in m.fields(original):text+='static_assert(std::is_same<decltype('+('CDarkel::' if phase=='before' else 'Darkel_')+name+'),'+kind+' '+star+array+'>::value,"Complete original canonical type '+name+'");\n'
    text+='static_assert(sizeof('+('CDarkel::RegisteredKills' if phase=='before' else 'Darkel_RegisteredKills')+')==sizeof(uint16_t)*NUM_DEFAULT_MODELS,"Complete registered-model counter array");\n'
    (probeDir/('fields-'+phase+'.cpp')).write_text(text,newline='\n')
results=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    options='/nologo /c /O2 /EHsc /std:c++14 /Zc:sizedDealloc- /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX '
    native=options+' '.join('/I"'+str(p)+'"' for p in dirs+deps);translated=options+' '.join('/I"'+str(p)+'"' for p in [overlay/p.relative_to(root) for p in dirs]+deps)
    current=root if production else overlay;flags=native if production else translated
    units=[('fields-before',probeDir/'fields-before.cpp',native),('fields-after',probeDir/'fields-after.cpp',flags)]
    if '--fields-only' not in sys.argv:
        units += [(Path(path).stem,current/path,flags) for path in changes if path.endswith('.cpp')]
        units += [('RampageSystem',current/'src/control/RampageSystem.c',flags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))]
    for name,source,compilerOptions in units:
        prefix=f'{"production" if production else "native"}{bits}-{name}';obj=out/(prefix+'.obj');driver=out/(prefix+'.cmd')
        driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+compilerOptions+' /Fo"'+str(obj)+'" "'+str(source)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,str(log),run.stdout[-3500:])
        results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-fields.json' if '--fields-only' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
