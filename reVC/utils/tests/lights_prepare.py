"""Stage the complete lighting conversion against one immutable coherent source tree."""
from pathlib import Path
import hashlib,json,shutil
import lights_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lights-c-tests';out.mkdir(exist_ok=True)
saved=out/'before/src'
if not saved.exists():
    # Capture source bytes once without interfering with the independently cleaned build folder
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
changes=[];untouched={}
for path in saved.rglob('*'):
    # Record the exact complete owner and all affected clients before any production edits
    if path.suffix not in {'.cpp','.c','.h'} or any(part.startswith('.') for part in path.relative_to(saved).parts):continue
    name='src/'+path.relative_to(saved).as_posix();current=root/name
    if not current.exists():current=root/m.RENAMES.get(name,name)
    assert current.exists() and (current.read_bytes()==path.read_bytes() or current.read_text()==m.calls(path.read_text())),name
    if m.calls(path.read_text())!=path.read_text():changes.append(name)
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Retain immutable source manifests for whole-owner and historical checks
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
for name,value in [('Lights_backend.h',m.backend_header()),('Lights_native.cpp',m.native())]:
    # Keep the native SDK boundary coherent with the complete C owner
    target=out/'overlay/src/rw'/name;assert m.calls(value)==value,name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
print('Staged',len(changes),'changed source paths and all 19 original lighting operations; production unchanged')
