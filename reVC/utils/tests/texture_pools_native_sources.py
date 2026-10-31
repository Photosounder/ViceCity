"""Compile whole native D3D9 project consumers and both inactive legacy-pool producers with MSVC."""
from pathlib import Path
import json,os,subprocess,sys

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/texture-pools-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay';probe=out/('production-sources' if production else 'native-sources');probe.mkdir(exist_ok=True);results=[]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
for bits in [32,64]:
    # Check both actual MSVC ABIs without pretending the unavailable proprietary RenderWare texture-pool integration is enabled
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for name in ['src/rw/TexturePools.cpp','src/rw/TexturePools.c','src/core/Game.cpp']:
        # Compile each entire owner or client on one coherent set of real project and vendor headers
        base=out/'before' if name.endswith('TexturePools.cpp') else tree;path=base/name;label=f'{bits}-{path.name}';dirs=[base/'src']+[p for p in (base/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(base/'src').parts)]
        flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if path.suffix=='.c' else '/std:c++14 /EHsc /Zc:sizedDealloc- ')+' '.join('/I"'+str(p)+'"' for p in dirs+deps);obj=probe/(label+'.obj');driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-5000:],run.stderr);results.append({'bits':bits,'source':name,'exit':0});print(label,'passed',flush=True)
(out/('production-native-compiles.json' if production else 'native-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
