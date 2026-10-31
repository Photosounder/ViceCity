"""Stage complete two-stage clump loading against an immutable coherent source tree."""
from pathlib import Path
import hashlib,json,shutil
import clump_read_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/clump-read-c-tests';out.mkdir(exist_ok=True)
saved=out/'before/src'
if not saved.exists():
    # Capture all current source bytes once after the applied C texture reader
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
changes=[];untouched={}
for path in saved.rglob('*'):
    # Reject concurrent edits before recording the exact complete owner boundary
    if path.suffix not in {'.cpp','.c','.h'} or any(part.startswith('.') for part in path.relative_to(saved).parts):continue
    name='src/'+path.relative_to(saved).as_posix();current=root/m.RENAMES.get(name,name)
    assert current.exists() and (current.read_bytes()==path.read_bytes() or current.read_text()==m.calls(path.read_text())),name
    if m.calls(path.read_text())!=path.read_text():changes.append(name)
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Preserve exact original hashes outside the staged clump owner
    target=out/(name+'.json')
    if target.exists():assert json.loads(target.read_text())==value,name
    else:target.write_text(json.dumps(value,indent=2),newline='\n')
if not (out/'overlay/src').exists():shutil.copytree(saved,out/'overlay/src')
for name in changes:
    # Write complete deterministic source projections using LF line endings
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name
    target=out/'overlay'/m.RENAMES.get(name,name)
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
for name,value in [('ClumpRead.h',m.header()),('ClumpRead_backend.h',m.backend_header()),('ClumpRead_native.cpp',m.native())]:
    # Keep actual SDK storage and primitives on the same staged version as the complete C policy
    target=out/'overlay/src/rw'/name;assert m.calls(value)==value,name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
print('Prepared all seven complete C clump-loading operations; production files not modified')
