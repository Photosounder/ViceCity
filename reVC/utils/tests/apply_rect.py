"""Apply complete C route ownership only after original/C behavior and native ABI checks."""
from pathlib import Path
import hashlib,json
import rect_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rect-c-tests'
for name,count in [('matrix-runs',4),('sanitized-runs',1),('native-runs',2),('native-source-compiles',60)]:
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
endpoint=root/'utils/tests/stored_car_migration.py';source=endpoint.read_text();assert 'def stored_car_calls(' not in source
snapshot=out/'before/utils/tests/stored_car_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def stored_car_calls(source):').replace('def project(source):','def stored_car_project(source):')
source+='''

def calls(source):
    # Preserve stored-car ownership before moving the complete rectangle owner to C
    import rect_migration as rect
    return rect.calls(stored_car_calls(source))

def project(source):
    # Retain stored-car source registration before the complete rectangle producer rename
    import rect_migration as rect
    return rect.project(stored_car_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed the full CRect class and C++ producer; C owns all geometry and explicit native initialization')
