"""Compare the complete actual SDK record with independent C and C++ public record layouts."""
from pathlib import Path
import json,os,struct,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/weapon-effects-c-tests';probe=out/'layouts';probe.mkdir(exist_ok=True);rows=[]
env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
for bits in [32,64]:
    # Inspect full native record declarations without linking or executing SDK initialization
    layouts=[]
    for phase in ['before','after-c','after-cpp']:
        # Compile each public language independently on a coherent original or staged source tree
        before=phase=='before';tree=out/('before' if before else 'overlay');record='CWeaponEffects' if before else 'WeaponEffects';cpp=phase!='after-c';label=f'{bits}-{phase}';source=probe/(label+('.cpp' if cpp else '.c'));obj=probe/(label+'.obj')
        fields=['m_bActive','m_vecPos','m_nRed','m_nGreen','m_nBlue','m_nAlpha','m_fSize','m_fRotation']
        text=('#include "common.h"\n' if before else '')+'#include "WeaponEffects.h"\n#include <stddef.h>\n#include <stdint.h>\n#pragma section(".weabi",read)\n'+('extern "C" {\n' if cpp else '')+'__declspec(allocate(".weabi")) '+('extern ' if cpp else '')+'const uint64_t weaponEffectsLayout[]={sizeof('+record+'),__alignof('+record+'),'+','.join('offsetof('+record+','+field+')' for field in fields)+'};\n'+('}\n' if cpp else '')
        source.write_text(text,newline='\n');includes=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir()]+[root/'vendor/librw',root/'vendor/openal-soft/include',root/'vendor/libsndfile/include',root/'vendor/mpg123/include',Path('C:/msys/home'),Path('C:/msys/home/github/CITAlloc'),root/'.diagnostics/events-c-tests/native-includes']
        flags='/nologo /c /MT /O2 /DLIBRW /DRW_D3D9 /DUSE_D3D9 /DAUDIO_OAL /DNOMINMAX '+('/std:c++14 /EHsc /Zc:sizedDealloc-' if cpp else '/std:c11 /TC')+' '+' '.join('/I"'+str(p)+'"' for p in includes)
        driver=probe/(label+'.cmd');driver.write_text('@echo off\ncall "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars'+str(bits)+'.bat" >nul\nif errorlevel 1 exit /b 1\ncl '+flags+' /Fo"'+str(obj)+'" "'+str(source)+'"\nexit /b %errorlevel%\n',newline='\n');run=subprocess.run(['cmd.exe','/c',str(driver)],env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stdout[-3000:],run.stderr)
        binary=probe/(label+'.bin');run=subprocess.run(['C:/msys/ucrt64/bin/llvm-objcopy.exe','--dump-section=.weabi='+str(binary),str(obj)],env=env,capture_output=True,text=True);assert run.returncode==0,(label,run.stderr);data=binary.read_bytes();assert len(data)>=80 and not any(data[80:]);layouts.append(struct.unpack('<10Q',data[:80]))
    assert layouts[0]==layouts[1]==layouts[2]==(28,4,0,4,16,17,18,19,20,24),(bits,layouts)
    rows.append({'bits':bits,'consumers':['original-sdk','new-c','new-cpp'],'layout':layouts[0],'exit':0});print(bits,'all original/C/C++ offsets passed',flush=True)
(out/'native-layouts.json').write_text(json.dumps(rows,indent=2),newline='\n')
