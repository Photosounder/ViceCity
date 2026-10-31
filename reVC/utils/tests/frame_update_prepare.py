"""Stage all complete frame callbacks, C dispatch data and native SDK primitives coherently."""
from pathlib import Path
import hashlib,json,shutil
import frame_update_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/frame-update-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    # Capture a complete original tree before writing any candidate callback or header
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix not in {'.c','.cpp','.h'} or any(part.startswith('.') for part in path.relative_to(root/'src').parts):continue
        name=path.relative_to(root).as_posix();original=path.read_text()
        if m.calls(original)!=original:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    assert len(changes)==2,changes
    shutil.copytree(root/'src',out/'before/src',ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
    for name,value in [('changes',changes),('untouched',untouched)]:
        # Record exact changed paths and immutable original byte digests
        (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
for name in json.loads((out/'changes.json').read_text()):
    # Project only isolated complete source files and retain all original branch structure
    value=m.calls((out/'before'/name).read_text());assert m.calls(value)==value,name
    (out/'overlay'/m.RENAMES.get(name,name)).write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
for name,value in [('AnimBlendFrameUpdate.h',m.header()),('FrameUpdate_backend.h',m.backend_header()),('FrameUpdate_native.cpp',m.native())]:
    # Keep the C data definition and primitive leaves in the same coherent candidate tree
    (out/'overlay/src/animation'/name).write_text(value,newline='\n')
print('Staged all nine C frame callbacks, C dispatch data and native SDK field primitives')
