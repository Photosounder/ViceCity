"""Apply complete cached-polygon ownership after scalar, VU and parent-layout comparisons."""
from pathlib import Path
import hashlib,json
import keyframes_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/keyframes-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',60)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=root/name;original=(out/'before'/name).read_bytes();expected=m.calls((out/'before'/name).read_text())
    assert path.read_bytes()==original or (path.read_text()==expected and b'\r' not in path.read_bytes()),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for name in changes:
    path=(root/name).resolve();expected=m.calls((out/'before'/name).read_text())
    if path.read_text()!=expected or b'\r' in path.read_bytes():
        originalBytes=path.read_bytes()
        for attempt in range(3):
            try:
                path.write_text(expected,newline='\n');break
            except OSError as error:
                if error.errno!=22 or attempt==2:raise
                assert path.read_bytes()==originalBytes,name
for name,value in [('KeyFrames.h',m.header()),('KeyFrames_native.h',m.native()),('KeyFrames.c',m.kernel())]:
    path=root/'src/animation'/name
    assert not path.exists() or path.read_text()==value,name
    path.write_text(value,newline='\n')
endpoint=root/'utils/tests/quaternion_migration.py';source=endpoint.read_text();assert 'def quaternion_calls(' not in source
snapshot=out/'before/utils/tests/quaternion_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def quaternion_calls(source):').replace('def project(source):','def quaternion_project(source):')
source+='''


def calls(source):
    # Preserve complete quaternion ownership before all four C keyframe records
    import keyframes_migration as frames
    return frames.calls(quaternion_calls(source))


def project(source):
    # Retain quaternion C11 integration before registering the complete keyframe producer
    import keyframes_migration as frames
    return frames.project(quaternion_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Converted all four keyframe records, both inheritance relationships and all six packed methods to C')
