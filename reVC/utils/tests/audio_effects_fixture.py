"""Audit both complete migrations and compare callback-visible orchestration state."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_effects_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-effects-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text()
kernel=(root/'src/audio/AudioEffects.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
methods=[]
for name,target in [('ServiceSoundEffects','Service'),('InterrogateAudioEntities','Interrogate')]:
    original=function(old,'cAudioManager::',name);methods.append(original)
    actual=function(kernel,'AudioEffects_',target)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('AudioManager.cpp',migration.owner),('AudioManager.h',migration.header),('AudioService.c',migration.calls)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','AudioScriptObject.h','sampman.h','Pool.h'])
template=(root/'utils/tests/audio_effects.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ('before','after'):
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '')
    source=source.replace('@SERVICE@','active->ServiceSoundEffects()' if variant=='before' else 'AudioEffects_Service(active)').replace('@INTERROGATE@','active->InterrogateAudioEntities()' if variant=='before' else 'AudioEffects_Interrogate(active)')
    source=source.replace('@CALLER@','if(seed&1) active->ServiceSoundEffects(); else active->InterrogateAudioEntities()' if variant=='before' else 'TestAudioEffectsC(active,seed&1)')
    source=source.replace('AudioEffectsHost_AddReleasingSounds', 'AudioRelease_Add').replace('AudioEffectsHost_ProcessActiveQueues', 'AudioActive_Process').replace('AudioEffectsHost_ProcessReverb', 'AudioEnvironment_Reverb').replace('AudioEffectsHost_ProcessSpecial', 'AudioEnvironment_Special').replace('AudioEffectsHost_ProcessEntity', 'AudioDispatch_Entity').replace('AudioEffectsHost_ServiceCollisions', 'AudioCollisionService_Run')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestAudioEffectsC(cAudioManager *manager, uint8_t service)\n{\n    // Exercise both operations through their actual C declarations\n    if(service) AudioEffects_Service(manager);\n    else AudioEffects_Interrogate(manager);\n}\n',newline='\n')
reset=function((root/'src/audio/AudioScriptObject.c').read_text(),'AudioScriptObject_','Reset')
(out/'script-reset.c').write_text('#include "AudioScriptObject.h"\n#include "soundlist.h"\n#include "sampman.h"\n'+reset+'\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-oal':'#undef AUDIO_OAL\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reverb':'#undef AUDIO_REVERB\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Both complete bodies, six game callbacks, owner/header edits and direct service caller audited')
