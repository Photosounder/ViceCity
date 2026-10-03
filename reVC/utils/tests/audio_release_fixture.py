"""Audit complete release conversion and drive real C geometry and submission kernels."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_release_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-release-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text();kernel=(root/'src/audio/AudioRelease.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
method=function(old,'cAudioManager::','AddReleasingSounds')
actual=function(kernel,'AudioRelease_','Add')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
assert normalize(migration.HELPERS) in normalize(kernel)
for name,transform in [('AudioManager.cpp',migration.owner),('AudioManager.h',migration.header),('AudioEffects.c',migration.calls)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','AudioMath.h','AudioGeometry.h'])
header=(root/'src/audio/AudioManager.h').read_text()
constants='enum {\n'+ '\n'.join(re.findall(r'^\s*PED_COMMENT_VOLUME(?:_BEHIND_WALL)?\s*=\s*\d+,',header,re.M))+'\n};'
template=(root/'utils/tests/audio_release.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@CONSTANTS@',constants)
for variant in ('before','after'):
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '')
    source=source.replace('@RELEASE@','active->AddReleasingSounds()' if variant=='before' else 'AudioRelease_Add(active)').replace('@CALLER@','active->AddReleasingSounds()' if variant=='before' else 'TestReleaseC(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestReleaseC(cAudioManager *manager)\n{\n    // Exercise the release operation through its actual C declaration\n    AudioRelease_Add(manager);\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-attachment':'#undef ATTACH_RELEASING_SOUNDS_TO_ENTITIES\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reflections':'#undef AUDIO_REFLECTIONS\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete release body, math helpers, three host adapters, owner/header edits and service caller audited')
