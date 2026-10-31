"""Compare complete original/C collision-model owners under actual MSVC x86 and x64 ABIs."""
from pathlib import Path
import ctypes,json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';probe=out/'runtime';results=[];env=dict(os.environ);key=next(k for k in env if k.lower()=='path');ctypes.windll.kernel32.SetErrorMode(0x8003)
for bits in [32,64]:
    for profile,defines in [('default',[]),('compressed',['COMPRESSED_COL_VECTORS']),('vu',['VU_COLLISION']),('heap',['USE_CUSTOM_ALLOCATOR'])]:
        # Build and execute both entire producers with the actual native Windows ABI
        outputs=[];vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
        for phase in ['before','after']:
            # Retain real C compilation for the owner and independent public caller
            tree=out/('before' if phase=='before' else 'overlay');label=f'msvc-{bits}-{profile}-{phase}';units=[probe/'fixture.cpp',probe/'cache.cpp']+([probe/'original.cpp',probe/'before-caller.cpp'] if phase=='before' else [tree/'src/collision/ColModel.c',probe/'native.cpp',probe/'after-caller.c'])+[tree/name for name in ['src/collision/ColBox.c','src/collision/ColTriangle.c','src/collision/CompressedVector.c']];objects=[];lines=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1']
            for path in units:
                # Keep each complete producer in a distinct object and suppress only legacy CRT deprecation diagnostics
                obj=probe/f'{label}-{path.stem}.obj';objects.append(obj);flags='/nologo /c /O2 /fp:precise /D_CRT_SECURE_NO_WARNINGS '+(' /DORIGINAL ' if phase=='before' else '')+' '.join('/D'+d for d in defines)+' '+('/std:c11 /TC' if path.suffix=='.c' else '/std:c++14 /EHsc')+' /I"'+str(probe)+'" /I"'+str(tree/'src/collision')+'" /I"'+str(tree/'src/core')+'" /I"'+str(tree/'src/math')+'"';lines+=['cl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"','if errorlevel 1 exit /b 1']
            exe=probe/(label+'.exe');lines+=['link /nologo '+' '.join('"'+str(obj)+'"' for obj in objects)+' /OUT:"'+str(exe)+'"','exit /b %errorlevel%'];driver=probe/(label+'.cmd');driver.write_text('\n'.join(lines)+'\n',newline='\n');run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr)
            runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(label,run.returncode,run.stderr);outputs.append(run.stdout.strip())
        assert outputs[0]==outputs[1],(bits,outputs);results.append({'bits':bits,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,outputs[0],flush=True)
(out/'native-runs.json').write_text(json.dumps(results,indent=2),newline='\n')
