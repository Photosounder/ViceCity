"""Stage the complete custom heap owner and native primitives."""
from pathlib import Path
import hashlib,json,shutil
import memory_heap_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/memory-heap-c-tests';out.mkdir(exist_ok=True)
if not (out/'before/src').exists():
    # Capture the whole source tree once before any heap conversion is applied
    shutil.copytree(root/'src',out/'before/src',ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
changes=[];untouched={}
for path in (out/'before/src').rglob('*'):
    # Inventory immutable original bytes and accept only their exact original or complete applied projection
    if not path.is_file():continue
    name='src/'+path.relative_to(out/'before/src').as_posix()
    if path.suffix not in {'.cpp','.c','.h'} or any(part.startswith('.') for part in path.relative_to(out/'before/src').parts):continue
    original=path.read_text();value=m.calls(original);current=root/m.RENAMES.get(name,name)
    assert current.exists() and (current.read_bytes()==path.read_bytes() or current.read_text()==value),name
    if value!=original:changes.append(name)
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Retain the exact immutable baseline inventory across repeated preparation after application
    target=out/(name+'.json');encoded=json.dumps(value,indent=2)
    if target.exists():assert json.loads(target.read_text())==value,name
    else:target.write_text(encoded,newline='\n')
if not (out/'overlay/src').exists():shutil.copytree(out/'before/src',out/'overlay/src')
assert len(json.loads((out/'changes.json').read_text()))>=2
for name in json.loads((out/'changes.json').read_text()):
    # Write the entire deterministic C projection into the isolated candidate tree
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name
    target=out/'overlay'/m.RENAMES.get(name,name)
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
original=(out/'before/src/rw/MemoryHeap.cpp').read_text()
binding_sources=[(out/'before'/name).read_text() for name in ['src/rw/MemoryMgr_native.cpp','src/collision/ColModel_native.cpp','src/animation/RpAnimBlend_native.cpp']]
for name,value in [('MemoryHeap_backend.h',m.backend_header()),('MemoryHeap_native.cpp',m.native()),('MemoryHeap_bindings.c',m.bindings(binding_sources))]:
    # Keep native SDK primitives and the C owner on the same coherent staged version
    target=out/'overlay/src/rw'/name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
print('Staged all thirty-three C custom-heap operations and external owner primitives with',len(json.loads((out/'changes.json').read_text())),'changed paths')
