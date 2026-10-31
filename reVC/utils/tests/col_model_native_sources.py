"""Compile the complete staged collision-model C owner and its native SDK boundaries against real SDK headers."""
from pathlib import Path
import json,os,subprocess,sys
import col_model_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay';probe=out/('production-probes' if production else 'native-probes');probe.mkdir(exist_ok=True)
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[];limitations=[]
paths=['src/collision/ColModel.c','src/collision/ColModel_native.cpp']
for bits in [32,64]:
    # Verify both real native ABIs and the complete actual allocator and cache primitives
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('default',''),('heap',' /DUSE_CUSTOM_ALLOCATOR')]:
        for name in paths:
            # Keep each entire source file on one coherent set of project and vendor headers
            path=tree/name;label=f'{bits}-{profile}-{path.stem}';obj=probe/(label+'.obj')
            flags=defines+' /nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if path.suffix=='.c' else '/std:c++14 /EHsc /Zc:sizedDealloc- ')+' '.join('/I"'+str(p)+'"' for p in dirs+deps)
            driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n')
            if bits==64 and profile=='heap' and name.endswith('_native.cpp'):
                # Prove the unchanged custom-heap layout limitation also rejects the complete original producer
                diagnostic="HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense";assert run.returncode!=0 and run.stdout.count('error C2338:')==1 and diagnostic in run.stdout,(label,run.stdout)
                original=out/'before/src/collision/ColModel.cpp';beforeFlags=flags.replace(str(tree/'src'),str(out/'before/src'));beforeLabel=label+'-baseline';beforeDriver=probe/(beforeLabel+'.cmd');beforeDriver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+beforeFlags+' /Fo"'+str(probe/(beforeLabel+'.obj'))+'" "'+str(original)+'"\nexit /b %errorlevel%\n',newline='\n');baseline=subprocess.run(['cmd.exe','/c',str(beforeDriver)],env=dict(os.environ),capture_output=True,text=True);(probe/(beforeLabel+'.log')).write_text(baseline.stdout+baseline.stderr,newline='\n');assert baseline.returncode!=0 and baseline.stdout.count('error C2338:')==1 and diagnostic in baseline.stdout,(beforeLabel,baseline.stdout)
                for phase,result in [('before',baseline),('after',run)]:limitations.append({'bits':bits,'profile':profile,'phase':phase,'exit':result.returncode,'diagnostic':diagnostic})
                print(label,'matches existing unsupported custom-heap x64 layout',flush=True);continue
            assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])
            results.append({'bits':bits,'profile':profile,'source':name,'exit':0});print(label,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
(out/('production-existing-limitations.json' if production else 'native-existing-limitations.json')).write_text(json.dumps(limitations,indent=2),newline='\n')
