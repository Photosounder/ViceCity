"""Compare complete roadblock behavior and packed payloads through native x86/x64 C ABIs."""
from pathlib import Path
import os,json,subprocess,ctypes,sys
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/roadblocks-c-tests';ctypes.windll.kernel32.SetErrorMode(0x8003)
debug='--debug' in sys.argv
profiles=[('fixed',['FIX_BUGS']),('classic',[]),('squeeze',['FIX_BUGS','SQUEEZE_PERFORMANCE']),('classic-squeeze',['SQUEEZE_PERFORMANCE']),('securom',['FIX_BUGS','SECUROM']),('master',['FIX_BUGS','MASTER']),('ps2',['FIX_BUGS','GTA_PS2'])];runs=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in profiles:
        outputs=[]
        for phase in ['before','after']:
            prefix=f'native{bits}'+('-debug' if debug else '')+f'-{profile}-{phase}';exe=out/(prefix+'.exe');driver=out/(prefix+'.cmd');objects=[]
            flags='/nologo /MT '+('/Od' if debug else '/O2')+' /fp:precise /EHsc /I"'+str(out)+'" /I"'+str(out/'preview')+'" '+' '.join('/D'+d for d in defines)+(' /DTEST_ROADBLOCKS_ORIGINAL' if phase=='before' else '')
            lines=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1'];sources=[('caller.c',False),('fixture.cpp',True),('math.cpp',True)]+([('original.cpp',True)] if phase=='before' else [('kernel.c',False),('host.cpp',True)])
            for i,(name,cpp) in enumerate(sources):
                obj=out/f'{prefix}-{i}.obj';objects.append(obj);lines+=['cl '+flags+(' /std:c++14 /TP' if cpp else ' /std:c11 /TC')+' /c /Fo"'+str(obj)+'" "'+str(out/name)+'"','if errorlevel 1 exit /b 1']
            lines+=['link /nologo /SUBSYSTEM:CONSOLE /OUT:"'+str(exe)+'" '+' '.join('"'+str(obj)+'"' for obj in objects),'if errorlevel 1 exit /b 1','"'+str(exe)+'"','exit /b %errorlevel%'];driver.write_text('\n'.join(lines)+'\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,str(log),run.stdout[-3500:]);outputs.append(run.stdout.strip().splitlines()[-1])
        assert outputs[0]==outputs[1],(bits,profile,outputs);runs.append({'bits':bits,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,profile,outputs[0],flush=True)
(out/('native-debug-runs.json' if debug else 'native-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
