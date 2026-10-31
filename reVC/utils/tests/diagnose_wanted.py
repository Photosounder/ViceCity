"""Locate the first compact checkpoint difference without dumping complete execution traces."""
from pathlib import Path
import os,subprocess,ctypes
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/wanted-c-tests';env=dict(os.environ);env['WANTED_TRACE']='1';ctypes.windll.kernel32.SetErrorMode(0x8003)
results=[]
for phase in ['before','after']:
    run=subprocess.run([str(out/f'clang-O0-classic-{phase}.exe')],env=env,cwd=root,capture_output=True,text=True);assert run.returncode==0,run.stderr[-1000:]
    (out/('trace-'+phase+'.log')).write_text(run.stderr,newline='\n');results.append(run.stderr.splitlines())
for i,(before,after) in enumerate(zip(*results)):
    if before!=after:
        print('First differing checkpoint',i+1,results[0][max(0,i-2):i+2],results[1][max(0,i-2):i+2]);break
else:print('All compact checkpoints agree')
