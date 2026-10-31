"""Run whole original/C plugin comparisons with static runtimes and no error dialogs."""
from pathlib import Path
import ctypes,json,os,re,subprocess,sys
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/node-name-c-tests';probe=out/'runtime';env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
ctypes.windll.kernel32.SetErrorMode(0x8003)
if '--prepared' not in sys.argv:subprocess.run([sys.executable,str(root/'utils/tests/node_name_fixture.py')],env=env,check=True)
sanitized='--sanitized' in sys.argv;matrix='--matrix' in sys.argv;compilers=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)] if matrix else [('clang',0)];runs=[]
for compiler,opt in compilers:
    # Compile both complete producers and exercise their actual SDK registration callbacks
    label=f'{compiler}-O{opt}'+('-ubsan' if sanitized else '');outputs=[]
    with (probe/(label+'.log')).open('w',newline='\n') as log:
        for phase in ['before','after']:
            # Keep the complete C owner separate from the typed native SDK callback adapters
            tree=out/('before' if phase=='before' else 'overlay');sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else [];flags=['-O'+str(opt),*sanitize,'-I'+str(probe),'-I'+str(tree/'src/rw')]
            cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
            units=[(cxx,probe/'fixture.cpp')]+([(cxx,probe/'original.cpp'),(cxx,probe/'before-caller.cpp')] if phase=='before' else [(cc,tree/'src/rw/NodeName.c'),(cxx,probe/'native.cpp'),(cc,probe/'after-caller.c')]);objects=[]
            for tool,path in units:
                # Compile every new algorithm and its independent public caller as strict C11
                obj=probe/f'{label}-{phase}-{path.stem}.o';objects.append(obj);command=[str(tool),'-std=c11' if path.suffix=='.c' else '-std=c++14',*flags,*(['-pedantic-errors'] if path.suffix=='.c' else []),'-c',str(path),'-o',str(obj)]
                run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(' '.join(command)+'\n'+run.stdout+run.stderr);log.flush();assert run.returncode==0,(phase,path,run.stderr[-4000:])
            exe=probe/f'{label}-{phase}.exe';run=subprocess.run([str(cxx),'-static','-static-libgcc','-static-libstdc++',*sanitize,*[str(p) for p in objects],'-o',str(exe)],env=env,capture_output=True,text=True);log.write(run.stdout+run.stderr);log.flush();assert run.returncode==0,run.stderr[-4000:]
            if compiler=='gcc':
                # Reject MSYS and GCC runtime DLL dependencies before system-path execution
                imports=subprocess.run([str(cc.with_name('objdump.exe')),'-p',str(exe)],env=env,capture_output=True,text=True,check=True);log.write(imports.stdout);names=re.findall(r'DLL Name: (\S+)',imports.stdout);assert names and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in names),names
            runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(phase,run.returncode,run.stderr);outputs.append(run.stdout.strip())
    assert outputs[0]==outputs[1],(label,outputs);runs.append({'compiler':compiler,'opt':opt,'before':outputs[0],'after':outputs[1],'exit':0});print(label,outputs[0],flush=True)
(out/('sanitized-runs.json' if sanitized else 'matrix-runs.json' if matrix else 'smoke-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
