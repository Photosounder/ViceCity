"""Apply the entire roadblock conversion after complete staged comparisons and native checks."""
from pathlib import Path
import json,hashlib
import roadblocks_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/roadblocks-c-tests'
for name,count in [('matrix-runs',28),('sanitized-runs',7),('native-runs',14),('native-debug-runs',14),('native-source-compiles',16)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for path in changes:(root/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['RoadBlockSystem.c','RoadBlocks_backend.h']:(root/'src/control'/name).write_text((out/'preview'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/stats_migration.py';source=endpoint.read_text();assert 'def stats_calls(' not in source
saved=out/'before/utils/tests/stats_migration.py';saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def stats_calls(source):').replace('def project(source):','def stats_project(source):')
source+='''

def calls(source):
    # Preserve the complete statistics projection before removing both roadblock classes
    import roadblocks_migration as roadblocks
    return roadblocks.calls(stats_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named roadblock C object
    import roadblocks_migration as roadblocks
    return roadblocks.project(stats_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed CRoadBlocks and CScriptRoadblock methods; applied all 7 C operations and 4 canonical objects')
