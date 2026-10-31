"""Verify C frame and parent layouts and every complete native frame consumer."""
from pathlib import Path
import os,json,sys,subprocess,re,struct
import anim_frame_migration as m
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-frame-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes'/('production' if '--production' in sys.argv else 'native');probe.mkdir(parents=True,exist_ok=True)
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
    # Read native compiler-emitted parent sizes and offsets directly from COFF data
    data=path.read_bytes();machine,sections,stamp,symbols,count,optional,flags=struct.unpack_from('<HHIIIHH',data);strings=symbols+count*18
    for n in range(count):
        entry=symbols+n*18;raw=data[entry:entry+8];name=data[strings+struct.unpack_from('<I',raw,4)[0]:].split(b'\0',1)[0] if raw[:4]==b'\0'*4 else raw.rstrip(b'\0')
        if name in [b'AnimFrameParents',b'_AnimFrameParents']:
            value,section=struct.unpack_from('<Ih',data,entry+8);assert section>0
            rawStart=struct.unpack_from('<I',data,20+optional+(section-1)*40+20)[0];return list(struct.unpack_from('<8I',data,rawStart+value))
    raise AssertionError('Missing blend-node parent layout constants')
layouts=[]
for bits in [32,64]:
    for profile,defines in [('float',''),('compressed','/DCOMPRESSED_COL_VECTORS'),('vu','/DVU_COLLISION')]:
        values=[]
        for phase,tree in [('before',saved),('after',base)]:
            text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/animation/AnimBlendClumpData.h').as_posix()+'"\n#include "'+(tree/'src/animation/RpAnimBlend.h').as_posix()+'"\n#include "'+(tree/'src/animation/AnimBlendAssociation.h').as_posix()+'"\n#include "'+(tree/'src/peds/Ped.h').as_posix()+'"\n#include <stddef.h>\n#include <type_traits>\n'
            for expression,value in [('sizeof(AnimBlendFrameData)',24 if bits==32 else 32),('alignof(AnimBlendFrameData)',4 if bits==32 else 8),('offsetof(AnimBlendFrameData,flag)',0),('offsetof(AnimBlendFrameData,resetPos)',4),('offsetof(AnimBlendFrameData,frame)',16),('offsetof(AnimBlendFrameData,hanimFrame)',16),('offsetof(AnimBlendFrameData,nodeID)',20 if bits==32 else 24)]:text+='static_assert('+expression+'=='+str(value)+',"Original animation frame layout");\n'
            text+='static_assert(std::is_trivially_copyable<AnimBlendFrameData>::value&&std::is_trivially_default_constructible<AnimBlendFrameData>::value&&std::is_trivially_destructible<AnimBlendFrameData>::value,"Original frame value lifetime");\n'
            text+='extern "C" const unsigned AnimFrameParents[]={sizeof(CAnimBlendClumpData),offsetof(CAnimBlendClumpData,numFrames),offsetof(CAnimBlendClumpData,frames),sizeof(CPed),offsetof(CPed,m_pFrames),sizeof(AnimBlendFrameUpdateData),sizeof(CAnimBlendAssociation),offsetof(CAnimBlendAssociation,link)};\n'
            path=probe/f'types{bits}-{profile}-{phase}.cpp';path.write_text(text,newline='\n');obj=compile(bits,profile+'-types-'+phase,path,tree,extra=defines);values.append(parent_layout(obj))
        assert values[0]==values[1],(bits,profile,values);layouts.append({'bits':bits,'profile':profile,'before':values[0],'after':values[1]})
        path=probe/f'types{bits}-{profile}-c.c';path.write_text('#include "'+(base/'src/animation/AnimBlendFrameData.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(AnimBlendFrameData)=='+str(24 if bits==32 else 32)+'&&offsetof(AnimBlendFrameData,frame)==16&&offsetof(AnimBlendFrameData,nodeID)=='+str(20 if bits==32 else 24)+',"Original C11 frame layout");\n',newline='\n');compile(bits,profile+'-types-c',path,base,True,extra=defines)
        for name in json.loads((out/'changes.json').read_text()):
            if name.endswith(('.cpp','.c')):
                path=current_path(base,m.RENAMES.get(name,name));compile(bits,profile+'-'+path.stem,path,base,path.suffix=='.c',extra=defines)
(out/('production-parent-layouts.json' if '--production' in sys.argv else 'native-parent-layouts.json')).write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
