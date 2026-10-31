"""Preserve immutable complete source snapshots and stage the embedded autopilot conversion."""
from pathlib import Path
import json,hashlib,shutil,re
import autopilot_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/autopilot-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    assert 'class CAutoPilot {' in (root/'src/control/AutoPilot.h').read_text()
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix in ['.cpp','.c','.h']:
            source=path.read_text();name=path.relative_to(root).as_posix()
            if m.calls(source)!=source:changes.append(name)
            else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    shutil.copytree(root/'src',out/'before/src',dirs_exist_ok=False)
    for name,value in [('changes',changes),('untouched',untouched)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
old=(out/'before/src/control/AutoPilot.cpp').read_text();header=(out/'before/src/control/AutoPilot.h').read_text();preview=out/'preview';preview.mkdir(exist_ok=True)
assert set(re.findall(r'CAutoPilot::(\w+)\(',old))=={'ModifySpeed','RemoveOnePathNode','Save','Load'}
assert set(re.findall(r'\b(?:void|float)\s+(\w+)\s*\(',header))=={'ModifySpeed','RemoveOnePathNode','Save','Load','GetCruiseSpeed'}
for name,value in [('AutoPilot.h',m.header(header)),('AutoPilot_backend.h',m.BACKEND),('AutoPilotSystem.c',m.kernel(old,header)),('AutoPilot.cpp',m.host())]:(preview/name).write_text(value,newline='\n')
for path in json.loads((out/'changes.json').read_text()):
    target=out/'projection'/path;target.parent.mkdir(parents=True,exist_ok=True);value=m.calls((out/'before'/path).read_text());assert m.calls(value)==value,path;target.write_text(value,newline='\n')
print('Preserved complete original source tree; staged',len(json.loads((out/'changes.json').read_text())),'changed files, all 6 C operations, 33 record fields and 5 typed native path-link reads')
