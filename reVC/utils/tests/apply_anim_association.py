"""Apply complete C hierarchy ownership after native layout and collection policy checks pass."""
from pathlib import Path
import hashlib,json
import anim_association_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-association-c-tests'
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8),('native-source-compiles',132),('classic-layout-compiles',4)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            assert row['before']==row['after']
            assert 'states=73734 checks=292889 frames=18432 hash=' in row['before'],name
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
for name,value in [('AnimBlendAssociation_backend.h',m.backend_header())]:
    path=root/'src/animation'/name;assert not path.exists() or path.read_text()==value,name;path.write_text(value,newline='\n')
endpoint=root/'utils/tests/anim_hierarchy_migration.py';source=endpoint.read_text();assert 'def anim_hierarchy_calls(' not in source
snapshot=out/'before/utils/tests/anim_hierarchy_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def anim_hierarchy_calls(source):').replace('def project(source):','def anim_hierarchy_project(source):')
source+='''


def calls(source):
    # Preserve complete hierarchy ownership before complete association ownership
    import anim_association_migration as clumps
    return clumps.calls(anim_hierarchy_calls(source))


def project(source):
    # Retain hierarchy integration before the renamed C11 association producer
    import anim_association_migration as clumps
    return clumps.project(anim_hierarchy_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Converted complete association ownership, all 28 operations and native lifetime callers to C')
