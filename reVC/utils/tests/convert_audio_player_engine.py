from pathlib import Path
import re,shutil
import audio_player_engine_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-player-engine-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original player engine snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','src/audio/AudioVehicleEngine.c','src/peds/Ped.cpp','src/control/Script.cpp','build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioGeometry.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "../vehicles/WheelIndices.h"
#include "sampman.h"

enum {
'''+m.CONSTANTS+'''};
#ifndef GTA_PS2
#define CHANNEL_PLAYER_VEHICLE_ENGINE manager->m_nActiveSamples
#endif

static uint8_t AudioPlayerEngine_Decision(uint8_t result)
{
    // Retain conditional repeated game queries in floating clamps and minima
    volatile uint8_t comparison = result;
    return comparison;
}
#define AudioPlayerEngine_Min(a,b) (AudioPlayerEngine_Decision((a) < (b)) ? (a) : (b))
#define AudioPlayerEngine_Max(a,b) (AudioPlayerEngine_Decision((a) > (b)) ? (a) : (b))
#define AudioPlayerEngine_Clamp(v,low,high) (AudioPlayerEngine_Decision((v) < (low)) ? (low) : AudioPlayerEngine_Decision((v) > (high)) ? (high) : (v))

uint8_t bPlayerJustEnteredCar;

void
AudioPlayerEngine_Entered(cAudioManager *manager)
'''+m.body(function(old,'cAudioManager::','PlayerJustGotInCar')).replace('{','{\n    // Retain the original initialized-owner gate before marking the next entry reset',1)+'''

void
AudioPlayerEngine_Left(cAudioManager *manager)
{
    // Retain the original empty exit notification
    (void)manager;
}

void
AudioPlayerEngine_Process(cAudioManager *manager, cVehicleParams *params, CVehicle *veh)
'''+m.body(m.extract(old)).replace('{','{\n    // Preserve shared engine history and all original player channel transitions\n    (void)veh;',1)+'\n//- rouz edit (ChatGPT)\n'
(root/'src/audio/AudioPlayerEngine.c').write_text(source,newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['src/peds/Ped.cpp','src/control/Script.cpp']:
    p=root/name;s=m.calls(p.read_text())
    if not s.startswith('//+ rouz edit (ChatGPT)'):s='//+ rouz edit (ChatGPT)\n'+s+'\n//- rouz edit (ChatGPT)\n'
    p.write_text(s,newline='\n')
p=root/'src/audio/AudioVehicleEngine.c';p.write_text(p.read_text().replace('AudioVehicleEngineHost_Player(manager, params, params->m_pVehicle)','AudioPlayerEngine_Process(manager, params, params->m_pVehicle)'),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehiclePlayer.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioVehiclePlayer.c','AudioPlayerEngine.c')),newline='\n')
p=root/'utils/tests/audio_vehicle_player_migration.py';s=p.read_text().replace('import re','import re\nimport audio_player_engine_migration as engine',1).replace("    return re.sub(r'\\bAddPlayerCarSample\\(', 'AudioVehiclePlayer_AddSample(this, ',source)","    return engine.logic(re.sub(r'\\bAddPlayerCarSample\\(', 'AudioVehiclePlayer_AddSample(this, ',source))").replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')","    return engine.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(s,newline='\n')
p=root/'utils/tests/audio_vehicle_engine_migration.py';p.write_text(p.read_text().replace("'AudioVehicleEngineHost_Player(manager, params, params->m_pVehicle)'","'AudioPlayerEngine_Process(manager, params, params->m_pVehicle)'"),newline='\n')
p=root/'utils/tests/audio_vehicle_engine.cpp.in';s=p.read_text();needle='extern "C" CVehicle *AudioPoliceHost_PlayerVehicle()';s=s.replace(needle,'''extern "C" void AudioPlayerEngine_Process(cAudioManager *manager,cVehicleParams *p,CVehicle *v) {
    // Retain this earlier fixture's isolated player-engine handoff observation
    manager->ProcessPlayersVehicleEngine(*p,v);
}
'''+needle);p.write_text(s,newline='\n')
prior=root/'build/audio-vehicle-player-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-player','audio-player-engine').replace('AudioVehiclePlayer','AudioPlayerEngine')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioPlayerEngine.c'","'src/audio/AudioVehiclePlayer.c','src/audio/AudioPlayerEngine.c','src/peds/Ped.cpp','src/control/Script.cpp'")
    else:
        # Link the actual C geometry and player sample helper in both fixture variants
        source=re.sub(r'(cl /nologo /c /std:c11 /O2[^\n]*?)AudioPlayerEngine(32|64).obj src/audio/AudioPlayerEngine.c',r'\1AudioVehiclePlayer\2.obj src/audio/AudioVehiclePlayer.c\nif errorlevel 1 exit /b 1\n\1AudioGeometry\2.obj src/audio/AudioGeometry.c\nif errorlevel 1 exit /b 1\n\1AudioPlayerEngine\2.obj src/audio/AudioPlayerEngine.c',source)
        source=source.replace('AudioRequests32.obj /out:', 'AudioRequests32.obj build\\audio-player-engine-c-tests\\AudioGeometry32.obj build\\audio-player-engine-c-tests\\AudioVehiclePlayer32.obj /out:').replace('AudioRequests64.obj /out:', 'AudioRequests64.obj build\\audio-player-engine-c-tests\\AudioGeometry64.obj build\\audio-player-engine-c-tests\\AudioVehiclePlayer64.obj /out:')
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_player_engine.ps1';s=(root/'utils/tests/test_audio_vehicle_player.ps1').read_text().replace('vehiclePlayer','playerEngine').replace('VehiclePlayer','PlayerEngine').replace('vehicle_player','player_engine').replace('vehicle-player','player-engine');s=s.replace("'AudioMath','AudioRequests','AudioSoundQueue'","'AudioMath','AudioRequests','AudioSoundQueue','AudioGeometry','AudioVehiclePlayer'");s=s.replace('"$playerEngineBuild/AudioSoundQueue.o")','"$playerEngineBuild/AudioSoundQueue.o","$playerEngineBuild/AudioGeometry.o","$playerEngineBuild/AudioVehiclePlayer.o")').replace('"$playerEngineSanBuild/AudioSoundQueue.o")','"$playerEngineSanBuild/AudioSoundQueue.o","$playerEngineSanBuild/AudioGeometry.o","$playerEngineSanBuild/AudioVehiclePlayer.o")');p.write_text(s,newline='\n')
print('Converted the full player engine state machine, entry notifications and direct C handoff')
