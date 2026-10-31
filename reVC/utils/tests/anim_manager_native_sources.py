"""Compile staged manager C operations and complete native consumers with real project headers."""
from pathlib import Path
import json,os,subprocess,sys
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests'
production='--production' in sys.argv;tree=root if production else out/'overlay';probe=out/('production-probes' if production else 'native-probes');probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
paths=['src/animation/AnimManager.c','src/animation/AnimManager_native.cpp']
if '--consumers' in sys.argv:
    # Compile each complete native consumer rather than a reduced declaration probe
    paths+=[name for name in json.loads((out/'changes.json').read_text()) if name.endswith(('.cpp','.c')) and name!='src/animation/AnimManager.cpp']
results=[]
for bits in [32,64]:
    # Use the actual Visual C++ toolchain for both supported native pointer widths
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('default',''),('compression','/DANIM_COMPRESSION'),('master','/DMASTER')]:
        if '--consumers' in sys.argv and profile!='default':continue
        for name in paths:
            # Preserve D3D9 backend selection and isolate every object and log from production build output
            path=tree/name;isC=path.suffix=='.c';label=f'{bits}-{profile}-{path.stem}'
            flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+defines+' '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')
            flags+=' '.join('/I"'+str(p)+'"' for p in dirs+deps)
            driver=probe/(label+'.cmd')
            driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(probe/(label+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True)
            (probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n')
            assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])
            results.append({'bits':bits,'profile':profile,'path':name,'exit':0})
            print(label,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-consumer-compiles.json' if '--consumers' in sys.argv else 'native-manager-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
