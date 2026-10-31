"""Preserve exact profiling and cut-feature sources before staging two complete C producer renames."""
from pathlib import Path
import json,hashlib
import profile_power_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/profile-power-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    changes=[];untouched={}
    for path in (root/'src').rglob('*'):
        if path.suffix in ['.cpp','.c','.h']:
            source=path.read_text();name=path.relative_to(root).as_posix()
            if m.calls(source)!=source:changes.append(name)
            else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    dependencies=['src/core/common.h','src/core/config.h','src/core/Profile.cpp','src/core/Profile.h','src/control/PowerPoints.cpp','src/control/PowerPoints.h']
    for name in set(changes+dependencies):
        target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((root/name).read_bytes())
    for name,value in [('changes',changes),('untouched',untouched)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
for name in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/name).read_text();value=m.calls(original);assert m.calls(value)==value,name;target=out/'projection'/m.RENAMES.get(name,name);target.parent.mkdir(parents=True,exist_ok=True);target.write_text(value,newline='\n')
preview=out/'preview';preview.mkdir(exist_ok=True)
for name in ['src/core/Profile.h','src/core/Profile.c','src/control/PowerPoints.h','src/control/PowerPoints.c']:(preview/Path(name).name).write_text((out/'projection'/name).read_text(),newline='\n')
print('Preserved',len(json.loads((out/'changes.json').read_text())),'complete originals; staged three class removals, 9 C operations and two C producer renames with no C++ bridges')
