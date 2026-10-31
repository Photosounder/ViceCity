"""Compile complete staged manager C algorithms under both compilers and pointer widths."""
from pathlib import Path
import json,os,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-manager-c-tests';tree=out/'overlay'
dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
results=[];env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key]
for compiler in ('clang','gcc'):
    # Check actual C11 syntax with both toolchains instead of compiling C files as C++
    for bits in (32,64):
        for profile,defines in [('default',[]),('compression',['ANIM_COMPRESSION']),('master',['MASTER']),('handheld',['GTA_HANDHELD'])]:
            # Let config.h control PC_PLAYER_CONTROLS and exercise its other branch through GTA_HANDHELD
            label=f'{compiler}-{bits}-{profile}'
            command=[str(Path('C:/msys/ucrt64/bin')/(compiler+'.exe')),'-std=c11','-pedantic-errors','-O2','-m'+str(bits),*['-D'+v for v in defines],*['-I'+str(p) for p in dirs],'-c',str(tree/'src/animation/AnimManager.c'),'-o',str(out/(label+'.o'))]
            run=subprocess.run(command,env=env,capture_output=True,text=True)
            (out/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n')
            assert run.returncode==0,(label,run.stderr[-5000:])
            results.append({'compiler':compiler,'bits':bits,'profile':profile,'exit':0})
            print(label,'passed',flush=True)
(out/'strict-c11-compiles.json').write_text(json.dumps(results,indent=2),newline='\n')
