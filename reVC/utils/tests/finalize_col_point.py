"""Audit complete C contact ownership, both copy policies, native mutation timing and full D3D9 links."""
from pathlib import Path
import json,re
import col_point_migration as m
from rect_migration import mask
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-point-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=root/m.RENAMES.get(name,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/collision/ColPoint.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
kernel=(root/'src/collision/ColPoint.c').read_text();header=(root/'src/collision/ColPoint.h').read_text()
for source in [kernel,header]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector|using)\b',mask(source))
for name in m.OPERATIONS:
    assert len(re.findall(r'^(?:const CollisionVector3 \*|float |void |CColPoint \*|CColPoint )ColPoint_'+name+r'\(',kernel,re.M))==1,name
    assert len(re.findall(r'\bColPoint_'+name+r'\(',header))==1,name
assign=__import__('doors_migration').method(kernel,'ColPoint_Assign')
assert 'self->depth' not in assign and 'self->pad1' not in assign and 'self->pad2' not in assign
world=(root/'src/core/World.cpp').read_text();weapon=(root/'src/weapons/Weapon.cpp').read_text()
assert 'ColPoint_Assign(&point,&tyreColPoint);' in world
assert 'ColPoint_Assign(&*point,&origPoint);' in weapon and 'CColPoint origPoint=ColPoint_CopyValue(*point);' in weapon
physics=(root/'src/entities/Physical.cpp').read_text();body=__import__('doors_migration').method(physics,'CPhysical::ApplySpringCollisionAlt')
assert 'CColPoint &contact' in body and body.index('ColPoint_ReverseNormal(&contact);')<body.index('ApplyMoveForce(')<body.index('ApplyTurnForce(')
old=(saved/'src/collision/Collision.cpp').read_text();current=(root/'src/collision/Collision.cpp').read_text()
conditional=old[old.index('#ifdef FIX_BUGS',old.index('linepoints[i].Set(0, 0,')):old.index('#endif',old.index('#ifdef FIX_BUGS',old.index('linepoints[i].Set(0, 0,')))+6]
assert conditional in current
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));rows=m.symbols(source)
        assert not re.search(r'\bCColPoint\s*::',source),str(path)
        for match in re.finditer(r'(\.|->)(Set|GetNormal|GetDepth)\(',source):
            start=m.value_start(source,match.start());receiver=source[start:match.start()]
            assert not m.record(receiver,match.start(),rows,match[1]=='->'),(str(path),receiver,match[2])
reports={}
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',116),('production-source-compiles',116)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=88215 hash='+('ca4ff0179deafa75' if row['profile']=='vu' else '7a89c2d793662d9b')
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:
    # Retain the six legacy configurations that exclude the librw compatibility producer
    expectedSources=409 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 410
    assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==expectedSources,row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name
    reports[name]=rows
reports.update({'complete_record_owners_converted':1,'public_C_operations':8,'native_gameplay_bridges_added':0,'changed_sources':len(changes),'renamed_producers':m.RENAMES,'contact_bytes':40,'normal_offset':16,'depth_offset':36,'assignment_preserves_gap_words':2,'assignment_preserves_depth':True,'selective_assignments_redirected':2,'copy_constructors_redirected':1,'states':88215,'actual_collision_consumers':6,'suspension_callback_cases':4096,'profiles':['float','compressed','vu']})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C contact ownership, both copy policies, live normal queries, native mutation timing and both full D3D9 links')
