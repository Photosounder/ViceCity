"""Compare original manager reference and cache operations with their complete C counterparts."""
from pathlib import Path
import ctypes,json,os,re,subprocess
from anim_association_migration import original_body
import anim_manager_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';probe=out/'policy';probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
tree=out/'overlay';saved=out/'before';original=(saved/'src/animation/AnimManager.cpp').read_text()
methods=['Initialise','UncompressAnimation','RemoveFromUncompressedCache','GetAnimationBlockByName','GetAnimationBlockIndex','RegisterAnimBlock','GetNumRefsToAnimBlock','AddAnimBlockRef','RemoveAnimBlockRefWithoutDelete','RemoveAnimBlockRef','GetAnimationByName']
shim='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
typedef int32_t int32;
struct RpClump;struct RwStream;
#define nil NULL
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#include "AnimBlendAssociation_flags.h"
#include "ModelIndexData.h"
#include "AnimManager.h"
extern "C" void re3_assert(const char*,const char*,unsigned,const char*);
#undef assert
#define assert(expr) ((void)((!!(expr)) || (re3_assert(#expr,__FILE__,__LINE__,__func__),0)))
#ifdef ORIGINAL
enum {STREAM_OFFSET_ANIM=1000};
class CStreaming {public:static void RemoveModel(int32);static void RemoveAnim(int32 id){RemoveModel(id+STREAM_OFFSET_ANIM);}};
#define MANAGER(name) CAnimManager::name
#define BLOCK_NAME(name) CAnimManager::GetAnimationBlock(name)
#define BLOCK_ID(id) CAnimManager::GetAnimationBlock(id)
#define ANIMATION_NAME(name,block) CAnimManager::GetAnimation(name,block)
#define ANIMATION_ID(id) CAnimManager::GetAnimation(id)
#else
#define MANAGER(name) AnimManager_##name
#define BLOCK_NAME(name) AnimManager_GetAnimationBlockByName(name)
#define BLOCK_ID(id) AnimManager_GetAnimationBlockById(id)
#define ANIMATION_NAME(name,block) AnimManager_GetAnimationByName(name,block)
#define ANIMATION_ID(id) AnimManager_GetAnimationById(id)
#endif
'''
(probe/'policy.h').write_text(shim,newline='\n')
(probe/'fixture.cpp').write_text((root/'utils/tests/anim_manager_policy.cpp.in').read_text(),newline='\n')
start=original.index('CAnimBlock CAnimManager::ms_aAnimBlocks[');end=original.index('void\nCAnimManager::Initialise(void)',start)
before='#include "policy.h"\n'+original[start:end]
for name in methods:
    # Compile exact original definitions with the real original class and C record declarations
    result,_,args=next(row for row in m.DECLARATIONS if row[1]==name)
    signature=m.SIGNATURES.get(name,name+'('+args.replace('int32_t','int32')+')')
    before+=result+' CAnimManager::'+signature+'\n{'+original_body(original,'CAnimManager::'+signature)+'}\n'
(probe/'original.cpp').write_text(before,newline='\n')
candidate=(tree/'src/animation/AnimManager.c').read_text()
after=candidate[:candidate.index('void AnimManager_Initialise(void)')]
for name in methods+['GetAnimationBlockById','GetAnimationById']:
    # Select tested complete operations from the actual staged producer without altering their bodies
    result,_,args=next(row for row in m.DECLARATIONS if row[1]==name)
    signature='AnimManager_'+name+'('+args+')'
    after+=result+' '+signature+'\n{'+original_body(candidate,signature)+'}\n'
(probe/'policy.c').write_text(after,newline='\n')
runs=[]
for compiler in ('clang','gcc'):
    # Run both optimization levels to expose aliasing or lifetime-sensitive policy differences
    for opt in (0,2):
        results=[]
        with (probe/f'{compiler}-O{opt}.log').open('w',newline='\n') as log:
            for phase in ('before','after'):
                # Build each implementation against its own coherent actual header tree
                current=saved if phase=='before' else tree
                dirs=[current/'src']+[p for p in (current/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(current/'src').parts)]
                flags=['-O'+str(opt),'-ffunction-sections','-fdata-sections','-I'+str(probe),*['-I'+str(p) for p in dirs]]+(['-DORIGINAL'] if phase=='before' else [])
                cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
                units=[(cxx,probe/'fixture.cpp'),(cxx,probe/'original.cpp')] if phase=='before' else [(cxx,probe/'fixture.cpp'),(cc,probe/'policy.c')]
                objects=[]
                for tool,path in units:
                    # Compile all policy code as its intended language with isolated object outputs
                    obj=probe/f'{compiler}-{opt}-{phase}-{path.stem}.o';objects.append(obj)
                    command=[str(tool),'-std=c11' if path.suffix=='.c' else '-std=c++14',*flags,'-c',str(path),'-o',str(obj)]
                    run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(' '.join(command)+'\n'+run.stdout+run.stderr);log.flush()
                    assert run.returncode==0,(phase,path,run.stderr[-4000:])
                exe=probe/f'{compiler}-{opt}-{phase}.exe';command=[str(cxx),'-static','-static-libgcc','-static-libstdc++','-Wl,--gc-sections',*[str(p) for p in objects],'-o',str(exe)]
                run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(run.stdout+run.stderr);log.flush();assert run.returncode==0,run.stderr[-5000:]
                if compiler=='gcc':
                    # Prove that quiet checks cannot depend on the previously missing GCC runtime DLLs
                    imports=subprocess.run([str(cc.with_name('objdump.exe')),'-p',str(exe)],env=env,capture_output=True,text=True,check=True)
                    dependencies=re.findall(r'DLL Name: (\S+)',imports.stdout)
                    assert dependencies and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in dependencies),dependencies
                # Run quietly without any MSYS runtime DLL directory on the executable search path
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows'
                run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(run.returncode,run.stderr)
                results.append(run.stdout.strip())
        assert results[0]==results[1],(compiler,opt,results)
        runs.append({'compiler':compiler,'opt':opt,'before':results[0],'after':results[1]});print(compiler,opt,results[0],flush=True)
(out/'policy-runs.json').write_text(json.dumps(runs,indent=2),newline='\n')
