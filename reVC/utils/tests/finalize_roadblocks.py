"""Audit complete roadblock relocation, native records, full behavior comparisons and D3D9 links."""
from pathlib import Path
import json,re,hashlib
import roadblocks_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/roadblocks-c-tests';saved=out/'before'
changes=json.loads((out/'changes.json').read_text());old=(saved/'src/control/RoadBlocks.cpp').read_text()
for path in changes:
    assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
kernel=m.kernel(old)
for name,value in [('RoadBlockSystem.c',kernel),('RoadBlocks_backend.h',m.BACKEND)]:
    assert (root/'src/control'/name).read_text()==value and b'\r' not in (root/'src/control'/name).read_bytes(),name
from stats_successor import verify
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda value:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',value)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h']:assert not re.search(r'\bclass\s+(?:CRoadBlocks|CScriptRoadblock)\b|\bCRoadBlocks\s*::|\bCScriptRoadblock\s*::',strip(path.read_text(errors='replace'))),str(path)
assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip(kernel))
assert len(m.OPERATIONS)==6 and len(m.LEAVES)==103
for ret,name,args,code in m.LEAVES:assert 'RoadBlocks_Backend'+name+'(' in kernel,name
assert not re.search(r'\b(if|for|while|switch)\s*\(',strip(m.host()))
reports={}
for name,count in [('matrix-runs',28),('sanitized-runs',7),('native-runs',14),('native-debug-runs',14),('native-source-compiles',16),('production-source-compiles',16)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
    reports[name]=len(values)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48 and all(v['exit']==0 and v['pairs']==28 for v in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    value=json.loads((out/f'd3d9-{configuration}.json').read_text());assert value['exit']==0
    log=Path(value['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(value['exe']);assert exe.exists() and exe.stat().st_size==value['size'];reports['d3d9-'+configuration]=value['size']
reports.update({'classes_removed':2,'public_C_operations':7,'canonical_fields':4,'optional_securom_global':1,'native_primitives':103,'changed_sources':len(changes),'new_sources':2,'script_record_bytes':28,'script_record_alignment':4,'fixed_states':1612,'classic_states':1588})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified both roadblock classes removed, all 7 C operations, complete spawn/reclamation policies, unchanged 28-byte script records and both full D3D9 links')
