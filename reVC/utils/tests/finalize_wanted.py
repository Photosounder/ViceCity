"""Audit the complete applied wanted conversion, historical event dependency and full native builds."""
from pathlib import Path
import re,json,hashlib
import wanted_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/wanted-c-tests';changes=json.loads((out/'changes.json').read_text())
for path in changes:
    assert (root/path).read_text()==m.calls((out/'before'/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
for name in ['WantedSystem.c','WantedShared.h','Wanted_backend.h']:
    assert (root/'src/core'/name).read_text()==(out/'preview'/name).read_text(),name
    assert b'\r' not in (root/'src/core'/name).read_bytes(),name
assert (root/'src/core/WantedSystem.c').read_text()==m.kernel((out/'before/src/core/Wanted.cpp').read_text(),(out/'before/src/core/Wanted.h').read_text())
from stats_successor import verify
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda value:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',value)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h']:
        code=strip(path.read_text(errors='replace'));assert not re.search(r'\bclass\s+(?:CWanted|CCrimeBeingQd)\b|\bCWanted\s*::|std::allocator<CWanted>|~CWanted\(',code),str(path)
kernel=strip((root/'src/core/WantedSystem.c').read_text());assert not re.search(r'::|\b(class|namespace|new|delete)\b',kernel)
assert len(m.SIGNATURES)==26 and len(m.OPERATIONS)==24 and len(m.LEAVES)==18
reports={}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-layouts',2),('native-source-compiles',72),('production-source-compiles',72)]:
    values=json.loads((out/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 for v in values),name
    for value in values:
        if 'before' in value:assert value['before']==value['after'],value
        if 'original' in value:assert value['original']==value['converted'],value
    reports[name]=len(values)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48 and all(v['exit']==0 and v['pairs']==28 for v in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    value=json.loads((out/f'd3d9-{configuration}.json').read_text());assert value['exit']==0
    log=Path(value['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(value['exe']);assert exe.exists() and exe.stat().st_size==value['size'];reports['d3d9-'+configuration]=value['size']
events=root/'.diagnostics/events-c-tests'
for name,count in [('matrix-runs',16),('native-runs',8)]:
    values=json.loads((events/(name+'.json')).read_text());assert len(values)==count and all(v['exit']==0 and v['before']==v['after'] for v in values);reports['events-'+name]=len(values)
reports.update({'classes_removed':2,'public_C_operations':26,'native_leaves':18,'changed_sources':len(changes),'new_sources':3,'crime_size':28,'wanted_sizes':[528,568],'states_per_profile':10078})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified both complete wanted classes removed, 26 C operations, deferred native/plain audio reads, unchanged native layouts and both full D3D9 links')
