"""Apply complete cached-polygon ownership after scalar, VU and parent-layout comparisons."""
from pathlib import Path
import hashlib,json
import stored_poly_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-poly-c-tests'
for name,count in [('matrix-runs',24),('sanitized-runs',6),('native-runs',12),('native-source-compiles',44)]:
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
for name,value in [('StoredCollPoly.h',m.header()),('StoredCollPoly_native.h',m.native()),('StoredCollPoly_backend.h',m.backend_header()),('StoredCollPoly.c',m.kernel((out/'before/src/collision/Collision.cpp').read_text()))]:
    path=root/'src/collision'/name
    assert not path.exists() or path.read_text()==value,name
    path.write_text(value,newline='\n')
endpoint=root/'utils/tests/col_point_migration.py';source=endpoint.read_text();assert 'def col_point_calls(' not in source
snapshot=out/'before/utils/tests/col_point_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def col_point_calls(source):').replace('def project(source):','def col_point_project(source):')
source+='''


def calls(source):
    # Preserve complete contact ownership before the cached polygon record and validation policy
    import stored_poly_migration as stored
    return stored.calls(col_point_calls(source))


def project(source):
    # Retain contact C11 integration before registering the complete stored-polygon producer
    import stored_poly_migration as stored
    return stored.project(col_point_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Converted the complete stored polygon record and scalar/VU cached validation to C')
