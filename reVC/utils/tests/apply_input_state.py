"""Apply all four complete input-record conversions after guarded original/C comparisons."""
from pathlib import Path
import hashlib,json
import input_state_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/input-state-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',22)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for name in ['InputState.h','InputState.c','InputState_backend.h']:assert not (root/'src/core'/name).exists(),name
for name in changes:(root/name).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
for name in ['InputState.h','InputState.c','InputState_backend.h']:(root/'src/core'/name).write_text((out/'overlay/src/core'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/streaming_info_migration.py';source=endpoint.read_text();assert 'def streaming_info_calls(' not in source
snapshot=out/'before/utils/tests/streaming_info_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def streaming_info_calls(source):').replace('def project(source):','def streaming_info_project(source):')
source+='''

def calls(source):
    # Preserve streaming-info ownership before removing the four complete input-state classes
    import input_state_migration as input_state
    return input_state.calls(streaming_info_calls(source))

def project(source):
    # Retain prior registrations before the independent C input-state producer
    import input_state_migration as input_state
    return input_state.project(streaming_info_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed four input-state classes and moved all ten operations plus canonical inversion state into InputState.c')
