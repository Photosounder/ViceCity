"""Verify the real route/vector ABI, empty default lifetime and all affected native consumers."""
from pathlib import Path
import os,json,sys,subprocess
import ped_routes_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/ped-routes-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'native-probes';probe.mkdir(exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[]

def compile(bits,name,path,tree,isC=False):
    # Compile complete real sources with one coherent set of native headers
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags='/nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,run.stdout[-5000:]);results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)

for bits in [32,64]:
    for phase,tree in [('before',saved),('after',base)]:
        text='#include "'+(tree/'src/core/common.h').as_posix()+'"\n#include "'+(tree/'src/peds/PedRoutes.h').as_posix()+'"\n#include <stddef.h>\n#include <type_traits>\n#include <stdint.h>\n'
        for expression,value in [('sizeof(CRouteNode)',16),('alignof(CRouteNode)',4),('offsetof(CRouteNode,m_route)',0),('offsetof(CRouteNode,m_pos)',4),('sizeof(CRouteNode[NUMPEDROUTES])',3200),('sizeof(decltype(CRouteNode::m_pos))',12)]:text+='static_assert('+expression+'=='+str(value)+',"Original native route layout");\n'
        text+='static_assert(std::is_same<decltype(CRouteNode::m_route),int16_t>::value,"Original signed route identifier");\nstatic_assert(std::is_trivially_copyable<CRouteNode>::value&&std::is_trivially_destructible<CRouteNode>::value,"Original copy and destruction");\n'
        if phase=='before':text+='static_assert(!std::is_trivially_default_constructible<CRouteNode>::value&&std::is_trivially_default_constructible<RwV3d>::value,"Original empty CVector constructor has no base initialization effects");\n'
        else:text+='static_assert(std::is_trivially_default_constructible<CRouteNode>::value&&sizeof(gaRoutes)==3200,"C table needs no dynamic constructor");\n'
        path=probe/f'types{bits}-{phase}.cpp';path.write_text(text,newline='\n');compile(bits,'types-'+phase,path,tree)
    text='#include "'+(base/'src/peds/PedRoutes.h').as_posix()+'"\n#include <stddef.h>\n_Static_assert(sizeof(CRouteNode)==16&&_Alignof(CRouteNode)==4&&offsetof(CRouteNode,m_pos)==4&&sizeof(gaRoutes)==3200,"Original C11 layout");\n_Static_assert(sizeof(PedRouteVector)==12&&offsetof(PedRouteVector,y)==4&&offsetof(PedRouteVector,z)==8,"Plain vector layout");\n'
    path=probe/f'types{bits}-c.c';path.write_text(text,newline='\n');compile(bits,'types-c',path,base,True)
    for name in json.loads((out/'changes.json').read_text()):
        if name.endswith(('.cpp','.c')):
            name=m.RENAMES.get(name,name);compile(bits,Path(name).stem,base/name,base,name.endswith('.c'))
(out/('production-source-compiles.json' if '--production' in sys.argv else 'native-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
