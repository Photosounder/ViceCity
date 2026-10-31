"""Apply complete C route ownership only after original/C behavior and native ABI checks."""
from pathlib import Path
import hashlib,json
import ped_routes_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/ped-routes-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',18)]:
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
endpoint=root/'utils/tests/damage_migration.py';source=endpoint.read_text();assert 'def damage_calls(' not in source
snapshot=out/'before/utils/tests/damage_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def damage_calls(source):').replace('def project(source):','def damage_project(source):')
source+='''

def calls(source):
    # Preserve complete damage ownership before moving the full pedestrian route table to C
    import ped_routes_migration as routes
    return routes.calls(damage_calls(source))

def project(source):
    # Retain the damage producer rename before the complete C11 route producer
    import ped_routes_migration as routes
    return routes.project(damage_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed the full CRouteNode class and C++ producer; C owns all six operations and 200 route records')
