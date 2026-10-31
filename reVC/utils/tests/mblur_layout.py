"""Compare C blur records and live reference views with actual SDK layouts on both native ABIs."""
from pathlib import Path
import ctypes,json,os,subprocess

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/mblur-c-tests';probe=out/'layouts';probe.mkdir(exist_ok=True);env=dict(os.environ);key=next(k for k in env if k.lower()=='path');ctypes.windll.kernel32.SetErrorMode(0x8003);results=[]
records={'MBlurRect':['x','y','w','h'],'MBlurColour':['red','green','blue','alpha'],'MBlurRectRef':['x','y','w','h']}
for bits in [32,64]:
    # Compare actual SDK records with independently compiled C and C++ public consumers
    observations=[]
    for phase in ['original','new-cpp','new-c']:
        # Derive the original native reference field types directly from actual SDK scalar subobjects
        tree=out/('before' if phase=='original' else 'overlay');cpp=phase!='new-c';prefix='#include <stdint.h>\n#include <stddef.h>\n#include <stdio.h>\n'
        if phase=='original':prefix+='#include <rwcore.h>\n#include <rpworld.h>\ntypedef RwRect MBlurRect;typedef RwRGBA MBlurColour;\nstruct MBlurRectRef {decltype(&((RwRect*)0)->x) x;decltype(&((RwRect*)0)->y) y;decltype(&((RwRect*)0)->w) w;decltype(&((RwRect*)0)->h) h;};\n'
        else:prefix+='#include "MBlur.h"\n'
        statements=[]
        for record,fields in records.items():
            # Include every data member, total extent and alignment rather than assuming SDK binary equivalence
            statements.append('printf("'+record+' %zu %zu",sizeof('+record+'),(size_t)__alignof('+record+'));');statements+=['printf(" '+field+'=%zu",offsetof('+record+','+field+'));' for field in fields];statements.append('puts("");')
        label=f'{bits}-{phase}';path=probe/(label+('.cpp' if cpp else '.c'));path.write_text(prefix+'int main(void)\n{\n    // Report complete data layouts without constructing SDK rendering resources\n    '+'\n    '.join(statements)+'\n    return 0;\n}\n',newline='\n');vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';exe=probe/(label+'.exe')
        includes=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir()]+[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
        flags='/nologo /O2 /Gy /Gw /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c++14 /EHsc /Zc:sizedDealloc- ' if cpp else '/std:c11 /TC ')+' '.join('/I"'+str(p)+'"' for p in includes);command='cl '+flags+' /Fo"'+str(probe/(label+'.obj'))+'" /Fe"'+str(exe)+'" "'+str(path)+'" /link /OPT:REF /LIBPATH:"C:/msys/home/github/quakespasm/Windows/SDL2/'+('lib' if bits==32 else 'lib64')+'"';driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\n'+command+'\nexit /b %errorlevel%\n',newline='\n')
        run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr);runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=10);assert run.returncode==0,(label,run.stderr);observations.append(run.stdout)
    assert len(set(observations))==1,(bits,observations);results.append({'bits':bits,'consumers':['original-sdk','new-cpp','new-c'],'layout':observations[0],'exit':0});print(bits,observations[0].strip(),flush=True)
(out/'native-layouts.json').write_text(json.dumps(results,indent=2),newline='\n')
