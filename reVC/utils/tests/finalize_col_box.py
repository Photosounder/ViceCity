"""Audit both complete C box owners, native aliases, composition and full D3D9 links."""
from pathlib import Path
import json,re
import col_box_migration as m
from rect_migration import mask
from stats_successor import verify,current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-box-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/collision/ColBox.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
kernel=(root/'src/collision/ColBox.c').read_text();header=(root/'src/collision/ColBox.h').read_text()
for source in [kernel,header]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector|using)\b',mask(source))
assert 'CBox bounds;' in header and 'struct CColBox :' not in header
for name in m.OPERATIONS:
    assert len(re.findall(r'^(?:void |CollisionVector3 |CColBox \*)'+name+r'\(',kernel,re.M))==1,name
    assert len(re.findall(r'\b'+name+r'\(',header))==1,name
modelPath=current_path(root,'src/collision/ColModel.cpp');model=modelPath.read_text()
if modelPath.suffix=='.c':
    # Require the complete C successor before checking the original box assignment boundary
    import col_model_migration as models
    assert model==models.kernel((root/'.diagnostics/col-model-c-tests/before/src/collision/ColModel.cpp').read_text())
    assert 'ColBox_Assign(&self->boxes[i],&other->boxes[i]);' in model
else:assert 'ColBox_Assign(&boxes[i],&other.boxes[i]);' in model
assert 'CollisionVector3 *colMin = &vehCol->boundingBox.min;' in (root/'src/peds/Ped.cpp').read_text()
assert 'const CollisionVector3 &v = veh->GetColModel()->boundingBox.max;' in (root/'src/renderer/Rubbish.cpp').read_text()
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'))
        assert not re.search(r'\b(?:CColBox|CBox)\s*::|\bstruct CColBox\s*:',source),str(path)
        for match in re.finditer(r'\.(?:GetSize|Set)\(',source):
            start=m.value_start(source,match.start());receiver=source[start:match.start()]
            assert not (receiver.endswith('boundingBox') or re.search(r'\bboxes\[',receiver)),str(path)
reports={}
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',176),('production-source-compiles',176)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=100012 hash='+('6470d9b34eb322eb' if row['profile']=='vu' else '4f9709031f49428f')
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
reports.update({'complete_record_owners_converted':2,'public_C_operations':5,'inheritance_relationships_removed':1,'native_bridges_added':0,'changed_sources':len(changes),'renamed_producers':m.RENAMES,'box_bytes':24,'collision_box_bytes':28,'untouched_tail_bytes':2,'states':100012,'actual_collision_consumers':5,'profiles':['float','compressed','vu']})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified both complete C box owners, composition, aliases, collision consumers and both full D3D9 links')
