# Audit complete reverse generator and compare actual C distance and request queues
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_reverse_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-reverse-c-tests'
def normalize(s):
    # Compare all statements independently of formatting and explanatory comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();method=function(old,'cAudioManager::','ProcessReverseGear');kernel=(root/'src/audio/AudioVehicleReverse.c').read_text();actual=function(kernel,'AudioVehicleReverse_','Process')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
for name,transform in [('AudioLogic.cpp',migration.logic),('AudioManager.h',migration.header)]:assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
header=(out/'before/src/audio/AudioManager.h').read_text();macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_REVERSE_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('uint8_t\nAudioVehicleReverse_Process')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
assert '#define AudioVehicleReverse_Abs(a) (((a) < 0) ? (-(a)) : (a))' in kernel
assert '#define ABS(a)  (((a) < 0) ? (-(a)) : (a))' in (root/'src/core/common.h').read_text()
assert 'int16 GetModelIndex(void) const { return m_modelIndex; }' in (root/'src/entities/Entity.h').read_text()
assert 'float m_fGasPedalAudio;' in (root/'src/vehicles/Automobile.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_reverse.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS).replace('@MODEL_IDS@',str(root/'src/modelinfo/ModelIds.h').replace('\\','/'))
for variant in ['before','after']:
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@RUN@','active->ProcessReverseGear(params)' if variant=='before' else 'TestVehicleReverseC(active,&params)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
uint8_t TestVehicleReverseC(cAudioManager *manager,cVehicleParams *params)
{
    // Return the actual C reverse generator result to the C++ test caller
    return AudioVehicleReverse_Process(manager,params);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-road-c-tests'/(name+'.h')).read_bytes())
print('Complete reverse generator, adapters, private constants and conditional macros audited')
