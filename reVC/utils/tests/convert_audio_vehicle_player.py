from pathlib import Path
import re,shutil
import audio_vehicle_player_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-player-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original player helper snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_PLAYER_'+name,macros)
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/VehicleTypes.h"
#include "sampman.h"
#include <math.h>

enum { VEHICLE_ENGINE_MAX_DIST = 50 };
#define AudioVehiclePlayer_Min(a,b) ((a) < (b) ? (a) : (b))
#define AudioVehiclePlayer_Max(a,b) ((a) > (b) ? (a) : (b))
'''+macros+'\n'
for name,args in [('UpdateGas','CVehicle *veh, int32_t vehType'),('AddSample','cAudioManager *manager, uint8_t Vol, uint32_t freq, uint32_t sample, uint8_t bank, uint8_t counter, uint8_t bLooping')]:
    original=next(k for k,v in m.NAMES.items() if v==name)
    source+='void\nAudioVehiclePlayer_'+name+'('+args+')\n'+m.body(function(old,'cAudioManager::',original)).replace('{','{\n    // Preserve the original smoothing or player sample operation with explicit C access',1)+'\n\n'
(root/'src/audio/AudioVehiclePlayer.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleEngine.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioVehicleEngine.c','AudioVehiclePlayer.c')),newline='\n')
p=root/'utils/tests/audio_vehicle_engine_migration.py';s=p.read_text().replace('import re','import re\nimport audio_vehicle_player_migration as player',1).replace("    return source.replace('ProcessVehicleEngine(params)','AudioVehicleEngine_Process(this, &params)')","    return player.logic(source.replace('ProcessVehicleEngine(params)','AudioVehicleEngine_Process(this, &params)'))").replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')","    return player.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(s,newline='\n')
prior=root/'build/audio-vehicle-engine-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-engine','audio-vehicle-player').replace('AudioVehicleEngine','AudioVehiclePlayer')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioVehiclePlayer.c'","'src/audio/AudioVehicleEngine.c','src/audio/AudioVehiclePlayer.c'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_vehicle_player.ps1';p.write_text((root/'utils/tests/test_audio_vehicle_engine.ps1').read_text().replace('vehicleEngine','vehiclePlayer').replace('VehicleEngine','VehiclePlayer').replace('vehicle_engine','vehicle_player').replace('vehicle-engine','vehicle-player'),newline='\n')
print('Converted gas smoothing and player-car sample construction to C')
