"""Apply the proven complete list conversion after immutable source and staged check guards."""
from pathlib import Path
import hashlib,json
import lists_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lists-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',20)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for old,new in m.RENAMES.items():
    source=root/old;target=root/new;assert source.resolve().is_relative_to(root.resolve()) and target.parent.resolve().is_relative_to(root.resolve())
    assert source.is_file() and not target.exists(),new
    source.rename(target)
for name in changes:(root/m.RENAMES.get(name,name)).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
endpoint=root/'utils/tests/profile_power_migration.py';source=endpoint.read_text();assert 'def profile_power_calls(' not in source
snapshot=out/'before/utils/tests/profile_power_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def profile_power_calls(source):').replace('def project(source):','def profile_power_project(source):')
source+='''

def calls(source):
    # Preserve prior conversions before replacing complete intrusive-list ownership
    import lists_migration as lists
    return lists.calls(profile_power_calls(source))

def project(source):
    # Retain previous registrations before the complete Lists.c producer rename
    import lists_migration as lists
    return lists.project(profile_power_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Applied 18 source changes, four C list records and 16 C operations with explicit embedded and static lifetimes')
