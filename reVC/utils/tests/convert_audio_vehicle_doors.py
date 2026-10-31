from pathlib import Path
import re,shutil
import audio_vehicle_doors_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-doors-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original door snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters','src/vehicles/Door.h','src/vehicles/DamageManager.h','src/vehicles/Automobile.h']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_DOORS_'+name,macros)
door=(root/'src/vehicles/Door.h').read_text();damage=(root/'src/vehicles/DamageManager.h').read_text()
state=re.search(r'enum eDoorState\n\{.*?\n\};',door,re.S).group();status=re.search(r'enum eDoorStatus\n\{.*?\n\};',damage,re.S).group()
(root/'src/vehicles/DoorTypes.h').write_text('//+ rouz edit (ChatGPT)\n#pragma once\n\n'+state+'\n\n'+status+'\n\nenum { VEHICLE_DOOR_COUNT = 6 };\n//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/vehicles/Door.h').write_text('//+ rouz edit (ChatGPT)\n'+door.replace(state,'#include "DoorTypes.h"')+'\n//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/vehicles/DamageManager.h').write_text(damage.replace(status,'#include "DoorTypes.h"'),newline='\n')
p=root/'src/vehicles/Automobile.h';p.write_text('//+ rouz edit (ChatGPT)\n'+p.read_text().replace('CDoor Doors[6];','CDoor Doors[VEHICLE_DOOR_COUNT];')+'\n//- rouz edit (ChatGPT)\n',newline='\n')
source='//+ rouz edit (ChatGPT)\n#include "AudioManager.h"\n#include "AudioSamples.h"\n#include "../vehicles/DoorTypes.h"\n#include "sampman.h"\n#include <math.h>\n\n#define AUDIO_VEHICLE_DOORS_MIN(a,b) ((a) < (b) ? (a) : (b))\n\nenum {\n'+m.CONSTANTS+'};\n'+macros+'\n'
source+='uint8_t\nAudioVehicleDoors_Process(cAudioManager *manager, cVehicleParams *params)\n'+m.body(function(old,'cAudioManager::','ProcessVehicleDoors')).replace('{','{\n\t// Preserve the complete per-door sound operation and reflection submission',1)+'\n\n'
(root/'src/audio/AudioVehicleDoors.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleHorn.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioVehicleHorn.c','AudioVehicleDoors.c')),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClInclude Include="[^"\n]*WheelStatus.h"(?: />|>.*?</ClInclude>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('WheelStatus.h','DoorTypes.h')),newline='\n')
p=root/'utils/tests/audio_vehicle_warning_migration.py' ;t=p.read_text().replace('import re','import re\nimport audio_vehicle_doors_migration as doors',1).replace('    return source\ndef header','    return doors.logic(source)\ndef header',1).replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')", "    return doors.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(t,newline='\n')
prior=root/'build/audio-vehicle-warning-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-warning','audio-vehicle-doors').replace('AudioVehicleWarning','AudioVehicleDoors')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioVehicleDoors.c'","'src/audio/AudioVehicleWarning.c','src/audio/AudioVehicleDoors.c','src/vehicles/Door.cpp','src/vehicles/DamageManager.cpp','src/vehicles/Automobile.cpp'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_vehicle_doors.ps1';p.write_text((root/'utils/tests/test_audio_vehicle_warning.ps1').read_text().replace('vehicleWarning','vehicleDoors').replace('VehicleWarning','VehicleDoors').replace('vehicle_warning','vehicle_doors').replace('vehicle-warning','vehicle-doors'),newline='\n')
print('Converted the complete vehicle door sound generator')
