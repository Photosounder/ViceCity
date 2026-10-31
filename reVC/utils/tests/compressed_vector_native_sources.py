"""Verify complete C collision vertex ownership across storage and native compiler profiles."""
from pathlib import Path
import os,json,sys,subprocess,re,struct
import compressed_vector_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/compressed-vector-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes'/('production' if '--production' in sys.argv else 'native');probe.mkdir(parents=True,exist_ok=True)
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
        packed=profile!='float';size=6 if packed else 12;alignment=2 if packed else 4
        for phase,tree in [('before',saved),('after',base)]:
            text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/collision/ColModel.h').as_posix()+'"\n#include <stddef.h>\n#include <type_traits>\n#include <stdint.h>\n'
            for expression,value in [('sizeof(CompressedVector)',size),('alignof(CompressedVector)',alignment),('offsetof(CompressedVector,y)',alignment),('offsetof(CompressedVector,z)',alignment*2),('sizeof(CColTrianglePlane)',8 if profile=='vu' else 20),('sizeof(CColModel)',76 if bits==32 else 104),('offsetof(CColModel,vertices)',64 if bits==32 else 80)]:text+='static_assert('+expression+'=='+str(value)+',"Original collision vertex and parent layout");\n'
            text+='static_assert(std::is_trivial<CompressedVector>::value&&std::is_trivially_copyable<CompressedVector>::value,"Original trivial lifetime");\nstatic_assert(std::is_same<decltype(CompressedVector::x),'+('int16_t' if packed else 'float')+'>::value,"Original component type");\n'
            path=probe/f'types{bits}-{profile}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,profile+'-types-'+phase,path,tree,extra=defines)
        path=probe/f'types{bits}-{profile}-c.c';path.write_text('#include "'+(base/'src/collision/CompressedVector.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(CompressedVector)=='+str(size)+'&&_Alignof(CompressedVector)=='+str(alignment)+'&&sizeof(CollisionVector3)==12,"Original C11 vertex layout");\n',newline='\n');compile(bits,profile+'-types-c',path,base,True,extra=defines)
        for name in ['src/collision/Collision.cpp','src/collision/ColModel.cpp','src/core/FileLoader.cpp','src/core/main.cpp','src/renderer/Glass.cpp','src/vehicles/Vehicle.cpp','src/collision/CompressedVector.c']+(['src/collision/VuCollision.cpp'] if profile=='vu' else []):compile(bits,profile+'-'+Path(name).stem,base/name,base,name.endswith('.c'),extra=defines)
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
