"""Stage complete input-state ownership with immutable full-source native baselines."""
from pathlib import Path
import json,hashlib,shutil
import input_state_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/input-state-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix not in ['.cpp','.c','.h']:continue
        name=path.relative_to(root).as_posix();source=path.read_text()
        if m.calls(source)!=source:changes.append(name)
        else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    shutil.copytree(root/'src',out/'before/src')
    for name,value in [('changes',changes),('untouched',untouched)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
shutil.copytree(out/'before/src',out/'overlay/src',dirs_exist_ok=True)
for name in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/name).read_text();value=m.calls(original);assert m.calls(value)==value,name
    (out/'overlay'/name).write_text(value,newline='\n')
original=(out/'before/src/core/Pad.cpp').read_text();header=(out/'before/src/core/Pad.h').read_text()
for name,value in [('InputState.h',m.header(header)),('InputState.c',m.kernel(original,header)),('InputState_backend.h',m.BACKEND)]:(out/'overlay/src/core'/name).write_text(value,newline='\n')
print('Staged',len(json.loads((out/'changes.json').read_text())),'changed sources, four C records and all ten input-state operations')
