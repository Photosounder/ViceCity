"""Verify native stored-car/garage ABI, partial lifetime and both real save branches."""
from pathlib import Path
import os,json,sys,subprocess,re,struct
import stored_car_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-car-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[]

def compile(bits,name,path,tree,isC=False,extra=""):
    # Compile complete real sources with one coherent set of native headers
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags=extra+' /nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,run.stdout[-5000:]);results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return out/(prefix+'.obj')

def layout(path):
    # Read the compiler-emitted unsigned layout constants directly from the native COFF object
    data=path.read_bytes();machine,sections,stamp,symbols,count,optional,flags=struct.unpack_from('<HHIIIHH',data)
    strings=symbols+count*18
    for n in range(count):
        entry=symbols+n*18;raw=data[entry:entry+8]
        name=data[strings+struct.unpack_from('<I',raw,4)[0]:].split(b'\0',1)[0] if raw[:4]==b'\0'*4 else raw.rstrip(b'\0')
        if name in [b'StoredNativeLayout',b'_StoredNativeLayout']:
            value,section=struct.unpack_from('<Ih',data,entry+8);assert section>0
            rawStart=struct.unpack_from('<I',data,20+optional+(section-1)*40+20)[0]
            return list(struct.unpack_from('<7I',data,rawStart+value))
    raise AssertionError('Missing native layout data')

expressions=['sizeof(CStoredCar)','alignof(CStoredCar)','sizeof(CGarage)','alignof(CGarage)','offsetof(CGarage,m_sStoredCar)','offsetof(CGarage,m_pTarget)','sizeof(CGarages::aCarsInSafeHouses)'];layouts=[]
for bits in [32,64]:
    observed=[]
    text='#include "'+(saved/'src/core/common.h').as_posix()+'"\n#include "'+(saved/'src/control/Garages.h').as_posix()+'"\n#include <stddef.h>\n'
    for phase,tree in [('before',saved),('after',base)]:
        text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/control/Garages.h').as_posix()+'"\n#include <stddef.h>\n#include <type_traits>\n'
        text+='static_assert(sizeof(CStoredCar)==40&&alignof(CStoredCar)==4,"Original stored record layout");\nstatic_assert(sizeof(CGarages::aCarsInSafeHouses)==1920,"Original canonical array extent");\n'
        text+='static_assert('+('!' if phase=='before' else '')+'std::is_trivially_copyable<CStoredCar>::value,"Explicit field assignment");\nstatic_assert(std::is_trivially_copy_constructible<CStoredCar>::value&&std::is_trivially_destructible<CStoredCar>::value,"Original copy construction and destruction");\n'
        if phase=='before':text+='static_assert(!std::is_trivially_default_constructible<CStoredCar>::value&&std::is_trivially_default_constructible<RwV3d>::value,"Empty vector constructors");\n'
        else:
            text+='static_assert(std::is_trivially_default_constructible<CStoredCar>::value,"No C++ record constructor");\n'
            for field,offset in zip(m.FIELDS,[0,4,16,28,32,33,34,35,36,37]):text+='static_assert(offsetof(CStoredCar,'+field+')=='+str(offset)+',"Original field offset");\n'
        text+='extern "C" const unsigned StoredNativeLayout[]={'+','.join(expressions)+'};\n'
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');observed.append(layout(compile(bits,'types-'+phase,path,tree)))
    assert observed[0]==observed[1],(bits,observed)
    layouts.append({'bits':bits,'expressions':expressions,'values':observed[0]})
    text='#include "'+(base/'src/control/StoredCar.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(CStoredCar)==40&&_Alignof(CStoredCar)==4&&sizeof(StoredCarVector)==12,"Original C11 layout");\n'
    for field,offset in zip(m.FIELDS,[0,4,16,28,32,33,34,35,36,37]):text+='_Static_assert(offsetof(CStoredCar,'+field+')=='+str(offset)+',"Original field offset");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    for name in ['src/control/Garages.cpp','src/control/StoredCar.c','src/save/GenericGameStorage.cpp']:
        compile(bits,Path(name).stem,base/name,base,name.endswith('.c'))
    for phase,tree in [('before',saved),('after',base)]:
        text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#undef COMPATIBLE_SAVES\n#undef FIX_INCOMPATIBLE_SAVES\n#include "'+(tree/'src/control/Garages.cpp').as_posix()+'"\n'
        path=probe/f'noncompatible{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'Garages-noncompatible-'+phase,path,tree)
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
