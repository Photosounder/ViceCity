from pathlib import Path
import re,shutil
import audio_vehicle_skid_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-skid-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original skid snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_SKID_'+name,macros)
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../core/SurfaceTypes.h"
#include "../modelinfo/VehicleTypes.h"
#include "../vehicles/WheelIndices.h"
#include "sampman.h"
#include <math.h>

enum {
'''+m.CONSTANTS+'''};

static uint8_t AudioVehicleSkid_Decision(uint8_t result)
{
    // Retain conditional repeated transmission and turn-speed reads in floating extrema
    volatile uint8_t comparison = result;
    return comparison;
}
#define AudioVehicleSkid_Min(a,b) (AudioVehicleSkid_Decision((a) < (b)) ? (a) : (b))
#define AudioVehicleSkid_Max(a,b) (AudioVehicleSkid_Decision((a) > (b)) ? (a) : (b))
'''+macros+'''
static float AudioVehicleSkid_Drive(CVehicle *veh, enum tWheelState wheelState, float gasPedalAudio, cTransmission *transmission, float velocityChange);
static float AudioVehicleSkid_NonDrive(CVehicle *veh, enum tWheelState wheelState, cTransmission *transmission, float velocityChange);

'''
for name,target in m.NAMES.items():
    args='cAudioManager *manager, cVehicleParams *params' if target=='Process' else 'CVehicle *veh, enum tWheelState wheelState, '+('float gasPedalAudio, ' if target=='Drive' else '')+'cTransmission *transmission, float velocityChange'
    source+=('uint8_t' if target=='Process' else 'static float')+'\nAudioVehicleSkid_'+target+'('+args+')\n'+m.body(function(old,'cAudioManager::',name)).replace('{','{\n    // Preserve the original wheel aggregation or skid-value arithmetic',1)+'\n\n'
(root/'src/audio/AudioVehicleSkid.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioPlayerEngine.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioPlayerEngine.c','AudioVehicleSkid.c')),newline='\n')
p=root/'utils/tests/audio_player_engine_migration.py';s=p.read_text().replace('import re','import re\nimport audio_vehicle_skid_migration as skid',1).replace('    return source\n\ndef header','    return skid.logic(source)\n\ndef header',1).replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')","    return skid.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(s,newline='\n')
prior=root/'build/audio-vehicle-player-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-player','audio-vehicle-skid').replace('AudioVehiclePlayer','AudioVehicleSkid')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioVehicleSkid.c'","'src/audio/AudioVehiclePlayer.c','src/audio/AudioPlayerEngine.c','src/audio/AudioVehicleSkid.c','src/peds/Ped.cpp','src/control/Script.cpp'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_vehicle_skid.ps1';p.write_text((root/'utils/tests/test_audio_vehicle_player.ps1').read_text().replace('vehiclePlayer','vehicleSkid').replace('VehiclePlayer','VehicleSkid').replace('vehicle_player','vehicle_skid').replace('vehicle-player','vehicle-skid'),newline='\n')
print('Converted the full skid generator and both wheel-value helpers to C')
