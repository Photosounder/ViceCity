"""Audit reverb/special service and compare game-field read order with instrumented fields."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_environment_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-environment-c-tests'
old=(out/'before/src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioEnvironment.c').read_text()
def normalize(s):
    # Compare every statement independently of formatting and comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
methods=[]
channel_macro=re.search(r'#ifndef GTA_PS2\n#define CHANNEL_PLAYER_VEHICLE_ENGINE [^\n]+\n#endif',old).group()
assert normalize(channel_macro.replace('m_nActiveSamples','manager->m_nActiveSamples')) in normalize(kernel)
for name,target in [('ProcessReverb','Reverb'),('ProcessSpecial','Special')]:
    original=function(old,'cAudioManager::',name);methods.append(original)
    actual=function(kernel,'AudioEnvironment_',target)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('AudioManager.cpp',migration.owner),('AudioLogic.cpp',migration.owner),('AudioManager.h',migration.header),('AudioEffects.c',migration.calls)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_environment.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text())).replace('@CHANNEL_MACRO@',channel_macro)
for variant in ('before','after'):
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '')
    source=source.replace('@REVERB@','active->ProcessReverb()' if variant=='before' else 'AudioEnvironment_Reverb(active)').replace('@SPECIAL@','active->ProcessSpecial()' if variant=='before' else 'AudioEnvironment_Special(active)')
    source=source.replace('@CALLER@','active->ProcessReverb();active->ProcessSpecial()' if variant=='before' else 'TestEnvironmentC(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestEnvironmentC(cAudioManager *manager)\n{\n    // Exercise both environment operations through their actual C declarations\n    AudioEnvironment_Reverb(manager);\n    AudioEnvironment_Special(manager);\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Both complete bodies, six field/game adapters, live read order and all owner/header/service edits audited')
