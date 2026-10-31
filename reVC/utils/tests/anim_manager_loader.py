"""Compare original/C complete IFP loader bodies with real converted animation dependencies."""
from pathlib import Path
import ctypes,json,os,re,subprocess,sys
from anim_association_migration import original_body
import anim_manager_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';probe=out/'loader';probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
tree=out/'overlay';saved=out/'before';original=(saved/'src/animation/AnimManager.cpp').read_text();candidate=(tree/'src/animation/AnimManager.c').read_text()
methods=['Initialise','UncompressAnimation','RemoveFromUncompressedCache','GetAnimationBlockByName','GetAnimationBlockIndex','RegisterAnimBlock','LoadAnimFileByName','LoadAnimStream','RemoveLastAnimFile']
for name in ['loader.h','loader.cpp']:
    # Generate the independent stream fixtures without changing production or compiler input trees
    (probe/name).write_text((root/'utils/tests'/('anim_manager_'+name+'.in')).read_text(),newline='\n')
start=original.index('CAnimBlock CAnimManager::ms_aAnimBlocks[');end=original.index('void\nCAnimManager::Initialise(void)',start)
before='#include "loader.h"\n'+original[start:end]
for name in methods:
    # Retain entire exact original method bodies with the actual original C++ class declaration
    result,_,args=next(row for row in m.DECLARATIONS if row[1]==name);signature=m.SIGNATURES.get(name,name+'('+args.replace('int32_t','int32')+')')
    before+=result+' CAnimManager::'+signature+'\n{'+original_body(original,'CAnimManager::'+signature)+'}\n'
before+='extern "C" void AnimHierarchy_HostRemoveCache(CAnimBlendHierarchy *self)\n{\n    // Forward real hierarchy cleanup through the original manager cache\n    CAnimManager::RemoveFromUncompressedCache(self);\n}\n'
(probe/'original.cpp').write_text(before,newline='\n')
after=candidate[:candidate.index('void AnimManager_Initialise(void)')]
for name in methods+['GetAnimationBlockById','GetAnimationById']:
    # Select complete loader-related operations from the actual staged C producer
    result,_,args=next(row for row in m.DECLARATIONS if row[1]==name);signature='AnimManager_'+name+'('+args+')'
    after+=result+' '+signature+'\n{'+original_body(candidate,signature)+'}\n'
signature='AnimHierarchy_HostRemoveCache(CAnimBlendHierarchy *self)'
after+='void '+signature+'\n{'+original_body(candidate,signature)+'}\n'
(probe/'manager.c').write_text(after,newline='\n')
sanitized='--sanitized' in sys.argv
compilers=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)]
runs=[]
for compiler,opt in compilers:
    # Compare ordinary, compression and MASTER behavior with the same actual dependencies in both phases
    for profile,defines in [('default',[]),('compression',['ANIM_COMPRESSION']),('master',['MASTER']),('compression-master',['ANIM_COMPRESSION','MASTER'])]:
        label=f'{compiler}-O{opt}-{profile}'+('-ubsan' if sanitized else '');results=[]
        with (probe/(label+'.log')).open('w',newline='\n') as log:
            for phase in ('before','after'):
                # Preserve coherent original and staged manager declarations while sharing verified C dependencies
                current=saved if phase=='before' else tree
                dirs=[current/'src']+[p for p in (current/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(current/'src').parts)]
                sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else []
                flags=['-O'+str(opt),'-fno-fast-math','-ffp-contract=off',*sanitize,'-I'+str(probe),*['-I'+str(p) for p in dirs],*['-D'+value for value in defines]]+(['-DORIGINAL'] if phase=='before' else [])
                cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
                units=[(cxx,probe/'loader.cpp'),(cxx,probe/'original.cpp')] if phase=='before' else [(cxx,probe/'loader.cpp'),(cc,probe/'manager.c')]
                units += [(cc,current/name) for name in ['src/animation/AnimBlendHierarchy.c','src/animation/AnimBlendSequence.c','src/animation/KeyFrames.c','src/math/Quaternion.c','src/core/General.c']]
                objects=[]
                for tool,path in units:
                    # Compile real hierarchy, sequence, keyframe, quaternion and General implementations
                    obj=probe/f'{label}-{phase}-{path.stem}.o';objects.append(obj)
                    command=[str(tool),'-std=c11' if path.suffix=='.c' else '-std=c++14',*flags,*(['-pedantic-errors'] if path.suffix=='.c' else []),'-c',str(path),'-o',str(obj)]
                    run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(' '.join(command)+'\n'+run.stdout+run.stderr);log.flush();assert run.returncode==0,(phase,path,run.stderr[-5000:])
                exe=probe/f'{label}-{phase}.exe';command=[str(cxx),'-static','-static-libgcc','-static-libstdc++',*sanitize,*[str(p) for p in objects],'-o',str(exe)]
                run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(run.stdout+run.stderr);log.flush();assert run.returncode==0,run.stderr[-5000:]
                if compiler=='gcc':
                    # Prove the diagnostic can run without the GCC runtime DLLs reported missing earlier
                    imports=subprocess.run([str(cc.with_name('objdump.exe')),'-p',str(exe)],env=env,capture_output=True,text=True,check=True)
                    dependencies=re.findall(r'DLL Name: (\S+)',imports.stdout);assert dependencies and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in dependencies),dependencies
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows'
                run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30)
                assert run.returncode==0,(phase,run.returncode,run.stderr[-3000:]);results.append(run.stdout.strip())
        assert results[0]==results[1],(label,results)
        runs.append({'compiler':compiler,'opt':opt,'profile':profile,'before':results[0],'after':results[1]});print(label,results[0],flush=True)
(out/('loader-sanitized-runs.json' if sanitized else 'loader-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
