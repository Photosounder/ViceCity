"""Compile every complete C blur operation across hardware, software and legacy preprocessing."""
from pathlib import Path
import json,os,subprocess

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/mblur-c-tests';tree=out/'overlay';probe=out/'strict';probe.mkdir(exist_ok=True);env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];results=[];limitations=[]
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
for language in ['c','cpp']:
    # Require self-contained public records, callbacks and SDK primitive declarations in both languages
    (probe/('api.'+language)).write_text('#include "MBlur.h"\n#include "MBlur_backend.h"\n#include "Camera_effects.h"\n',newline='\n')
profiles=[('extended',['LIBRW'],[]),('classic',['LIBRW'],['EXTENDED_COLOURFILTER']),('software',['LIBRW','REVC_SOFTWARE_POLYGONS'],['EXTENDED_COLOURFILTER']),('legacy',[],['EXTENDED_COLOURFILTER']),('ps2',['GTA_PS2','VANILLA_DEFINES'],['EXTENDED_COLOURFILTER']),('master',['LIBRW','MASTER'],[]),('final',['LIBRW','FINAL'],[]),('unscaled',['LIBRW'],['EXTENDED_COLOURFILTER','ASPECT_RATIO_SCALE','PROPER_SCALING'])]
for compiler in ['clang','gcc']:
    # Exercise all original preprocessing alternatives with real C11 rather than a C++ compatibility mode
    for bits in [32,64]:
        for profile,defines,undefines in profiles:
            # Configure one coherent immutable project header before the producer
            name=f'{compiler}-{bits}-{profile}';header=probe/(name+'-profile.h');header.write_text('#include "'+(tree/'src/core/config.h').as_posix()+'"\n'+''.join('#undef '+symbol+'\n' for symbol in undefines),newline='\n');tool=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');command=[str(tool),'-std=c11','-pedantic-errors','-O2','-m'+str(bits),*['-D'+d for d in defines],'-include',str(header),*['-I'+str(p) for p in dirs],'-c',str(tree/'src/renderer/MBlur.c'),'-o',str(probe/(name+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(name+'.log')).write_text(run.stdout+run.stderr,newline='\n');
            if profile=='ps2' and bits==64:
                # Retain the original vanilla configuration guard instead of suppressing it to force a test through
                assert run.returncode!=0 and 'Vanilla can only be built for win-x86' in run.stderr,(name,run.stderr);limitations.append({'compiler':compiler,'bits':bits,'profile':profile,'reason':'Original vanilla profile is Win32 only','exit':run.returncode});continue
            assert run.returncode==0,(name,run.stderr[-5000:]);results.append({'compiler':compiler,'bits':bits,'profile':profile,'unit':'producer','exit':0})
        for language in ['c','cpp']:
            # Compile the public interface without SDK or common C++ declarations
            name=f'{compiler}-{bits}-api-{language}';tool=Path('C:/msys/ucrt64/bin')/('clang++.exe' if compiler=='clang' and language=='cpp' else 'g++.exe' if language=='cpp' else compiler+'.exe');command=[str(tool),'-std=c++14' if language=='cpp' else '-std=c11','-pedantic-errors','-m'+str(bits),*['-I'+str(p) for p in dirs],'-c',str(probe/('api.'+language)),'-o',str(probe/(name+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(name+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(name,run.stderr[-5000:]);results.append({'compiler':compiler,'bits':bits,'profile':'api','unit':language,'exit':0})
assert len(results)==38 and len(limitations)==2
(out/'strict-compiles.json').write_text(json.dumps(results,indent=2),newline='\n');print('Passed all',len(results),'complete C producer and self-contained interface compilations')

(out/'strict-existing-limitations.json').write_text(json.dumps(limitations,indent=2),newline='\n')
