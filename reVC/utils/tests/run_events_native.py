"""Compare all seventeen original/C operations using actual x86 and x64 MSVC ABIs."""
from pathlib import Path
import os,subprocess,json,ctypes
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/events-c-tests'
ctypes.windll.kernel32.SetErrorMode(0x8003)
runs=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('fixed',['FIX_BUGS']),('squeeze',['FIX_BUGS','SQUEEZE_PERFORMANCE']),('unfixed',[]),('squeeze-unfixed',['SQUEEZE_PERFORMANCE'])]:
        outputs=[]
        for phase in ['before','after']:
            prefix=f'native{bits}-{profile}-{phase}';driver=out/(prefix+'.cmd');exe=out/(prefix+'.exe')
            sources=[('caller.c',False),('fixture.cpp',True)]+([('original.cpp',True)] if phase=='before' else [('preview/GameplayEvents.c',False),('event-host.cpp',True),('accident-host.cpp',True)])
            flags='/nologo /MT /O2 /fp:precise /EHsc '+ ' '.join('/D'+d for d in defines)+(' /DTEST_EVENTS_ORIGINAL' if phase=='before' else '')+f' /I"{out}" /I"{out/"preview"}"'
            lines=['@echo off',f'call "{vcvars}" >nul','if errorlevel 1 exit /b 1'];objects=[]
            for i,(source,cpp) in enumerate(sources):
                obj=out/f'{prefix}-{i}.obj';objects.append(obj)
                lines+=[f'cl {flags} '+('/std:c++14 /TP' if cpp else '/std:c11 /TC')+f' /c /Fo"{obj}" "{out/source}"','if errorlevel 1 exit /b 1']
            lines+=['link /nologo /SUBSYSTEM:CONSOLE /OUT:"'+str(exe)+'" '+' '.join('"'+str(obj)+'"' for obj in objects),'if errorlevel 1 exit /b 1',f'"{exe}"','exit /b %errorlevel%'];driver.write_text('\n'.join(lines)+'\n',newline='\n')
            result=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(result.stdout+result.stderr,newline='\n')
            assert result.returncode==0,(prefix,result.stdout,result.stderr)
            outputs.append(result.stdout.strip().splitlines()[-1])
        assert outputs[0]==outputs[1],(bits,profile,outputs)
        runs.append({'bits':bits,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,profile,outputs[0],flush=True)
(out/'native-runs.json').write_text(json.dumps(runs,indent=2),newline='\n')
