"""Compare entire original/C bone mapping producers through an independent C11 caller."""
from pathlib import Path
import ctypes,json,os,re,subprocess,sys
import bones_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/bones-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
saved=out/'before';tree=out/'overlay'
(probe/'PedModelInfo.h').write_text('#pragma once\n'+m.ped_enum((saved/'src/modelinfo/PedModelInfo.h').read_text())+'\n',newline='\n')
(probe/'common.h').write_text('''#pragma once
#include <stddef.h>
#define nil NULL
extern "C" void re3_assert(const char*,const char*,unsigned,const char*);
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
''',newline='\n')
(probe/'entry.h').write_text('''#pragma once
#ifdef __cplusplus
extern "C" {
#endif
int Run_Node(int);
const char *Run_Name(int);
int Run_Constant(unsigned);
int Before_Node(int);
const char *Before_Name(int);
#ifdef __cplusplus
}
#endif
''',newline='\n')
(probe/'wrapper.cpp').write_text('''#include "Bones.h"
#include "entry.h"
extern "C" int Before_Node(int node)
{
    // Invoke the original whole C++ function through the independent C caller
    return ConvertPedNode2BoneTag(node);
}
extern "C" const char *Before_Name(int tag)
{
    // Borrow the original function's exact literal or null result
    return ConvertBoneTag2BoneName(tag);
}
''',newline='\n')
ped=m.ped_enum((saved/'src/modelinfo/PedModelInfo.h').read_text());bones=(saved/'src/animation/Bones.h').read_text()
pedNames=re.findall(r'^\s*(PED_\w+)',ped,re.M);boneNames=re.findall(r'^\s*(BONE_\w+)',bones,re.M);assert len(pedNames)==19 and len(boneNames)==22
caller='''#include "entry.h"
#include "Bones.h"
#ifdef ORIGINAL
#include "PedModelInfo.h"
#else
#include "PedNode.h"
#endif
int Run_Node(int node)
{
    // Cross actual C language linkage after conversion and the original C++ wrapper before it
#ifdef ORIGINAL
    return Before_Node(node);
#else
    return ConvertPedNode2BoneTag(node);
#endif
}
const char *Run_Name(int tag)
{
    // Compare the public mapping from an independently compiled C11 translation unit
#ifdef ORIGINAL
    return Before_Name(tag);
#else
    return ConvertBoneTag2BoneName(tag);
#endif
}
int Run_Constant(unsigned index)
{
    // Compare every shared enum value without replacing constants by a second generated numbering scheme
    static const int values[]={'''+','.join(pedNames+boneNames)+'''};
    return values[index];
}
'''
(probe/'caller.c').write_text(caller,newline='\n');(probe/'fixture.c').write_text((root/'utils/tests/bones_fixture.c.in').read_text(),newline='\n')
sanitized='--sanitized' in sys.argv;runs=[]
for compiler,opt in ([('clang',1)] if sanitized else [('clang',0),('clang',2),('gcc',0),('gcc',2)]):
    # Exercise optimizer and project assertion configurations with complete original and C producers
    for profile,defines in [('default',[]),('master',['MASTER'])]:
        label=f'{compiler}-O{opt}-{profile}'+('-ubsan' if sanitized else '');results=[]
        with (probe/(label+'.log')).open('w',newline='\n') as log:
            for phase,current in [('before',saved),('after',tree)]:
                # Keep each phase's actual public headers and compile every caller as strict C11
                sanitize=['-fsanitize=undefined','-fsanitize-trap=all'] if sanitized else []
                flags=['-O'+str(opt),*sanitize,'-I'+str(probe),'-I'+str(current/'src/animation'),'-I'+str(current/'src/modelinfo'),'-I'+str(current/'src/core'),*['-D'+d for d in defines]]+(['-DORIGINAL'] if phase=='before' else [])
                cc=Path('C:/msys/ucrt64/bin')/(compiler+'.exe');cxx=cc.with_name('clang++.exe' if compiler=='clang' else 'g++.exe')
                units=[(cc,probe/'caller.c'),(cc,probe/'fixture.c')]+([(cxx,current/'src/animation/Bones.cpp'),(cxx,probe/'wrapper.cpp')] if phase=='before' else [(cc,current/'src/animation/Bones.c')]);objects=[]
                for tool,path in units:
                    # Preserve intended languages and keep generated objects out of the user's build directory
                    obj=probe/f'{label}-{phase}-{path.stem}.o';objects.append(obj)
                    command=[str(tool),'-std=c11' if path.suffix=='.c' else '-std=c++14',*flags,*(['-pedantic-errors'] if path.suffix=='.c' else []),'-c',str(path),'-o',str(obj)]
                    run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(' '.join(command)+'\n'+run.stdout+run.stderr);log.flush();assert run.returncode==0,(phase,path,run.stderr[-4000:])
                exe=probe/f'{label}-{phase}.exe';command=[str(cxx),'-static','-static-libgcc','-static-libstdc++',*sanitize,*[str(p) for p in objects],'-o',str(exe)]
                run=subprocess.run(command,env=env,capture_output=True,text=True);log.write(run.stdout+run.stderr);log.flush();assert run.returncode==0,run.stderr[-4000:]
                if compiler=='gcc':
                    # Reject GCC or MSYS runtime DLL imports before quiet Windows-only execution
                    imports=subprocess.run([str(cc.with_name('objdump.exe')),'-p',str(exe)],env=env,capture_output=True,text=True,check=True);log.write(imports.stdout)
                    names=re.findall(r'DLL Name: (\S+)',imports.stdout);assert names and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in names),names
                runEnv=dict(env);runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(phase,run.returncode,run.stderr);results.append(run.stdout.strip())
        assert results[0]==results[1],(label,results)
        runs.append({'compiler':compiler,'opt':opt,'profile':profile,'before':results[0],'after':results[1],'exit':0});print(label,results[0],flush=True)
(out/('sanitized-runs.json' if sanitized else 'matrix-runs.json')).write_text(json.dumps(runs,indent=2),newline='\n')
