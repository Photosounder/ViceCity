"""Check original native layouts, embedded owners and every changed source on x86 and x64."""
from pathlib import Path
import os,json,sys,subprocess,re
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lists-c-tests';before=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
changes=json.loads((out/'changes.json').read_text());results=[];layouts=[]
expressions=[]
for record,fields in [('CPtrNode',['item','prev','next']),('CPtrList',['first']),('CEntryInfoNode',['list','listnode','sector','prev','next']),('CEntryInfoList',['first']),('CSector',['m_lists']),('CPhysical',['m_entryInfoList','m_movingListNode','m_pDamageEntity']),('CDummy',['m_entryInfoList'])]:
    expressions+=['sizeof('+record+')','alignof('+record+')']+['offsetof('+record+','+field+')' for field in fields]
def compile(bits,name,path,tree,isC=False,failure=False):
    # Keep intentional layout discovery separate from successful compile checks
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir()]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags='/nologo /c /O2 /DLIBRW /DRW_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n')
    assert (run.returncode!=0 and 'OriginalListLayout<' in run.stdout) if failure else run.returncode==0,(prefix,run.stdout[-4000:])
    if not failure:results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return run.stdout
def includes(tree):
    # Use a single coherent source tree to retain pragma-once identity across sibling headers
    return ''.join('#include "'+(tree/'src'/name).as_posix()+'"\n' for name in ['core/common.h','core/World.h','entities/Physical.h','entities/Dummy.h'])+'#include <stddef.h>\n#include <type_traits>\n'
for bits in [32,64]:
    discovery=probe/f'discovery{bits}.cpp';discovery.write_text(includes(before)+'template<unsigned... Values> struct OriginalListLayout;OriginalListLayout<'+','.join(expressions)+'> observedLayout;\n',newline='\n')
    log=compile(bits,'discovery',discovery,before,failure=True)
    matches=re.findall(r'OriginalListLayout<([^>]+)>',log);values=next(([int(v) for v in match.split(',')] for match in matches if re.fullmatch(r'[\d, ]+',match)),None)
    assert values and len(values)==len(expressions),(bits,matches)
    layouts.append({'bits':bits,'expressions':expressions,'values':values})
    for phase,tree in [('before',before),('after',base)]:
        text=includes(tree)+''.join('static_assert('+e+'=='+str(v)+',"Original layout");\n' for e,v in zip(expressions,values))
        if phase=='after':text+=''.join('static_assert(std::is_trivially_default_constructible<'+name+'>::value,"C record");\n' for name in ['CPtrNode','CPtrList','CEntryInfoNode','CEntryInfoList','CSector'])
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'types-'+phase,path,tree)
    text='#include "'+(base/'src/core/Lists.h').as_posix()+'"\n'
    for e,v in zip(expressions[:18],values[:18]):text+='_Static_assert('+e.replace('alignof','_Alignof')+'=='+str(v)+',"C list layout");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    for name in changes:
        if name.endswith('.cpp') and not name.endswith('Lists.cpp'):compile(bits,Path(name).stem,base/name,base)
    compile(bits,'Lists',base/'src/core/Lists.c',base,True)
(out/'native-layouts.json').write_text(json.dumps(layouts,indent=2),newline='\n')
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
