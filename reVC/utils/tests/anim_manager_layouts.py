"""Prove descriptor and block layouts through actual C/C++ headers under both MSVC ABIs."""
from pathlib import Path
import json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';probe=out/'layouts';probe.mkdir(exist_ok=True);results=[]
for bits in [32,64]:
    # Compare original C++ records with the complete staged C API and direct C11 consumers
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';pointer=bits//8
    for phase,language in [('before','cpp'),('after','cpp'),('after','c')]:
        tree=out/('before' if phase=='before' else 'overlay');label=f'{bits}-{phase}-{language}'
        dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
        prefix='#include <stdint.h>\n#include <stddef.h>\n'
        if phase=='before':prefix+='typedef int32_t int32;\nstruct RpClump;struct RwStream;\n#define nil nullptr\n'
        prefix+='#include "AnimManager.h"\n'
        expected=[('sizeof(AnimAssocDesc)',8),('offsetof(AnimAssocDesc,animId)',0),('offsetof(AnimAssocDesc,flags)',4),('sizeof(AnimAssocDefinition)',24 if bits==32 else 40),('offsetof(AnimAssocDefinition,name)',0),('offsetof(AnimAssocDefinition,blockName)',pointer),('offsetof(AnimAssocDefinition,modelIndex)',2*pointer),('offsetof(AnimAssocDefinition,numAnims)',2*pointer+4),('offsetof(AnimAssocDefinition,animNames)',2*pointer+8),('offsetof(AnimAssocDefinition,animDescs)',3*pointer+8),('sizeof(CAnimBlock)',32),('offsetof(CAnimBlock,firstIndex)',24)]
        for expression,value in expected:
            # Ask the real native compiler to verify each field instead of inferring ABI from host Python
            prefix+=('_Static_assert' if language=='c' else 'static_assert')+'('+expression+'=='+str(value)+',"Original manager descriptor layout");\n'
        path=probe/(label+('.c' if language=='c' else '.cpp'));path.write_text(prefix,newline='\n')
        flags='/nologo /c '+('/std:c11 /TC ' if language=='c' else '/std:c++14 /EHsc ')+' '.join('/I"'+str(p)+'"' for p in dirs)
        driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(probe/(label+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout,run.stderr)
        results.append({'bits':bits,'phase':phase,'language':language,'layouts':expected,'exit':0});print(label,'passed',flush=True)
(out/'native-layouts.json').write_text(json.dumps(results,indent=2),newline='\n')
