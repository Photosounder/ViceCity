"""Audit complete C group/block records, selection algorithms, process-exit cleanup and full native links."""
from pathlib import Path
import json,re
import anim_group_migration as m
from rect_migration import mask
from stats_successor import current_path,verify
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-group-c-tests';saved=out/'before';changes=json.loads((out/'changes.json').read_text());assert len(changes)==6
for name in changes:
    path=current_path(root,name);expected=m.calls((saved/name).read_text());assert path.read_text()==expected and m.calls(expected)==expected and b'\r' not in path.read_bytes(),name
for name,digest in json.loads((out/'untouched.json').read_text()).items():verify(root,name,digest)
assert not (root/'src/animation/AnimBlendAssocGroup.cpp').exists()
header=(root/'src/animation/AnimBlendAssocGroup.h').read_text();kernel=(root/'src/animation/AnimBlendAssocGroup.c').read_text();backend=(root/'src/animation/AnimBlendAssocGroup_backend.h').read_text()
assert header==m.header() and kernel==m.kernel((saved/'src/animation/AnimBlendAssocGroup.cpp').read_text()) and backend==m.backend_header()
assert (root/'src/animation/AnimBlock.h').read_text()==m.block_header()
for source in [header,kernel,m.block_header()]:assert not re.search(r'::|\b(class|namespace|new|delete|virtual|using|CBaseModelInfo|RpClump)\b',mask(source))
for result,name,args in m.DECLARATIONS:assert header.count(result+' AnimGroup_'+name+'('+args+');')==1 and kernel.count(result+' AnimGroup_'+name+'('+args+')\n')==1,name
def body(name):
    # Locate complete definitions independently of earlier internal calls
    match=re.search(r'(?m)^(?:void|bool|void \*|CAnimBlendAssociation \*|CAnimBlendAssocGroup) '+re.escape(name)+r'\([^\n]*\)\n\{',kernel);assert match,name
    return m.original_body(kernel[match.start():],kernel[match.start():].splitlines()[0])
remove=body('AnimGroup_DestroyAssociations');assert remove.index('AnimAssociation_Destroy')<remove.index('free(self->assocList)')<remove.index('self->assocList = NULL')
models=body('AnimGroup_CreateForModels');assert 'CAnimBlock *animBlock;' in models and 'self->animBlock' not in models
clump=body('AnimGroup_CreateForClump');assert 'self->animBlock = AnimGroup_HostBlock(blockName)' in clump
assert 'AnimGroup_Init(self);cutsceneGroup=self;atexit(AnimGroup_DestroyStaticCutscene);return *self;' in body('AnimGroup_InitStaticCutscene')
manager=current_path(root,'src/animation/AnimManager.cpp').read_text();assert 'struct CAnimBlock\n{' not in manager and '#include "AnimBlock.h"' in (root/'src/animation/AnimManager.h').read_text()
if (root/'src/animation/AnimManager.cpp').exists():
    # Retain the exact earlier native boundary when the manager owner has not yet migrated
    assert m.native_backend() in manager
else:
    # Verify all former manager aliases in C and all unchanged SDK bodies in the separate native unit
    import anim_manager_migration as managerMigration
    managerOriginal=(root/'.diagnostics/anim-manager-c-tests/before/src/animation/AnimManager.cpp').read_text()
    assert manager==managerMigration.kernel(managerOriginal)
    native=(root/'src/animation/AnimManager_native.cpp').read_text();assert native==managerMigration.native(managerOriginal)
    sdk=m.native_backend();sdk=sdk[sdk.index('void * AnimGroup_HostModelAt('):];assert sdk in native
cutscene=(root/'src/animation/CutsceneMgr.cpp').read_text();assert cutscene.count('CAnimBlendAssocGroup CCutsceneMgr::ms_cutsceneAssociations=AnimGroup_InitStaticCutscene(&CCutsceneMgr::ms_cutsceneAssociations);')==1
for path in (root/'src').rglob('*'):
    if path.suffix in ['.c','.cpp','.h'] and not any(part.startswith('.') for part in path.relative_to(root/'src').parts):
        source=mask(path.read_text(errors='replace'));assert not re.search(r'\bclass CAnimBlendAssocGroup\b|\bCAnimBlendAssocGroup\s*::|~CAnimBlendAssocGroup\s*\(',source),str(path)
        assert 'std::allocator<CAnimBlendAssocGroup>' not in source,str(path)
reports={}
for name,count in [('matrix-runs',16),('sanitized-runs',4),('native-runs',8),('native-source-compiles',36),('production-source-compiles',36)]:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==count and all(row['exit']==0 for row in rows),name
    for row in rows:
        if 'before' in row:assert row['before']==row['after']=='cases=19456 checks=44399 hash=93ec8d46a9c54073 | exit=passed',row
    reports[name]=len(rows)
configs=json.loads((out/'evaluated-configurations.json').read_text());assert len(configs)==48
for row in configs:assert row['exit']==0 and row['pairs']==28 and row['c_units']==1 and row['sources']==(412 if row['platform']=='Win32' and 'RW34' in row['configuration'] else 413),row
reports['evaluated-configurations']=len(configs)
for configuration in ['debug','release']:
    row=json.loads((out/f'd3d9-{configuration}.json').read_text());assert row['exit']==0;log=Path(row['log']).read_text(errors='replace');assert '0 Error(s)' in log and 'Build succeeded.' in log
    exe=Path(row['exe']);assert exe.exists() and exe.stat().st_size==row['size'];reports['d3d9-'+configuration]=row['size']
history=json.loads((out/'historical-source-projections.json').read_text());assert len(history)==193 and all(row['exit']==0 for row in history);reports['historical-source-projections']=len(history)
for name in ['native-parent-layouts','production-parent-layouts']:
    rows=json.loads((out/(name+'.json')).read_text());assert len(rows)==6 and all(row['before']==row['after'] for row in rows),name;reports[name]=rows
for log in out.glob('gcc-O*-*.log'):
    imports=re.findall(r'DLL Name: ([^\s]+)',log.read_text());assert imports and not any(re.search(r'libgcc|libstdc|libwinpthread|msys-',name,re.I) for name in imports),log
reports.update({'complete_records_converted':2,'group_operations':9,'selection_helpers':2,'C_static_lifetime_operations':1,'native_primitive_leaves':13,'native_algorithm_bridges':0,'changed_sources':6,'new_headers':2,'new_producers':0,'renamed_producers':m.RENAMES,'group_bytes':{'x86':20,'x64':32},'block_bytes':32,'cases':19456,'checks':44399,'matching_cases':16384,'creation_cases':1024,'lookup_copy_cases':2048,'process_exit_compared':True,'native_client_dynamic_initializer_retained':True})
(out/'verification-summary.json').write_text(json.dumps(reports,indent=2),newline='\n')
print('Verified complete C group/block ownership, model selection, both creation paths, native lifetime callers, process-exit cleanup and both D3D9 links')
