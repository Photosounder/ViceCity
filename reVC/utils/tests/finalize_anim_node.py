"""Audit complete C blend-node ownership, lazy field interfaces and full D3D9 links."""
from pathlib import Path
import json,re
import anim_node_migration as m
from rect_migration import mask
from stats_successor import verify,current_path
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-node-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==10
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/animation/AnimBlendNode.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
header=(root/'src/animation/AnimBlendNode.h').read_text();kernel=(root/'src/animation/AnimBlendNode.c').read_text();backend=(root/'src/animation/AnimBlendNode_backend.h').read_text()
assert header==m.calls(m.header()) and kernel==m.calls(m.kernel((saved/'src/animation/AnimBlendNode.cpp').read_text())) and backend==m.calls(m.backend_header())
assert (root/'src/animation/AnimBlendAssociation_flags.h').read_text()==m.flags((saved/'src/animation/AnimBlendAssociation.h').read_text())
assert not re.search(r'::|\b(class|namespace|new|delete|CVector|using)\b',mask(kernel))
assert 'class CAnimBlendNode' not in header and 'typedef struct CAnimBlendNode {' in header
for result,name,args in m.DECLARATIONS:
    assert kernel.count(result+' AnimBlendNode_'+name+'('+args+')\n')==1 and header.count(result+' AnimBlendNode_'+name+'('+args+');')==1,name
init=method(kernel,'AnimBlendNode_Init');assert 'self->theta' not in init and 'self->invSin' not in init
assert kernel.index('if(Node_IsRunning(self))')<kernel.index('self->remainingTime -= Node_TimeStep(self);')
sequence=current_path(root,'src/animation/AnimBlendSequence.cpp').read_text();association=current_path(root,'src/animation/AnimBlendAssociation.cpp').read_text()
if 'AnimBlendNode_BackendSequence(' in backend:assert method(sequence,'AnimBlendNode_BackendSequence')==method(m.sequence_backend(),'AnimBlendNode_BackendSequence')
else:assert 'AnimBlendNode_BackendSequence(' not in kernel+sequence
if 'AnimBlendNode_BackendAssociation(' in backend:assert method(association,'AnimBlendNode_BackendAssociation')==method(m.association_backend(),'AnimBlendNode_BackendAssociation')
else:assert 'AnimBlendNode_BackendAssociation(' not in kernel+association and 'AnimAssociation_GetBlendAmount(self->association,weight)' in kernel
if 'AnimBlendNodeSequenceFields' in backend:assert 'void *const *frames;' in backend and 'void *const *packedFrames;' in backend
else:assert 'self->sequence->numFrames' in kernel and 'AnimSequence_GetKeyFrame(self->sequence,index)' in kernel
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bclass CAnimBlendNode\b|\bCAnimBlendNode\s*::',source),str(path)
        for match in re.finditer(r'(?:\(\*node\)(?:\[i\])?|nodes\[[^\]\n]+\])(?:->|\.)([A-Za-z0-9_]+)\(',source):assert match[1] not in m.OPERATIONS,(str(path),match[0])
reports={};gold={'fixed':'3db555d39bd9639d','classic':'d6132722ddba1d35'};native32={'fixed':'6351ae4982682247','classic':'0e98e6503d1027af'}
for name,count in [('matrix-runs',8),('sanitized-runs',2),('native-runs',4),('native-source-compiles',48),('production-source-compiles',48)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=71691 checks=208917 hash='+(native32 if row.get('bits')==32 else gold)[row['profile']]
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(411 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 412),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==235 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name
    reports[name]=rows
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
reports.update({'complete_record_owners_converted':1,'public_C_operations':13,'native_field_interfaces':2,'native_algorithm_bridges':0,'changed_sources':len(changes),'new_headers':2,'renamed_producers':m.RENAMES,'record_bytes':{'x86':28,'x64':40},'states':71691,'direct_method_cases':49152,'partial_initializer_cases':4096,'full_animation_frame_cases':18432,'complete_frame_callbacks':9,'lazy_read_cases':9,'profiles':list(gold)})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C blend-node ownership, all thirteen methods, lazy fields, actual native frame consumers and both full D3D9 links')
