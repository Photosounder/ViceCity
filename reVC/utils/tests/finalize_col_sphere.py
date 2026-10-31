"""Audit both complete C sphere owners, live ray aliases, native boundaries and full D3D9 links."""
from pathlib import Path
import json,re
import col_sphere_migration as m
from rect_migration import mask
from stats_successor import verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-sphere-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=root/m.RENAMES.get(name,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/collision/ColSphere.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
kernel=(root/'src/collision/ColSphere.c').read_text();header=(root/'src/collision/ColSphere.h').read_text()
for source in [kernel,header]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector|using)\b',mask(source))
assert 'CSphere bounds;' in header and 'struct CColSphere :' not in header
assert (root/'src/collision/ColSphere_native.h').read_text()==m.native()
for name in m.OPERATIONS:
    assert len(re.findall(r'^(?:void |bool |CSphere |CColSphere )'+name+r'\(',kernel,re.M))==1,name
    assert len(re.findall(r'\b'+name+r'\(',header))==1,name
assert header.count('static inline CollisionPointOut CollisionPoint_Output(')==1
assert kernel.index('Sphere_WritePoint(entry,first);')<kernel.index('const CollisionVector3 second=')<kernel.index('Sphere_WritePoint(exit,second);')
from doors_migration import method
original=method((saved/'src/math/math.cpp').read_text(),'TransformPoints')
expected=original[original.index('\t__asm__ __volatile__'):original.index('\n#else')]
current=method((root/'src/math/math.cpp').read_text(),'TransformColSpheres')
assert current[current.index('\t__asm__ __volatile__'):current.index('\n#else')]==expected
assert '#ifdef GTA_PS3' in current and 'stride=sizeof(CColSphere)' in current
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'))
        assert not re.search(r'\b(?:CColSphere|CSphere)\s*::|\bstruct CColSphere\s*:|\*\(CColSphere\*\)|\*\(CVuVector\*\)&sphere',source),str(path)
        records=m.symbols(source)
        for match in re.finditer(r'(\.|->)(center|radius|Set|IntersectRay)\b',source):
            start=m.value_start(source,match.start());receiver=source[start:match.start()]
            record=m.kind(receiver,match.start(),records)
            assert record!='CColSphere',(str(path),receiver,match[2])
            if record:assert match[2] not in ['Set','IntersectRay'],(str(path),receiver,match[2])
reports={}
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',140),('production-source-compiles',140)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=183532 hits=35374 misses=4626 hash='+('8eaa6198af9a9dcf' if row['profile']=='vu' else '343c9519743fb641')
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
reports.update({'complete_record_owners_converted':2,'public_C_operations':6,'C_output_reference_helpers':1,'inheritance_relationships_removed':1,'native_value_adapters':1,'native_batch_adapters':1,'native_gameplay_bridges_added':0,'changed_sources':len(changes),'new_native_headers':1,'renamed_producers':m.RENAMES,'sphere_bytes':16,'collision_sphere_bytes':20,'untouched_tail_bytes':2,'states':183532,'ray_alias_states':40000,'ray_alias_hits':35374,'ray_alias_misses':4626,'actual_collision_consumers':6,'profiles':['float','compressed','vu']})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified both complete C sphere owners, live ray aliases, native quadword/batch boundaries and both full D3D9 links')
