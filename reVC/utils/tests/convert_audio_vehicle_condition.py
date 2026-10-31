from pathlib import Path
import re,shutil
import audio_vehicle_condition_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-condition-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original condition snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters','src/vehicles/Vehicle.h']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_CONDITION_'+name,macros)
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "../entities/EntityStatus.h"
#include "../vehicles/VehicleBombTypes.h"
#include "sampman.h"

enum {
'''+m.C_CONSTANTS+'};\n'+macros+'\n'
for name,target in m.NAMES.items():
    args='cAudioManager *manager, cVehicleParams *params'
    source+='uint8_t\nAudioVehicleCondition_'+target+'('+args+')\n'+m.body(function(old,'cAudioManager::',name)).replace('{','{\n    // Preserve the complete vehicle condition sound operation with live state reads',1)+'\n\n'
(root/'src/audio/AudioVehicleCondition.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleHorn.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioVehicleHorn.c','AudioVehicleCondition.c')),newline='\n')
p=root/'src/vehicles/Vehicle.h';t=p.read_text();enum=re.search(r'enum eBombType\s*\{.*?\n\};',t,re.S).group();(root/'src/vehicles/VehicleBombTypes.h').write_text('//+ rouz edit (ChatGPT)\n#pragma once\n\n'+enum+'\n//- rouz edit (ChatGPT)\n',newline='\n');p.write_text(m.types(t),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;t=p.read_text();block=re.search(r'    <ClInclude Include="[^"\n]*WheelStatus.h"(?: />|>.*?</ClInclude>)',t,re.S).group();p.write_text(t.replace(block,block+'\n'+block.replace('WheelStatus.h','VehicleBombTypes.h')),newline='\n')
p=root/'utils/tests/audio_vehicle_doors_migration.py';t=p.read_text().replace('import re','import re\nimport audio_vehicle_condition_migration as condition',1).replace("    return source.replace('ProcessVehicleDoors(params)','AudioVehicleDoors_Process(this, &params)')", "    return condition.logic(source.replace('ProcessVehicleDoors(params)','AudioVehicleDoors_Process(this, &params)'))").replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')", "    return condition.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))").replace("    return source.replace('CDoor Doors[6];','CDoor Doors[VEHICLE_DOOR_COUNT];')", "    return condition.types(source.replace('CDoor Doors[6];','CDoor Doors[VEHICLE_DOOR_COUNT];'))");p.write_text(t,newline='\n')
prior=root/'build/audio-vehicle-doors-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-doors','audio-vehicle-condition').replace('AudioVehicleDoors','AudioVehicleCondition')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioVehicleCondition.c'","'src/audio/AudioVehicleDoors.c','src/audio/AudioVehicleCondition.c'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_vehicle_condition.ps1';p.write_text((root/'utils/tests/test_audio_vehicle_doors.ps1').read_text().replace('vehicleDoors','vehicleCondition').replace('VehicleDoors','VehicleCondition').replace('vehicle_doors','vehicle_condition').replace('vehicle-doors','vehicle-condition'),newline='\n')
print('Converted complete engine-damage and car-bomb ticking generators')
