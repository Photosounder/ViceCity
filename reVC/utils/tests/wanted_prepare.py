"""Snapshot the diagnosed whole wanted owner and generate an unapplied C preview."""
from pathlib import Path
import json,hashlib
import wanted_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/wanted-c-tests';out.mkdir(exist_ok=True)
if not (out/'changes.json').exists():
    assert 'class CWanted\n' in (root/'src/core/Wanted.h').read_text()
    changes=[]
    for path in (root/'src').rglob('*'):
        if path.suffix in ['.cpp','.c','.h']:
            original=path.read_text();changed=m.calls(original)
            if changed!=original:changes.append(path.relative_to(root).as_posix())
    assert len(changes)==len(set(changes))
    dependencies=['src/core/common.h','src/core/config.h','src/core/Pool.h','src/core/Pools.h','src/math/Vector.h','src/math/maths.h','src/core/ZoneCull.h','src/core/General.h','src/core/Stats.h','src/control/Darkel.h','src/peds/CopPed.h','src/entities/EntityStatus.h','src/modelinfo/ModelIndices.h','src/modelinfo/ModelIndexData.h','src/audio/AudioCrimes.h','src/audio/AudioCrimes.c','src/audio/AudioPoliceGame.h','src/audio/AudioManager.h','src/audio/AudioGameEntries.c','src/audio/DMAudio.h','src/core/Wanted.h','src/core/Wanted.cpp','src/core/Crime.h']
    for name in set(changes+dependencies):
        target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((root/name).read_bytes())
    for name,value in [('changes',changes),('dependencies',dependencies)]: (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
    (out/'untouched.json').write_text(json.dumps({p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (root/'src').rglob('*') if p.suffix in ['.c','.cpp','.h'] and p.relative_to(root).as_posix() not in changes},indent=2),newline='\n')
old=lambda name:(out/'before/src/core'/name).read_text()
preview=out/'preview';preview.mkdir(exist_ok=True)
for name,value in [('WantedShared.h',m.SHARED),('Crime.h',m.CRIME_HEADER),('Wanted.h',m.header(old('Wanted.h'))),('Wanted_backend.h',m.BACKEND),('WantedSystem.c',m.kernel(old('Wanted.cpp'),old('Wanted.h'))),('Wanted.cpp',m.host())]:
    (preview/name).write_text(value,newline='\n')
for path in json.loads((out/'changes.json').read_text()):
    target=out/'projection'/path;target.parent.mkdir(parents=True,exist_ok=True);target.write_text(m.calls((out/'before'/path).read_text()),newline='\n')
audio=m.calls((out/'before/src/audio/DMAudio.h').read_text())
audio=audio.replace('"../core/config.h"','"'+(root/'src/core/config.h').as_posix()+'"').replace('"../core/CrimeTypes.h"','"'+(root/'src/core/CrimeTypes.h').as_posix()+'"')
(preview/'DMAudio.h').write_text(audio,newline='\n')
print('Preserved',len(json.loads((out/'changes.json').read_text())),'complete original sources; generated 26 C operations, queued-crime records and deferred audio position reads; production untouched')
