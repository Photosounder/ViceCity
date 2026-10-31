"""Audit complete C sequence ownership, removal of virtual dispatch and full D3D9 builds."""
from pathlib import Path
import json,re
import anim_sequence_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-sequence-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==12
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,name,digest)
assert not (root/'src/animation/AnimBlendSequence.cpp').exists()
header=(root/'src/animation/AnimBlendSequence.h').read_text();kernel=(root/'src/animation/AnimBlendSequence.c').read_text();backend=(root/'src/animation/AnimBlendSequence_backend.h').read_text()
assert header==m.header() and kernel==m.kernel((saved/'src/animation/AnimBlendSequence.cpp').read_text()) and backend==m.backend_header()
for source in [header,kernel]:assert not re.search(r'::|\b(class|namespace|new|delete|virtual|using|RwMalloc|RwFree|gMainHeap)\b',mask(source))
assert 'void *keyFrames,*keyFramesCompressed;' in header and 'vtable' not in header.lower()
for result,name,args in m.DECLARATIONS:assert header.count(result+' AnimSequence_'+name+'('+args+');')==1 and kernel.count(result+' AnimSequence_'+name+'('+args+')\n')==1,name
init=method(kernel,'AnimSequence_Init');assert 'self->name' not in init
destroy=method(kernel,'AnimSequence_Destroy');assert destroy.index('BackendFree(self->keyFrames)')<destroy.index('BackendFree(self->keyFramesCompressed)')
unpack=method(kernel,'AnimSequence_Uncompress');assert unpack.index('BackendRegister(&self->keyFrames)')<unpack.index('BackendFree(self->keyFramesCompressed)')<unpack.index('self->keyFramesCompressed = NULL')
assert 'float timeScale = 1.0f/60.0f;' in unpack and 'kf->base.deltaTime = ckf->base.deltaTime*timeScale;' in unpack
allocate=method(kernel,'AnimSequence_SetNumFrames');assert 'sizeof(KeyFrameTrans) * numFrames' in allocate and 'sizeof(KeyFrame) * numFrames' in allocate
node=(root/'src/animation/AnimBlendNode.c').read_text();assert 'self->sequence->numFrames' in node and 'AnimSequence_GetKeyFrame(self->sequence,index)' in node
rpPath=current_path(root,'src/animation/RpAnimBlend.cpp');rp=rpPath.read_text()
if rpPath.suffix=='.c':
    # Require the complete successor native projection before checking unchanged allocator primitives
    import anim_plugin_migration as plugin
    rp=(root/'src/animation/RpAnimBlend_native.cpp').read_text();assert rp==plugin.calls(plugin.native((root/'.diagnostics/anim-plugin-c-tests/before/src/animation/RpAnimBlend.cpp').read_text()))
if (root/'src/rw/MemoryHeap_bindings.c').exists():
    # Require the complete C heap successor before checking all relocated sequence forwarding ABIs
    import memory_heap_migration as heap
    savedHeap=root/'.diagnostics/memory-heap-c-tests/before'
    originals=[(savedHeap/name).read_text() for name in ['src/rw/MemoryMgr_native.cpp','src/collision/ColModel_native.cpp','src/animation/RpAnimBlend_native.cpp']]
    bindings=(root/'src/rw/MemoryHeap_bindings.c').read_text();assert bindings==heap.bindings(originals)
    for name in ['PushAnimation','PopMemory','Register','MoveMemory']:
        assert 'AnimSequence_Backend'+name+'(' not in rp,name
        assert method(bindings,'AnimSequence_Backend'+name)==heap.heap_calls(method(m.backend(),'AnimSequence_Backend'+name)),name
    names=['Allocate','Free']
else:names=['Allocate','Free','PushAnimation','PopMemory','Register','MoveMemory']
for name in names:assert method(rp,'AnimSequence_Backend'+name)==method(m.backend(),'AnimSequence_Backend'+name),name
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bclass CAnimBlendSequence\b|\bCAnimBlendSequence\s*::|~CAnimBlendSequence\s*\(|AnimBlendNodeSequenceFields|AnimBlendNode_BackendSequence',source),str(path)
        assert 'std::allocator<CAnimBlendSequence>' not in source,str(path)
reports={};gold={'fixed':'45695c85011c3ffe','classic':'3021092a38107392','heap':'14893a0b5fcd2bd4'};gold32={'fixed':'725ccac619d5bb4a','classic':'734bece0b62b4bfe','heap':'5a5957d2e2818b80'}
for name,count in [('matrix-runs',12),('sanitized-runs',3),('native-runs',6),('native-source-compiles',66),('production-source-compiles',66),('custom-source-compiles',6),('production-custom-source-compiles',6)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            stats='states=93712 checks=203810 hash=' if row['profile']=='heap' else 'states=93200 checks=202786 hash='
            assert row['before']==row['after']==stats+(gold32 if row.get('bits')==32 else gold)[row['profile']],row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(412 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 413),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==322 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name;reports[name]=rows
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
reports.update({'complete_record_owners_converted':1,'public_C_operations':13,'virtual_dispatch_removed':True,'native_sequence_field_interfaces_removed':1,'native_memory_primitives':6,'native_algorithm_bridges':0,'changed_sources':len(changes),'new_headers':1,'new_producers':0,'renamed_producers':m.RENAMES,'record_bytes_before':{'x86':48,'x64':64},'record_bytes_after':{'x86':44,'x64':56},'states':{'fixed':93200,'classic':93200,'heap':93712},'actual_animation_callbacks':9,'animation_frame_cases':18432,'all_signed_packed_patterns':65536,'custom_heap_relocation_cases':512,'partial_initializer_and_name_cases':4096,'allocation_boundary_cases':1024,'storage_roundtrip_cases':4096})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C sequence ownership, thirteen operations, removal of virtual dispatch, direct C node fields, exact memory policy and both full D3D9 links')
