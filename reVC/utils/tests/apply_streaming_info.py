"""Apply the complete streaming-info record conversion after staged native and behavior checks."""
from pathlib import Path
import hashlib,json
import streaming_info_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/streaming-info-c-tests'
for name,count in [('matrix-runs',4),('sanitized-runs',1),('native-runs',2),('native-source-compiles',12)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']
changes=json.loads((out/'changes.json').read_text())
for name in changes:assert (root/name).read_bytes()==(out/'before'/name).read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
for name in ['StreamingInfo.h','StreamingInfo.c']:assert not (root/'src/core'/name).exists(),name
for name in changes:(root/name).write_text(m.calls((out/'before'/name).read_text()),newline='\n')
for name in ['StreamingInfo.h','StreamingInfo.c']:(root/'src/core'/name).write_text((out/'overlay/src/core'/name).read_text(),newline='\n')
endpoint=root/'utils/tests/lists_migration.py';source=endpoint.read_text();assert 'def lists_calls(' not in source
snapshot=out/'before/utils/tests/lists_migration.py';snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(endpoint.read_bytes())
source=source.replace('def calls(source):','def lists_calls(source):').replace('def project(source):','def lists_project(source):')
source+='''

def calls(source):
    # Preserve complete list ownership before replacing the streaming-info record class
    import streaming_info_migration as streaming_info
    return streaming_info.calls(lists_calls(source))

def project(source):
    # Retain previous registrations before adding the independent C streaming-info producer
    import streaming_info_migration as streaming_info
    return streaming_info.project(lists_project(source))
'''
endpoint.write_text(source,newline='\n')
print('Removed the complete CStreamingInfo class and moved all six operations into StreamingInfo.c without native bridges')
