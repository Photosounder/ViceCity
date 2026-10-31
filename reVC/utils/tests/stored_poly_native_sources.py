"""Verify cached-polygon layout, actual parent offsets and every affected native producer."""
from pathlib import Path
import os,json,sys,subprocess,re,struct
import stored_poly_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-poly-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes'/('production' if '--production' in sys.argv else 'native');probe.mkdir(parents=True,exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[]

def compile(bits,name,path,tree,isC=False,extra=""):
    # Compile complete real sources with one coherent set of native headers
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags=extra+' /nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,run.stdout[-5000:]);results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return out/(prefix+'.obj')

def parent_layout(path):
    # Read the actual compiler-emitted parent-layout constants from the native COFF object
    data=path.read_bytes();machine,sections,stamp,symbols,count,optional,flags=struct.unpack_from('<HHIIIHH',data);strings=symbols+count*18
    for n in range(count):
        entry=symbols+n*18;raw=data[entry:entry+8];name=data[strings+struct.unpack_from('<I',raw,4)[0]:].split(b'\0',1)[0] if raw[:4]==b'\0'*4 else raw.rstrip(b'\0')
        if name in [b'ContactParentLayout',b'_ContactParentLayout']:
            value,section=struct.unpack_from('<Ih',data,entry+8);assert section>0
            rawStart=struct.unpack_from('<I',data,20+optional+(section-1)*40+20)[0];return list(struct.unpack_from('<6I',data,rawStart+value))
    raise AssertionError('Missing contact parent layout constants')
layouts=[]
for bits in [32,64]:
    for profile,defines in [('float',''),('compressed','/DCOMPRESSED_COL_VECTORS'),('vu','/DVU_COLLISION')]:
        packed=profile=='vu';size=64 if packed else 40;alignment=16 if packed else 4;stride=16 if packed else 12;values=[]
        for phase,tree in [('before',saved),('after',base)]:
            text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/peds/Ped.h').as_posix()+'"\n#include "'+(tree/'src/vehicles/Vehicle.h').as_posix()+'"\n#include <stddef.h>\n#include <type_traits>\n'
            for expression,value in [('sizeof(CStoredCollPoly)',size),('alignof(CStoredCollPoly)',alignment),('sizeof(((CStoredCollPoly*)0)->verts[0])',stride),('offsetof(CStoredCollPoly,valid)',stride*3)]:text+='static_assert('+expression+'=='+str(value)+',"Original cached polygon layout");\n'
            text+='static_assert(std::is_trivially_copyable<CStoredCollPoly>::value&&std::is_trivially_destructible<CStoredCollPoly>::value,"Original record copying and destruction");\n'
            text+='static_assert('+('!' if phase=='before' else '')+'std::is_trivially_default_constructible<CStoredCollPoly>::value,"Empty original vector construction removed");\n'
            if phase=='before':text+='static_assert(std::is_trivially_default_constructible<RwV3d>::value,"Original vector base performs no initialization");\n'
            text+='extern "C" const unsigned ContactParentLayout[]={sizeof(CPed),sizeof(CVehicle),offsetof(CPed,m_collPoly),offsetof(CVehicle,m_aCollPolys),offsetof(CPed,m_fCollisionSpeed),offsetof(CVehicle,m_fSteerInput)};\n'
            path=probe/f'types{bits}-{profile}-{phase}.cpp';path.write_text(text,newline='\n');obj=compile(bits,profile+'-types-'+phase,path,tree,extra=defines);values.append(parent_layout(obj))
        assert values[0]==values[1],(bits,profile,values);layouts.append({'bits':bits,'profile':profile,'before':values[0],'after':values[1]})
        path=probe/f'types{bits}-{profile}-c.c';path.write_text('#include "'+(base/'src/collision/StoredCollPoly.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(CStoredCollPoly)=='+str(size)+'&&_Alignof(CStoredCollPoly)=='+str(alignment)+'&&offsetof(CStoredCollPoly,valid)=='+str(stride*3)+',"Original C11 cached polygon layout");\n',newline='\n');compile(bits,profile+'-types-c',path,base,True,extra=defines)
        for name in json.loads((out/'changes.json').read_text()):
            if name.endswith(('.cpp','.c')):compile(bits,profile+'-'+Path(name).stem,base/name,base,name.endswith('.c'),extra=defines)
        compile(bits,profile+'-StoredCollPoly',base/'src/collision/StoredCollPoly.c',base,True,extra=defines)
        if packed:compile(bits,profile+'-VuCollision',base/'src/collision/VuCollision.cpp',base,extra=defines)
(out/('production-parent-layouts.json' if '--production' in sys.argv else 'native-parent-layouts.json')).write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
