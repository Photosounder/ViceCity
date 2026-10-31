"""Execute complete frame callback comparisons under MSVC x86/x64 with actual C ABI boundaries."""
from pathlib import Path
import ctypes,json,os,subprocess,sys
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/frame-update-c-tests';fixture=out/'runtime';probe=out/'native-runtime';probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');runs=[];mutations='--mutations' in sys.argv
dependencies=['src/animation/AnimBlendAssociation.c','src/animation/AnimBlendNode.c','src/animation/AnimBlendSequence.c','src/animation/AnimBlendList.c','src/animation/AnimBlendClumpData.c','src/animation/KeyFrames.c','src/math/Quaternion.c']
for bits in [32,64]:
    # Compare both native pointer-width ABIs rather than inferring C/C++ struct return conventions
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('fixed',[]),('classic',['CLASSIC']),('compression',['ANIM_COMPRESSION']),('compression-classic',['ANIM_COMPRESSION','CLASSIC'])]:
        results=[]
        for phase in ['before','after']:
            # Compile the entire original and C producers with all genuine converted interpolation dependencies
            tree=out/('before' if phase=='before' else 'overlay');label=f'{bits}-{profile}-{phase}'+('-mutations' if mutations else '');directory=probe/label;directory.mkdir(exist_ok=True)
            dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
            flags='/nologo /c /O2 /MT /fp:precise '+' '.join('/D'+d for d in defines+(['FRAME_MUTATIONS'] if mutations else [])+(['ORIGINAL'] if phase=='before' else []))+' '+' '.join('/I"'+str(p)+'"' for p in [fixture]+dirs)
            units=[fixture/'fixture.cpp',fixture/'caller.c',fixture/'primitives.cpp']+([fixture/'original.cpp'] if phase=='before' else [tree/'src/animation/FrameUpdate.c',fixture/'native.cpp'])+[tree/name for name in dependencies]
            commands=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1'];objects=[]
            for path in units:
                # Preserve strict C11 compilation for all C algorithms and their independent caller
                obj=directory/(path.stem+'.obj');objects.append(obj)
                commands+=['cl '+flags+(' /std:c11 /TC ' if path.suffix=='.c' else ' /std:c++14 /EHsc /Zc:sizedDealloc- ')+'/Fo"'+str(obj)+'" "'+str(path)+'"','if errorlevel 1 exit /b 1']
            exe=directory/'frames.exe';commands+=['link /nologo /OUT:"'+str(exe)+'" '+' '.join('"'+str(p)+'"' for p in objects),'exit /b %errorlevel%']
            driver=directory/'build.cmd';driver.write_text('\n'.join(commands)+'\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(directory/'build.log').write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])
            runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(label,run.returncode,run.stderr);results.append(run.stdout.strip())
        assert results[0]==results[1],(bits,profile,results)
        runs.append({'bits':bits,'profile':profile,'before':results[0],'after':results[1],'exit':0});print(bits,profile,results[0],flush=True)
(out/('mutation-native-runs.json' if mutations else 'native-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
