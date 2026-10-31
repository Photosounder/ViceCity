"""Stage the complete texture reader and conversion policy with one coherent original baseline."""
from pathlib import Path
import hashlib,json,shutil
import tex_read_migration as m
from tex_read_schema import HOST

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/tex-read-c-tests';out.mkdir(exist_ok=True)
saved=out/'before/src'
if not saved.exists():
    # Capture current source bytes once after the applied C TXD owner
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
changes=[];untouched={}
for path in saved.rglob('*'):
    # Reject concurrent edits before recording the exact whole-source conversion boundary
    if path.suffix not in {'.cpp','.c','.h'} or any(part.startswith('.') for part in path.relative_to(saved).parts):continue
    name='src/'+path.relative_to(saved).as_posix();current=root/m.RENAMES.get(name,name)
    assert current.exists() and (current.read_bytes()==path.read_bytes() or current.read_text()==m.calls(path.read_text())),name
    if m.calls(path.read_text())!=path.read_text():changes.append(name)
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Preserve exact original bytes outside the complete texture owner boundary
    target=out/(name+'.json')
    if target.exists():assert json.loads(target.read_text())==value,name
    else:target.write_text(json.dumps(value,indent=2),newline='\n')
if not (out/'overlay/src').exists():shutil.copytree(saved,out/'overlay/src')
for name in changes:
    # Write whole deterministic source projections and preserve LF line endings
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name
    target=out/'overlay'/m.RENAMES.get(name,name)
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
new=[('src/rw/TexRead.h',m.header()),('src/rw/TexRead_backend.h',m.backend_header()),('src/rw/TexRead_native.cpp',m.native())]
new.append(('src/rw/TexRead_bindings.c',m.bindings((out/'before/src/rw/TxdStore_native.cpp').read_text())))
for name,original,kind in [('src/core/CdStream_constants.h','src/core/CdStream.h','cd'),('src/skel/crossplatform_constants.h','src/skel/crossplatform.h','os'),('src/core/Streaming_constants.h','src/core/Streaming.h','stream')]:
    # Share existing C-compatible constants rather than copying SDK values into the C algorithm
    new.append((name,m.constants((out/'before'/original).read_text(),kind)))
for name,value in new:
    # Keep actual SDK primitives and shared declarations on the same staged version
    target=out/'overlay'/name;assert m.calls(value)==value,name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
print('Prepared complete LIBRW and legacy texture policy with',len(HOST)+2,'external primitives; production files not modified')
