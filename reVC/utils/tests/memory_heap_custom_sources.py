"""Compile the complete staged custom-heap C owner and its native SDK boundaries against real SDK headers."""
from pathlib import Path
import json,os,subprocess,sys
import memory_heap_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/memory-heap-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay';probe=out/('production-custom-probes' if production else 'custom-probes');probe.mkdir(exist_ok=True)
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[];limitations=[]
paths=['src/rw/MemoryHeap.c','src/rw/MemoryHeap_native.cpp','src/rw/MemoryHeap_bindings.c']
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
            if profile=='heap' and (bits==64 or name.endswith('_native.cpp')):
                # Preserve diagnosed real-SDK limitations rather than supplying fixture-only legacy level bindings
                baseline=next(row for row in json.loads((out/'baseline-source-compiles.json').read_text()) if row['bits']==bits and row['custom'])
                baseline_text=Path(baseline['log']).read_text()
                legacy=['RemoveColModelsFromOtherLevels','LEVEL_INDUSTRIAL','LEVEL_COMMERCIAL','LEVEL_SUBURBAN','LoadCollisionFromDatFile']
                diagnostics=[]
                if bits==64:
                    diagnostic="HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense"
                    assert diagnostic in baseline_text and diagnostic in run.stdout,(label,run.stdout)
                    diagnostics.append(diagnostic)
                if name.endswith('_native.cpp'):
                    assert all(value in baseline_text and value in run.stdout for value in legacy),(label,run.stdout)
                    diagnostics+=legacy
                assert run.returncode!=0,(label,run.stdout)
                assert all(any(value in line for value in diagnostics) for line in run.stdout.splitlines() if 'error C' in line),(label,run.stdout)
                limitations.append({'bits':bits,'profile':profile,'source':name,'exit':run.returncode,'diagnostics':diagnostics})
                print(label,'retains existing custom-heap SDK limitations',flush=True);continue
            assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])
            results.append({'bits':bits,'profile':profile,'source':name,'exit':0});print(label,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
(out/('production-existing-limitations.json' if production else 'native-existing-limitations.json')).write_text(json.dumps(limitations,indent=2),newline='\n')
