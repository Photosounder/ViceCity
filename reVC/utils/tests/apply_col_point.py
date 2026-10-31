"""Apply complete contact ownership after both copy policies, native layout and mutation timing pass."""
from pathlib import Path
import hashlib,json
import col_point_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-point-c-tests'
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',116)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=root/name if (root/name).exists() else root/m.RENAMES.get(name,name)
    original=(out/'before'/name).read_bytes();expected=m.calls((out/'before'/name).read_text())
    assert path.read_bytes()==original or (path.read_text()==expected and b'\r' not in path.read_bytes()),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    source=root/old;target=root/new;assert source.resolve().is_relative_to(root.resolve()) and target.parent.resolve().is_relative_to(root.resolve())
    if source.is_file():
        assert not target.exists();source.rename(target)
    else:assert target.is_file()
for name in changes:
    path=(root/m.RENAMES.get(name,name)).resolve();expected=m.calls((out/'before'/name).read_text())
    if path.read_text()!=expected or b'\r' in path.read_bytes():
        originalBytes=path.read_bytes()
        for attempt in range(3):
            try:
                path.write_text(expected,newline='\n');break
            except OSError as error:
                if error.errno!=22 or attempt==2:raise
                assert path.read_bytes()==originalBytes,name
endpoint=root/'utils/tests/col_sphere_migration.py';source=endpoint.read_text();assert 'def col_sphere_calls(' not in source
snapshot=out/'before/utils/tests/col_sphere_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def col_sphere_calls(source):').replace('def project(source):','def col_sphere_project(source):')
source+='''


def calls(source):
    # Preserve complete sphere ownership before the contact record and its native mutation boundaries
    import col_point_migration as contacts
    return contacts.calls(col_sphere_calls(source))


def project(source):
    # Retain sphere C11 integration before the complete contact producer rename
    import col_point_migration as contacts
    return contacts.project(col_sphere_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Converted complete contact ownership, selective assignment, full copy values and native normal mutations to C')
