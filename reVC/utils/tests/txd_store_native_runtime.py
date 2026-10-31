"""Compare complete original/C TXD owners under the actual MSVC x86 and x64 ABIs."""
from pathlib import Path
import ctypes,json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/txd-store-c-tests';probe=out/'runtime';results=[];env=dict(os.environ);key=next(k for k in env if k.lower()=='path');ctypes.windll.kernel32.SetErrorMode(0x8003)
for bits in [32,64]:
    for optimization in ['Od','O2']:
        for profile,defines in [('default',[]),('master',['MASTER'])]:
            # Build and execute both entire producers with the actual native Windows ABI
            outputs=[];vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
            for phase in ['before','after']:
                # Retain real C compilation for the owner and independent public caller
                tree=out/('before' if phase=='before' else 'overlay');label=f'msvc-{bits}-{optimization}-{profile}-{phase}';units=[probe/'fixture.cpp',tree/'src/core/General.c']+([probe/'original.cpp',probe/'before-caller.cpp',probe/'before-bindings.cpp'] if phase=='before' else [tree/'src/rw/TxdStore.c',tree/'src/rw/TxdStore_bindings.c',probe/'native.cpp',probe/'after-caller.c',probe/'after-bindings-caller.c']);objects=[];lines=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1']
                for path in units:
                    # Keep each complete producer in a distinct object and suppress only legacy CRT deprecation diagnostics
                    obj=probe/f'{label}-{path.stem}.obj';objects.append(obj);flags='/nologo /c /'+optimization+' /D_CRT_SECURE_NO_WARNINGS '+(' /DORIGINAL ' if phase=='before' else '')+' '.join('/D'+d for d in defines)+' '+('/std:c11 /TC' if path.suffix=='.c' else '/std:c++14 /EHsc')+' /I"'+str(probe)+'" /I"'+str(tree/'src/rw')+'" /I"'+str(tree/'src/core')+'" '+ ' '.join('/I"'+str(p)+'"' for p in (tree/'src').rglob('*') if p.is_dir())+' /I"C:/msys/home/github/CITAlloc"';lines+=['cl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"','if errorlevel 1 exit /b 1']
                exe=probe/(label+'.exe');lines+=['link /nologo '+' '.join('"'+str(obj)+'"' for obj in objects)+' /OUT:"'+str(exe)+'"','exit /b %errorlevel%'];driver=probe/(label+'.cmd');driver.write_text('\n'.join(lines)+'\n',newline='\n');run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr)
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);(probe/(label+'-run.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.returncode,run.stderr);outputs.append(run.stdout.strip())
            assert outputs[0]==outputs[1],(bits,outputs);results.append({'bits':bits,'optimization':optimization,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,outputs[0],flush=True)
(out/'native-runs.json').write_text(json.dumps(results,indent=2),newline='\n')
