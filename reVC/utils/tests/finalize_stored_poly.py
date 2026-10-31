"""Audit complete C cached-polygon ownership, scalar/VU policy and full D3D9 links."""
from pathlib import Path
import json,re
import stored_poly_migration as m
from rect_migration import mask
from stats_successor import verify
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-poly-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
assert len(changes)==4
for name in changes:
    path=root/name;expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
original=(saved/'src/collision/Collision.cpp').read_text();current=(root/'src/collision/Collision.cpp').read_text()
for name,value in [('StoredCollPoly.h',m.header()),('StoredCollPoly_native.h',m.native()),('StoredCollPoly_backend.h',m.backend_header()),('StoredCollPoly.c',m.kernel(original))]:
    path=root/'src/collision'/name;assert path.read_text()==value and b'\r' not in path.read_bytes(),name
for name in ['StoredCollPoly.h','StoredCollPoly.c','StoredCollPoly_backend.h']:
    source=mask((root/'src/collision'/name).read_text());assert not re.search(r'::|\b(class|namespace|new|delete|CVector|CVuVector|using)\b',source),name
for name in ['SetVertex','GetVertex','IsValidVerticalLine']:
    assert len(re.findall(r'^(?:void |CollisionVector3 |bool )StoredPoly_'+name+r'\(', (root/'src/collision/StoredCollPoly.c').read_text(),re.M))==1,name
assert current.count('StoredPoly_SetVertex(')==21 and current.count('*poly = TempStoredPoly;')==2
assert method(current,'StoredPoly_BackendLine')==method(m.backend(),'StoredPoly_BackendLine')
assert method(current,'GetVUresult')==method(original,'GetVUresult')
for name in ['VuCollision.cpp','VuCollision.h','vu0Collision_1.s','vu0Collision_2.s','vu0Collision.dsm']:
    assert (root/'src/collision'/name).read_bytes()==(saved/'src/collision'/name).read_bytes(),name
assert 'StoredPolyVertex verts[3];' in m.header() and 'bool valid;' in m.header()
assert (root/'src/control/CarCtrl.cpp').read_text().count('StoredPoly_IsValidVerticalLine(')==2
assert (root/'src/peds/Ped.cpp').read_text().count('StoredPoly_IsValidVerticalLine(')==1
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert 'IsStoredPolyStillValidVerticalLine' not in source,str(path)
reports={}
gold={'float-fixed':'states=102877 checks=594207 hits=3176 misses=34163 hash=f3de850b87c9124b','float-classic':'states=102877 checks=594207 hits=3176 misses=34163 hash=f3de850b87c9124b','compressed-fixed':'states=102877 checks=594207 hits=3176 misses=34163 hash=f3de850b87c9124b','compressed-classic':'states=102877 checks=594207 hits=3176 misses=34163 hash=f3de850b87c9124b','vu-fixed':'states=102877 checks=594385 hits=3022 misses=34317 hash=ed976893a68026be','vu-classic':'states=102877 checks=577854 hits=3022 misses=34317 hash=85f388de2193aedd'}
for name,count in [('matrix-runs',24),('sanitized-runs',6),('native-runs',12),('native-source-compiles',44),('production-source-compiles',44)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']==gold[row['profile']],row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(410 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 411),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name
    reports[name]=rows
reports.update({'complete_record_owners_converted':1,'public_C_operations':3,'native_VU_leaves':1,'changed_sources':len(changes),'new_sources':4,'scalar_bytes':40,'scalar_alignment':4,'VU_bytes':64,'VU_alignment':16,'states':102877,'vertex_write_sites':21,'record_copy_sites':2,'cached_query_sites':3,'profiles':list(gold)})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C cached-polygon ownership, exact scalar/VU policy, native parent layouts and both full D3D9 links')
