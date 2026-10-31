"""Check real door/vehicle ABI and compile every native caller plus isolated train call sites."""
from pathlib import Path
import os,json,sys,subprocess,re
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/doors-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[];layouts=[]
fields={'CDoor':['m_fMaxAngle','m_fMinAngle','m_nDirn','m_nAxis','m_nDoorState','m_fAngle','m_fPrevAngle','m_fAngVel','m_vecSpeed'],'CTrainDoor':['m_fClosedPosn','m_fOpenPosn','m_nDirn','m_nDoorState','m_nAxis','m_fPosn','m_fPrevPosn','field_14']}
expressions=[]
for record in fields:expressions+=['sizeof('+record+')','alignof('+record+')']+['offsetof('+record+','+name+')' for name in fields[record]]
doorCount=len(expressions)
for record,names in [('CAutomobile',['Damage','Doors','m_aCarNodes','m_aGroundOffset','m_fGasPedalAudio']),('CBoat',['m_boom','pBoatHandling','m_vecBuoyancePoint','m_avec2dWakePoints']),('CTrain',['Doors','m_aTrainNodes'])]:expressions+=['sizeof('+record+')','alignof('+record+')']+['offsetof('+record+','+name+')' for name in names]
def compile(bits,name,path,tree,isC=False,failure=False):
    # Keep deliberate layout discovery errors separate from successful native source checks
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n')
    assert (run.returncode!=0 and 'OriginalDoorLayout<' in run.stdout) if failure else run.returncode==0,(prefix,run.stdout[-5000:])
    if not failure:results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return run.stdout
def includes(tree):
    # Use a single coherent native source tree so sibling SDK headers keep their identity
    return ''.join('#include "'+(tree/'src'/name).as_posix()+'"\n' for name in ['core/common.h','vehicles/Automobile.h','vehicles/Boat.h','vehicles/Train.h'])+'#include <stddef.h>\n#include <stdint.h>\n#include <type_traits>\n'
for bits in [32,64]:
    discovery=probe/f'discovery{bits}.cpp';discovery.write_text(includes(saved)+'template<unsigned... Values> struct OriginalDoorLayout;OriginalDoorLayout<'+','.join(expressions)+'> observedLayout;\n',newline='\n')
    log=compile(bits,'discovery',discovery,saved,failure=True);matches=re.findall(r'OriginalDoorLayout<([^>]+)>',log);values=next(([int(v) for v in match.split(',')] for match in matches if re.fullmatch(r'[\d, ]+',match)),None);assert values and len(values)==len(expressions)
    layouts.append({'bits':bits,'expressions':expressions,'values':values})
    for phase,tree in [('before',saved),('after',base)]:
        text=includes(tree)+''.join('static_assert('+e+'=='+str(v)+',"Original native layout");\n' for e,v in zip(expressions,values))
        text+='static_assert(std::is_trivially_default_constructible<RwV3d>::value,"No observable SDK work crossed by explicit door initialization");\n'
        for record in fields:
            text+='static_assert('+('!' if phase=='before' else '')+'std::is_trivially_default_constructible<'+record+'>::value,"Explicit C initialization");\nstatic_assert(std::is_trivially_copyable<'+record+'>::value && std::is_trivially_destructible<'+record+'>::value,"Original trivial copy/destruction");\n'
            for name in fields[record]:
                kind=('CVector' if phase=='before' else 'DoorVector') if name=='m_vecSpeed' else 'int8_t' if name in ['m_nAxis','m_nDirn','m_nDoorState'] else 'int' if name=='field_14' else 'float'
                text+='static_assert(std::is_same<decltype('+record+'::'+name+'),'+kind+'>::value,"Original field type");\n'
        text+='static_assert(sizeof('+('CVector' if phase=='before' else 'DoorVector')+')==12,"Original cached velocity extent");\n'
        text+='bool trainConsumer(CVehicle *train){return '+('((CTrain*)train)->Doors[0].IsFullyOpen()' if phase=='before' else 'TrainDoor_IsFullyOpen(&((CTrain*)train)->Doors[0])')+';}\n'
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'types-'+phase,path,tree)
    text='#include "'+(base/'src/vehicles/Door.h').as_posix()+'"\n#include <stddef.h>\n'
    for e,v in zip(expressions[:doorCount],values[:doorCount]):text+='_Static_assert('+e.replace('alignof','_Alignof')+'=='+str(v)+',"Original door layout in C11");\n'
    text+='_Static_assert(sizeof(DoorVector)==12 && _Alignof(DoorVector)==4,"Native velocity record in C11");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    for name in json.loads((out/'changes.json').read_text()):
        if name.endswith('.cpp'):compile(bits,Path(name).stem,base/name,base)
    compile(bits,'DoorSystem',base/'src/vehicles/DoorSystem.c',base,True)
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
