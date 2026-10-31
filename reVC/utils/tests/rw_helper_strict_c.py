"""Compile the entire C helper producer across both compiler families, ABIs and original policy branches."""
from pathlib import Path
import os,json,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rw-helper-c-tests';tree=out/'overlay';probe=out/'strict';probe.mkdir(exist_ok=True);env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];rows=[]
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
for language in ['c','cpp']:
    # Check SDK-free public declarations independently of any precompiled common header
    (probe/('api.'+language)).write_text('#include "RwHelper.h"\n#include "RwHelper_backend.h"\n',newline='\n')
profiles=[('d3d9',['LIBRW','RW_D3D9'],[]),('gl',['LIBRW','RW_GL3'],[]),('neutral',['LIBRW'],[]),('master',['LIBRW','RW_D3D9','MASTER'],[]),('final',['LIBRW','RW_D3D9','FINAL'],[]),('classic',['LIBRW','RW_D3D9'],['FIX_BUGS']),('legacy',[],[]),('legacy-classic',[],['FIX_BUGS'])]
for compiler in ['clang','gcc']:
    for bits in [32,64]:
        for profile,defines,undefines in profiles:
            # Compile every complete original policy branch as actual C11 object code
            label=f'{compiler}-{bits}-{profile}';header=probe/(label+'-profile.h');header.write_text('#include "'+(tree/'src/core/config.h').as_posix()+'"\n'+''.join('#undef '+name+'\n' for name in undefines),newline='\n');tool=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');command=[str(tool),'-std=c11','-pedantic-errors','-Werror','-Wno-unused-value','-O2','-m'+str(bits),*['-D'+d for d in defines],'-include',str(header),*['-I'+str(p) for p in dirs],'-c',str(tree/'src/rw/RwHelper.c'),'-o',str(probe/(label+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr[-6000:]);rows.append({'compiler':compiler,'bits':bits,'profile':profile,'unit':'RwHelper.c','exit':0})
        for language in ['c','cpp']:
            # Validate both public consumer languages without bringing SDK C++ types into the C interface
            label=f'{compiler}-{bits}-api-{language}';tool=Path('C:/msys/ucrt64/bin')/('clang++.exe' if compiler=='clang' and language=='cpp' else 'g++.exe' if language=='cpp' else compiler+'.exe');command=[str(tool),'-std=c++14' if language=='cpp' else '-std=c11','-pedantic-errors','-Werror','-m'+str(bits),*['-I'+str(p) for p in dirs],'-c',str(probe/('api.'+language)),'-o',str(probe/(label+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr[-4000:]);rows.append({'compiler':compiler,'bits':bits,'profile':'api','unit':language,'exit':0})
assert len(rows)==40
(out/'strict-compiles.json').write_text(json.dumps(rows,indent=2),newline='\n');print('Passed all 40 whole C producer and independent API compilations')
