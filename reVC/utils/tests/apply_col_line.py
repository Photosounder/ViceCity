"""Apply complete C line ownership after aliases, quadword boundaries and native layouts pass."""
from pathlib import Path
import hashlib,json
import col_line_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-line-c-tests'
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',80)]:
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
for name,value in [('CollisionVector.h',m.shared()),('CollisionVector_native.h',m.native_vectors()),('ColLine_native.h',m.native_lines())]:
    path=root/'src/collision'/name;assert not path.exists();path.write_text(value,newline='\n')
endpoint=root/'utils/tests/col_triangle_migration.py';source=endpoint.read_text();assert 'def col_triangle_calls(' not in source
snapshot=out/'before/utils/tests/col_triangle_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def col_triangle_calls(source):').replace('def project(source):','def col_triangle_project(source):')
source+='''


def calls(source):
    # Retain complete triangle ownership before the line owner and shared coordinate boundaries
    import col_line_migration as lines
    return lines.calls(col_triangle_calls(source))


def project(source):
    # Retain triangle C11 integration before the complete line producer rename
    import col_line_migration as lines
    return lines.project(col_triangle_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Converted the complete collision-line owner to C with explicit native coordinate and quadword boundaries')
