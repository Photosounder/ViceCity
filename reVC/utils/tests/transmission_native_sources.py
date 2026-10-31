"""Verify transmission/handling/audio ABI and every affected producer with real native headers."""
from pathlib import Path
import os,json,sys,subprocess,re
import transmission_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/transmission-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[];layouts=[]
fields=['Gears','nDriveType','nEngineType','nNumberOfGears','Flags','fEngineAcceleration','fMaxVelocity','fMaxCruiseVelocity','fMaxReverseVelocity','fCurVelocity']
types=['tGear[6]','char','char','int8_t','uint8_t','float','float','float','float','float']
expressions=['sizeof(cTransmission)','alignof(cTransmission)']+['offsetof(cTransmission,'+field+')' for field in fields]+['sizeof(tGear)','alignof(tGear)']+['offsetof(tGear,'+field+')' for field in ['fMaxVelocity','fShiftUpVelocity','fShiftDownVelocity']]+['sizeof(tHandlingData)','alignof(tHandlingData)']+['offsetof(tHandlingData,'+field+')' for field in ['Transmission','fBrakeDeceleration','nMonetaryValue','RearLights']]+['sizeof(cVehicleParams)','alignof(cVehicleParams)','offsetof(cVehicleParams,m_pTransmission)']
def compile(bits,name,path,tree,isC=False,failure=False):
    # Separate intentional ABI discovery errors from required successful source compilations
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n')
    assert (run.returncode!=0 and 'OriginalTransmissionLayout<' in run.stdout) if failure else run.returncode==0,(prefix,run.stdout[-4500:])
    if not failure:results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return run.stdout
def includes(tree):
    # Use one coherent source tree so native sibling headers retain pragma-once identity
    return ''.join('#include "'+(tree/'src'/name).as_posix()+'"\n' for name in ['core/common.h','vehicles/HandlingMgr.h','audio/AudioState.h'])+'#include <stddef.h>\n#include <stdint.h>\n#include <type_traits>\n'
for bits in [32,64]:
    discovery=probe/f'discovery{bits}.cpp';discovery.write_text(includes(saved)+'template<unsigned... Values> struct OriginalTransmissionLayout;OriginalTransmissionLayout<'+','.join(expressions)+'> observedLayout;\n',newline='\n')
    log=compile(bits,'discovery',discovery,saved,failure=True);matches=re.findall(r'OriginalTransmissionLayout<([^>]+)>',log);values=next(([int(v) for v in match.split(',')] for match in matches if re.fullmatch(r'[\d, ]+',match)),None);assert values and len(values)==len(expressions)
    layouts.append({'bits':bits,'expressions':expressions,'values':values})
    for phase,tree in [('before',saved),('after',base)]:
        text=includes(tree)+''.join('static_assert('+e+'=='+str(v)+',"Original native layout");\n' for e,v in zip(expressions,values))
        for field,kind in zip(fields,types):text+='static_assert(std::is_same<decltype(cTransmission::'+field+'),'+kind+'>::value,"Original field type");\n'
        text+='static_assert(std::is_trivially_default_constructible<cTransmission>::value && std::is_trivially_copyable<cTransmission>::value && std::is_trivially_destructible<cTransmission>::value,"Original trivial lifetime");\nstatic_assert(std::is_same<decltype(cVehicleParams::m_pTransmission),cTransmission *>::value,"Original audio pointer type");\n'
        for name,value in re.findall(r'\b(HANDLING_\w+)\s*=\s*(0x[0-9a-fA-F]+|\d+)',(saved/'src/vehicles/HandlingMgr.h').read_text()):text+='static_assert('+name+'=='+value+',"Original handling flag");\n'
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'types-'+phase,path,tree)
    text='#include "'+(base/'src/vehicles/Transmission.h').as_posix()+'"\n#include <stddef.h>\n'
    for e,v in zip(expressions[:17],values[:17]):text+='_Static_assert('+e.replace('alignof','_Alignof')+'=='+str(v)+',"Original record layout in C11");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    for path in json.loads((out/'changes.json').read_text()):
        if path.endswith(('.c','.cpp')):
            path=m.RENAMES.get(path,path);compile(bits,Path(path).stem,base/path,base,path.endswith('.c'))
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
