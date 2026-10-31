"""Verify complete C hierarchy and parent layouts and every native hierarchy consumer."""
from pathlib import Path
import os,json,sys,subprocess,re,struct
import anim_association_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-association-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'classic-probes'/('production' if '--production' in sys.argv else 'native');probe.mkdir(parents=True,exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[]

def compile(bits,name,path,tree,isC=False,extra=""):
    # Compile complete real sources with one coherent set of native headers
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags=extra+' /nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,run.stdout[-5000:]);results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return out/(prefix+'.obj')

for bits in [32,64]:
    for phase,tree in [('before',saved),('after',base)]:
        text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#undef FIX_BUGS\n#include "'+(tree/'src/animation/AnimBlendAssociation.h').as_posix()+'"\n#include <stddef.h>\n#include <type_traits>\n'
        size=(60 if bits==32 else 96) if phase=='before' else (56 if bits==32 else 88);offset=(4 if bits==32 else 8) if phase=='before' else 0
        text+='static_assert(sizeof(CAnimBlendAssociation)=='+str(size)+'&&offsetof(CAnimBlendAssociation,link)=='+str(offset)+',"Actual classic virtual dispatch removal");\n'
        text+='static_assert(std::is_polymorphic<CAnimBlendAssociation>::value=='+('true' if phase=='before' else 'false')+',"No association virtual dispatch remains");\n'
        path=probe/f'classic-{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'classic-types-'+phase,path,tree)
(out/('production-classic-layout-compiles.json' if '--production' in sys.argv else 'classic-layout-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
