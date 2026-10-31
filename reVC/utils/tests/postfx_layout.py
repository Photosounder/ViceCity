"""Check every public PostFX storage extent, alignment and enum using independent C/C++ consumers."""
from pathlib import Path
import ctypes,json,os,subprocess
import postfx_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/postfx-c-tests';probe=out/'layouts';probe.mkdir(exist_ok=True);env=dict(os.environ);key=next(k for k in env if k.lower()=='path');ctypes.windll.kernel32.SetErrorMode(0x8003);rows=[]
for bits in [32,64]:
    # Inspect the original SDK pointer declarations and every independent C storage declaration
    values=[]
    for phase in ['original','new-cpp','new-c']:
        # Compare unevaluated real public declarations without allocating graphics resources
        cpp=phase!='new-c';tree=out/('before' if phase=='original' else 'overlay');prefix='#include <stdio.h>\n#include <stdint.h>\n#include "config.h"\n'
        if phase=='original':prefix+='#include <rwcore.h>\n#include <rpworld.h>\ntypedef int32_t int32;typedef uint32_t uint32;\n'
        prefix+='#include "postfx.h"\n';owner='CPostFX::' if phase=='original' else 'PostFX_';statements=[]
        for name in m.VARIABLES:
            # Observe full smoothing arrays as well as scalar and SDK handle storage
            expression=owner+name;alignment='decltype('+expression+')' if cpp else expression;statements.append('printf("'+name+' %zu %zu\\n",sizeof('+expression+'),(size_t)__alignof('+alignment+'));')
        constants=['POSTFX_OFF','POSTFX_SIMPLE','POSTFX_NORMAL','POSTFX_MOBILE','NUMAVERAGE'];expressions=[('CPostFX::'+v if phase=='original' else ('POSTFX_NUMAVERAGE' if v=='NUMAVERAGE' else v)) for v in constants];statements.append('printf("enum %d %d %d %d %d\\n",'+','.join(expressions)+');')
        label=f'{bits}-{phase}';path=probe/(label+('.cpp' if cpp else '.c'));path.write_text(prefix+'int main(void)\n{\n    // Report all public application storage without reading unspecified object values\n    '+'\n    '.join(statements)+'\n    return 0;\n}\n',newline='\n');exe=probe/(label+'.exe');vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';includes=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir()]+[root/'vendor/librw',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];flags='/nologo /MT /O2 /Gy /Gw /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DNOMINMAX '+('/std:c++14 /EHsc /Zc:sizedDealloc- ' if cpp else '/std:c11 /TC ')+' '.join('/I"'+str(p)+'"' for p in includes);driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(probe/(label+'.obj'))+'" /Fe"'+str(exe)+'" "'+str(path)+'" /link /OPT:REF\nexit /b %errorlevel%\n',newline='\n');run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-4000:],run.stderr);runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=10);assert run.returncode==0,(label,run.stderr);values.append(run.stdout)
    assert len(set(values))==1,(bits,values);rows.append({'bits':bits,'consumers':['original-sdk','new-cpp','new-c'],'layout':values[0],'exit':0});print(bits,values[0],flush=True)
(out/'native-layouts.json').write_text(json.dumps(rows,indent=2),newline='\n')
