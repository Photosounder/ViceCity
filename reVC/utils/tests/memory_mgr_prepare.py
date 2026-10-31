"""Stage the complete memory manager owner and native primitives."""
from pathlib import Path
import hashlib,json,shutil
import memory_mgr_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/memory-mgr-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    # Inventory exact owner and caller changes before capturing a coherent immutable source tree
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix not in {'.cpp','.c','.h'} or any(part.startswith('.') for part in path.relative_to(root/'src').parts):continue
        name=path.relative_to(root).as_posix();original=path.read_text()
        if m.calls(original)!=original:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    shutil.copytree(root/'src',out/'before/src',ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
    for name,value in [('changes',changes),('untouched',untouched)]:
        # Preserve exact original byte hashes outside the audited conversion boundary
        (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
assert len(json.loads((out/'changes.json').read_text()))>=2
for name in json.loads((out/'changes.json').read_text()):
    # Write the entire deterministic C projection into the isolated candidate tree
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name
    (out/'overlay'/m.RENAMES.get(name,name)).write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
original=(out/'before/src/rw/MemoryMgr.cpp').read_text()
for name,value in [('MemoryMgr_backend.h',m.backend_header()),('MemoryMgr_native.cpp',m.native(original)),('MemoryMgr_native.h',m.native_header((out/'before/src/rw/MemoryMgr.h').read_text()))]:
    # Keep native SDK primitives and the C owner on the same coherent staged version
    (out/'overlay/src/rw'/name).write_text(value,newline='\n')
print('Staged seven complete C memory operations and native allocator primitives with',len(json.loads((out/'changes.json').read_text())),'changed paths')
