"""Apply both complete C triangle owners after all representations, aliases and native layouts pass."""
from pathlib import Path
import hashlib,json
import col_triangle_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-triangle-c-tests'
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',56)]:
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
endpoint=root/'utils/tests/compressed_vector_migration.py';source=endpoint.read_text();assert 'def compressed_vector_calls(' not in source
snapshot=out/'before/utils/tests/compressed_vector_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def compressed_vector_calls(source):').replace('def project(source):','def compressed_vector_project(source):')
source+='''

def calls(source):
    # Preserve collision vertex storage before the complete triangle and plane owners and their consumers
    import col_triangle_migration as triangles
    return triangles.calls(compressed_vector_calls(source))

def project(source):
    # Retain the vertex producer before the complete C11 triangle producer rename
    import col_triangle_migration as triangles
    return triangles.project(compressed_vector_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed both complete native triangle owners; C now owns all triangle/plane operations and both scalar/VU policies')
