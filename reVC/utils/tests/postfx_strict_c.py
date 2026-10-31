"""Compile complete C post-processing and C forwarding across every original graphics preprocessing branch."""
from pathlib import Path
import json,os,subprocess

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/postfx-c-tests';tree=out/'overlay';probe=out/'strict';probe.mkdir(exist_ok=True);env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];results=[];limits=[]
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
for language in ['c','cpp']:
    # Require SDK-free public C and C++ consumers of both application and primitive declarations
    (probe/('api.'+language)).write_text('#include "postfx.h"\n#include "PostFX_backend.h"\n',newline='\n')
profiles=[('d3d9',['LIBRW','RW_D3D9'],[]),('gl',['LIBRW','RW_GL3'],[]),('neutral',['LIBRW'],[]),('master',['LIBRW','RW_D3D9','MASTER'],[]),('final',['LIBRW','RW_D3D9','FINAL'],[]),('inactive',['LIBRW'],['EXTENDED_COLOURFILTER']),('legacy-inactive',[],['EXTENDED_COLOURFILTER']),('legacy-active',[],[])]
for compiler in ['clang','gcc']:
    # Compile both entire C producers on real 32-bit and 64-bit Windows compiler targets
    for bits in [32,64]:
        for profile,defines,undefines in profiles:
            header=probe/f'{compiler}-{bits}-{profile}-profile.h';header.write_text('#include "'+(tree/'src/core/config.h').as_posix()+'"\n'+''.join('#undef '+name+'\n' for name in undefines),newline='\n')
            if profile=='legacy-active':header.write_text(header.read_text()+'#define EXTENDED_COLOURFILTER\n',newline='\n')
            for unit in ['postfx.c','PostFX_bindings.c']:
                # Preserve the original librw requirement instead of manufacturing a legacy extended-filter implementation
                if profile=='legacy-active' and unit!='postfx.c':continue
                label=f'{compiler}-{bits}-{profile}-{unit}';tool=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');command=[str(tool),'-std=c11','-pedantic-errors','-O2','-m'+str(bits),*['-D'+d for d in defines],'-include',str(header),*['-I'+str(p) for p in dirs],'-c',str(tree/'src/extras'/unit),'-o',str(probe/(label+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n')
                if profile=='legacy-active':assert run.returncode!=0 and 'Need librw for EXTENDED_COLOURFILTER' in run.stderr,(label,run.stderr);limits.append({'compiler':compiler,'bits':bits,'profile':profile,'exit':run.returncode});continue
                assert run.returncode==0,(label,run.stderr[-5000:]);results.append({'compiler':compiler,'bits':bits,'profile':profile,'unit':unit,'exit':0})
        for language in ['c','cpp']:
            # Compile self-contained public declarations without the old C++ common header
            label=f'{compiler}-{bits}-api-{language}';tool=Path('C:/msys/ucrt64/bin')/('clang++.exe' if compiler=='clang' and language=='cpp' else 'g++.exe' if language=='cpp' else compiler+'.exe');command=[str(tool),'-std=c++14' if language=='cpp' else '-std=c11','-pedantic-errors','-m'+str(bits),*['-I'+str(p) for p in dirs],'-c',str(probe/('api.'+language)),'-o',str(probe/(label+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr[-4000:]);results.append({'compiler':compiler,'bits':bits,'profile':'api','unit':language,'exit':0})
assert len(results)==64 and len(limits)==4
(out/'strict-compiles.json').write_text(json.dumps(results,indent=2),newline='\n');(out/'strict-existing-limitations.json').write_text(json.dumps(limits,indent=2),newline='\n');print('Passed all',len(results),'complete producer, forwarding and standalone API compilations')
