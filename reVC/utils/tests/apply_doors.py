"""Apply both complete C door owners after behavior, lifecycle and native ABI checks."""
from pathlib import Path
import hashlib,json
import doors_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/doors-c-tests'
for name,count in [('matrix-runs',4),('sanitized-runs',1),('native-runs',2),('native-source-compiles',20)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for name in ['DoorSystem.c','Door_backend.h']:assert not (root/'src/vehicles'/name).exists(),name
for name in changes:(root/name).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
for name in ['DoorSystem.c','Door_backend.h']:(root/'src/vehicles'/name).write_text((out/'overlay/src/vehicles'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/transmission_migration.py';source=endpoint.read_text();assert 'def transmission_calls(' not in source
snapshot=out/'before/utils/tests/transmission_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def transmission_calls(source):').replace('def project(source):','def transmission_project(source):')
source+='''

def calls(source):
    # Preserve complete transmission ownership before removing both complete door classes
    import doors_migration as doors
    return doors.calls(transmission_calls(source))

def project(source):
    # Retain prior registrations before adding the independent C door-system producer
    import doors_migration as doors
    return doors.project(transmission_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed both door classes, moved all 16 operations into C and replaced all three owner constructor sites with explicit C initialization')
