from pathlib import Path
import re,shutil
import audio_vehicle_warning_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-warning-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original warning snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_WARNING_'+name,macros)
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "AudioVehicleModels.h"
#include "sampman.h"

enum {
'''+m.CONSTANTS+'};\n'+macros+'\n'
for name,target in m.NAMES.items():
    args='uint32_t model' if target in ['UsesReverse','HasAirBrakes'] else 'cAudioManager *manager, cVehicleParams *params'
    source+='uint8_t\nAudioVehicleWarning_'+target+'('+args+')\n'+m.body(function(old,'cAudioManager::',name)).replace('{','{\n    // Preserve the original model predicate or complete warning sound operation',1)+'\n\n'
(root/'src/audio/AudioVehicleWarning.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleHorn.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioVehicleHorn.c','AudioVehicleWarning.c')),newline='\n')
p=root/'utils/tests/audio_vehicle_horn_migration.py';s=p.read_text().replace('import re','import re\nimport audio_vehicle_warning_migration as warning',1).replace("    return source.replace('ProcessVehicleHorn(params)','AudioVehicleHorn_Horn(this, &params)').replace('ProcessVehicleSirenOrAlarm(params)','AudioVehicleHorn_Siren(this, &params)')","    return warning.logic(source.replace('ProcessVehicleHorn(params)','AudioVehicleHorn_Horn(this, &params)').replace('ProcessVehicleSirenOrAlarm(params)','AudioVehicleHorn_Siren(this, &params)'))").replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')","    return warning.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(s,newline='\n')
prior=root/'build/audio-vehicle-horn-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-horn','audio-vehicle-warning').replace('AudioVehicleHorn','AudioVehicleWarning')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioVehicleWarning.c'","'src/audio/AudioVehicleHorn.c','src/audio/AudioVehicleWarning.c'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_vehicle_warning.ps1';p.write_text((root/'utils/tests/test_audio_vehicle_horn.ps1').read_text().replace('vehicleHorn','vehicleWarning').replace('VehicleHorn','VehicleWarning').replace('vehicle_horn','vehicle_warning').replace('vehicle-horn','vehicle-warning'),newline='\n')
print('Converted reversing and air-brake generators and both model predicates')
