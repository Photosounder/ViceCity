"""Compare complete original/C manager producers with all real converted animation owners."""
from pathlib import Path
import ctypes,json,os,re,subprocess,sys
import anim_manager_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';probe=out/'full';probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
saved=out/'before';tree=out/'overlay';original=(saved/'src/animation/AnimManager.cpp').read_text()
for name in ['loader.h','full.h']:
    # Keep SDK declarations in the independent fixture while using actual manager and owner headers
    (probe/name).write_text((root/'utils/tests'/('anim_manager_'+name+'.in')).read_text(),newline='\n')
arrays=re.findall(r'char const\* (a\w+Animations)\[\] = \{([\s\S]*?)\};',original)
names=sorted(set(value for _,body in arrays for value in re.findall(r'"([^"]+)"',body)))
assert len(names)<450 and all(len(name)<24 for name in names)
base=(root/'utils/tests/anim_manager_loader.cpp.in').read_text().split('int main(void)\n')[0].replace('#include "loader.h"','#include "full.h"')
base=base.replace('#include "full.h"','#include "full.h"\nstatic bool expectedAssertion;\nstatic unsigned assertionCalls;')
handler='fprintf(stderr,"assertion %s line %u\\n",expression,line);exit(3);'
assert handler in base
base=base.replace(handler,'if(expectedAssertion){++assertionCalls;return;}'+handler)
start=base.index('#ifndef ORIGINAL\nextern "C" int AnimManager_HostCompareNames');end=base.index('#endif',start)+len('#endif')
base=base[:start]+base[end:]
base=base.replace('require(strcmp(filename,"fixture.ifp")==0);','require(strcmp(filename,"fixture.ifp")==0||strcmp(filename,"ANIM\\\\PED.IFP")==0);')
declarations='static const char *managerNames[]={'+','.join(json.dumps(name) for name in names)+'};\n'
(probe/'fixture.cpp').write_text(base+declarations+(root/'utils/tests/anim_manager_full.cpp.in').read_text(),newline='\n')
start=original.index('CAnimBlock CAnimManager::ms_aAnimBlocks[')
(probe/'original.cpp').write_text('#include "full.h"\n'+original[start:],newline='\n')
native=m.native(original);native=re.sub(r'^#include[^\n]*\n','',native,flags=re.M)
(probe/'native.cpp').write_text('#include "full.h"\n'+native,newline='\n')
sanitized='--sanitized' in sys.argv;matrix='--matrix' in sys.argv
compilers=[('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)] if matrix else [('clang',0)]
profiles=[('default',[]),('compression',['ANIM_COMPRESSION']),('master',['MASTER']),('compression-master',['ANIM_COMPRESSION','MASTER'])] if matrix or sanitized else [('default',[])]
runs=[]
for compiler,opt in compilers:
    # Exercise complete producers with genuine sequence, hierarchy, association, group, list and node implementations
    for profile,defines in profiles:
        label=f'{compiler}-O{opt}-{profile}'+('-ubsan' if sanitized else '');results=[]
        with (probe/(label+'.log')).open('w',newline='\n') as log:
            for phase in ('before','after'):
                # Compile the entire manager producer rather than extracting the methods selected by a focused test
                current=saved if phase=='before' else tree
                dirs=[current/'src']+[p for p in (current/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(current/'src').parts)]
                sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else []
                flags=['-O'+str(opt),'-fno-fast-math','-ffp-contract=off',*sanitize,'-I'+str(probe),*['-I'+str(p) for p in dirs],*['-D'+value for value in defines]]+(['-DORIGINAL'] if phase=='before' else [])
                cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
                units=[(cxx,probe/'fixture.cpp'),(cxx,probe/'original.cpp')] if phase=='before' else [(cxx,probe/'fixture.cpp'),(cc,current/'src/animation/AnimManager.c'),(cxx,probe/'native.cpp')]
                units += [(cc,current/name) for name in ['src/animation/AnimBlendHierarchy.c','src/animation/AnimBlendSequence.c','src/animation/KeyFrames.c','src/math/Quaternion.c','src/core/General.c','src/animation/AnimBlendAssocGroup.c','src/animation/AnimBlendAssociation.c','src/animation/AnimBlendNode.c','src/animation/AnimBlendList.c','src/animation/AnimBlendClumpData.c']]
                objects=[]
                for tool,path in units:
                    # Keep each actual C producer in C11 and native fixture code in C++14
                    obj=probe/f'{label}-{phase}-{path.stem}.o';objects.append(obj)
                    command=[str(tool),'-std=c11' if path.suffix=='.c' else '-std=c++14',*flags,*(['-pedantic-errors'] if path.suffix=='.c' else []),'-c',str(path),'-o',str(obj)]
                    run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(' '.join(command)+'\n'+run.stdout+run.stderr);log.flush();assert run.returncode==0,(phase,path,run.stderr[-5000:])
                exe=probe/f'{label}-{phase}.exe';command=[str(cxx),'-static','-static-libgcc','-static-libstdc++',*sanitize,*[str(p) for p in objects],'-o',str(exe)]
                run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(run.stdout+run.stderr);log.flush();assert run.returncode==0,run.stderr[-5000:]
                if compiler=='gcc':
                    # Verify complete manager diagnostics do not depend on missing MSYS GCC runtime DLLs
                    imports=subprocess.run([str(cc.with_name('objdump.exe')),'-p',str(exe)],env=env,capture_output=True,text=True,check=True)
                    dependencies=re.findall(r'DLL Name: (\S+)',imports.stdout);assert dependencies and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in dependencies),dependencies
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows'
                run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(phase,run.returncode,run.stderr[-3000:]);results.append(run.stdout.strip())
        assert results[0]==results[1],(label,results)
        runs.append({'compiler':compiler,'opt':opt,'profile':profile,'before':results[0],'after':results[1]});print(label,results[0],flush=True)
(out/('full-sanitized-runs.json' if sanitized else 'full-matrix-runs.json' if matrix else 'full-smoke-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
