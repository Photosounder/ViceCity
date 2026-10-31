"""Capture a coherent immutable TXD owner baseline and prepare its complete C producer."""
from pathlib import Path
import hashlib,json,shutil
import txd_store_migration as m

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/txd-store-c-tests'
out.mkdir(exist_ok=True)
saved=out/'before/src'
if not saved.exists():
    # Capture all current owner and consumer bytes once before the TXD migration begins
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
original_header=(saved/'rw/TxdStore.h').read_text()
frozen=root/'utils/tests/txd_store_original_header.py'
if not frozen.exists():
    # Keep exact original inline slot lookup declarations available to complete source projections
    frozen.write_text('"""Immutable original TXD declarations for complete owner projections."""\nORIGINAL_HEADER='+repr(original_header)+'\n',newline='\n')
else:
    # Reject accidental replacement of the captured inline original
    from txd_store_original_header import ORIGINAL_HEADER
    assert ORIGINAL_HEADER==original_header
changes=[];untouched={}
for path in saved.rglob('*'):
    # Inventory exact source projections before changing any production source
    if path.suffix not in {'.c','.cpp','.h'} or any(part.startswith('.') for part in path.relative_to(saved).parts):continue
    name='src/'+path.relative_to(saved).as_posix()
    current=root/m.RENAMES.get(name,name)
    assert current.exists() and (current.read_bytes()==path.read_bytes() or current.read_text()==m.calls(path.read_text())),name
    if m.calls(path.read_text())!=path.read_text():changes.append(name)
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Save immutable source boundaries while the candidate is expanded in later steps
    target=out/(name+'.json')
    if target.exists():assert json.loads(target.read_text())==value,name
    else:target.write_text(json.dumps(value,indent=2),newline='\n')
if not (out/'overlay/src').exists():shutil.copytree(saved,out/'overlay/src')
for name in changes:
    # Stage the complete C owner and self-contained API without applying partial production changes
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name
    target=out/'overlay'/m.RENAMES.get(name,name)
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
originals=[(out/'before'/name).read_text() for name in m.BINDING_PATHS]
for name,value in [('TxdStore_backend.h',m.backend_header()),('TxdStore_native.cpp',m.native()),('TxdStore_bindings.h',m.bindings_header(originals)),('TxdStore_bindings.c',m.bindings(originals))]:
    # Keep the C owner and actual SDK boundary on the same staged version
    target=out/'overlay/src/rw'/name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
print('Staged 21 complete C TXD operations and 12 external owner primitives; production unchanged')
