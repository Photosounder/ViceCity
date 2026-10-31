"""Apply complete C damage ownership after immutable source, behavior and native ABI guards."""
from pathlib import Path
import hashlib,json
import damage_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/damage-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',36)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
assert not (root/'src/vehicles/VehicleDamageParts.h').exists()
for old,new in m.RENAMES.items():
    source=root/old;target=root/new;assert source.resolve().is_relative_to(root.resolve()) and target.parent.resolve().is_relative_to(root.resolve())
    assert source.is_file() and not target.exists();source.rename(target)
for name in changes:(root/m.RENAMES.get(name,name)).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
(root/'src/vehicles/VehicleDamageParts.h').write_text((out/'overlay/src/vehicles/VehicleDamageParts.h').read_text(),newline='\n')
endpoint=root/'utils/tests/doors_migration.py';source=endpoint.read_text();assert 'def doors_calls(' not in source
snapshot=out/'before/utils/tests/doors_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def doors_calls(source):').replace('def project(source):','def doors_project(source):')
source+='''

def calls(source):
    # Preserve both complete door owners before the C damage owner and its typed callers
    import damage_migration as damage
    return damage.calls(doors_calls(source))

def project(source):
    # Retain prior registrations before the complete damage producer rename to C11
    import damage_migration as damage
    return damage.project(doors_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed the complete CDamageManager class, moved all 20 operations into C and made automobile damage initialization explicit before doors')
