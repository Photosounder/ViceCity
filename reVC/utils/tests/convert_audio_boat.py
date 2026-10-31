from pathlib import Path
import re,shutil
import audio_boat_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-boat-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original boat snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_BOAT_'+name,macros)
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../entities/EntityStatus.h"
#include "AudioPoliceGame.h"
#include <math.h>
#include "sampman.h"

enum {
'''+m.CONSTANTS+'};\n'+macros+'\n'
source+=m.HELPER+'\n#define AUDIO_BOAT_MIN(a,b) ((a) < (b) ? (a) : (b))\n#define AUDIO_BOAT_MAX(a,b) (AudioBoat_Decision((a) > (b)) ? (a) : (b))\n\n'
for name,target in m.NAMES.items():
    args='cAudioManager *manager, cVehicleParams *params'
    source+='uint8_t\nAudioBoat_'+target+'('+args+')\n'+m.body(function(old,'cAudioManager::',name)).replace('{','{\n    // Preserve the complete boat sound operation and shared player history',1)+'\n\n'
(root/'src/audio/AudioBoat.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleHorn.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioVehicleHorn.c','AudioBoat.c')),newline='\n')
p=root/'utils/tests/audio_vehicle_condition_migration.py';t=p.read_text().replace('import re','import re\nimport audio_boat_migration as boat',1).replace('    return source\n\ndef header','    return boat.logic(source)\n\ndef header',1).replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')", "    return boat.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(t,newline='\n')
prior=root/'build/audio-vehicle-condition-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-condition','audio-boat').replace('AudioVehicleCondition','AudioBoat')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioBoat.c'","'src/audio/AudioVehicleCondition.c','src/audio/AudioBoat.c','src/vehicles/Boat.cpp'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_boat.ps1';p.write_text((root/'utils/tests/test_audio_vehicle_condition.ps1').read_text().replace('vehicleCondition','boat').replace('VehicleCondition','Boat').replace('vehicle_condition','boat').replace('vehicle-condition','boat'),newline='\n')
print('Converted complete boat engine and water-movement generators')
