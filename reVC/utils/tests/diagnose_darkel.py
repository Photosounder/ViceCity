"""Locate the first compact original/C rampage checkpoint difference."""
from pathlib import Path
import os,sys,subprocess,ctypes
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/darkel-c-tests';prefix=sys.argv[1] if len(sys.argv)>1 else 'gcc-O0-fixed';env=dict(os.environ);env['DARKEL_TRACE']='1';ctypes.windll.kernel32.SetErrorMode(0x8003)
results=[]
for phase in ['before','after']:
    run=subprocess.run([str(out/f'{prefix}-{phase}.exe')],cwd=root,env=env,capture_output=True,text=True);assert run.returncode==0,run.stderr[-1000:]
    (out/(prefix+'-trace-'+phase+'.log')).write_text(run.stderr,newline='\n');results.append(run.stderr.splitlines())
for i,(before,after) in enumerate(zip(*results)):
    if before!=after:
        print('First differing checkpoint',i+1,results[0][max(0,i-2):i+2],results[1][max(0,i-2):i+2]);break
else:print('All compact checkpoints agree')
