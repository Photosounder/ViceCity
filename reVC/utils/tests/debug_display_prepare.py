"""Preserve complete original debug sources and generate an unapplied C11 preview."""
from pathlib import Path
import json,hashlib
import debug_display_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/debug-display-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    assert 'class CDebug\n' in (root/'src/core/Debug.h').read_text()
    changes=[]
    for path in (root/'src').rglob('*'):
        if path.suffix in ['.cpp','.c','.h']:
            source=path.read_text()
            if m.calls(source)!=source:changes.append(path.relative_to(root).as_posix())
    dependencies=['src/core/Debug.h','src/core/Debug.cpp','src/core/common.h','src/core/config.h','src/core/main.h','src/rw/RwHelper.h','src/rw/RwHelper.cpp','src/renderer/Lines.h','src/renderer/Lines.c','src/renderer/Font.h','src/renderer/Font.cpp','src/renderer/FontStyles.h','src/text/Text.h','src/math/Vector.h']
    for path in set(changes+dependencies):
        target=out/'before'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((root/path).read_bytes())
    for name,value in [('changes',changes),('dependencies',dependencies)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
    (out/'untouched.json').write_text(json.dumps({p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (root/'src').rglob('*') if p.suffix in ['.cpp','.c','.h'] and p.relative_to(root).as_posix() not in changes},indent=2),newline='\n')
old=(out/'before/src/core/Debug.cpp').read_text();preview=out/'preview';preview.mkdir(exist_ok=True)
for name,value in [('Debug.h',m.HEADER),('Debug_backend.h',m.BACKEND),('DebugDisplaySystem.c',m.kernel(old)),('Debug.cpp',m.host())]:(preview/name).write_text(value,newline='\n')
for path in json.loads((out/'changes.json').read_text()):
    target=out/'projection'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(m.calls((out/'before'/path).read_text()),newline='\n')
print('Preserved',len(json.loads((out/'changes.json').read_text())),'complete originals; generated all 7 C operations, 6 queue objects and',len(m.LEAVES),'native primitives without editing production')
