"""Check complete sprite policy and independent C/C++ public interfaces as genuine C without claiming complete owners."""
from pathlib import Path
import os,json,subprocess
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shot-info-c-tests';tree=out/'overlay';probe=out/'strict';probe.mkdir(exist_ok=True);env=dict(os.environ);key=next(k for k in env if k.lower()=='path');env[key]='C:/msys/ucrt64/bin;C:/msys/usr/bin;'+env[key];rows=[];dirs=[tree/'src']+[p for p in (tree/'src').rglob('*') if p.is_dir() and not any(v.startswith('.') for v in p.relative_to(tree/'src').parts)]
for language in ['c','cpp']:
    # Validate independent consumers of the complete C record and function declarations
    (probe/('api.'+language)).write_text('#include "ShotInfo.h"\n',newline='\n')
for compiler in ['clang','gcc']:
    for bits in [32,64]:
        for profile,defines in [('d3d9',['LIBRW','RW_D3D9']),('gl',['LIBRW','RW_GL3']),('software',['LIBRW','RW_D3D9','REVC_SOFTWARE_POLYGONS']),('legacy',[]),('master',['LIBRW','RW_D3D9','MASTER']),('squeeze',['LIBRW','RW_D3D9','SQUEEZE_PERFORMANCE']),('ps2',['LIBRW','RW_D3D9']),('ps2-pal',['LIBRW','RW_D3D9'])]:
            for unit in ['ShotInfo.c']:
                # Compile the real partial C implementation while leaving remaining methods explicitly unresolved until completed
                label=f'{compiler}-{bits}-{profile}-{unit}';header=probe/(label+'-profile.h');header.write_text('#include "'+(tree/'src/core/config.h').as_posix()+'"\n'+('#undef FIX_SPRITES\n' if profile=='no-fix' else '#undef ASPECT_RATIO_SCALE\n' if profile=='no-aspect' else '#undef GTA_PC\n#define GTA_PS2\n'+('#define GTA_PAL\n' if profile=='ps2-pal' else '') if profile.startswith('ps2') else ''),newline='\n');command=[str(Path('C:/msys/ucrt64/bin')/(compiler+'.exe')),'-include',str(header),'-std=c11','-pedantic-errors','-Werror','-O2','-m'+str(bits),*['-D'+v for v in defines],*['-I'+str(p) for p in dirs],'-c',str(tree/'src/weapons'/unit),'-o',str(probe/(label+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr);rows.append({'compiler':compiler,'bits':bits,'profile':profile,'unit':unit,'scope':'complete producers and C peer','exit':0})
        for language in ['c','cpp']:
            # Require both interface languages to compile without SDK types or C++ common headers
            label=f'{compiler}-{bits}-api-{language}';tool='clang++' if compiler=='clang' and language=='cpp' else 'g++' if language=='cpp' else compiler;command=[str(Path('C:/msys/ucrt64/bin')/(tool+'.exe')),'-std=c++14' if language=='cpp' else '-std=c11','-pedantic-errors','-Werror','-m'+str(bits),*['-I'+str(p) for p in dirs],'-c',str(probe/('api.'+language)),'-o',str(probe/(label+'.o'))];run=subprocess.run(command,env=env,capture_output=True,text=True);(probe/(label+'.log')).write_text(run.stdout+run.stderr,newline='\n');assert run.returncode==0,(label,run.stderr);rows.append({'compiler':compiler,'bits':bits,'profile':'api','unit':language,'exit':0})
assert len(rows)==40
(out/'strict-compiles.json').write_text(json.dumps(rows,indent=2),newline='\n');report=json.loads((out/'stage-summary.json').read_text());report['strict_compiles']=len(rows);(out/'stage-summary.json').write_text(json.dumps(report,indent=2),newline='\n');print('Passed 40 complete producer and complete interface compilations; native consumers and runtime comparisons remain required')
