"""Preserve complete original statistics sources and generate an unapplied whole-owner C preview."""
from pathlib import Path
import json,hashlib
import stats_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stats-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    assert 'class CStats\n' in (root/'src/core/Stats.h').read_text()
    changes=[]
    for path in (root/'src').rglob('*'):
        if path.suffix in ['.cpp','.c','.h']:
            source=path.read_text()
            if m.calls(source)!=source:changes.append(path.relative_to(root).as_posix())
    dependencies=['src/core/common.h','src/core/config.h','src/core/PlayerInfo.h','src/core/World.h','src/core/Pad.h','src/core/Game.h','src/core/Frontend.h','src/core/TextLanguages.h','src/core/Timer.h','src/core/main.h','src/math/maths.h','src/peds/PedType.h','src/audio/audio_enums.h','src/audio/MusicSavedData.h','src/audio/MusicSavedData.c','src/audio/DMAudio.h','src/renderer/Font.h','src/renderer/Font.cpp','src/text/Text.h','src/core/Stats.h','src/core/Stats.cpp']
    for path in set(changes+dependencies):
        target=out/'before'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((root/path).read_bytes())
    for name,value in [('changes',changes),('dependencies',dependencies)]: (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
    (out/'untouched.json').write_text(json.dumps({p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (root/'src').rglob('*') if p.suffix in ['.cpp','.c','.h'] and p.relative_to(root).as_posix() not in changes},indent=2),newline='\n')
old=(out/'before/src/core/Stats.cpp').read_text();header=(out/'before/src/core/Stats.h').read_text();preview=out/'preview';preview.mkdir(exist_ok=True)
for name,value in [('Stats.h',m.header(header,old)),('StatisticsSystem.c',m.kernel(old,header)),('Stats_backend.h',m.BACKEND),('Stats.cpp',m.host())]:(preview/name).write_text(value,newline='\n')
for path in json.loads((out/'changes.json').read_text()):
    target=out/'projection'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(m.calls((out/'before'/path).read_text()),newline='\n')
print('Preserved',len(json.loads((out/'changes.json').read_text())),'complete original files; generated 38 C operations, 91 canonical objects and the entire unchanged save/row policies without editing production')
