"""Compare C frame ownership, all native animation callbacks and live SDK pose mutations."""
from pathlib import Path
import os,sys,json,subprocess,ctypes,re
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-sequence-c-tests';env=dict(os.environ);pathKey=next(k for k in env if k.lower()=='path');env[pathKey]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[pathKey]
ctypes.windll.kernel32.SetErrorMode(0x8003)
if '--prepared' not in sys.argv:subprocess.run([sys.executable,str(root/'utils/tests/anim_sequence_fixture.py')],env=env,check=True)
sanitized='--sanitized' in sys.argv;compilers=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)] if '--matrix' in sys.argv else [('clang',0)]
sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else [];runs=[]
def command(args,log):
    # Require static diagnostics to run without MSYS DLL paths or native error windows
    runEnv=dict(env)
    if Path(args[0]).parent==out:runEnv[pathKey]='C:/Windows/System32;C:/Windows'
    run=subprocess.run([str(v) for v in args],cwd=root,env=runEnv,capture_output=True);output=run.stdout.decode(errors='replace')+run.stderr.decode(errors='replace');log.write('$ '+' '.join(str(v) for v in args)+'\n'+output+'\n');log.flush()
    assert run.returncode==0,(str(args[0]),run.returncode,output[-4000:]);return output.strip()
for compiler,opt in compilers:
    cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
    for profile,defines in [('fixed',[]),('classic',['CLASSIC']),('heap',['USE_CUSTOM_ALLOCATOR'])]:
        prefix=f'{compiler}-O{opt}-{profile}'+('-ubsan' if sanitized else '');outputs=[]
        with (out/(prefix+'.log')).open('w',newline='\n') as log:
            for phase in ['before','after']:
                flags=['-O'+str(opt),'-fno-fast-math','-ffp-contract=off','-I'+str(out)]+sanitize+['-D'+d for d in defines]+(['-DORIGINAL'] if phase=='before' else []);objects=[]
                for name,cpp in [('caller.c',False),('fixture.cpp',True),('math.cpp',True),('AnimBlendNode.c',False),('KeyFrames.c',False),('Quaternion.c',False),('FrameUpdate.cpp',True)]+([('original.cpp',True)] if phase=='before' else [('AnimBlendSequence.c',False)]):
                    obj=out/f'{prefix}-{phase}-{name}.o';objects.append(obj);command([cxx if cpp else cc,'-std=c++14' if cpp else '-std=c11',*flags,*([] if cpp else ['-pedantic-errors']),'-c',out/name,'-o',obj],log)
                exe=out/f'{prefix}-{phase}.exe';command([cxx,'-static','-static-libgcc','-static-libstdc++',*sanitize,*objects,'-o',exe],log)
                if compiler=='gcc':
                    imports=command([cc.with_name('objdump.exe'),'-p',exe],log)
                    dependencies=re.findall(r'DLL Name: ([^\s]+)',imports)
                    assert dependencies and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in dependencies),(exe,dependencies)
                outputs.append(command([exe],log))
        assert outputs[0]==outputs[1],(prefix,outputs);runs.append({'compiler':compiler,'opt':opt,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(prefix,outputs[0],flush=True)
(out/('sanitized-runs.json' if sanitized else 'matrix-runs.json' if '--matrix' in sys.argv else 'smoke-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
