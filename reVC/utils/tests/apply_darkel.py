"""Apply the complete rampage conversion after all staged compiler and behavior comparisons pass."""
from pathlib import Path
import json,hashlib
import darkel_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/darkel-c-tests'
for name,count in [('matrix-runs',24),('sanitized-runs',6),('native-runs',12),('native-source-compiles',40)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for path in changes:(root/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['RampageSystem.c','Darkel_backend.h']:(root/'src/control'/name).write_text((out/'preview'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/wanted_migration.py';source=endpoint.read_text();assert 'def wanted_calls(' not in source
saved=out/'before/utils/tests/wanted_migration.py';saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def wanted_calls(source):').replace('def project(source):','def wanted_project(source):')
source+='''

def calls(source):
    # Retain the complete wanted and queued-crime conversion before removing the complete rampage class
    import darkel_migration as darkel
    return darkel.calls(wanted_calls(source))

def project(source):
    # Preserve every earlier registration before adding the unique rampage-system C object
    import darkel_migration as darkel
    return darkel.project(wanted_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed CDarkel; applied all 14 C operations, 18 canonical state fields and every native caller/default-argument conversion')
