"""Apply the complete wanted conversion only after whole-owner and native checks pass."""
from pathlib import Path
import json,hashlib
import wanted_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/wanted-c-tests'
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-layouts',2),('native-source-compiles',72)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
        if 'original' in value:assert value['original']==value['converted'],value
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for path in changes:(root/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['WantedSystem.c','WantedShared.h','Wanted_backend.h']:(root/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/events_migration.py';source=endpoint.read_text();assert 'def event_calls(' not in source
target=out/'before/utils/tests/events_migration.py';target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def event_calls(source):').replace('def project(source):','def event_project(source):')
source+='''

def calls(source):
    # Preserve the complete event and accident conversion before removing the complete wanted owner
    import wanted_migration as wanted
    return wanted.calls(event_calls(source))

def project(source):
    # Retain every prior source registration before adding the unique wanted-system C object
    import wanted_migration as wanted
    return wanted.project(event_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed CWanted and CCrimeBeingQd; applied 26 C operations, all 37 caller/header/audio projections and explicit value initialization/replay copy')
