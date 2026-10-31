"""Execute whole animation plugin comparisons under both real MSVC pointer-width ABIs."""
from pathlib import Path
import ctypes,json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-plugin-c-tests';fixture=out/'runtime';probe=out/'native-runtime';probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');runs=[]
dependencies=['src/animation/AnimBlendAssociation.c','src/animation/AnimBlendNode.c','src/animation/AnimBlendSequence.c','src/animation/AnimBlendHierarchy.c','src/animation/AnimBlendList.c','src/animation/AnimBlendClumpData.c','src/animation/KeyFrames.c','src/math/Quaternion.c','src/core/General.c','src/animation/Bones.c','src/animation/FrameUpdate.c']
for bits in [32,64]:
    # Verify actual C/C++ ABI behavior for registration callbacks, slot offsets and SDK value transport
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('fixed',[]),('classic',['CLASSIC']),('compression',['ANIM_COMPRESSION']),('compression-classic',['ANIM_COMPRESSION','CLASSIC'])]:
        results=[]
        for phase in ['before','after']:
            # Compile both entire plugin producers with identical real C playback dependencies
            tree=out/('before' if phase=='before' else 'overlay');label=f'{bits}-{profile}-{phase}';directory=probe/label;directory.mkdir(exist_ok=True)
            dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
            flags='/nologo /c /O2 /MT /fp:precise /DLIBRW '+' '.join('/D'+d for d in defines)+' '+' '.join('/I"'+str(p)+'"' for p in [fixture]+dirs)
            units=[fixture/'fixture.cpp',fixture/'frame-native.cpp']+([fixture/'original.cpp',fixture/'before-caller.cpp'] if phase=='before' else [tree/'src/animation/RpAnimBlend.c',fixture/'native.cpp',fixture/'after-caller.c'])+[tree/name for name in dependencies]
            commands=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1'];objects=[]
            for path in units:
                # Keep the complete C owner and its independent caller under strict C11 rules
                obj=directory/(path.stem+'.obj');objects.append(obj);commands+=['cl '+flags+(' /std:c11 /TC ' if path.suffix=='.c' else ' /std:c++14 /EHsc /Zc:sizedDealloc- ')+'/Fo"'+str(obj)+'" "'+str(path)+'"','if errorlevel 1 exit /b 1']
            exe=directory/'plugin.exe';commands+=['link /nologo /OUT:"'+str(exe)+'" '+' '.join('"'+str(p)+'"' for p in objects),'exit /b %errorlevel%']
            driver=directory/'build.cmd';driver.write_text('\n'.join(commands)+'\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(directory/'build.log').write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])
            runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(label,run.returncode,run.stderr);results.append(run.stdout.strip())
        assert results[0]==results[1],(bits,profile,results)
        runs.append({'bits':bits,'profile':profile,'before':results[0],'after':results[1],'exit':0});print(bits,profile,results[0],flush=True)
(out/'native-runs.json').write_text(json.dumps(runs,indent=2),newline='\n')
