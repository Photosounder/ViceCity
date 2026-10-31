"""Preserve complete original roadblock sources and generate an unapplied whole-owner C preview."""
from pathlib import Path
import json,hashlib
import roadblocks_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/roadblocks-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    assert 'class CRoadBlocks\n' in (root/'src/control/RoadBlocks.h').read_text()
    changes=[]
    for path in (root/'src').rglob('*'):
        if path.suffix in ['.cpp','.c','.h']:
            source=path.read_text()
            if m.calls(source)!=source:changes.append(path.relative_to(root).as_posix())
    dependencies=['src/core/config.h','src/core/common.h','src/core/Pool.h','src/control/PathFind.h','src/math/Vector.h','src/math/Vector.cpp','src/math/Vector2D.h','src/math/Matrix.h','src/math/Matrix.cpp','src/math/maths.h','src/peds/CopPed.h','src/peds/Ped.h','src/peds/PlayerPed.h','src/vehicles/Vehicle.h','src/vehicles/Automobile.h','src/objects/Object.h','src/entities/EntityStatus.h','src/weapons/WeaponType.h','src/modelinfo/ModelIds.h','src/modelinfo/ModelIndices.h','src/modelinfo/ModelIndexData.h','src/control/AutoPilot.h','src/core/Streaming.h','src/core/Wanted.h','src/core/Pools.h','src/renderer/Draw_constants.h','src/core/General.h','src/core/Timer.h','src/control/RoadBlocks.cpp','src/control/RoadBlocks.h','src/core/PlayerInfo.cpp','src/core/re3.cpp','src/core/General.c']
    for path in set(changes+dependencies):
        target=out/'before'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((root/path).read_bytes())
    for name,value in [('changes',changes),('dependencies',dependencies)]:(out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
    (out/'untouched.json').write_text(json.dumps({p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (root/'src').rglob('*') if p.suffix in ['.cpp','.c','.h'] and p.relative_to(root).as_posix() not in changes},indent=2),newline='\n')
old=(out/'before/src/control/RoadBlocks.cpp').read_text();preview=out/'preview';preview.mkdir(exist_ok=True)
for name,value in [('RoadBlocks.h',m.HEADER),('RoadBlockSystem.c',m.kernel(old)),('RoadBlocks_backend.h',m.BACKEND),('RoadBlocks.cpp',m.host())]:(preview/name).write_text(value,newline='\n')
for path in json.loads((out/'changes.json').read_text()):
    target=out/'projection'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(m.calls((out/'before'/path).read_text()),newline='\n')
print('Preserved',len(json.loads((out/'changes.json').read_text())),'complete original files; generated all 7 C operations, 4 canonical objects and',len(m.LEAVES),'native primitives without editing production')
