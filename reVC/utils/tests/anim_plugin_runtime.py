"""Compare whole original/C plugin producers with actual converted animation owners beneath the SDK fixture."""
from pathlib import Path
import ctypes,json,os,re,subprocess,sys
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-plugin-c-tests';probe=out/'runtime';env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
ctypes.windll.kernel32.SetErrorMode(0x8003)
if '--prepared' not in sys.argv:subprocess.run([sys.executable,str(root/'utils/tests/anim_plugin_fixture.py')],env=env,check=True)
sanitized='--sanitized' in sys.argv;matrix='--matrix' in sys.argv;compilers=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)] if matrix else [('clang',0)]
profiles=[('fixed',[]),('classic',['CLASSIC']),('compression',['ANIM_COMPRESSION']),('compression-classic',['ANIM_COMPRESSION','CLASSIC'])] if matrix or sanitized else [('fixed',[])];runs=[]
dependencies=['src/animation/AnimBlendAssociation.c','src/animation/AnimBlendNode.c','src/animation/AnimBlendSequence.c','src/animation/AnimBlendHierarchy.c','src/animation/AnimBlendList.c','src/animation/AnimBlendClumpData.c','src/animation/KeyFrames.c','src/math/Quaternion.c','src/core/General.c','src/animation/Bones.c','src/animation/FrameUpdate.c']
for compiler,opt in compilers:
    # Compile entire original/C plugin owners with real node, association, hierarchy and frame implementations
    for profile,defines in profiles:
        label=f'{compiler}-O{opt}-{profile}'+('-ubsan' if sanitized else '');outputs=[]
        with (probe/(label+'.log')).open('w',newline='\n') as log:
            for phase in ['before','after']:
                # Retain a complete coherent actual header tree for each producer and consumer
                tree=out/('before' if phase=='before' else 'overlay');dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
                sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else [];flags=['-O'+str(opt),'-fno-fast-math','-ffp-contract=off','-DLIBRW',*sanitize,*['-D'+d for d in defines],'-I'+str(probe),*['-I'+str(p) for p in dirs]]
                cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
                units=[(cxx,probe/'fixture.cpp'),(cxx,probe/'frame-native.cpp')]+([(cxx,probe/'original.cpp'),(cxx,probe/'before-caller.cpp')] if phase=='before' else [(cc,tree/'src/animation/RpAnimBlend.c'),(cxx,probe/'native.cpp'),(cc,probe/'after-caller.c')])+[(cc,tree/name) for name in dependencies];objects=[]
                for tool,path in units:
                    # Keep every C algorithm and the new public caller under strict C11 compilation
                    obj=probe/f'{label}-{phase}-{path.stem}.o';objects.append(obj);command=[str(tool),'-std=c11' if path.suffix=='.c' else '-std=c++14',*flags,*(['-pedantic-errors'] if path.suffix=='.c' else []),'-c',str(path),'-o',str(obj)]
                    run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(' '.join(command)+'\n'+run.stdout+run.stderr);log.flush();assert run.returncode==0,(phase,path,run.stderr[-5000:])
                exe=probe/f'{label}-{phase}.exe';run=subprocess.run([str(cxx),'-static','-static-libgcc','-static-libstdc++',*sanitize,*[str(p) for p in objects],'-o',str(exe)],env=env,capture_output=True,text=True);log.write(run.stdout+run.stderr);log.flush();assert run.returncode==0,run.stderr[-5000:]
                if compiler=='gcc':
                    # Reject missing GCC/MSYS runtime DLL dependencies before quiet system-path execution
                    imports=subprocess.run([str(cc.with_name('objdump.exe')),'-p',str(exe)],env=env,capture_output=True,text=True,check=True);log.write(imports.stdout);names=re.findall(r'DLL Name: (\S+)',imports.stdout);assert names and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in names),names
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(phase,run.returncode,run.stderr);outputs.append(run.stdout.strip())
        assert outputs[0]==outputs[1],(label,outputs);runs.append({'compiler':compiler,'opt':opt,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(label,outputs[0],flush=True)
(out/('sanitized-runs.json' if sanitized else 'matrix-runs.json' if matrix else 'smoke-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
