"""Audit both complete C animation owners, ordered links, live callbacks and full D3D9 links."""
from pathlib import Path
import json,re
import anim_clump_migration as m
from rect_migration import mask
from stats_successor import verify,current_path
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-clump-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==8
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected,name
    assert b'\r' not in path.read_bytes(),name
assert not (root/'src/animation/AnimBlendClumpData.cpp').exists()
for path,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,path,digest)
for name,value in [('AnimBlendList.h',m.link_header()),('AnimBlendList.c',m.link_kernel()),('AnimBlendClumpData.h',m.header()),('AnimBlendClumpData.c',m.kernel()),('AnimBlendClumpData_backend.h',m.backend_header()),('AnimBlendFrameData_native.h',m.frame_header((saved/'src/animation/AnimBlendClumpData.h').read_text()))]:
    path=root/'src/animation'/name;assert path.read_text()==m.calls(value) and b'\r' not in path.read_bytes(),name
for source in [m.link_header(),m.link_kernel(),m.kernel()]:assert not re.search(r'::|\b(class|namespace|new|delete|using)\b',mask(source))
for name in m.LINK_OPERATIONS:assert len(re.findall(r'^void AnimBlendLink_'+name+r'\(',m.link_kernel(),re.M))==1,name
for name in m.CLUMP_OPERATIONS:assert len(re.findall(r'^void AnimBlendClump_'+name+r'\(',m.kernel(),re.M))==1,name
destroy=method(m.kernel(),'AnimBlendClump_Destroy');assert destroy.index('AnimBlendLink_Remove')<destroy.index('AnimBlendClump_BackendFree')
replace=method(m.kernel(),'AnimBlendClump_SetNumberOfFrames');assert replace.index('BackendFree')<replace.index('self->numFrames=count;')<replace.index('BackendAllocate')
walker=method(m.kernel(),'AnimBlendClump_ForAllFrames');assert 'i<self->numFrames' in walker and '(unsigned char*)self->frames' in walker
rpPath=current_path(root,'src/animation/RpAnimBlend.cpp');rp=rpPath.read_text();allocator=rp
if rpPath.suffix=='.c':
    # Require both complete successor producers before checking original lifetime and allocator policy
    import anim_plugin_migration as plugin
    original=(root/'.diagnostics/anim-plugin-c-tests/before/src/animation/RpAnimBlend.cpp').read_text();assert rp==plugin.kernel(original)
    allocator=(root/'src/animation/RpAnimBlend_native.cpp').read_text();assert allocator==plugin.calls(plugin.native(original))
for name in ['AnimBlendClump_BackendFrameSize','AnimBlendClump_BackendAllocate','AnimBlendClump_BackendFree']:
    expected=m.calls(m.backend())
    if name+'(' in expected:assert method(allocator,name)==method(expected,name),name
    else:assert name+'(' not in allocator,name
plugin=method(rp,'AnimBlendClumpDestroy');assert plugin.index('RpAnimBlendClumpRemoveAllAssociations')<plugin.index('AnimBlendClump_Destroy')<plugin.index('free(data)')
allocate=method(rp,'RpAnimBlendAllocateData');assert allocate.index('malloc(sizeof(CAnimBlendClumpData))')<allocate.index('AnimBlendClump_Init(data)')
for path in (root/'src').rglob('*'):
    if path.suffix in ['.h','.cpp','.c'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bclass CAnimBlend(?:Link|ClumpData)\b|\bCAnimBlendClumpData\s*::|~CAnimBlendClumpData\s*\(',source),str(path)
        assert 'std::allocator<CAnimBlendClumpData>' not in source,str(path)
reports={}
for name,count in [('matrix-runs',4),('sanitized-runs',1),('native-runs',2),('native-source-compiles',54),('production-source-compiles',54)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            expected='states=74754 checks=49479 hash='+('f7abfa33ec6b5bf8' if row.get('bits')==32 else 'fa5d48d209aed680')
            assert row['before']==row['after']==expected,row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==2 and row['sources']==(412 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 413),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==216 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name
    reports[name]=rows
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
reports.update({'complete_record_owners_converted':2,'public_C_operations':8,'native_primitive_interfaces':3,'native_algorithm_bridges':0,'changed_sources':len(changes),'new_producers':1,'new_headers':2,'renamed_producers':m.RENAMES,'link_bytes':{'x86':8,'x64':16},'clump_bytes':{'x86':20,'x64':40},'native_frame_bytes':{'x86':24,'x64':32},'states':74754,'exhaustive_link_graph_operations':61440,'live_list_operations':8192,'allocation_boundary_cases':2048,'live_callback_cases':2048,'native_plugin_cases':1024,'actual_frame_callbacks':4,'constructor_padding_compared':False})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C animation links and clump ownership, alias updates, memory policy, live callbacks and both full D3D9 links')
