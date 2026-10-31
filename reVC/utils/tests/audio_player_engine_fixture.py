# Audit the complete player engine state machine and generate before/after fixtures
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_player_engine_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-player-engine-c-tests'
def normalize(s):
    # Compare full production statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioPlayerEngine.c').read_text()
original=migration.extract(old)
# Extract the C operation using its final file boundary because paired feature branches have alternate braces
actual=kernel[kernel.index('void\nAudioPlayerEngine_Process'):kernel.index('//- rouz edit (ChatGPT)')].rstrip()
assert normalize(migration.body(original).replace('{','{\n(void)veh;',1))==normalize(actual[actual.index('{'):])
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.body(function(old,'cAudioManager::','PlayerJustGotInCar')))==normalize(function(kernel,'AudioPlayerEngine_','Entered').split('{',1)[1].join(['{','']))
assert normalize(migration.CONSTANTS) in normalize(kernel)
for path in ['src/peds/Ped.cpp','src/control/Script.cpp']:assert normalize(migration.calls((out/'before'/path).read_text()))==normalize((root/path).read_text())
assert normalize((out/'before/src/audio/AudioVehicleEngine.c').read_text().replace('AudioVehicleEngineHost_Player(manager, params, params->m_pVehicle)','AudioPlayerEngine_Process(manager, params, params->m_pVehicle)'))==normalize((root/'src/audio/AudioVehicleEngine.c').read_text())
assert '#define Clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))' in (root/'src/core/common.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_player_engine.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@CONSTANTS@',migration.CONSTANTS)
for name,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('VEHICLE_TYPES','src/modelinfo/VehicleTypes.h'),('WHEEL_STATE','src/vehicles/WheelState.h'),('WHEEL_INDICES','src/vehicles/WheelIndices.h')]:template=template.replace('@'+name+'@',str(root/path).replace('\\','/'))
methods='\n'.join([function(old,'cAudioManager::','PlayerJustGotInCar'),function(old,'cAudioManager::','PlayerJustLeftCar'),original])
for variant in ['before','after']:
    source=template.replace('@METHODS@',methods if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@ENTRY_STORAGE@','uint8_t bPlayerJustEnteredCar;' if variant=='before' else 'extern uint8_t bPlayerJustEnteredCar;')
    source=source.replace('@ENTER@','active->PlayerJustGotInCar()' if variant=='before' else 'TestPlayerEngineC(active,&params,NULL,1)').replace('@LEFT@','active->PlayerJustLeftCar()' if variant=='before' else 'TestPlayerEngineC(active,&params,NULL,2)').replace('@RUN@','active->ProcessPlayersVehicleEngine(params,NULL)' if variant=='before' else 'TestPlayerEngineC(active,&params,NULL,0)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestPlayerEngineC(cAudioManager *manager,cVehicleParams *params,CVehicle *vehicle,unsigned kind)
{
    // Invoke the complete player engine and both entry notifications through their actual C declarations
    if(kind==0)AudioPlayerEngine_Process(manager,params,vehicle);
    else if(kind==1)AudioPlayerEngine_Entered(manager);
    else AudioPlayerEngine_Left(manager);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-player-c-tests'/(name+'.h')).read_bytes())
(out/'no-time-scale.h').write_text('#include "config.h"\n#define AUDIO_OAL\n#undef USE_TIME_SCALE_FOR_AUDIO\n',newline='\n')
print('Complete player state machine, shared history, lazy timer, notification calls and C handoff audited')
