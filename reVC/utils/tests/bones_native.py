"""Compile real bone consumers, compare model layouts and execute both native pointer-width runtimes."""
from pathlib import Path
import ctypes,json,os,re,struct,subprocess,sys
import bones_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/bones-c-tests';saved=out/'before';production='--production' in sys.argv;tree=root if production else out/'overlay';probe=out/('production-native' if production else 'native');probe.mkdir(exist_ok=True)
ctypes.windll.kernel32.SetErrorMode(0x8003)
deps=[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes'];env=dict(os.environ);results=[];runs=[];layouts=[]
consumers=[]
for path in (saved/'src').rglob('*.cpp'):
    # Include every actual native caller and the complete producer in source compilation coverage
    if any(part.startswith('.') for part in path.relative_to(saved/'src').parts):continue
    if re.search(r'\b(?:ConvertPedNode2BoneTag|ConvertBoneTag2BoneName)\s*\(',path.read_text()):consumers.append(path.relative_to(saved).as_posix())
assert len(consumers)==7,consumers

def command(bits,label,commands):
    # Run native compiler commands quietly with separate logs and no app launch
    vcvars=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build')/f'vcvars{bits}.bat'
    driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 1\n'+'\nif errorlevel 1 exit /b 1\n'.join(commands)+'\nexit /b %errorlevel%\n',newline='\n')
    run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-5000:],run.stderr[-1000:])

def flags(current,isC,extra=''):
    # Keep every native compilation on a complete coherent header tree with the real D3D9 selection
    dirs=[current/'src']+[p for p in (current/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(current/'src').parts)]
    return '/nologo /c /O2 /MT /fp:precise /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+extra+' '+('/std:c11 /TC ' if isC else '/std:c++14 /EHsc /Zc:sizedDealloc- ')+' '.join('/I"'+str(p)+'"' for p in dirs+deps)

def layout_data(path):
    # Read the native compiler's emitted size and offset constants from the actual COFF object
    data=path.read_bytes();_,sections,_,symbols,count,optional,_=struct.unpack_from('<HHIIIHH',data);strings=symbols+18*count
    for n in range(count):
        # Resolve both short and string-table symbol names under either pointer-width ABI
        entry=symbols+18*n;raw=data[entry:entry+8];name=data[strings+struct.unpack_from('<I',raw,4)[0]:].split(b'\0',1)[0] if raw[:4]==b'\0'*4 else raw.rstrip(b'\0')
        if name in [b'BoneTypes',b'_BoneTypes']:
            value,section=struct.unpack_from('<Ih',data,entry+8);assert section>0;start=struct.unpack_from('<I',data,20+optional+(section-1)*40+20)[0];return list(struct.unpack_from('<10I',data,start+value))
    raise AssertionError('Missing native model and enum layout data')

for bits in [32,64]:
    # Compile all real call sites that receive C linkage through the public bone header
    for name in consumers:
        path=tree/m.RENAMES.get(name,name);label=f'{bits}-{path.stem}';obj=probe/(label+'.obj')
        command(bits,label,['cl '+flags(tree,path.suffix=='.c')+' /Fo"'+str(obj)+'" "'+str(path)+'"']);results.append({'bits':bits,'source':path.relative_to(tree).as_posix(),'exit':0});print(label,'passed',flush=True)
    values=[]
    for phase,current in [('before',saved),('after',tree)]:
        # Compare actual native model class fields and enum representations before and after the dependency split
        label=f'{bits}-{phase}-layout';path=probe/(label+'.cpp');obj=probe/(label+'.obj')
        path.write_text('#include "common.h"\n#include "PedModelInfo.h"\n#include "Bones.h"\n#include <stddef.h>\nextern "C" const unsigned BoneTypes[]={sizeof(CPedModelInfo),alignof(CPedModelInfo),offsetof(CPedModelInfo,m_animGroup),offsetof(CPedModelInfo,m_pedType),offsetof(CPedModelInfo,m_hitColModel),offsetof(CPedModelInfo,radio1),sizeof(enum PedNode),sizeof(enum BoneTag),PED_NODE_MAX,BONE_l_finger};\n',newline='\n')
        command(bits,label,['cl '+flags(current,False)+' /Fo"'+str(obj)+'" "'+str(path)+'"']);values.append(layout_data(obj))
    assert values[0]==values[1],(bits,values);layouts.append({'bits':bits,'before':values[0],'after':values[1],'exit':0})
    for profile,defines in [('default',''),('master','/DMASTER')]:
        # Compare complete original/C functions through a C11 caller under the real x86 and x64 runtimes
        outputs=[]
        for phase,current in [('before',saved),('after',tree)]:
            label=f'{bits}-{profile}-{phase}';fixture=out/'runtime';objects=[];commands=[]
            units=[fixture/'caller.c',fixture/'fixture.c']+([current/'src/animation/Bones.cpp',fixture/'wrapper.cpp'] if phase=='before' else [current/'src/animation/Bones.c'])
            for path in units:
                # Use only the exact enum shim for the original runtime dependency while retaining actual public headers
                options='/nologo /c /O2 /MT /fp:precise '+defines+(' /DORIGINAL' if phase=='before' else '')+(' /std:c11 /TC ' if path.suffix=='.c' else ' /std:c++14 /EHsc ')+' '.join('/I"'+str(p)+'"' for p in [fixture,current/'src/animation',current/'src/modelinfo',current/'src/core'])
                obj=probe/(label+'-'+path.stem+'.obj');objects.append(obj);commands.append('cl '+options+' /Fo"'+str(obj)+'" "'+str(path)+'"')
            exe=probe/(label+'.exe');commands.append('link /nologo /OUT:"'+str(exe)+'" '+' '.join('"'+str(p)+'"' for p in objects));command(bits,label,commands)
            runEnv=dict(env);key=next(k for k in runEnv if k.lower()=='path');runEnv[key]='C:/Windows/System32;C:/Windows';run=subprocess.run([str(exe)],env=runEnv,capture_output=True,text=True,timeout=30);assert run.returncode==0,(label,run.returncode,run.stderr);outputs.append(run.stdout.strip())
        assert outputs[0]==outputs[1],(bits,profile,outputs);runs.append({'bits':bits,'profile':profile,'before':outputs[0],'after':outputs[1],'exit':0});print(bits,profile,outputs[0],flush=True)
for name,value in [('native-source-compiles',results),('native-layouts',layouts),('native-runs',runs)]:
    # Preserve separate staged and actual production evidence
    (out/(('production-' if production else '')+name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
