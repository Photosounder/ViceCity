"""Audit reflection scheduling and compare original callbacks with the actual C owner."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_reflections_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-reflections-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text();kernel=(root/'src/audio/AudioReflections.c').read_text()
def normalize(s):
    # Compare statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
member=function(old,'cAudioManager::','UpdateReflections');actual=function(kernel,'AudioReflections_','Update')
assert normalize(migration.body(member))==normalize(actual[actual.index('{'):])
for name,transform in [('AudioManager.cpp',migration.owner),('AudioManager.h',migration.header)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
vector=(root/'src/math/Vector.h').read_text()
assert 'return (v2 - v1).Magnitude();' in vector
assert 'float Magnitude(void) const { return Sqrt(x*x + y*y + z*z); }' in vector
assert 'return CVector(left.x - right.x, left.y - right.y, left.z - right.z);' in vector
expected='static float AudioReflections_Distance(const AudioSoundPosition *start, const AudioSoundPosition *hit) { float x = hit->x - start->x; float y = hit->y - start->y; float z = hit->z - start->z; return sqrtf(x*x + y*y + z*z); }'
assert normalize(expected)==normalize(function(kernel,'AudioReflections_','Distance'))
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h'])
template=(root/'utils/tests/audio_reflections.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
camera=function((root/'src/audio/AudioLogic.cpp').read_text(),'AudioGeometryHost_','CameraPosition')
for variant in ('before','after'):
    source=template.replace('@METHOD@','#ifdef AUDIO_REFLECTIONS\n'+member+'\n#endif' if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@CAMERA@',camera if variant=='after' else '')
    source=source.replace('@UPDATE@','active->UpdateReflections()' if variant=='before' else 'TestReflectionUpdateC(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\n#ifdef AUDIO_REFLECTIONS\nvoid TestReflectionUpdateC(cAudioManager *manager)\n{\n    // Update the live owner\'s reflection probes through the actual C declaration\n    AudioReflections_Update(manager);\n}\n#endif\n',newline='\n')
for name,flags in {'current':'','early':'#undef GTA_VERSION\n#define GTA_VERSION GTAVC_PS2\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-reflections':'#undef AUDIO_REFLECTIONS\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete scheduler, both versions, distance formula, adapters and service/header edits audited')
