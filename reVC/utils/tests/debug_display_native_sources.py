"""Check actual debug-display field types and record layouts, then compile every changed consumer."""
from pathlib import Path
import os,json,sys,shutil,subprocess,re
import debug_display_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/debug-display-c-tests';overlay=out/'native-overlay';production='--production' in sys.argv
probeDir=out/'native-probes';probeDir.mkdir(exist_ok=True);shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
changes=json.loads((out/'changes.json').read_text());old=(out/'before/src/core/Debug.cpp').read_text();header=(out/'before/src/core/Debug.h').read_text()
for path in changes:(overlay/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['DebugDisplaySystem.c','Debug_backend.h']:(overlay/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
dirs=[root/'src']+[p for p in (root/'src').rglob('*') if p.is_dir()];deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
for phase,base in [('before',root),('after',root if production else overlay)]:
    text='#include "'+(base/'src/core/common.h').as_posix()+'"\n#include <type_traits>\n#include <stddef.h>\n'
    text+=header.replace('#pragma once','').replace('class CDebug\n{','class CDebug\n{\npublic:')+'\n' if phase=='before' else '#include "'+(base/'src/core/Debug.h').as_posix()+'"\n'
    prefix='CDebug::' if phase=='before' else 'Debug_';screen=prefix+'ScreenStr';line=prefix+'Line'
    for name,kind in [('ms_nCurrentTextLine','int16_t'),('ms_aTextBuffer','char[15][80]'),('ms_aScreenStrs',screen+'[100]'),('ms_nScreenStrs','int'),('ms_aLines',line+'[100]'),('ms_nLines','int')]:text+='static_assert(std::is_same<decltype('+prefix+name+'),'+kind+'>::value,"Original '+name+' storage type");\n'
    for name,size in [('MAX_LINES',15),('MAX_STR_LEN',80),('MAX_SCREEN_STRS',100),('MAX_DEBUG_LINES',100)]:text+='static_assert('+prefix+name+'=='+str(size)+',"Original capacity");\n'
    text+='static_assert(sizeof('+screen+')==264 && alignof('+screen+')==4,"Screen record extent and alignment");\n'
    text+='static_assert(offsetof('+screen+',x)==0 && offsetof('+screen+',y)==4 && offsetof('+screen+',str)==8,"Screen record field order");\n'
    text+='static_assert(sizeof('+line+')==32 && alignof('+line+')==4,"Line record extent and alignment");\n'
    text+='static_assert(offsetof('+line+',p1)==0 && offsetof('+line+',p2)==12 && offsetof('+line+',c1)==24 && offsetof('+line+',c2)==28,"Line record field order");\n'
    text+='static_assert(std::is_same<decltype(gbDebugStuffInRelease),bool>::value,"Original shared flag type");\n'
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
        units += [('DebugDisplaySystem',current/'src/core/DebugDisplaySystem.c',flags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))]
    for name,source,compilerOptions in units:
        prefix=f'{"production" if production else "native"}{bits}-{name}';obj=out/(prefix+'.obj');driver=out/(prefix+'.cmd')
        driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+compilerOptions+' /Fo"'+str(obj)+'" "'+str(source)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,str(log),run.stdout[-3500:])
        results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-types.json' if '--types-only' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
