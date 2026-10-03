"""Audit collision service and compare aliased record/counter/history state at controlled generators."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_collision_service_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-collision-service-c-tests'
old=(out/'before/src/audio/AudioCollision.cpp').read_text();kernel=(root/'src/audio/AudioCollisionService.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
method=function(old,'cAudioManager::','ServiceCollisions');actual=function(kernel,'AudioCollisionService_','Run')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
for name,transform in [('AudioManager.cpp',migration.owner),('AudioCollision.cpp',migration.owner),('AudioManager.h',migration.header),('AudioEffects.c',migration.calls)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','SurfaceTypes.h'])
template=(root/'utils/tests/audio_collision_service.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ('before','after'):
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '')
    source=source.replace('@SERVICE@','active->ServiceCollisions()' if variant=='before' else 'AudioCollisionService_Run(active)').replace('@CALLER@','active->ServiceCollisions()' if variant=='before' else 'TestCollisionServiceC(active)')
    if variant=='after': source=migration.calls(source)
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestCollisionServiceC(cAudioManager *manager)\n{\n    // Service collision history through the actual C operation declaration\n    AudioCollisionService_Run(manager);\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete service body, aliased collision adapters and all owner/header/pipeline edits audited')
