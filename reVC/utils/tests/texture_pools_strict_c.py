"""Compile complete original/C pools with real Direct3D C and C++ SDK declarations."""
from pathlib import Path
import json,os,re,subprocess

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/texture-pools-c-tests';tree=out/'overlay';probe=out/'strict';probe.mkdir(exist_ok=True)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];results=[]
original=(out/'before/src/rw/TexturePools.cpp').read_text().replace('#include "common.h"','#include <stdint.h>\n#include <stdlib.h>\ntypedef uint32_t uint32;typedef int32_t int32;\n#define nil NULL\n#define CITA_TLS thread_local\nextern "C" {\n#include <cita_windows.h>\n}')
(probe/'original.cpp').write_text(original,newline='\n')
for language in ['c','cpp']:
    # Require self-contained public declarations in both languages without the old common C++ header
    (probe/('api.'+language)).write_text('#include "TexturePools.h"\n',newline='\n')
for compiler in ['clang','gcc']:
    # Compile real active legacy algorithms and the intentionally inactive librw translation unit
    for bits in [32,64]:
        for profile,defines in [('fixed',[]),('classic',[]),('librw',['LIBRW']),('master',['MASTER'])]:
            # Configure the same immutable project header before either whole producer
            header=probe/f'{compiler}-{bits}-{profile}-config.h';value='#include "'+(tree/'src/core/config.h').as_posix()+'"\n'
            if profile=='classic':value+='#undef FIX_BUGS\n'
            header.write_text(value,newline='\n')
            units=[('producer',tree/'src/rw/TexturePools.c'),('api-c',probe/'api.c'),('api-cpp',probe/'api.cpp')]
            if profile in ['fixed','classic']:units.append(('original',probe/'original.cpp'))
            for label,path in units:
                # Keep SDK COM call signatures, pointer widths, integer comparisons and C language rules real
                cpp=path.suffix=='.cpp';tool=Path('C:/msys/ucrt64/bin')/('clang++.exe' if compiler=='clang' and cpp else 'g++.exe' if cpp else compiler+'.exe');name=f'{compiler}-{bits}-{profile}-{label}'
                base=out/'before' if label=='original' else tree
                command=[str(tool),'-std=c++14' if cpp else '-std=c11','-pedantic-errors','-O2','-m'+str(bits),*['-D'+d for d in defines],'-include',str(header),'-I'+str(base/'src/rw'),'-I'+str(base/'src/core'),'-isystem','C:/msys/home/github/CITAlloc','-c',str(path),'-o',str(probe/(name+'.o'))]
                run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(name+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(name,run.stderr[-5000:]);results.append({'compiler':compiler,'bits':bits,'profile':profile,'unit':label,'exit':0})
assert len(results)==56
(out/'strict-compiles.json').write_text(json.dumps(results,indent=2),newline='\n')
print('Passed all',len(results),'whole original/C and public API compilations with actual Direct3D SDK headers')
