"""Compare all original/C operations using quiet static Clang/GCC executables."""
from pathlib import Path
import os,sys,json,subprocess,ctypes
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rect-c-tests';env=dict(os.environ);pathKey=next(k for k in env if k.lower()=='path');env[pathKey]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[pathKey]
ctypes.windll.kernel32.SetErrorMode(0x8003)
if '--prepared' not in sys.argv:subprocess.run([sys.executable,str(root/'utils/tests/rect_fixture.py')],env=env,check=True)
sanitized='--sanitized' in sys.argv;compilers=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)] if '--matrix' in sys.argv else [('clang',0)]
sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else [];runs=[]
def command(args,log):
    # Keep static diagnostic executables independent of all MSYS runtime DLL search paths
    runEnv=dict(env)
    if len(args)==1:runEnv[pathKey]='C:/Windows/System32;C:/Windows'
    run=subprocess.run([str(v) for v in args],cwd=root,env=runEnv,capture_output=True);output=run.stdout.decode(errors='replace')+run.stderr.decode(errors='replace');log.write('$ '+' '.join(str(v) for v in args)+'\n'+output+'\n');log.flush()
    assert run.returncode==0,(str(args[0]),run.returncode,output[-4000:]);return output.strip()
for compiler,opt in compilers:
    cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe');prefix=f'{compiler}-O{opt}'+('-ubsan' if sanitized else '');outputs=[]
    with (out/(prefix+'.log')).open('w',newline='\n') as log:
        for phase in ['before','after']:
            flags=['-O'+str(opt),'-fno-fast-math','-ffp-contract=off','-I'+str(out)]+sanitize+(['-DORIGINAL'] if phase=='before' else []);objects=[]
            for name,cpp in [('caller.c',False),('fixture.cpp',True)]+([('original.cpp',True)] if phase=='before' else [('Rect.c',False)]):
                obj=out/f'{prefix}-{phase}-{name}.o';objects.append(obj);command([cxx if cpp else cc,'-std=c++14' if cpp else '-std=c11',*flags,*([] if cpp else ['-pedantic-errors']),'-c',out/name,'-o',obj],log)
            exe=out/f'{prefix}-{phase}.exe';command([cxx,'-static','-static-libgcc','-static-libstdc++',*sanitize,*objects,'-o',exe],log);outputs.append(command([exe],log))
    assert outputs[0]==outputs[1],(prefix,outputs);runs.append({'compiler':compiler,'opt':opt,'before':outputs[0],'after':outputs[1],'exit':0});print(prefix,outputs[0],flush=True)
(out/('sanitized-runs.json' if sanitized else 'matrix-runs.json' if '--matrix' in sys.argv else 'smoke-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
