"""Apply the complete mutually dependent C conversion only after its staged checks pass."""
from pathlib import Path
import sys,json,hashlib
import events_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/events-c-tests'
for name,count in [('matrix-runs',16),('native-runs',8),('sanitized-runs',4),('native-layouts',4),('native-source-compiles',36)]:
    results=json.loads((out/(name+'.json')).read_text());assert len(results)==count,(name,len(results));assert all(item['exit']==0 for item in results)
changes=json.loads((out/'changes.json').read_text())
for path in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
for path,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
for path in changes:(root/path).write_text(m.calls((out/'before'/path).read_text()),newline='\n')
for name in ['EventsShared.h','Events_backend.h','GameplayEvents.c']:(root/'src/core'/name).write_text((out/'preview'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/timecycle_migration.py';source=endpoint.read_text();assert 'def cycle_calls(' not in source
source=source.replace('def calls(source):','def cycle_calls(source):').replace('def project(source):','def cycle_project(source):')
source+='''

def calls(source):
    # Retain the complete time-cycle conversion before the mutually dependent C event and accident owners
    import events_migration as events
    return events.calls(cycle_calls(source))

def project(source):
    # Preserve prior registrations and object disambiguation before registering the C gameplay-event owner
    import events_migration as events
    return events.project(cycle_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed CEventList, CAccident and CAccidentManager; applied 17 C operations and all 15 native caller/header conversions')
