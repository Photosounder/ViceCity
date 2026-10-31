"""Derive actual original native layouts and verify the complete staged embedded autopilot conversion."""
from pathlib import Path
import os,json,sys,shutil,subprocess,re
import autopilot_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/autopilot-c-tests';before=out/'before';overlay=out/'native-overlay';production='--production' in sys.argv
probeDir=out/'native-probes';probeDir.mkdir(exist_ok=True);shutil.copytree(root/'src',overlay/'src',dirs_exist_ok=True)
changes=json.loads((out/'changes.json').read_text());header=(before/'src/control/AutoPilot.h').read_text()
for path in changes:(overlay/path).write_text(m.calls((before/path).read_text()),newline='\n')
for name in ['AutoPilotSystem.c','AutoPilot_backend.h']:(overlay/'src/control'/name).write_text((out/'preview'/name).read_text(),newline='\n')
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
fields=m.fields(header);addressable=[name for kind,name,suffix in fields if ':' not in suffix]
anchors=['AutoPilot','m_currentColour1','pDriver','pPassengers','m_nRoadblockType','m_nSetPieceExtendedRangeTime','m_aCollPolys','m_vehType']
expressions=['sizeof(CAutoPilot)','alignof(CAutoPilot)']+['offsetof(CAutoPilot,'+name+')' for name in addressable]+['sizeof(CVehicle)','alignof(CVehicle)']+['offsetof(CVehicle,'+name+')' for name in anchors]
save=(before/'src/control/AutoPilot.cpp').read_text();saveBody=m.operation(save,'Save');loadBody=m.operation(save,'Load')
savedFields=re.findall(r'WriteSaveBuf\(&buf, &(m_\w+(?:\.[xyz])?), sizeof\(',saveBody);loadedFields=re.findall(r'ReadSaveBuf\(&(m_\w+(?:\.[xyz])?), &buf, sizeof\(',loadBody);assert savedFields==loadedFields
zeros=[int(value) for value in re.findall(r'ZeroSaveBuf\(&buf, (\d+)\)',saveBody)];skips=[int(value) for value in re.findall(r'SkipSaveBuf\(&buf, (\d+)\)',loadBody)];assert zeros==skips
packed='+'.join('sizeof(((CAutoPilot*)0)->'+name+')' for name in savedFields)+'+sizeof(uint8_t)+'+str(sum(zeros))
results=[];layouts=[]

def compiler(bits,name,source,base,options,expectFailure=False):
    # Keep native compile diagnostics exact while separating intentional layout discovery from passing checks
    dirs=[base/'src']+[p for p in (base/'src').rglob('*') if p.is_dir()]
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    prefix=f'{"production" if production else "native"}{bits}-{name}';obj=out/(prefix+'.obj');driver=out/(prefix+'.cmd')
    args=options+' '+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+args+' /Fo"'+str(obj)+'" "'+str(source)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);log=out/(prefix+'.log');log.write_text(run.stdout+run.stderr,newline='\n')
    if expectFailure:assert run.returncode!=0 and 'NativeAutopilotLayout<' in run.stdout,(prefix,run.stdout[-4000:])
    else:assert run.returncode==0,(prefix,str(log),run.stdout[-4000:]);results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return run.stdout

for bits in [32,64]:
    options='/nologo /c /O2 /EHsc /std:c++14 /Zc:sizedDealloc- /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX'
    originalIncludes='#include "'+(before/'src/core/common.h').as_posix()+'"\n#include "'+(before/'src/vehicles/Vehicle.h').as_posix()+'"\n#include <type_traits>\n#include <stddef.h>\n'
    discovery=probeDir/f'layout-discovery{bits}.cpp';discovery.write_text(originalIncludes+'template<unsigned... Values> struct NativeAutopilotLayout;NativeAutopilotLayout<'+','.join(expressions)+'> observedLayout;\n',newline='\n')
    log=compiler(bits,'layout-discovery',discovery,before,options,True)
    matches=re.findall(r'NativeAutopilotLayout<([^>]+)>',log);values=None
    for match in matches:
        if re.fullmatch(r'[\d, ]+',match):values=[int(value) for value in match.split(',')];break
    assert values and len(values)==len(expressions),(bits,matches)
    layouts.append({'bits':bits,'expressions':expressions,'values':values,'packed_save':120})
    for phase,base in [('before',before),('after',root if production else overlay)]:
        text=originalIncludes if phase=='before' else '#include "'+(base/'src/core/common.h').as_posix()+'"\n#include "'+(base/'src/vehicles/Vehicle.h').as_posix()+'"\n#include <type_traits>\n#include <stddef.h>\n'
        for expression,value in zip(expressions,values):text+='static_assert('+expression+'=='+str(value)+',"Original '+expression+'");\n'
        for kind,name,suffix in fields:
            expected=m.scalar(kind).replace('CVector','CVector' if phase=='before' else 'AutoPilotVector')+(suffix if ':' not in suffix else '')
            text+='static_assert(std::is_same<decltype(CAutoPilot::'+name+'),'+expected+'>::value,"Original '+name+' type");\n'
        text+='static_assert(std::is_trivially_copyable<CAutoPilot>::value && std::is_trivially_destructible<CAutoPilot>::value,"Original trivial copy and destruction");\n'
        text+='static_assert('+('!' if phase=='before' else '')+'std::is_trivially_default_constructible<CAutoPilot>::value,"Explicit C initialization replaces the native constructor");\n'
        text+='static_assert('+packed+'==120,"Original compatible save payload");\n'
        path=probeDir/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compiler(bits,'types-'+phase,path,base,options)
    base=root if production else overlay
    csource='#include "'+(base/'src/control/AutoPilot.h').as_posix()+'"\n#include <stddef.h>\n'
    for expression,value in zip(expressions[:2+len(addressable)],values):csource+='_Static_assert('+expression.replace('alignof','_Alignof')+'=='+str(value)+',"Original record layout in C11");\n'
    csource+='_Static_assert('+packed+'==120,"Original packed save payload in C11");\n'
    cpath=probeDir/f'types{bits}-c.c';cpath.write_text(csource,newline='\n');compiler(bits,'types-c',cpath,base,options.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))
    if '--types-only' not in sys.argv:
        for path in changes:
            if path.endswith('.cpp'):compiler(bits,Path(path).stem,base/path,base,options)
        compiler(bits,'AutoPilotSystem',base/'src/control/AutoPilotSystem.c',base,options.replace('/std:c++14','/std:c11 /TC').replace('/EHsc',''))
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
(out/'save-fields.json').write_text(json.dumps({'save':savedFields,'load':loadedFields,'zero':zeros,'skip':skips,'packed_bytes':120},indent=2),newline='\n')
(out/('production-source-compiles.json' if production else 'native-types.json' if '--types-only' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
