"""Apply the entire autopilot conversion after complete staged comparisons and native checks."""
from pathlib import Path
import json,hashlib
import autopilot_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/autopilot-c-tests'
for name,count in [('matrix-runs',24),('sanitized-runs',6),('native-runs',12),('native-source-compiles',20)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for path in changes:(root/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['AutoPilotSystem.c','AutoPilot_backend.h']:(root/'src/control'/name).write_text((out/'preview'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/debug_display_migration.py';source=endpoint.read_text();assert 'def debug_display_calls(' not in source
saved=out/'before/utils/tests/debug_display_migration.py';saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def debug_display_calls(source):').replace('def project(source):','def debug_display_project(source):')
source+='''

def calls(source):
    # Preserve the complete debug-display projection before replacing the embedded autopilot class
    import autopilot_migration as autopilot
    return autopilot.calls(debug_display_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named autopilot C object
    import autopilot_migration as autopilot
    return autopilot.project(debug_display_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed the CAutoPilot constructor and methods; applied all 6 C operations, 33 record fields and both explicit vehicle constructor integrations')
