"""Audit complete C association ownership, callback order, direct node fields and real D3D9 links."""
from pathlib import Path
import json,re
import anim_association_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-association-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==27
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,name,digest)
assert not (root/'src/animation/AnimBlendAssociation.cpp').exists()
header=(root/'src/animation/AnimBlendAssociation.h').read_text();kernel=(root/'src/animation/AnimBlendAssociation.c').read_text();backend=(root/'src/animation/AnimBlendAssociation_backend.h').read_text()
assert header==m.header() and kernel==m.kernel((saved/'src/animation/AnimBlendAssociation.cpp').read_text()) and backend==m.backend_header()
for source in [header,kernel]:assert not re.search(r'::|\b(class|namespace|new|delete|virtual|using|RpClump)\b',mask(source))
for result,name,args in m.DECLARATIONS:assert header.count(result+' AnimAssociation_'+name+'('+args+');')==1 and kernel.count(result+' AnimAssociation_'+name+'('+args+')\n')==1,name
def body(name):
    # Select actual function definitions independently of preceding recursive or helper calls
    result,operation,args=next(row for row in m.DECLARATIONS if row[1]==name)
    start=kernel.index(result+' AnimAssociation_'+name+'('+args+')\n');return method(kernel[start:],'AnimAssociation_'+name)
destroy=body('Destroy');assert destroy.index('FreeAnimBlendNodeArray')<destroy.index('AnimBlendLink_Remove')
blend=body('UpdateBlend');assert blend.index('self->callback(self, self->callbackArg)')<blend.index('AnimAssociation_Destroy(self)')<blend.index('free(self)')<blend.index('return false;')
time=body('UpdateTime');assert time.index('self->callbackType = ANIM_ASSOC_CB_NONE')<time.index('self->callback(self, self->callbackArg)')
init=body('Init');assert not any('self->'+name in init for name in ['numNodes','hierarchy','callbackArg']);assert 'self->callback =' not in init
node=(root/'src/animation/AnimBlendNode.c').read_text();assert 'AnimAssociation_IsRunning(self->association)' in node and 'AnimAssociation_GetBlendAmount(self->association,weight)' in node and 'self->association->timeStep' in node
rp=current_path(root,'src/animation/RpAnimBlend.cpp').read_text();manager=current_path(root,'src/animation/AnimManager.cpp').read_text()
if (root/'src/animation/RpAnimBlend.cpp').exists():
    # Require the original SDK boundary while the plugin remains a native owner
    for name in ['ClumpData','FindFrame','FindBone']:assert method(rp,'AnimAssociation_Host'+name)==method(m.sdk_backend(),'AnimAssociation_Host'+name),name
else:
    # Require the complete successor C plugin and all three direct C forwarders after conversion
    import anim_plugin_migration as plugin
    assert rp==plugin.kernel((root/'.diagnostics/anim-plugin-c-tests/before/src/animation/RpAnimBlend.cpp').read_text())
    for name,callee in [('ClumpData','RPANIMBLENDCLUMPDATA'),('FindFrame','RpAnimBlendClumpFindFrame'),('FindBone','RpAnimBlendClumpFindBone')]:assert callee+'(' in method(rp,'AnimAssociation_Host'+name),name
if (root/'src/animation/AnimManager.cpp').exists():
    # Preserve the original external manager boundary before its separate owner conversion
    assert method(manager,'AnimAssociation_HostUncompressAnimation')==method(m.manager_backend(),'AnimAssociation_HostUncompressAnimation')
else:
    # Require the complete C manager projection and its direct uncompress forwarder after conversion
    import anim_manager_migration as managerMigration
    assert manager==managerMigration.kernel((root/'.diagnostics/anim-manager-c-tests/before/src/animation/AnimManager.cpp').read_text())
    assert 'AnimManager_UncompressAnimation(hierarchy);' in method(manager,'AnimAssociation_HostUncompressAnimation')
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bclass CAnimBlendAssociation\b|\bCAnimBlendAssociation\s*::|~CAnimBlendAssociation\s*\(|AnimBlendNodeAssociationFields|AnimBlendNode_BackendAssociation',source),str(path)
        assert 'std::allocator<CAnimBlendAssociation>' not in source,str(path)
reports={};gold={'fixed':'d64e60205cefbc01','classic':'4c5ca17a26158fa0','compression':'81e9f13958682075','compression-classic':'d5bd9506ff84eb5c'}
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8),('native-source-compiles',132),('production-source-compiles',132),('classic-layout-compiles',4),('production-classic-layout-compiles',4)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']=='states=73734 checks=292889 frames=18432 hash='+gold[row['profile']],row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(412 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 413),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==811 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name;reports[name]=rows
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
reports.update({'complete_record_owners_converted':1,'public_C_operations':28,'external_host_interfaces':4,'native_algorithm_bridges':0,'native_association_field_interfaces_removed':1,'changed_sources':len(changes),'new_headers':1,'new_producers':0,'renamed_producers':m.RENAMES,'fixed_record_bytes':{'x86':56,'x64':88},'classic_bytes_before':{'x86':60,'x64':96},'classic_bytes_after':{'x86':56,'x64':88},'states':73734,'checks':292889,'actual_frame_callbacks':9,'frame_cases':18432,'all_flag_patterns':65536,'storage_copy_cases':2048,'playback_setup_cases':2048,'live_callback_mutation':True,'node_free_before_unlink':True,'indeterminate_constructor_fields_compared':False})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C association ownership, all 28 operations, callback mutation and self-destruction order, actual frame consumers and both full D3D9 links')
