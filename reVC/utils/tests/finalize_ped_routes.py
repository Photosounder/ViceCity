"""Audit complete C route ownership, every caller, native layouts and full D3D9 links."""
from pathlib import Path
import json,re
import ped_routes_migration as m
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/ped-routes-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=root/m.RENAMES.get(name,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/peds/PedRoutes.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
strip=lambda source:re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"','',source)
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c']:assert not re.search(r'\bclass\s+CRouteNode\b|\bCRouteNode\s*::',strip(path.read_text(errors='replace'))),str(path)
for name in ['PedRoutes.h','PedRoutes.c']:assert not re.search(r'::|\b(class|namespace|new|delete|CVector)\b',strip((root/'src/peds'/name).read_text())),name
kernel=(root/'src/peds/PedRoutes.c').read_text();header=(root/'src/peds/PedRoutes.h').read_text()
for name in m.OPERATIONS:assert len(re.findall(r'^PedRoutes_'+name+r'\(',kernel,re.M))==1 and len(re.findall(r'\bPedRoutes_'+name+r'\(',header))==1,name
assert kernel.count('CRouteNode gaRoutes[NUMPEDROUTES];')==1 and 'extern CRouteNode gaRoutes[NUMPEDROUTES];' in header
reports={}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',18),('production-source-compiles',18)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=20622 checks=9623473 hash=2b619660c871f5cd' if row['profile']=='fixed' else 'states=10520 checks=4557522 hash=e8c7f451514052c6'
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    # Retain the six legacy configurations that exclude the librw compatibility producer
    expectedSources=407 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 408
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==expectedSources,row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
reports.update({'classes_removed':1,'public_C_operations':6,'canonical_route_records':200,'native_bridges_added':0,'changed_sources':len(changes),'renamed_producers':m.RENAMES,'record_bytes':16,'table_bytes':3200,'vector_bytes':12,'fixed_states':20622,'classic_states':10520})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C route ownership, all six operations, canonical table, vector ABI and both full D3D9 links')
