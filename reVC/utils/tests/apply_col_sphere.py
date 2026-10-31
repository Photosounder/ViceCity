"""Apply both complete sphere owners after live ray aliases, native layout and geometry pass."""
from pathlib import Path
import hashlib,json
import col_sphere_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-sphere-c-tests'
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',140)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    source=root/old;target=root/new;assert source.resolve().is_relative_to(root.resolve()) and target.parent.resolve().is_relative_to(root.resolve())
    assert source.is_file() and not target.exists();source.rename(target)
for name in changes:(root/m.RENAMES.get(name,name)).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
header=root/'src/collision/ColSphere_native.h';assert not header.exists();header.write_text(m.native(),newline='\n')
endpoint=root/'utils/tests/col_box_migration.py';source=endpoint.read_text();assert 'def col_box_calls(' not in source
snapshot=out/'before/utils/tests/col_box_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def col_box_calls(source):').replace('def project(source):','def col_box_project(source):')
source+='''


def calls(source):
    # Preserve complete box ownership before removing both sphere owners and their native vector members
    import col_sphere_migration as spheres
    return spheres.calls(col_box_calls(source))


def project(source):
    # Retain box C11 integration before the sphere producer rename and native boundary header
    import col_sphere_migration as spheres
    return spheres.project(col_box_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Converted both complete sphere owners and ray intersection to C with explicit composition and live references')
