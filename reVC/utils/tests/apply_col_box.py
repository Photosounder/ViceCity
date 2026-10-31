"""Apply both complete C box owners after aliases, padding, composition and collision behavior pass."""
from pathlib import Path
import hashlib,json
import col_box_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-box-c-tests'
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',176)]:
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
endpoint=root/'utils/tests/col_line_migration.py';source=endpoint.read_text();assert 'def col_line_calls(' not in source
snapshot=out/'before/utils/tests/col_line_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def col_line_calls(source):').replace('def project(source):','def col_line_project(source):')
source+='''


def calls(source):
    # Preserve complete line ownership before removing both native box owners and their inheritance
    import col_box_migration as boxes
    return boxes.calls(col_line_calls(source))


def project(source):
    # Retain the complete line producer before the box producer rename and C11 metadata
    import col_box_migration as boxes
    return boxes.project(col_line_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Converted both complete box owners to C, replacing inheritance with explicit composition')
