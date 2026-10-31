"""Check the real streaming record ABI, manager arrays and complete native callers on both architectures."""
from pathlib import Path
import os,json,sys,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/streaming-info-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[]
fields=['m_next','m_prev','m_loadState','m_flags','m_nextID','m_position','m_size']
types=['CStreamingInfo *','CStreamingInfo *','uint8_t','uint8_t','int16_t','uint32_t','uint32_t']
def compile(bits,name,path,tree,isC=False,master=False):
    # Keep compiler invocations tied to coherent complete before and after source trees
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir()];prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags='/nologo /c /O2 /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/DMASTER ' if master else '')+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,run.stdout[-4000:]);results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
for bits in [32,64]:
    pointer=bits//8;offsets=[0,pointer,2*pointer,2*pointer+1,2*pointer+2,2*pointer+4,2*pointer+8];size={32:20,64:32}[bits]
    for phase,tree in [('before',saved),('after',base)]:
        text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/core/Streaming.h').as_posix()+'"\n#include <stddef.h>\n#include <stdint.h>\n#include <type_traits>\n'
        text+='static_assert(sizeof(CStreamingInfo)=='+str(size)+' && alignof(CStreamingInfo)=='+str(pointer)+',"Native record layout");\n'
        for field,kind,offset in zip(fields,types,offsets):text+='static_assert(offsetof(CStreamingInfo,'+field+')=='+str(offset)+',"Native field offset");\nstatic_assert(std::is_same<decltype(CStreamingInfo::'+field+'),'+kind+'>::value,"Native field type");\n'
        text+='static_assert(std::is_trivially_default_constructible<CStreamingInfo>::value && std::is_trivially_copyable<CStreamingInfo>::value && std::is_trivially_destructible<CStreamingInfo>::value,"Original trivial lifetime");\n'
        for field in ['ms_aInfoForModel','ms_startLoadedList','ms_endLoadedList','ms_startRequestedList','ms_endRequestedList']:
            text+='static_assert(std::is_same<decltype(CStreaming::'+field+'),CStreamingInfo'+('[NUMSTREAMINFO]' if field=='ms_aInfoForModel' else '')+'>::value,"Canonical manager record storage");\n'
        text+='static_assert(STREAMFLAGS_DONT_REMOVE==1 && STREAMFLAGS_SCRIPTOWNED==2 && STREAMFLAGS_DEPENDENCY==4 && STREAMFLAGS_PRIORITY==8 && STREAMFLAGS_NOFADE==16 && STREAMFLAGS_20==32 && STREAMFLAGS_CANT_REMOVE==3 && STREAMFLAGS_KEEP_IN_MEMORY==7,"Original flags");\nstatic_assert(STREAMSTATE_NOTLOADED==0 && STREAMSTATE_LOADED==1 && STREAMSTATE_INQUEUE==2 && STREAMSTATE_READING==3 && STREAMSTATE_STARTED==4,"Original states");\n'
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'types-'+phase,path,tree)
    text='#include "'+(base/'src/core/StreamingInfo.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(CStreamingInfo)=='+str(size)+' && _Alignof(CStreamingInfo)=='+str(pointer)+',"Native layout in C11");\n'
    for field,offset in zip(fields,offsets):text+='_Static_assert(offsetof(CStreamingInfo,'+field+')=='+str(offset)+',"C field offset");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    compile(bits,'Streaming',base/'src/core/Streaming.cpp',base)
    compile(bits,'Streaming-master',base/'src/core/Streaming.cpp',base,master=True)
    compile(bits,'StreamingInfo',base/'src/core/StreamingInfo.c',base,True)
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
