"""Keep an immutable full-source baseline and stage explicit C list ownership."""
from pathlib import Path
import json,hashlib,shutil
import lists_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lists-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    shutil.copytree(root/'src',out/'before/src')
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix not in ['.h','.cpp','.c']:continue
        name=path.relative_to(root).as_posix();source=path.read_text()
        if m.calls(source)!=source:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    for name,value in [('changes',changes),('untouched',untouched)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
for name in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/name).read_text();value=m.calls(original);assert m.calls(value)==value,name
    target=out/'overlay'/m.RENAMES.get(name,name);target.write_text(value,newline='\n')
    if name in m.RENAMES:(out/'overlay'/name).unlink(missing_ok=True)
print('Staged',len(json.loads((out/'changes.json').read_text())),'source changes and the complete C list producer')
