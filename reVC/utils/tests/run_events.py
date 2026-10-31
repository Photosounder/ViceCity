"""Run quiet before/after event/accident comparisons with self-contained diagnostic runtimes."""
from pathlib import Path
import sys,os,subprocess,json,ctypes
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/events-c-tests'
env=dict(os.environ);pathKey=next(key for key in env if key.lower()=='path');env[pathKey]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[pathKey]
ctypes.windll.kernel32.SetErrorMode(0x8003)
if '--prepared' not in sys.argv:subprocess.run([sys.executable,str(root/'utils/tests/events_fixture.py')],env=env,check=True)
sanitized='--sanitized' in sys.argv
profiles=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)] if '--matrix' in sys.argv else [('clang',0)]
sanitizerFlags=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else []
runs=[]
def command(args,log):
    # Preserve the exact failing command and quiet compiler diagnostics for diagnosis
    runEnv=dict(env)
    if len(args)==1:runEnv[pathKey]='C:/Windows/System32;C:/Windows'
    result=subprocess.run([str(a) for a in args],cwd=root,env=runEnv,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    log.write('$ '+' '.join(str(a) for a in args)+'\n'+result.stdout.decode(errors='replace')+'\n');log.flush()
    assert result.returncode==0,(' '.join(str(a) for a in args),result.returncode,result.stdout.decode(errors='replace'))
    return result.stdout.decode(errors='replace').strip()
for compiler,opt in profiles:
    cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
    for profile,defines in [('fixed',['FIX_BUGS']),('squeeze',['FIX_BUGS','SQUEEZE_PERFORMANCE']),('unfixed',[]),('squeeze-unfixed',['SQUEEZE_PERFORMANCE'])]:
        outputs=[];prefix=f'{compiler}-O{opt}-{profile}'+('-ubsan' if sanitized else '')
        with (out/(prefix+'.log')).open('w',newline='\n') as log:
            for phase in ['before','after']:
                flags=['-O'+str(opt),'-fno-fast-math','-I'+str(out),'-I'+str(out/'preview')]+sanitizerFlags+['-D'+d for d in defines]+(['-DTEST_EVENTS_ORIGINAL'] if phase=='before' else [])
                sources=[('caller.c',False),('fixture.cpp',True)]+([('original.cpp',True)] if phase=='before' else [('preview/GameplayEvents.c',False),('event-host.cpp',True),('accident-host.cpp',True)])
                objects=[]
                for i,(source,cpp) in enumerate(sources):
                    obj=out/f'{prefix}-{phase}-{i}.o';objects.append(obj)
                    command([cxx if cpp else cc,'-std=c++11' if cpp else '-std=c11',*flags,*([] if cpp else ['-pedantic-errors']),'-c',out/source,'-o',obj],log)
                exe=out/f'{prefix}-{phase}.exe';command([cxx,'-static','-static-libgcc','-static-libstdc++',*sanitizerFlags,*objects,'-o',exe],log)
                outputs.append(command([exe],log))
        assert outputs[0]==outputs[1],(prefix,outputs)
        item={'compiler':compiler,'opt':opt,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0};runs.append(item);print(prefix,outputs[0],flush=True)
(out/('sanitized-runs.json' if sanitized else 'matrix-runs.json' if '--matrix' in sys.argv else 'smoke-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
