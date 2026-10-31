"""Run complete original/C list and shutdown checks with both native MSVC architectures."""
from pathlib import Path
import os,json,subprocess,ctypes
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lists-c-tests';ctypes.windll.kernel32.SetErrorMode(0x8003);runs=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('normal',[]),('master',['MASTER'])]:
        outputs=[]
        for phase in ['before','after']:
            prefix=f'native{bits}-{profile}-{phase}';exe=out/(prefix+'.exe');driver=out/(prefix+'.cmd');objects=[]
            flags='/nologo /MT /O2 /I"'+str(out)+'" '+' '.join('/D'+d for d in defines)+(' /DORIGINAL' if phase=='before' else '')
            lines=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1']
            for name,cpp in [('fixture.cpp',True)]+([('original.cpp',True)] if phase=='before' else [('Lists.c',False)]):
                obj=out/(prefix+'-'+name+'.obj');objects.append(obj);lines+=['cl '+flags+(' /EHsc /std:c++14 /TP' if cpp else ' /std:c11 /TC')+' /c /Fo"'+str(obj)+'" "'+str(out/name)+'"','if errorlevel 1 exit /b 1']
            lines+=['link /nologo /SUBSYSTEM:CONSOLE /OUT:"'+str(exe)+'" '+' '.join('"'+str(obj)+'"' for obj in objects),'if errorlevel 1 exit /b 1','"'+str(exe)+'"','exit /b %errorlevel%'];driver.write_text('\n'.join(lines)+'\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,run.stdout[-4000:]);outputs.append('\n'.join(line for line in run.stdout.splitlines() if line.startswith(('states=','shutdown '))))
        assert outputs[0]==outputs[1],(bits,profile,outputs);runs.append({'bits':bits,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,profile,outputs[0],flush=True)
(out/'native-runs.json').write_text(json.dumps(runs,indent=2),newline='\n')
