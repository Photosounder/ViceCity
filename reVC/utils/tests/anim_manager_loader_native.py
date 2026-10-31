"""Run complete loader comparisons with MSVC x86/x64 and the real C animation dependencies."""
from pathlib import Path
import ctypes,json,os,subprocess,sys
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';full='--full' in sys.argv;fixture=out/('full' if full else 'loader');probe=out/('native-full' if full else 'native-loader');probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path')
deps=[root/'.diagnostics/events-c-tests/native-includes'];runs=[]
for bits in [32,64]:
    # Retain both actual native ABIs rather than inferring x86 behavior from x64 execution
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for profile,defines in [('default',[]),('compression',['ANIM_COMPRESSION']),('master',['MASTER']),('compression-master',['ANIM_COMPRESSION','MASTER'])]:
        results=[];label=f'{bits}-{profile}'
        for phase in ('before','after'):
            # Build each implementation against the same coherent tree used in its source compilation checks
            tree=out/('before' if phase=='before' else 'overlay');objdir=probe/(label+'-'+phase);objdir.mkdir(exist_ok=True)
            dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
            flags='/nologo /c /O2 /MT /fp:precise /DNOMINMAX '+' '.join('/D'+value for value in defines+(['ORIGINAL'] if phase=='before' else []))+' '+' '.join('/I"'+str(p)+'"' for p in [fixture]+dirs+deps)
            units=[fixture/'loader.cpp',fixture/('original.cpp' if phase=='before' else 'manager.c')]+[tree/name for name in ['src/animation/AnimBlendHierarchy.c','src/animation/AnimBlendSequence.c','src/animation/KeyFrames.c','src/math/Quaternion.c','src/core/General.c']]
            if full:
                # Replace the focused manager extraction with complete producers and every real animation owner
                units[0]=fixture/'fixture.cpp';units[1]=fixture/'original.cpp' if phase=='before' else tree/'src/animation/AnimManager.c'
                if phase=='after':units.append(fixture/'native.cpp')
                units += [tree/name for name in ['src/animation/AnimBlendAssocGroup.c','src/animation/AnimBlendAssociation.c','src/animation/AnimBlendNode.c','src/animation/AnimBlendList.c','src/animation/AnimBlendClumpData.c']]
            commands=['@echo off','call "'+str(vcvars)+'" >nul','if errorlevel 1 exit /b 1'];objects=[]
            for path in units:
                # Compile the entire selected manager operations and actual complete C dependency producers
                obj=objdir/(path.stem+'.obj');objects.append(obj)
                commands+=['cl '+flags+(' /std:c11 /TC ' if path.suffix=='.c' else ' /EHsc /std:c++14 /Zc:sizedDealloc- ')+'/Fo"'+str(obj)+'" "'+str(path)+'"','if errorlevel 1 exit /b 1']
            exe=objdir/'loader.exe';commands+=['link /nologo /OUT:"'+str(exe)+'" '+' '.join('"'+str(p)+'"' for p in objects),'exit /b %errorlevel%']
            driver=objdir/'build.cmd';driver.write_text('\n'.join(commands)+'\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=env,capture_output=True,text=True);(objdir/'build.log').write_text(run.stdout+run.stderr,newline='\n')
            assert run.returncode==0,(label,phase,run.stdout[-5000:],run.stderr[-1000:])
            runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows'
            run=subprocess.run([str(exe)],cwd=root,env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(label,phase,run.returncode,run.stderr)
            results.append(run.stdout.strip())
        assert results[0]==results[1],(label,results)
        runs.append({'bits':bits,'profile':profile,'before':results[0],'after':results[1]});print(label,results[0],flush=True)
(out/('full-native-runs.json' if full else 'loader-native-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
