"""Compare complete original and C rendering-helper producers with independent native ABI callers."""
from pathlib import Path
import ctypes,json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shadow-cameras-c-tests';probe=out/'peer-runtime';results=[];env=dict(os.environ);key=next(k for k in env if k.lower()=='path');ctypes.windll.kernel32.SetErrorMode(0x8003)
for bits in [32,64]:
    for optimization in ['Od','O2']:
        for profile,defines in [('d3d9',['LIBRW','RW_D3D9']),('gl',['LIBRW','RW_GL3']),('neutral',['LIBRW']),('master',['LIBRW','RW_D3D9','MASTER']),('final',['LIBRW','RW_D3D9','FINAL']),('classic',['LIBRW','RW_D3D9']),('legacy',[]),('legacy-classic',[])]:
            # Compare both entire producers using matching flags and the actual Windows ABI
            outputs=[];vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
            for phase in ['before','after']:
                # Keep C policy, SDK primitives, and independent public callers in separate translation units
                tree=(root/'.diagnostics/rw-helper-c-tests/before') if phase=='before' else out/'overlay';label=f'msvc-{bits}-{optimization}-{profile}-{phase}';units=[probe/'fixture.cpp']+([probe/'original.cpp',probe/'before-caller.cpp'] if phase=='before' else [tree/'src/rw/RwHelper.c',probe/'native.cpp',probe/'after-caller.c']);objects=[];lines=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1']
                header=probe/(label+'-profile.h');header.write_text('#include "'+(tree/'src/core/config.h').as_posix()+'"\n'+('#undef FIX_BUGS\n' if profile in ['classic','legacy-classic'] else ''),newline='\n')
                for path in units:
                    # Compile complete original or converted owners without replacing application algorithms
                    obj=probe/f'{label}-{path.stem}.obj';objects.append(obj);flags='/nologo /c /'+optimization+' /MT /fp:precise /D_CRT_SECURE_NO_WARNINGS /FI"'+str(header)+'" '+('/DORIGINAL ' if phase=='before' else '')+' '.join('/D'+d for d in defines)+' '+('/std:c11 /TC' if path.suffix=='.c' else '/std:c++14 /EHsc')+' /I"'+str(probe)+'" '+' '.join('/I"'+str(p)+'"' for p in (tree/'src').rglob('*') if p.is_dir());lines+=['cl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"','if errorlevel 1 exit /b 1']
                exe=probe/(label+'.exe');lines+=['link /nologo '+' '.join('"'+str(obj)+'"' for obj in objects)+' /OUT:"'+str(exe)+'"','exit /b %errorlevel%'];driver=probe/(label+'.cmd');driver.write_text('\n'.join(lines)+'\n',newline='\n');run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-5000:],run.stderr)
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);(probe/(label+'-run.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.returncode,run.stderr);outputs.append(run.stdout.strip())
            assert outputs[0]==outputs[1],(bits,optimization,profile,outputs);results.append({'bits':bits,'optimization':optimization,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,optimization,profile,outputs[0],flush=True)
(out/'peer-native-runs.json').write_text(json.dumps(results,indent=2),newline='\n')
