"""Check damage/automobile ABI, shared enums and all affected real native producers."""
from pathlib import Path
import os,json,sys,subprocess,re
import damage_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/damage-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[];layouts=[]
fields=['m_fWheelDamageEffect','m_engineStatus','m_wheelStatus','m_doorStatus','m_lightStatus','m_panelStatus','field_18'];types=['float','uint8_t','uint8_t[4]','uint8_t[6]','uint32_t','uint32_t','uint8_t']
expressions=['sizeof(CDamageManager)','alignof(CDamageManager)']+['offsetof(CDamageManager,'+field+')' for field in fields]+['sizeof(CAutomobile)','alignof(CAutomobile)']+['offsetof(CAutomobile,'+field+')' for field in ['Damage','Doors','m_aCarNodes','m_aGroundOffset','m_fGasPedalAudio']]
def compile(bits,name,path,tree,isC=False,failure=False):
    # Keep deliberate layout discovery separate from required successful compilations
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n')
    assert (run.returncode!=0 and 'OriginalDamageLayout<' in run.stdout) if failure else run.returncode==0,(prefix,run.stdout[-5000:])
    if not failure:results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return run.stdout
def includes(tree):
    # Use complete native headers from one source tree without mixing pragma-once identities
    return '#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/vehicles/Automobile.h').as_posix()+'"\n#include <stddef.h>\n#include <stdint.h>\n#include <type_traits>\n'
for bits in [32,64]:
    discovery=probe/f'discovery{bits}.cpp';discovery.write_text(includes(saved)+'template<unsigned... Values> struct OriginalDamageLayout;OriginalDamageLayout<'+','.join(expressions)+'> observedLayout;\n',newline='\n')
    log=compile(bits,'discovery',discovery,saved,failure=True);matches=re.findall(r'OriginalDamageLayout<([^>]+)>',log);values=next(([int(v) for v in match.split(',')] for match in matches if re.fullmatch(r'[\d, ]+',match)),None);assert values and len(values)==len(expressions)
    layouts.append({'bits':bits,'expressions':expressions,'values':values})
    for phase,tree in [('before',saved),('after',base)]:
        text=includes(tree)+''.join('static_assert('+e+'=='+str(v)+',"Original native layout");\n' for e,v in zip(expressions,values))
        for field,kind in zip(fields,types):text+='static_assert(std::is_same<decltype(CDamageManager::'+field+'),'+kind+'>::value,"Original field type");\n'
        text+='static_assert('+('!' if phase=='before' else '')+'std::is_trivially_default_constructible<CDamageManager>::value,"Explicit C initialization");\nstatic_assert(std::is_trivially_copyable<CDamageManager>::value && std::is_trivially_destructible<CDamageManager>::value,"Original copy/destruction");\nstatic_assert(VEHPANEL_FRONT_LEFT==0 && VEHPANEL_WINDSCREEN==4 && VEHBUMPER_FRONT==5 && VEHBUMPER_REAR==6 && VEHLIGHT_FRONT_LEFT==0 && VEHLIGHT_REAR_RIGHT==3,"Original part identifiers");\n'
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'types-'+phase,path,tree)
    text='#include "'+(base/'src/vehicles/DamageManager.h').as_posix()+'"\n#include <stddef.h>\n'
    for e,v in zip(expressions[:9],values[:9]):text+='_Static_assert('+e.replace('alignof','_Alignof')+'=='+str(v)+',"Original record layout in C11");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    for name in json.loads((out/'changes.json').read_text()):
        if name.endswith(('.cpp','.c')):
            name=m.RENAMES.get(name,name);compile(bits,Path(name).stem,base/name,base,name.endswith('.c'))
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
