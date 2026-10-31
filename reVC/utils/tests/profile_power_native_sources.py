"""Compile both complete C replacements, their native caller and original/native/C ABI probes."""
from pathlib import Path
import os,json,sys,shutil,subprocess
import profile_power_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/profile-power-c-tests';before=out/'before';overlay=out/'native-overlay';production='--production' in sys.argv
probeDir=out/'native-probes';probeDir.mkdir(exist_ok=True);shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
changes=json.loads((out/'changes.json').read_text())
for path in changes:
    target=overlay/m.RENAMES.get(path,path);target.parent.mkdir(parents=True,exist_ok=True);target.write_text(m.calls((before/path).read_text()),newline='\n')
    stale=overlay/path
    if path in m.RENAMES and stale.exists():stale.unlink()
profile=(before/'src/core/Profile.h').read_text().replace('#pragma once','').replace('class CProfile\n{','class CProfile\n{\npublic:')
power=(before/'src/control/PowerPoints.h').read_text().replace('#pragma once','')
for phase,base in [('before',root),('after',root if production else overlay)]:
    text='#include "'+(base/'src/core/common.h').as_posix()+'"\n#include <type_traits>\n#include <stddef.h>\n'
    text+=profile+'\n'+power if phase=='before' else '#include "'+(base/'src/core/Profile.h').as_posix()+'"\n#include "'+(base/'src/control/PowerPoints.h').as_posix()+'"\n'
    prefix='CProfile::' if phase=='before' else 'Profile_';color='RwRGBA' if phase=='before' else 'ProfileColor'
    for name in m.PROFILE_FIELDS[:5]:text+='static_assert(std::is_same<decltype('+prefix+name+'),float[7]>::value,"Original '+name+' storage");\n'
    text+='static_assert(std::is_same<decltype('+prefix+'ms_pProfileString),const char *[7]>::value,"Original string table");\n'
    text+='static_assert(std::is_same<decltype('+prefix+'ms_aBarColours),'+color+'[7]>::value,"Original color table");\n'
    text+='static_assert(sizeof('+color+')==4 && alignof('+color+')==1,"Original color representation");\n'
    for offset,name in enumerate(['red','green','blue','alpha']):text+='static_assert(offsetof('+color+','+name+')=='+str(offset)+',"Original color component offset");\n'
    text+='static_assert(sizeof(CPowerPoint)==1 && alignof(CPowerPoint)==1,"Original empty-instance extent");\nstatic_assert(NUM_PROFILES==7 && PROFILE_FRAME_RATE==0 && PROFILE_TOTAL==6,"Original profile identifiers");\nstatic_assert(POWERPOINT_NONE==0 && POWERPOINT_HIDEOUT_SUBURBAN==4,"Original cut-feature identifiers");\n'
    (probeDir/('types-'+phase+'.cpp')).write_text(text,newline='\n')
base=root if production else overlay
c='#include "'+(base/'src/core/Profile.h').as_posix()+'"\n#include "'+(base/'src/control/PowerPoints.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(ProfileColor)==4 && _Alignof(ProfileColor)==1,"Native color layout in C11");\n_Static_assert(sizeof(CPowerPoint)==1 && _Alignof(CPowerPoint)==1,"Native empty-instance extent in C11");\n_Static_assert(sizeof(Profile_ms_afStartTime)==28 && NUM_PROFILES==7,"Native profile extent in C11");\n'
(probeDir/'types-c.c').write_text(c,newline='\n')
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
results=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for name,path,tree,isC in [('types-before',probeDir/'types-before.cpp',root,False),('types-after',probeDir/'types-after.cpp',base,False),('types-c',probeDir/'types-c.c',base,True),('Script3',base/'src/control/Script3.cpp',base,False),('Profile',base/'src/core/Profile.c',base,True),('PowerPoints',base/'src/control/PowerPoints.c',base,True)]:
        dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir()]
        flags='/nologo /c /O2 /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')
        flags+=' '.join('/I"'+str(p)+'"' for p in dirs+deps)
        prefix=f'{"production" if production else "native"}{bits}-{name}';obj=out/(prefix+'.obj');driver=out/(prefix+'.cmd')
        driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,str(log),run.stdout[-4000:])
        results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
