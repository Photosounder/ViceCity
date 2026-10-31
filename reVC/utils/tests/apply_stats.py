"""Apply the entire statistics conversion after complete staged comparisons and native checks."""
from pathlib import Path
import json,hashlib
import stats_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stats-c-tests'
for name,count in [('matrix-runs',20),('sanitized-runs',5),('native-runs',10),('native-source-compiles',74)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for path in changes:(root/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['StatisticsSystem.c','Stats_backend.h']:(root/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/darkel_migration.py';source=endpoint.read_text();assert 'def darkel_calls(' not in source
saved=out/'before/utils/tests/darkel_migration.py';saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def darkel_calls(source):').replace('def project(source):','def darkel_project(source):')
source+='''

def calls(source):
    # Preserve the complete rampage projection before relocating the complete statistics owner
    import stats_migration as stats
    return stats.calls(darkel_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named statistics C object
    import stats_migration as stats
    return stats.project(darkel_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed CStats; applied all 38 C operations, 91 canonical objects and every statistics caller')
