"""Run quiet whole original/C comparisons with static diagnostic runtimes and strict C callers."""
from pathlib import Path
import os,sys,subprocess,json,ctypes
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/wanted-c-tests'
env=dict(os.environ);pathKey=next(k for k in env if k.lower()=='path');env[pathKey]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[pathKey]
ctypes.windll.kernel32.SetErrorMode(0x8003)
if '--prepared' not in sys.argv:subprocess.run([sys.executable,str(root/'utils/tests/wanted_fixture.py')],env=env,check=True)
sanitized='--sanitized' in sys.argv
profiles=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)] if '--matrix' in sys.argv else [('clang',0)]
sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else []
printHeader=out/'print.h';printText='#pragma once\n#include <stdio.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\nint WantedTest_Print(const char *format,...);\n#ifdef __cplusplus\n}\n#endif\n#define printf WantedTest_Print\n'
if not printHeader.exists() or printHeader.read_text()!=printText:printHeader.write_text(printText,newline='\n')
runs=[]
def command(args,log):
    # Keep exact failing commands and compiler diagnostics while running the linked checks without MSYS paths
    runEnv=dict(env)
    if len(args)==1:runEnv[pathKey]='C:/Windows/System32;C:/Windows'
    run=subprocess.run([str(v) for v in args],cwd=root,env=runEnv,capture_output=True)
    output=run.stdout.decode(errors='replace')+run.stderr.decode(errors='replace');log.write('$ '+' '.join(str(v) for v in args)+'\n'+output+'\n');log.flush()
    assert run.returncode==0,(str(args[0]),run.returncode,output[-4500:])
    return output.strip()
for compiler,opt in profiles:
    cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
    for profile,defines in [('classic',[]),('significant',['FIX_SIGNIFICANT_BUGS'])]:
        prefix=f'{compiler}-O{opt}-{profile}'+('-ubsan' if sanitized else '');outputs=[]
        with (out/(prefix+'.log')).open('w',newline='\n') as log:
            for phase in ['before','after']:
                flags=['-O'+str(opt),'-fno-fast-math','-I'+str(out),'-I'+str(out/'preview'),'-include',printHeader]+sanitize+['-D'+d for d in defines]+(['-DTEST_WANTED_ORIGINAL'] if phase=='before' else [])
                sources=[('caller.c',False),('fixture.cpp',True),('AudioCrimes.c',False),('audio-'+phase+'.c',False)]+([('original.cpp',True)] if phase=='before' else [('kernel.c',False),('host.cpp',True)])
                objects=[]
                for i,(name,cpp) in enumerate(sources):
                    obj=out/f'{prefix}-{phase}-{i}.o';objects.append(obj);command([cxx if cpp else cc,'-std=c++11' if cpp else '-std=c11',*flags,*([] if cpp else ['-pedantic-errors']),'-c',out/name,'-o',obj],log)
                exe=out/f'{prefix}-{phase}.exe';command([cxx,'-static','-static-libgcc','-static-libstdc++',*sanitize,*objects,'-o',exe],log);outputs.append(command([exe],log))
        assert outputs[0]==outputs[1],(prefix,outputs[0][-200:],outputs[1][-200:])
        runs.append({'compiler':compiler,'opt':opt,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(prefix,outputs[0],flush=True)
(out/('sanitized-runs.json' if sanitized else 'matrix-runs.json' if '--matrix' in sys.argv else 'smoke-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
