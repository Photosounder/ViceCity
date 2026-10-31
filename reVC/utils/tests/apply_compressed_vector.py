"""Apply the complete C collision vertex owner after all representations and native consumers pass."""
from pathlib import Path
import hashlib,json
import compressed_vector_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/compressed-vector-c-tests'
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',62)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
assert not (root/'src/collision/CompressedVector.c').exists()
for name in changes:(root/name).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
(root/'src/collision/CompressedVector.c').write_text((out/'overlay/src/collision/CompressedVector.c').read_text(),newline='\n')
endpoint=root/'utils/tests/rect_migration.py';source=endpoint.read_text();assert 'def rect_calls(' not in source
snapshot=out/'before/utils/tests/rect_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def rect_calls(source):').replace('def project(source):','def rect_project(source):')
source+='''

def calls(source):
    # Preserve complete rectangle ownership before moving collision vertex packing and all consumers into C
    import compressed_vector_migration as vertex
    return vertex.calls(rect_calls(source))

def project(source):
    # Retain the rectangle rename before registering the complete C11 collision vertex producer
    import compressed_vector_migration as vertex
    return vertex.project(rect_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed all CompressedVector instance methods; C owns both vertex formats, decoding, packing and unpacking')
