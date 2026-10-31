"""Apply the entire debug-display conversion after complete staged comparisons and native checks."""
from pathlib import Path
import json,hashlib
import debug_display_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/debug-display-c-tests'
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8),('native-source-compiles',26)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for path in changes:(root/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['DebugDisplaySystem.c','Debug_backend.h']:(root/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/roadblocks_migration.py';source=endpoint.read_text();assert 'def roadblocks_calls(' not in source
saved=out/'before/utils/tests/roadblocks_migration.py';saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def roadblocks_calls(source):').replace('def project(source):','def roadblocks_project(source):')
source+='''

def calls(source):
    # Preserve the complete roadblock projection before removing the complete debug display class
    import debug_display_migration as debug_display
    return debug_display.calls(roadblocks_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named debug display C object
    import debug_display_migration as debug_display
    return debug_display.project(roadblocks_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed CDebug; applied all 7 C operations, 6 canonical queue objects and the existing shared flag')
