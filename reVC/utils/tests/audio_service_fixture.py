"""Compare service and timer control flow with controlled game callbacks and real C primitives."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_service_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-service-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text();oldlogic=(out/'before/src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioService.c').read_text()
def normalize(s):
    # Compare all statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
methods=[]
for source,name,target in [(old,'Service','Run'),(oldlogic,'ResetAudioLogicTimers','ResetLogicTimers')]:
    original=function(source,'cAudioManager::',name);methods.append(original)
    actual=function(kernel,'AudioService_',target);assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('AudioManager.cpp',migration.owner),('AudioLogic.cpp',migration.owner),('AudioManager.h',migration.header),('DMAudio.cpp',migration.facade)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
facade=function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','Service').replace('AudioManager.Service()', 'AudioService_Run(&AudioManager)')
assert normalize(facade)==normalize(function(kernel,'DMAudio_','Service'))
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_service.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ['before','after']:
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@HOST@',migration.HOST.replace('AudioServiceHost_SoundEffects', 'AudioEffects_Service')+'\n'+migration.PED_HOST if variant=='after' else '')
    source=source.replace('@SERVICE@','active->Service()' if variant=='before' else 'AudioService_Run(active)').replace('@RESET@','active->ResetAudioLogicTimers(time)' if variant=='before' else 'AudioService_ResetLogicTimers(active,time)').replace('@FACADE@','AudioManager.Service()' if variant=='before' else 'TestAudioServiceC()')
    source=source.replace('AudioServiceHost_SoundEffects', 'AudioEffects_Service')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "DMAudio.h"\nvoid TestAudioServiceC(void)\n{\n    // Service the live owner through the actual C facade declaration\n    DMAudio_Service();\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-reflections':'#undef AUDIO_REFLECTIONS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
random=function((root/'src/audio/AudioManagerInit.c').read_text(),'AudioManager_','GenerateRandomTable')
(out/'random.c').write_text('#include <stdint.h>\n#include "Random.h"\n'+random+'\n',newline='\n')
print('Both bodies, ped access order, live timer reads, service facade and complete game caller edits audited')
