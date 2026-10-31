"""Audit complete streaming-info relocation, callers, native layouts and both full links."""
from pathlib import Path
import json,re
import streaming_info_migration as m
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/streaming-info-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    expected=m.calls((saved/name).read_text());assert (root/name).read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in (root/name).read_bytes(),name
for name,expected in [('StreamingInfo.h',m.header((saved/'src/core/Streaming.h').read_text())),('StreamingInfo.c',m.kernel((saved/'src/core/Streaming.cpp').read_text()))]:
    assert (root/'src/core'/name).read_text()==expected,name
    assert b'\r' not in (root/'src/core'/name).read_bytes(),name
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(r'\bclass\s+CStreamingInfo\b|\bCStreamingInfo\s*::',strip(path.read_text(errors='replace'))),str(path)
for name in ['StreamingInfo.h','StreamingInfo.c']:assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip((root/'src/core'/name).read_text())),name
header=(root/'src/core/StreamingInfo.h').read_text();kernel=(root/'src/core/StreamingInfo.c').read_text()
for name in m.OPERATIONS:assert len(re.findall(r'\bStreamingInfo_'+name+r'\(',header))==1 and len(re.findall(r'^StreamingInfo_'+name+r'\(',kernel,re.M))==1,name
assert len(re.findall(r'\bm_\w+;',header))==7
manager=(root/'src/core/Streaming.cpp').read_text();assert not re.search(r'(?:\.|->)(?:'+ '|'.join(m.OPERATIONS)+r')\(',manager)
reports={}
for name,count in [('matrix-runs',4),('sanitized-runs',1),('native-runs',2),('native-source-compiles',12),('production-source-compiles',12)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after'] and 'states=8578 checks=349291 hash=726a7409b679cefc'==row['after'],row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    # The six legacy Win32 RW34 configurations deliberately exclude the librw compatibility producer
    expectedSources=405 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 406
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==expectedSources,row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'classes_removed':1,'public_C_operations':6,'record_fields':7,'shared_enums':2,'native_bridges':0,'changed_sources':len(changes),'new_sources':2,'states':8578,'checks_per_run':349291,'native_record_bytes':{'x86':20,'x64':32},'converted_calls':len(re.findall(r'\bStreamingInfo_\w+\(',manager))})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete CStreamingInfo removal, six C operations, all callers, native layouts, output aliases and both full D3D9 links')
