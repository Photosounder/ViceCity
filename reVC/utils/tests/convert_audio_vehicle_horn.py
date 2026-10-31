from pathlib import Path
import re,shutil
import audio_vehicle_horn_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-horn-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original horn and siren snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
(root/'src/audio/AudioVehicleModels.h').write_text('//+ rouz edit (ChatGPT)\n#pragma once\n\n'+m.models(old)+'\n//- rouz edit (ChatGPT)\n',newline='\n')
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_HORN_'+name,macros)
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "AudioVehicleModels.h"
#include "../modelinfo/ModelIds.h"
#include "../entities/EntityStatus.h"
#include "sampman.h"

enum {
'''+m.CONSTANTS+'''};
#define AudioVehicleHorn_Min(a,b) ((a) < (b) ? (a) : (b))
'''+macros+'\n'+m.pattern(old).replace('Const static bool8','static const uint8_t').replace('TRUE','1').replace('FALSE','0')+'''

static uint8_t AudioVehicleHorn_Uses(cVehicleParams *params);
static uint8_t AudioVehicleHorn_Switching(cVehicleParams *params);

'''
for name,target in m.NAMES.items():
    args='cVehicleParams *params' if target in ['Uses','Switching'] else 'cAudioManager *manager, cVehicleParams *params'
    source+=('static uint8_t' if target in ['Uses','Switching'] else 'uint8_t')+'\nAudioVehicleHorn_'+target+'('+args+')\n'+m.body(function(old,'cAudioManager::',name)).replace('{','{\n    // Preserve the original horn, siren or switching operation and its query order',1)+'\n\n'
(root/'src/audio/AudioVehicleHorn.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleSkid.c"(?: />|>.*?</ClCompile>)',s,re.S).group();s=s.replace(block,block+'\n'+block.replace('AudioVehicleSkid.c','AudioVehicleHorn.c'));block=re.search(r'    <ClInclude Include="[^"\n]*AudioSamples.h"(?: />|>.*?</ClInclude>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioSamples.h','AudioVehicleModels.h')),newline='\n')
p=root/'utils/tests/audio_vehicle_skid_migration.py';s=p.read_text().replace('import re','import re\nimport audio_vehicle_horn_migration as horn',1).replace("    return source.replace('ProcessVehicleSkidding(params)','AudioVehicleSkid_Process(this, &params)')","    return horn.logic(source.replace('ProcessVehicleSkidding(params)','AudioVehicleSkid_Process(this, &params)'))").replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')","    return horn.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(s,newline='\n')
prior=root/'build/audio-vehicle-skid-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-skid','audio-vehicle-horn').replace('AudioVehicleSkid','AudioVehicleHorn')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioVehicleHorn.c'","'src/audio/AudioVehicleSkid.c','src/audio/AudioVehicleHorn.c'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_vehicle_horn.ps1';p.write_text((root/'utils/tests/test_audio_vehicle_skid.ps1').read_text().replace('vehicleSkid','vehicleHorn').replace('VehicleSkid','VehicleHorn').replace('vehicle_skid','vehicle_horn').replace('vehicle-skid','vehicle-horn'),newline='\n')
print('Converted horn and siren/alarm generators, predicates, pattern and shared audio model indices')
