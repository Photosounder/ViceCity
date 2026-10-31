"""Audit complete C hierarchy ownership, unchanged native layout and full D3D9 links."""
from pathlib import Path
import json,re
import anim_hierarchy_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-hierarchy-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==5
def hierarchy_method(source,name):
    # Select the actual definition when an earlier hierarchy method calls this operation
    match=re.search(r'(?m)^(?:void|bool) '+re.escape(name)+r'\(',source);assert match,name
    return method(source[match.start():],name)
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,name,digest)
assert not (root/'src/animation/AnimBlendHierarchy.cpp').exists()
header=(root/'src/animation/AnimBlendHierarchy.h').read_text();kernel=(root/'src/animation/AnimBlendHierarchy.c').read_text();backend=(root/'src/animation/AnimBlendHierarchy_backend.h').read_text()
assert header==m.header() and kernel==m.kernel((saved/'src/animation/AnimBlendHierarchy.cpp').read_text()) and backend==m.backend_header()
for source in [header,kernel]:assert not re.search(r'::|\b(class|namespace|new|delete|virtual|using)\b',mask(source))
for result,name,args in m.DECLARATIONS:assert header.count(result+' AnimHierarchy_'+name+'('+args+');')==1 and kernel.count(result+' AnimHierarchy_'+name+'('+args+')\n')==1,name
init=hierarchy_method(kernel,'AnimHierarchy_Init');assert 'self->name' not in init and 'self->keepCompressed' not in init
shutdown=hierarchy_method(kernel,'AnimHierarchy_Shutdown');assert shutdown.index('HostRemoveCache(self)')<shutdown.index('AnimHierarchy_RemoveAnimSequences(self)')<shutdown.index('self->totalLength = 0.0f;')<shutdown.index('self->compressed = 0;')
remove=hierarchy_method(kernel,'AnimHierarchy_RemoveAnimSequences');assert remove.index('AnimSequence_Destroy')<remove.index('free(self->sequences)')<remove.index('self->sequences = NULL')<remove.index('self->numSequences = 0')
assert '#define Hierarchy_Max(a,b) ((a)>(b)?(a):(b))' in kernel and 'fmax' not in kernel
assert 're3_assert(#expression,__FILE__,__LINE__,__func__)' in kernel and '#ifndef MASTER' in kernel
assert 'Hierarchy_Assert(self->compressed);' in kernel and 'Hierarchy_Assert(!self->compressed);' in kernel
for name in ['CalcTotalTime','CalcTotalTimeCompressed']:assert '#ifdef FIX_BUGS' in hierarchy_method(kernel,'AnimHierarchy_'+name)
for name in ['Uncompress','RemoveUncompressedData']:assert '#ifdef ANIM_COMPRESSION' in hierarchy_method(kernel,'AnimHierarchy_'+name)
manager=current_path(root,'src/animation/AnimManager.cpp').read_text()
if (root/'src/animation/AnimManager.cpp').exists():
    # Verify the original native manager storage and callback before its owner migration
    assert method(manager,'AnimHierarchy_HostRemoveCache')==method(m.backend(),'AnimHierarchy_HostRemoveCache')
    assert manager.count('CAnimBlendHierarchy CAnimManager::ms_aAnimations[NUMANIMATIONS];')==1
else:
    # Verify the entire C manager projection and the same static zero-initialized hierarchy array
    import anim_manager_migration as managerMigration
    assert manager==managerMigration.kernel((root/'.diagnostics/anim-manager-c-tests/before/src/animation/AnimManager.cpp').read_text())
    assert manager.count('static CAnimBlendHierarchy ms_aAnimations[NUMANIMATIONS];')==1
    assert 'AnimManager_RemoveFromUncompressedCache(self);' in method(manager,'AnimHierarchy_HostRemoveCache')
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bclass CAnimBlendHierarchy\b|\bCAnimBlendHierarchy\s*::',source),str(path)
reports={};gold={'fixed':'241a4b2b9ffab7f6','classic':'f924dec84a1ed982','compression':'2d6f25d559eaa4b5','compression-classic':'8efbbc5204670e11','heap':'e9a3ad1d6aff1dc2','heap-compression':'a1128a4aaf34eed1','master':'a4b7faac4eee4104'}
for name,count in [('master-runs',1),('matrix-runs',24),('sanitized-runs',6),('native-runs',12),('native-source-compiles',42),('production-source-compiles',42),('custom-source-compiles',18),('production-custom-source-compiles',18)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:
            stats='states=25603 checks=221210 hash=' if row['profile'].startswith('heap') else 'states=24579 checks=217114 hash='
            assert row['before']==row['after']==stats+gold[row['profile']],row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(412 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 413),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0
    log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==150 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
lifetimes=json.loads((out/'native-static-lifetime-symbols.json').read_text());assert len(lifetimes)==8 and all(row['exit']==0 for row in lifetimes);reports['native-static-lifetime-symbols']=lifetimes
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name;reports[name]=rows
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
baseline=(out/'custom-heap-baseline-failure.log').read_text();assert all(value in baseline for value in ['LEVEL_INDUSTRIAL','LEVEL_COMMERCIAL','LEVEL_SUBURBAN','RemoveColModelsFromOtherLevels','LoadCollisionFromDatFile'])
for prefix in ['native','production']:
    for phase,tree in [('before',saved),('after',root if prefix=='production' else out/'overlay')]:
        game=(tree/'src/core/Game.cpp').read_text();omitted=method(game,'CGame::DrasticTidyUpMemory');focused=(out/'custom-probes'/prefix/('heap-Game-'+phase+'.cpp')).read_text();assert focused==game.replace(omitted,'') and omitted==method((saved/'src/core/Game.cpp').read_text(),'CGame::DrasticTidyUpMemory')
        assert 'AnimHierarchy_' not in omitted and '->MoveMemory(' not in omitted
reports.update({'complete_record_owners_converted':1,'public_C_operations':11,'external_cache_interfaces':1,'native_algorithm_bridges':0,'changed_sources':len(changes),'new_headers':1,'new_producers':0,'renamed_producers':m.RENAMES,'record_bytes':{'x86':40,'x64':48},'states':{'ordinary':24579,'custom_heap':25603},'time_and_sign_cases':16384,'constructor_and_name_cases':4096,'owned_storage_and_cache_cases':4096,'custom_heap_cases':1024,'static_zero_state_preserved':True,'CXX_dynamic_initializer_removed':True,'indeterminate_constructor_fields_compared':False,'project_assertion_policy_preserved':True,'failed_precondition_cases':2,'optional_heap_Game_full_compile':'Original fails in unchanged DrasticTidyUpMemory; both changed callers compiled with only that identical function omitted'})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C hierarchy ownership, all eleven operations, original layout and collection policy, live cache cleanup and both full D3D9 links')
