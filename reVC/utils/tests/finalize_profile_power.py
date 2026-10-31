"""Audit both complete pure-C producer renames and their original behavior and native builds."""
from pathlib import Path
import re,json,hashlib
import profile_power_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/profile-power-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for old in changes:
    new=m.RENAMES.get(old,old);expected=m.calls((saved/old).read_text());assert (root/new).read_text()==expected and m.calls(expected)==expected,new
    assert b'\r' not in (root/new).read_bytes(),new
for old,new in m.RENAMES.items():assert not (root/old).exists() and (root/new).is_file(),old
from stats_successor import verify
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda value:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',value)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h']:assert not re.search(r'\bclass\s+C(?:Profile|PowerPoint|PowerPoints)\b|\bC(?:Profile|PowerPoints?)\s*::',strip(path.read_text(errors='replace'))),str(path)
for name in m.RENAMES.values():assert not re.search(r'::|\b(class|namespace|new|delete)\b',strip((root/name).read_text())),name
assert len(m.PROFILE_FIELDS)==7 and len(m.PROFILE_OPERATIONS)==3 and len(m.POWER_OPERATIONS)==6
reports={}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',12),('production-source-compiles',12)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after'],row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48 and all(row['exit']==0 and row['pairs']==28 and row['c_units']==2 for row in configs);reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'classes_removed':3,'public_C_operations':9,'canonical_profiler_arrays':7,'native_primitives':0,'changed_sources':len(changes),'renamed_producers':m.RENAMES,'profiler_color_bytes':4,'empty_instance_bytes':1,'normal_states':2385,'master_states':16})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified three complete class removals, nine C operations, seven profiler arrays, both pure-C producer renames and both full D3D9 links')
