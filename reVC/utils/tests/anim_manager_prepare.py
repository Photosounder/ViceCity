"""Stage the complete manager owner and all diagnosed consumers in one coherent source tree."""
from pathlib import Path
import hashlib,json,shutil
import anim_manager_migration as m
root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/anim-manager-c-tests'
out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    # Preserve partial saved originals only after proving they still match production
    for existing in (out/'before/src').rglob('*'):
        if existing.is_file():
            # Reject a mixed baseline instead of silently replacing earlier saved originals
            assert existing.read_bytes()==(root/'src'/existing.relative_to(out/'before/src')).read_bytes(),existing
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        # Inventory all changed consumers before writing an immutable source snapshot
        if path.suffix not in {'.cpp','.c','.h'} or any(p.startswith('.') for p in path.relative_to(root/'src').parts):continue
        name=path.relative_to(root).as_posix();source=path.read_text()
        if m.calls(source)!=source:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    shutil.copytree(root/'src',out/'before/src',dirs_exist_ok=True,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
    for name,value in [('changes',changes),('untouched',untouched)]:
        # Record exactly which source paths belong to this conversion stage
        (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
for name in json.loads((out/'changes.json').read_text()):
    # Apply the full deterministic projection only to the isolated overlay
    value=m.calls((out/'before'/name).read_text())
    assert m.calls(value)==value,name
    (out/'overlay'/m.RENAMES.get(name,name)).write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
original=(out/'before/src/animation/AnimManager.cpp').read_text()
for name,value in [('AnimManagerData.h',m.data_header()),('AnimManager_backend.h',m.backend_header()),('AnimManager_native.cpp',m.native(original))]:
    # Keep newly introduced native leaves and C records in the same staged source tree
    (out/'overlay/src/animation'/name).write_text(value,newline='\n')
print('Staged complete C manager and',len(json.loads((out/'changes.json').read_text())),'changed source/header paths')
