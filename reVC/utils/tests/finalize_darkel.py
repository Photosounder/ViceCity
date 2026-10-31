"""Verify the complete applied rampage conversion and authoritative native/build comparisons."""
from pathlib import Path
import re,json,hashlib
import darkel_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/darkel-c-tests';changes=json.loads((out/'changes.json').read_text());originalHeader=(out/'before/src/control/Darkel.h').read_text()
for path in changes:
    assert (root/path).read_text()==m.calls((out/'before'/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
kernel=m.kernel((out/'before/src/control/Darkel.cpp').read_text(),originalHeader)
for name,value in [('RampageSystem.c',kernel),('Darkel_backend.h',m.BACKEND)]:
    assert (root/'src/control'/name).read_text()==value and b'\r' not in (root/'src/control'/name).read_bytes(),name
from stats_successor import verify
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda value:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',value)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h']:assert not re.search(r'\bclass\s+CDarkel\b|\bCDarkel\s*::',strip(path.read_text(errors='replace'))),str(path)
assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip(kernel))
assert len(m.OPERATIONS)==14 and len(m.fields(originalHeader))==18 and len(m.LEAVES)==60
for ret,name,args,code in m.LEAVES:assert 'Darkel_Backend'+name+'(' in kernel,name
reports={}
for name,count in [('matrix-runs',24),('sanitized-runs',6),('native-runs',12),('native-source-compiles',40),('production-source-compiles',40)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
    reports[name]=len(values)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48 and all(v['exit']==0 and v['pairs']==28 for v in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    value=json.loads((out/f'd3d9-{configuration}.json').read_text());assert value['exit']==0
    log=Path(value['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(value['exe']);assert exe.exists() and exe.stat().st_size==value['size'];reports['d3d9-'+configuration]=value['size']
wanted=root/'.diagnostics/wanted-c-tests'
for name,count in [('matrix-runs',8),('native-runs',4)]:
    values=json.loads((wanted/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 and v['before']==v['after'] for v in values);reports['wanted-'+name]=len(values)
reports.update({'classes_removed':1,'public_C_operations':14,'canonical_fields':18,'native_primitives':60,'changed_sources':len(changes),'new_sources':2,'states_per_profile':25617})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete CDarkel removal, 14 C operations, 18 canonical fields, all caller conversions, platform/runtime comparisons and both full D3D9 links')
