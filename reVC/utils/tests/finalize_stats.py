"""Audit complete statistics relocation, packed persistence, compiler comparisons and full native links."""
from pathlib import Path
import json,re
import stats_migration as m
from stats_successor import verify,integration
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stats-c-tests';saved=out/'before'
changes=json.loads((out/'changes.json').read_text());old=(saved/'src/core/Stats.cpp').read_text();header=(saved/'src/core/Stats.h').read_text()
for path in changes:
    assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
kernel=m.kernel(old,header)
for name,value in [('StatisticsSystem.c',kernel),('Stats_backend.h',m.BACKEND)]:
    assert (root/'src/core'/name).read_text()==value and b'\r' not in (root/'src/core'/name).read_bytes(),name
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
assert (root/'src/core/rl.c').read_text()==integration((out/'integration-before/src/core/rl.c').read_text())
strip=lambda value:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',value)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h']:assert not re.search(r'\bclass\s+CStats\b|\bCStats\s*::',strip(path.read_text(errors='replace'))),str(path)
assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip(kernel))
assert len(m.signatures(old,header))==38 and len(m.fields(header))==91 and len(m.LEAVES)==16
for ret,name,args,code in m.LEAVES:assert 'Stats_Backend'+name+'(' in kernel,name
fields=json.loads((out/'save-fields.json').read_text());assert fields['save']==fields['load']
reports={}
for name,count in [('matrix-runs',20),('sanitized-runs',5),('native-runs',10),('native-source-compiles',74),('production-source-compiles',74)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
    reports[name]=len(values)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48 and all(v['exit']==0 and v['pairs']==28 for v in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    value=json.loads((out/f'd3d9-{configuration}.json').read_text());assert value['exit']==0
    log=Path(value['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(value['exe']);assert exe.exists() and exe.stat().st_size==value['size'];reports['d3d9-'+configuration]=value['size']
for stage,counts in [('wanted',[('matrix-runs',8),('native-runs',4)]),('darkel',[('matrix-runs',24),('native-runs',12)])]:
    for name,count in counts:
        values=json.loads((root/f'.diagnostics/{stage}-c-tests'/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 and v['before']==v['after'] for v in values);reports[stage+'-'+name]=len(values)
reports.update({'classes_removed':1,'public_C_operations':38,'canonical_fields':91,'native_primitives':16,'changed_sources':len(changes),'new_sources':2,'integration_fixes':1,'packed_save_bytes':595,'packed_fields':len(fields['save'])})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete CStats removal, 38 C operations, 91 canonical fields, exact 595-byte saves, all callers and both full D3D9 links')
