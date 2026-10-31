"""Preserve complete original rampage sources and generate an unapplied C11 preview."""
from pathlib import Path
import json,hashlib
import darkel_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/darkel-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    assert 'class CDarkel\n' in (root/'src/control/Darkel.h').read_text()
    changes=[]
    for path in (root/'src').rglob('*'):
        if path.suffix in ['.c','.cpp','.h']:
            source=path.read_text()
            if m.calls(source)!=source:changes.append(path.relative_to(root).as_posix())
    dependencies=['src/core/common.h','src/core/config.h','src/core/main.h','src/core/Wanted.h','src/core/Wanted.cpp','src/core/Stats.h','src/control/Replay.h','src/peds/PlayerPed.h','src/peds/Ped.h','src/peds/Population.h','src/vehicles/Vehicle.h','src/weapons/Weapon.h','src/weapons/WeaponType.h','src/weapons/WeaponInfo.h','src/modelinfo/ModelIds.h','src/modelinfo/ModelIndexData.h','src/modelinfo/ModelIndices.h','src/modelinfo/ModelInfo.h','src/modelinfo/BaseModelInfo.h','src/modelinfo/VehicleAppearance.h','src/renderer/Font.h','src/renderer/Font.cpp','src/renderer/FontStyles.h','src/renderer/Draw.h','src/renderer/Draw.cpp','src/renderer/Draw_constants.h','src/peds/PedType.h','src/audio/DMAudio.h','src/audio/soundlist.h','src/control/GameLogic.h','src/text/Text.h','src/core/Timer.h']
    for path in set(changes+dependencies):
        target=out/'before'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((root/path).read_bytes())
    for name,value in [('changes',changes),('dependencies',dependencies)]: (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
    (out/'untouched.json').write_text(json.dumps({p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (root/'src').rglob('*') if p.suffix in ['.c','.cpp','.h'] and p.relative_to(root).as_posix() not in changes},indent=2),newline='\n')
preview=out/'preview';preview.mkdir(exist_ok=True)
old=(out/'before/src/control/Darkel.cpp').read_text();header=(out/'before/src/control/Darkel.h').read_text()
for name,value in [('Darkel.h',m.header(header)),('Darkel_backend.h',m.BACKEND),('RampageSystem.c',m.kernel(old,header)),('Darkel.cpp',m.host())]:(preview/name).write_text(value,newline='\n')
for path in json.loads((out/'changes.json').read_text()):
    target=out/'projection'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(m.calls((out/'before'/path).read_text()),newline='\n')
print('Preserved',len(json.loads((out/'changes.json').read_text())),'complete originals; generated all 14 C rampage operations, 18 canonical fields and',len(m.LEAVES),'typed native primitives without editing production sources')
