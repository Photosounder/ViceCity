# Audit both complete helper bodies and generate mixed C/C++ comparisons
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_player_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-player-c-tests'
def normalize(s):
    # Ignore formatting and comments while comparing every production statement
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehiclePlayer.c').read_text()
methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioVehiclePlayer_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_PLAYER_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioVehiclePlayer_UpdateGas')])
assert 'inline float Abs(float x) { return fabsf(x); }' in (root/'src/math/maths.h').read_text()
assert '#define AudioVehiclePlayer_Min(a,b) ((a) < (b) ? (a) : (b))' in kernel
assert '#define AudioVehiclePlayer_Max(a,b) ((a) > (b) ? (a) : (b))' in kernel
assert 'enum { VEHICLE_ENGINE_MAX_DIST = 50 };' in kernel and 'VEHICLE_ENGINE_MAX_DIST = 50,' in old
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_player.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@VEHICLE_TYPES@',str(root/'src/modelinfo/VehicleTypes.h').replace('\\','/'))
for variant in ['before','after']:
    run='if(kind==0)active->UpdateGasPedalAudio(selected,type);else active->AddPlayerCarSample(volume,frequency,sample,bank,counter,looping)' if variant=='before' else 'TestVehiclePlayerC(active,selected,type,volume,frequency,sample,bank,counter,looping,kind)'
    (out/(variant+'.cpp')).write_text(template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@',run),newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestVehiclePlayerC(cAudioManager *manager,CVehicle *vehicle,int32_t type,uint8_t volume,uint32_t frequency,uint32_t sample,uint8_t bank,uint8_t counter,uint8_t looping,unsigned kind)
{
    // Invoke both helpers through their actual C declarations
    if(kind==0)AudioVehiclePlayer_UpdateGas(vehicle,type);
    else AudioVehiclePlayer_AddSample(manager,volume,frequency,sample,bank,counter,looping);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-engine-c-tests'/(name+'.h')).read_bytes())
print('Complete gas smoothing, player sample construction, callers and conditional macros audited')
