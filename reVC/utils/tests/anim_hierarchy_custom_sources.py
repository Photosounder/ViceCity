"""Compile native compression and custom allocator paths while recording the unchanged Game limitation."""
from pathlib import Path
import os,json,sys,subprocess,re,struct
import anim_hierarchy_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-hierarchy-c-tests';saved=out/'before';base=root if '--production' in sys.argv else out/'overlay';probe=out/'custom-probes'/('production' if '--production' in sys.argv else 'native');probe.mkdir(parents=True,exist_ok=True)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];results=[]

def compile(bits,name,path,tree,isC=False,extra=""):
    # Compile complete real sources with one coherent set of native headers
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat';dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(part.startswith('.') for part in p.relative_to(tree/'src').parts)]
    prefix=('production' if '--production' in sys.argv else 'native')+str(bits)+'-'+name
    flags=extra+' /nologo /c /O2 /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c11 /TC ' if isC else '/EHsc /std:c++14 /Zc:sizedDealloc- ')+ ' '.join('/I"'+str(p)+'"' for p in dirs+deps)
    driver=out/(prefix+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(out/(prefix+'.obj'))+'" "'+str(path)+'"\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],cwd=root,env=dict(os.environ),capture_output=True,text=True);(out/(prefix+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(prefix,run.stdout[-5000:]);results.append({'bits':bits,'unit':name,'exit':0});print(bits,name,'passed',flush=True)
    return out/(prefix+'.obj')

for profile,bits,defines in [('compression32',32,'/DANIM_COMPRESSION'),('compression64',64,'/DANIM_COMPRESSION'),('heap32',32,'/DUSE_CUSTOM_ALLOCATOR /DANIM_COMPRESSION')]:
    for phase,tree in [('before',saved),('after',base)]:
        for name in ['src/animation/AnimBlendHierarchy.cpp','src/animation/AnimManager.cpp','src/core/Game.cpp']:
            path=name if phase=='before' else m.RENAMES.get(name,name)
            sourcePath=current_path(tree,path)
            if profile=='heap32' and name=='src/core/Game.cpp':
                source=sourcePath.read_text();unsupported=method(source,'CGame::DrasticTidyUpMemory')
                assert all(value in unsupported for value in ['LEVEL_INDUSTRIAL','LEVEL_COMMERCIAL','LEVEL_SUBURBAN','RemoveColModelsFromOtherLevels','LoadCollisionFromDatFile'])
                assert 'AnimHierarchy_' not in unsupported and '->MoveMemory(' not in unsupported
                assert unsupported==method((saved/name).read_text(),'CGame::DrasticTidyUpMemory')
                sourcePath=probe/('heap-Game-'+phase+'.cpp');sourcePath.write_text(source.replace(unsupported,''),newline='\n')
            compile(bits,profile+'-'+phase+'-'+Path(path).stem,sourcePath,tree,sourcePath.suffix=='.c',defines)
(out/('production-custom-source-compiles.json' if '--production' in sys.argv else 'custom-source-compiles.json')).write_text(json.dumps(results,indent=2),newline='\n')
