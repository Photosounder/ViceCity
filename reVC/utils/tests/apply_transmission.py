"""Apply the complete C transmission owner and remove its six obsolete audio bridges."""
from pathlib import Path
import hashlib,json
import transmission_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/transmission-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',28)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
assert not (root/'src/vehicles/HandlingFlags.h').exists()
for old,new in m.RENAMES.items():
    source=root/old;target=root/new;assert source.resolve().is_relative_to(root.resolve()) and target.parent.resolve().is_relative_to(root.resolve())
    assert source.is_file() and not target.exists();source.rename(target)
for name in changes:(root/m.RENAMES.get(name,name)).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
(root/'src/vehicles/HandlingFlags.h').write_text((out/'overlay/src/vehicles/HandlingFlags.h').read_text(),newline='\n')
endpoint=root/'utils/tests/input_state_migration.py';source=endpoint.read_text();assert 'def input_state_calls(' not in source
snapshot=out/'before/utils/tests/input_state_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def input_state_calls(source):').replace('def project(source):','def input_state_project(source):')
source+='''

def calls(source):
    # Preserve complete input ownership before the C transmission producer and direct audio field reads
    import transmission_migration as transmission
    return transmission.calls(input_state_calls(source))

def project(source):
    # Retain prior registrations before renaming the complete transmission producer to C11
    import transmission_migration as transmission
    return transmission.project(input_state_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed the complete cTransmission class, renamed its producer to C11 and removed six pure audio reader bridges')
