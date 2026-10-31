"""Compile the complete plugin producer and standalone public C/C++ API with both MSYS toolchains."""
from pathlib import Path
import json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/node-name-c-tests';tree=out/'overlay';probe=out/'strict';probe.mkdir(exist_ok=True)
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];results=[]
for language in ['c','cpp']:
    # Prove the API header is self-contained and requires no SDK or common native declarations
    (probe/('api.'+language)).write_text('#include "NodeName.h"\n#include "NodeName_backend.h"\n',newline='\n')
for compiler in ['clang','gcc']:
    # Require real C11 compilation for every frame-name plugin algorithm
    for bits in [32,64]:
        for profile,defines in [('default',[])]:
            label=f'{compiler}-{bits}-{profile}';tool=Path('C:/msys/ucrt64/bin')/(compiler+'.exe')
            command=[str(tool),'-std=c11','-pedantic-errors','-O2','-m'+str(bits),*['-D'+d for d in defines],*['-I'+str(p) for p in dirs],'-c',str(tree/'src/rw/NodeName.c'),'-o',str(probe/(label+'.o'))]
            run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr[-4000:]);results.append({'compiler':compiler,'bits':bits,'profile':profile,'unit':'producer','exit':0})
        for language in ['c','cpp']:
            # Exercise the C declarations and their C++ linkage wrapper without importing SDK types
            label=f'{compiler}-{bits}-api-{language}';tool=Path('C:/msys/ucrt64/bin')/(compiler+'.exe' if language=='c' else 'clang++.exe' if compiler=='clang' else 'g++.exe')
            command=[str(tool),'-std=c11' if language=='c' else '-std=c++14','-pedantic-errors','-m'+str(bits),*['-I'+str(p) for p in dirs],'-c',str(probe/('api.'+language)),'-o',str(probe/(label+'.o'))]
            run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr[-4000:]);results.append({'compiler':compiler,'bits':bits,'profile':'api','unit':language,'exit':0})
print('Passed',len(results),'strict producer/API compilations')
(out/'strict-compiles.json').write_text(json.dumps(results,indent=2),newline='\n')
