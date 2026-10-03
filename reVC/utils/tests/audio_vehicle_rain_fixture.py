# Audit rain-on-vehicle generation and compare actual C math and requested queues
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_rain_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-rain-c-tests'
def normalize(s):
    # Compare complete statements independently of comments and layout
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();method=function(old,'cAudioManager::','ProcessRainOnVehicle');kernel=(root/'src/audio/AudioVehicleRain.c').read_text();actual=function(kernel,'AudioVehicleRain_','Process')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
for name,transform in [('AudioLogic.cpp',migration.logic),('AudioManager.h',migration.header)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
header=(out/'before/src/audio/AudioManager.h').read_text();macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:
    converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_RAIN_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioVehicleRain_Process')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
assert 'uint8 m_bRainAudioCounter;' in (root/'src/vehicles/Vehicle.h').read_text()
assert 'uint8 m_bRainSamplesCounter;' in (root/'src/vehicles/Vehicle.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_rain.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros)
for variant in ['before','after']:
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@RUN@','active->ProcessRainOnVehicle(params)' if variant=='before' else 'TestVehicleRainC(active,&params)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestVehicleRainC(cAudioManager *manager,cVehicleParams *params)
{
    // Generate the vehicle rain request through the actual C operation declaration
    AudioVehicleRain_Process(manager,params);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:
    (out/(name+'.h')).write_bytes((root/'build/audio-collision-sounds-c-tests'/(name+'.h')).read_bytes())
print('Complete rain generator, counter adapters, constants, macros and caller audited')
