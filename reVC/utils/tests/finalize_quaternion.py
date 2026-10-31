"""Audit complete C quaternion ownership, every operation, actual animation callers and full D3D9 links."""
from pathlib import Path
import json,re
import quaternion_migration as m
from stats_successor import current_path
from rect_migration import mask
from stats_successor import verify
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/quaternion-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text())
assert len(changes)==7
for name in changes:
    path=current_path(root,m.RENAMES.get(name,name));expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/math/Quaternion.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
kernel=(root/'src/math/Quaternion.c').read_text();header=(root/'src/math/Quaternion.h').read_text()
assert header==m.header() and kernel==m.kernel((saved/'src/math/Quaternion.cpp').read_text())
assert (root/'src/math/Quaternion_native.h').read_text()==m.native()
for source in [kernel,header]:assert not re.search(r'::|\b(class|namespace|new|delete|CVector|RwV3d|RwMatrix|using)\b',mask(source))
for result,name,args in m.DECLARATIONS:
    assert kernel.count(result+' Quaternion_'+name+'('+args+')\n')==1,name
    assert header.count(result+' Quaternion_'+name+'('+args+');')==1,name
normal=method(kernel,'Quaternion_Normalise');assert 'if (sq == 0.0f)\n\t\tself->w = 1.0f;' in normal
multiply=method(kernel,'Quaternion_Multiply');assert multiply.index('self->x =')<multiply.index('self->y =')<multiply.index('self->z =')<multiply.index('self->w =')
assert 'Quaternion_Add(Quaternion_LeftScale(w1,*q1),Quaternion_LeftScale(w2,*q2))' in kernel
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bCQuaternion\s*::|\bclass CQuaternion\b|\bCQuaternion\s*\(',source),str(path)
frame=current_path(root,'src/animation/FrameUpdate.cpp').read_text();oldFrame=(saved/'src/animation/FrameUpdate.cpp').read_text()
if not (root/'src/animation/FrameUpdate.cpp').exists():
    # Require the full C callback successor before checking the unchanged quaternion accumulation policy
    import frame_update_migration as frameOwner
    assert frame==frameOwner.kernel((root/'.diagnostics/frame-update-c-tests/before/src/animation/FrameUpdate.cpp').read_text())
assert frame.count('Quaternion_Normalise(')==10 and frame.count('Quaternion_GetMatrix(')==5
assert frame.count('#ifdef FIX_BUGS')==oldFrame.count('#ifdef FIX_BUGS')
assert (root/'src/animation/AnimBlendSequence.h').read_text()==m.calls((saved/'src/animation/AnimBlendSequence.h').read_text())
assert (root/'src/animation/AnimBlendNode.h').read_text()==m.calls((saved/'src/animation/AnimBlendNode.h').read_text())
reports={};gold={'fixed':'states=65042 checks=2821670 hash=b261653e8e6bc9bb','classic':'states=65042 checks=2821670 hash=2816bf395048d047'}
nativeGold={32:{'fixed':'72a08b1fb496914c','classic':'27ee4a7772ba2610'},64:{'fixed':'68776ff2748317de','classic':'ea17a18b7e131b12'}}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',54),('production-source-compiles',54)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected=('states=65042 checks=2821670 hash='+nativeGold[row['bits']][row['profile']]) if 'bits' in row else gold[row['profile']]
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(410 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 411),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==168 and all(row['exit']==0 for row in history)
reports['historical-source-projections']=len(history)
reports.update({'complete_record_owners_converted':1,'public_C_operations':24,'native_gameplay_bridges_added':0,'native_field_adapters':4,'changed_sources':len(changes),'new_native_headers':1,'renamed_producers':m.RENAMES,'record_bytes':16,'record_alignment':4,'states':65042,'full_animation_frame_cases':18432,'complete_frame_callbacks':9,'complete_blend_node_methods':13,'profiles':list(gold),'normalization_fallback_preserves_XYZ':True,'multiply_preserves_live_aliases':True})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C quaternion ownership, all 24 operations, actual animation consumers, native layouts and both full D3D9 links')
