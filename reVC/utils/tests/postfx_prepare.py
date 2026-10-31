"""Stage the complete post-processing owner against one immutable coherent source tree."""
from pathlib import Path
import hashlib,json,shutil
import postfx_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/postfx-c-tests';out.mkdir(exist_ok=True);saved=out/'before/src'
if not saved.exists():
    # Capture all source bytes once after the applied C texture and palette pools
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
changes=[];untouched={}
for path in saved.rglob('*'):
    # Record the exact whole owner and every affected native client before production edits
    if path.suffix not in {'.cpp','.c','.h'} or any(part.startswith('.') for part in path.relative_to(saved).parts):continue
    name='src/'+path.relative_to(saved).as_posix();current=root/name
    if not current.exists():current=root/m.RENAMES.get(name,name)
    assert current.exists() and (current.read_bytes()==path.read_bytes() or current.read_text()==m.calls(path.read_text())),name
    if m.calls(path.read_text())!=path.read_text():changes.append(name)
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Retain immutable manifests for complete source and historical checks
    target=out/(name+'.json')
    if target.exists():assert json.loads(target.read_text())==value,name
    else:target.write_text(json.dumps(value,indent=2),newline='\n')
if not (out/'overlay/src').exists():shutil.copytree(saved,out/'overlay/src')
for name in changes:
    # Write complete deterministic source projections with LF line endings
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name;target=out/'overlay'/m.RENAMES.get(name,name)
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
extras={'src/extras/PostFX_backend.h':m.backend_header(),'src/extras/PostFX_native.cpp':m.native(),'src/extras/PostFX_bindings.c':m.bindings()}
for name,value in extras.items():
    # Keep shared C constants and the SDK primitive boundary coherent with the staged C owner
    target=out/'overlay'/name;assert m.calls(value)==value,name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
print('Staged all 13 complete C post-processing operations across',len(changes),'changed paths; production unchanged')
