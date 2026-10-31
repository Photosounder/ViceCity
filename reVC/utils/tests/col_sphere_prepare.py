"""Stage both complete C sphere owners against an immutable coherent native source tree."""
from pathlib import Path
import hashlib,json,shutil
import col_sphere_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-sphere-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix not in ['.cpp','.c','.h'] or any(part.startswith('.') for part in path.relative_to(root/'src').parts):continue
        name=path.relative_to(root).as_posix();source=path.read_text()
        if m.calls(source)!=source:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    shutil.copytree(root/'src',out/'before/src',ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
    for name,value in [('changes',changes),('untouched',untouched)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
for name in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/name).read_text();value=m.calls(original);assert m.calls(value)==value,name
    (out/'overlay'/m.RENAMES.get(name,name)).write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
(out/'overlay/src/collision/ColSphere_native.h').write_text(m.native(),newline='\n')
print('Staged both complete C sphere owners, ray aliases, explicit composition and native boundaries')
