"""Verify actual native plugin storage and shared animation layouts against both C and C++ headers."""
from pathlib import Path
import json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-plugin-c-tests';probe=out/'layouts';probe.mkdir(exist_ok=True);results=[]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
for bits in [32,64]:
    # Verify both real pointer widths for the plugin slot and every shared animation record this owner operates on
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for phase,language in [('before','cpp'),('after','cpp'),('after','c')]:
        tree=out/('before' if phase=='before' else 'overlay');label=f'{bits}-{phase}-{language}';isC=language=='c';path=probe/(label+('.c' if isC else '.cpp'))
        prefix=('#include "common.h"\n' if not isC else '')+'#include "RpAnimBlend.h"\n#include "AnimBlendAssociation.h"\n#include "AnimBlendClumpData.h"\n#include <stddef.h>\n'
        expected=[('sizeof(ClumpOffset)',4),('sizeof(CAnimBlendClumpData)',20 if bits==32 else 40),('offsetof(CAnimBlendClumpData,frames)',16 if bits==32 else 32),('sizeof(AnimBlendFrameData)',24 if bits==32 else 32),('sizeof(AnimBlendFrameUpdateData)',68 if bits==32 else 136),('sizeof(CAnimBlendAssociation)',56 if bits==32 else 88),('sizeof(KeyFrameVector3)',12)]
        if not isC:expected += [('sizeof(RwInt32)',4),('sizeof(RwV3d)',12)]
        text=prefix+''.join(('_Static_assert' if isC else 'static_assert')+'('+expression+'=='+str(value)+',"Original plugin animation storage layout");\n' for expression,value in expected);path.write_text(text,newline='\n')
        dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
        flags='/nologo /c /O2 /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/std:c++14 /EHsc ')+' '.join('/I"'+str(p)+'"' for p in dirs+deps)
        driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(probe/(label+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr[-1000:]);results.append({'bits':bits,'phase':phase,'language':language,'exit':0});print(label,'passed',flush=True)
(out/'native-layouts.json').write_text(json.dumps(results,indent=2),newline='\n')
