"""Apply two complete pure-C producer renames after staged behavior and native checks."""
from pathlib import Path
import json,hashlib
import profile_power_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/profile-power-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',12)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after'],row
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for old,new in m.RENAMES.items():
    source=root/old;target=root/new;assert source.resolve().is_relative_to(root.resolve()) and target.parent.resolve().is_relative_to(root.resolve())
    assert source.is_file() and not target.exists(),new
for old,new in m.RENAMES.items():(root/old).rename(root/new)
for path in changes:(root/m.RENAMES.get(path,path)).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
endpoint=root/'utils/tests/autopilot_migration.py';source=endpoint.read_text();assert 'def autopilot_calls(' not in source
snapshot=out/'before/utils/tests/autopilot_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def autopilot_calls(source):').replace('def project(source):','def autopilot_project(source):')
source+='''

def calls(source):
    # Preserve the complete autopilot projection before removing all profiler and cut-feature classes
    import profile_power_migration as profile_power
    return profile_power.calls(autopilot_calls(source))

def project(source):
    # Preserve earlier registrations before renaming both complete producers to C11
    import profile_power_migration as profile_power
    return profile_power.project(autopilot_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed three complete classes; renamed Profile.cpp and PowerPoints.cpp to standalone C11 producers with all 9 original operations and no C++ helpers')
