"""Compile the complete C texture reader owner and every whole native consumer against real SDK headers."""
from pathlib import Path
import json,os,subprocess,sys
import tex_read_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/tex-read-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay';probe=out/('production-consumer-probes' if production else 'consumer-probes');probe.mkdir(exist_ok=True)
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[]
paths=['src/rw/TexRead.c','src/rw/TexRead_native.cpp','src/rw/TexRead_bindings.c']
for bits in [32,64]:
    # Verify both real native ABIs for entire original and C texture reader owners
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('default',''),('custom',' /DUSE_CUSTOM_ALLOCATOR')]:
        for name in ['src/rw/TexRead.cpp']+paths:
            # Keep each entire source file on one coherent set of project and vendor headers
            owner_tree=out/'before' if name.endswith('TexRead.cpp') else tree
            phase='before' if owner_tree==out/'before' else 'after'
            path=owner_tree/name;label=f'{bits}-{profile}-{phase}-{path.stem}';obj=probe/(label+'.obj')
            flags=defines+' /nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if path.suffix=='.c' else '/std:c++14 /EHsc /Zc:sizedDealloc- ')+' '.join('/I"'+str(p)+'"' for p in [owner_tree/'src'/p.relative_to(tree/'src') for p in dirs]+deps)
            driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])
            results.append({'bits':bits,'source':name,'phase':phase,'profile':profile,'exit':0});print(label,'passed',flush=True)
(out/('production-owner-compiles.json' if production else 'native-owner-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
