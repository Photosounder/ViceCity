"""Audit active-channel control flow and compare real C geometry/math with observed backend calls."""
from pathlib import Path
import re
from audio_owner_init_migration import fields
import audio_active_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-active-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text();kernel=(root/'src/audio/AudioActive.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
method=migration.extract(old);actual=migration.extract(kernel,'void\nAudioActive_Process(')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
assert normalize(migration.MACROS) in normalize(kernel)
common=(root/'src/core/common.h').read_text()
for name,new in [('Min','AudioActive_Min'),('Max','AudioActive_Max'),('Clamp2','AudioActive_Clamp')]:
    original=re.search(r'^#define '+name+r'\([^\n]+',common,re.M).group()
    original=original.replace('Clamp2','AudioActive_Clamp').replace('Min','AudioActive_Min').replace('Max','AudioActive_Max')
    assert normalize(original) in normalize(migration.MACROS),name
for name,transform in [('AudioManager.cpp',migration.owner),('AudioManager.h',migration.header),('AudioEffects.c',migration.calls)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','AudioMath.h','AudioGeometry.h'])
template=(root/'utils/tests/audio_active.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ('before','after'):
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '')
    source=source.replace('@PROCESS@','active->ProcessActiveQueues()' if variant=='before' else 'AudioActive_Process(active)').replace('@CALLER@','active->ProcessActiveQueues()' if variant=='before' else 'TestActiveC(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestActiveC(cAudioManager *manager)\n{\n    // Process active channels through the actual C operation declaration\n    AudioActive_Process(manager);\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','unscaled':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reflections':'#undef AUDIO_REFLECTIONS\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete conditional body, original clamp macros, camera adapter, owner/header edits and service caller audited')
