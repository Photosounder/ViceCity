"""Compile complete frame callbacks and the SDK consumer with real native project headers."""
from pathlib import Path
import json,os,subprocess,sys
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/frame-update-c-tests';production='--production' in sys.argv;tree=root if production else out/'overlay';probe=out/('production-probes' if production else 'native-probes');probe.mkdir(exist_ok=True)
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[];layouts=[]
for bits in [32,64]:
    # Compile both actual native pointer-width ABIs and the real SDK field-access primitives
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    for name in ([] if '--layouts' in sys.argv else ['src/animation/FrameUpdate.c','src/animation/FrameUpdate_native.cpp','src/animation/RpAnimBlend.cpp']):
        # Retain the entire native consumer rather than replacing it with a declaration-only probe
        path=current_path(tree,name);label=f'{bits}-{path.stem}';obj=probe/(label+'.obj')
        flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if path.suffix=='.c' else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+' '.join('/I"'+str(p)+'"' for p in dirs+deps)
        driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(obj)+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])
        results.append({'bits':bits,'source':name,'exit':0});print(label,'passed',flush=True)
    if '--layouts' in sys.argv:
        # Prove the exact original dispatch record and the new C-callable reference descriptors under both ABIs
        for phase,language in [('before','cpp'),('after','cpp'),('after','c')]:
            current=out/('before' if phase=='before' else 'overlay');label=f'{bits}-{phase}-{language}-layout';isC=language=='c';path=probe/(label+('.c' if isC else '.cpp'))
            includes='#include "AnimBlendFrameUpdate.h"\n#include "FrameUpdate_backend.h"\n' if isC else '#include "common.h"\n#include "RpAnimBlend.h"\n'+('#include "FrameUpdate_backend.h"\n' if phase=='after' else '')
            expressions=[('sizeof(AnimBlendFrameUpdateData)',68 if bits==32 else 136),('offsetof(AnimBlendFrameUpdateData,foobar)',0),('offsetof(AnimBlendFrameUpdateData,nodes)',4 if bits==32 else 8),('sizeof(((AnimBlendFrameUpdateData*)0)->nodes)',16*(bits//8))]
            if phase=='after':expressions += [('sizeof(FrameUpdateVector2Out)',2*(bits//8)),('sizeof(FrameUpdateQuaternionOut)',4*(bits//8))]
            text=includes+'#include <stddef.h>\n'+''.join(('_Static_assert' if isC else 'static_assert')+'('+expression+'=='+str(value)+',"Original frame dispatch or field reference layout");\n' for expression,value in expressions)
            path.write_text(text,newline='\n');currentDirs=[current/'src']+[p for p in (current/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(current/'src').parts)]
            flags='/nologo /c /O2 /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/std:c++14 /EHsc ')+' '.join('/I"'+str(p)+'"' for p in currentDirs+deps)
            driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(probe/(label+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
            run=subprocess.run(['cmd.exe','/c',str(driver)],env=dict(os.environ),capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr[-1000:])
            layouts.append({'bits':bits,'phase':phase,'language':language,'exit':0});print(label,'passed',flush=True)
(out/(('production-' if production else '')+('native-layout-compiles.json' if '--layouts' in sys.argv else 'source-compiles.json' if production else 'native-source-compiles.json'))).write_text(json.dumps(layouts if '--layouts' in sys.argv else results,indent=2),newline='\n')
