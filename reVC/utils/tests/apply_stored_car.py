"""Apply complete C stored-car ownership after behavior, native ABI and both save branches pass."""
from pathlib import Path
import hashlib,json
import stored_car_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-car-c-tests'
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8),('native-source-compiles',16)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for name in ['StoredCar.h','StoredCar.c','StoredCar_backend.h']:assert not (root/'src/control'/name).exists(),name
for name in changes:(root/name).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
for name in ['StoredCar.h','StoredCar.c','StoredCar_backend.h']:(root/'src/control'/name).write_text((out/'overlay/src/control'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/ped_routes_migration.py';source=endpoint.read_text();assert 'def ped_routes_calls(' not in source
snapshot=out/'before/utils/tests/ped_routes_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def ped_routes_calls(source):').replace('def project(source):','def ped_routes_project(source):')
source+='''

def calls(source):
    # Preserve full C route ownership before converting the complete stored-car owner and save callers
    import stored_car_migration as stored
    return stored.calls(ped_routes_calls(source))

def project(source):
    # Retain the route producer rename before the independent C11 stored-car producer
    import stored_car_migration as stored
    return stored.project(ped_routes_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed the complete CStoredCar class and assignment operator; all six operations and save copy policy now use C')
