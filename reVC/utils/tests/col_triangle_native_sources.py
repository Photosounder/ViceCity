"""Verify complete C triangle/plane ownership and scalar/VU behavior with native callers."""
from pathlib import Path
import os,json,sys,subprocess,re,struct
import col_triangle_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-triangle-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes'/('production' if '--production' in sys.argv else 'native');probe.mkdir(parents=True,exist_ok=True)
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
    for profile,defines in [('float',''),('compressed','/DCOMPRESSED_COL_VECTORS'),('vu','/DVU_COLLISION')]:
        packed=profile=='vu';size=8 if packed else 20;alignment=2 if packed else 4
        for phase,tree in [('before',saved),('after',base)]:
            text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/collision/ColModel.h').as_posix()+'"\n#include <stddef.h>\n#include <type_traits>\n#include <stdint.h>\n'
            for expression,value in [('sizeof(CColTriangle)',8),('alignof(CColTriangle)',2),('offsetof(CColTriangle,surface)',6),('sizeof(CColTrianglePlane)',size),('alignof(CColTrianglePlane)',alignment),('offsetof(CColTrianglePlane,dist)',6 if packed else 12),('sizeof(CColModel)',76 if bits==32 else 104),('offsetof(CColModel,trianglePlanes)',72 if bits==32 else 96)]:text+='static_assert('+expression+'=='+str(value)+',"Original triangle/plane/parent layout");\n'
            text+='static_assert(std::is_trivially_copyable<CColTriangle>::value&&std::is_trivially_copyable<CColTrianglePlane>::value,"Original value copying");\n'
            if phase=='after':text+='static_assert(sizeof(CollisionPointRef)==3*sizeof(float*)&&std::is_trivially_copyable<CollisionPointRef>::value,"Plain C reference record ABI");\n'
            if phase=='before' and not packed:text+='static_assert(std::is_trivially_default_constructible<RwV3d>::value,"Empty native vector base construction");\n'
            path=probe/f'types{bits}-{profile}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,profile+'-types-'+phase,path,tree,extra=defines)
        path=probe/f'types{bits}-{profile}-c.c';path.write_text('#include "'+(base/'src/collision/ColTriangle.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(CColTriangle)==8&&offsetof(CColTriangle,surface)==6&&sizeof(CColTrianglePlane)=='+str(size)+'&&_Alignof(CColTrianglePlane)=='+str(alignment)+',"Original C11 triangle/plane layout");\n',newline='\n');compile(bits,profile+'-types-c',path,base,True,extra=defines)
        for name in json.loads((out/'changes.json').read_text()):
            if name.endswith(('.cpp','.c')):
                name=m.RENAMES.get(name,name);compile(bits,profile+'-'+Path(name).stem,base/name,base,name.endswith('.c'),extra=defines)
        if profile=='vu':compile(bits,profile+'-VuCollision',base/'src/collision/VuCollision.cpp',base,extra=defines)
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
