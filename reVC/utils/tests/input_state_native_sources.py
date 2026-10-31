"""Check complete input-record layouts, embedded pad ABI and real native consumers on x86/x64."""
from pathlib import Path
import os,json,sys,subprocess,re
import input_state_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/input-state-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[];layouts=[]
header=(saved/'src/core/Pad.h').read_text();fields=[]
for record in m.RECORDS:
    text=re.sub(r'//[^\n]*','',m.fields(header,record))
    for kind,names in re.findall(r'\b(int16|bool|char|float)\s+([^;]+);',text):
        for name in names.split(','):
            match=re.fullmatch(r'\s*(\w+)(\[\d+\])?\s*',name);assert match,(record,name)
            fields.append((record,match[1],{'int16':'int16_t'}.get(kind,kind)+(match[2] or '')))
expressions=['sizeof(CPad)','alignof(CPad)']+['offsetof(CPad,'+field+')' for field in ['NewState','OldState','PCTempKeyState','PCTempJoyState','PCTempMouseState','Phase','LastTimeTouched']]+['offsetof('+record+','+name+')' for record,name,kind in fields]
def compile(bits,name,path,tree,isC=False,glfw=False,failure=False):
    # Keep deliberate native layout discovery distinct from required successful compilations
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags='/nologo /c /O2 /DLIBRW '+('/DRW_GL3 ' if glfw else '/DRW_D3D9 /DUSE_D3D9 ')+'/DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n')
    assert (run.returncode!=0 and 'OriginalPadLayout<' in run.stdout) if failure else run.returncode==0,(prefix,run.stdout[-4500:])
    if not failure:results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return run.stdout
def includes(tree):
    # Include one coherent real source tree to retain sibling header identity
    return '#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/core/Pad.h').as_posix()+'"\n#include <stddef.h>\n#include <stdint.h>\n#include <type_traits>\n'
for bits in [32,64]:
    discovery=probe/f'discovery{bits}.cpp';discovery.write_text(includes(saved)+'template<unsigned... Values> struct OriginalPadLayout;OriginalPadLayout<'+','.join(expressions)+'> observedLayout;\n',newline='\n')
    log=compile(bits,'discovery',discovery,saved,failure=True);matches=re.findall(r'OriginalPadLayout<([^>]+)>',log);values=next(([int(v) for v in match.split(',')] for match in matches if re.fullmatch(r'[\d, ]+',match)),None);assert values and len(values)==len(expressions)
    layouts.append({'bits':bits,'expressions':expressions,'values':values})
    for phase,tree in [('before',saved),('after',base)]:
        text=includes(tree)+''.join('static_assert('+e+'=='+str(v)+',"Original pad layout");\n' for e,v in zip(expressions,values))
        for record,size,alignment in [('CControllerState',42,2),('CMouseControllerState',16,4),('CMousePointerStateHelper',2,1),('CKeyboardState',624,2)]:
            text+='static_assert(sizeof('+record+')=='+str(size)+' && alignof('+record+')=='+str(alignment)+',"Original input record layout");\n'
            text+='static_assert(std::is_trivially_copyable<'+record+'>::value && std::is_trivially_destructible<'+record+'>::value,"Original trivial copy/destruction");\n'
            text+='static_assert('+('!' if phase=='before' and record=='CMouseControllerState' else '')+'std::is_trivially_default_constructible<'+record+'>::value,"Explicit C initialization");\n'
        for record,name,kind in fields:text+='static_assert(std::is_same<decltype('+record+'::'+name+'),'+kind+'>::value,"Original field type");\n'
        for name in ['OldMouseControllerState','NewMouseControllerState','PCTempMouseControllerState']:text+='static_assert(std::is_same<decltype(CPad::'+name+'),CMouseControllerState>::value,"Canonical mouse storage");\n'
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'types-'+phase,path,tree)
    text='#include "'+(base/'src/core/InputState.h').as_posix()+'"\n#include <stddef.h>\n'
    for record,size in [('CControllerState',42),('CMouseControllerState',16),('CMousePointerStateHelper',2),('CKeyboardState',624)]:text+='_Static_assert(sizeof('+record+')=='+str(size)+',"Native record extent in C11");\n'
    for field,offset in [('LMB',0),('MXB2',6),('_pad0',7),('x',8),('y',12)]:text+='_Static_assert(offsetof(CMouseControllerState,'+field+')=='+str(offset)+',"Original mouse offset");\n'
    for expression,value in zip(expressions[9:],values[9:]):text+='_Static_assert('+expression+'=='+str(value)+',"Original field offset in C11");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    if '--types-only' not in sys.argv:
        for name in ['src/core/Pad.cpp','src/core/Frontend.cpp','src/core/ControllerConfig.cpp','src/control/Record.cpp','src/skel/win/win.cpp','src/skel/glfw/glfw.cpp']:compile(bits,Path(name).stem,base/name,base)
        compile(bits,'InputState-d3d9',base/'src/core/InputState.c',base,True)
        compile(bits,'InputState-glfw',base/'src/core/InputState.c',base,True,True)
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-types.json' if '--types-only' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
