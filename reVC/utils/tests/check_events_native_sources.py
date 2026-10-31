"""Compile actual event/accident owners and all changed consumers before production mutation."""
from pathlib import Path
import subprocess,os,json,shutil,sys
import events_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/events-c-tests';overlay=out/'native-overlay';production='--production' in sys.argv
changes=json.loads((out/'changes.json').read_text());before=out/'before'
shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
for path in changes:
    target=overlay/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(m.calls((before/path).read_text()),newline='\n')
for name in ['EventsShared.h','Events_backend.h','GameplayEvents.c']:(overlay/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
includes=out/'native-includes';includes.mkdir(exist_ok=True)
shutil.copytree('C:/msys/home/lib/SDL2/include',includes/'SDL2',dirs_exist_ok=True)
dirs=[root/'src']+[p for p in (root/'src').rglob('*') if p.is_dir()]
nativeIncludes=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),includes]
beforeProbe='#include "'+(root/'src/core/common.h').as_posix()+'"\n#include <type_traits>\n#include <stddef.h>\n'+(before/'src/core/EventList.h').read_text().replace('#pragma once','')+'\n'+(before/'src/core/Accident.h').read_text().replace('#pragma once','').replace('class CAccidentManager\n{','class CAccidentManager\n{\npublic:')+'\n'
afterProbe='#include "'+(overlay/'src/core/common.h').as_posix()+'"\n#include "'+(overlay/'src/core/EventList.h').as_posix()+'"\n#include "'+(overlay/'src/core/Accident.h').as_posix()+'"\n#include <type_traits>\n#include <stddef.h>\n'
shared='''static_assert(sizeof(CEvent)==(sizeof(void*)==4?36:48),"Complete native event size");
static_assert(alignof(CEvent)==alignof(void*),"Native event alignment");
static_assert(offsetof(CEvent,type)==0&&offsetof(CEvent,entityType)==4&&offsetof(CEvent,entityRef)==8,"Native event enum/ref offsets");
static_assert(offsetof(CEvent,criminal)==(sizeof(void*)==4?12:16),"Native criminal-pointer offset");
static_assert(offsetof(CEvent,posn)==(sizeof(void*)==4?16:24)&&offsetof(CEvent,timeout)==(sizeof(void*)==4?28:36)&&offsetof(CEvent,state)==(sizeof(void*)==4?32:40),"Every remaining native event field offset");
static_assert(sizeof(CAccident)==(sizeof(void*)==4?12:16)&&alignof(CAccident)==alignof(void*),"Complete native accident representation");
static_assert(offsetof(CAccident,m_pVictim)==0&&offsetof(CAccident,m_nMedicsAttending)==sizeof(void*)&&offsetof(CAccident,m_nMedicsPerformingCPR)==sizeof(void*)+4,"Every native accident field offset");
static_assert(sizeof(CAccidentManager)==NUM_ACCIDENTS*sizeof(CAccident)&&alignof(CAccidentManager)==alignof(CAccident),"Complete native manager representation");
static_assert(sizeof(CVector)==12&&alignof(CVector)==4&&offsetof(CVector,x)==0&&offsetof(CVector,y)==4&&offsetof(CVector,z)==8,"Actual native vector representation");
'''
(out/'actual-layout-before.cpp').write_text(beforeProbe+shared+'static_assert(!std::is_trivially_default_constructible<CAccidentManager>::value&&std::is_trivially_destructible<CAccidentManager>::value,"Original array constructor and trivial destructor");\n',newline='\n')
(out/'actual-layout-after.cpp').write_text(afterProbe+shared+'static_assert(sizeof(EventsVector)==12&&alignof(EventsVector)==4&&std::is_trivial<CAccidentManager>::value,"Plain C records and explicit lifecycle");\n',newline='\n')
results=[]
for bits in [32,64]:
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    base='/nologo /c /EHsc /std:c++14 /Zc:sizedDealloc- /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS '
    flags=base+' '.join('/I"'+str(p)+'"' for p in dirs+nativeIncludes)
    previewFlags=base+' '.join('/I"'+str(p)+'"' for p in [overlay/p.relative_to(root) for p in dirs]+nativeIncludes)
    units=[('layout-before',out/'actual-layout-before.cpp',flags),('layout-after',out/'actual-layout-after.cpp',previewFlags)]
    units += [(Path(path).stem,overlay/path,previewFlags) for path in changes if path.endswith('.cpp')]
    units += [('GameplayEvents',overlay/'src/core/GameplayEvents.c',previewFlags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))]
    if production:units=units[:2]+[(name,root/source.relative_to(overlay),options.replace(previewFlags,flags)) for name,source,options in units[2:]]
    if production:units[-1]=(units[-1][0],units[-1][1],flags.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))
    if '--layouts-only' in sys.argv:units=units[:2]
    for i,(name,source,options) in enumerate(units):
        prefix=f'{"production" if production else "actual"}-{bits}-{name}';obj=out/(prefix+'.obj');driver=out/(prefix+'.cmd')
        driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+options+' /Fo"'+str(obj)+'" "'+str(source)+'"\nexit /b %errorlevel%\n',newline='\n')
        result=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(result.stdout+result.stderr,newline='\n')
        assert result.returncode==0,(prefix,result.stdout,result.stderr)
        results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
(out/('production-source-compiles.json' if production else 'native-layouts.json' if '--layouts-only' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
