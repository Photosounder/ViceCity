from pathlib import Path
import re,shutil
import audio_vehicle_engine_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-engine-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original engine snapshots instead of running this conversion twice')
files=['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','src/entities/Entity.h','src/vehicles/Vehicle.h','src/vehicles/Automobile.h','src/vehicles/Bike.h','build/reVC.vcxproj','build/reVC.vcxproj.filters']
for name in files:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
def write(name,content):
    (root/name).write_text(content,newline='\n')
blocks=[]
for owner,pattern,new in [('src/entities/Entity.h',r'enum eEntityStatus\s*\{.*?\};','src/entities/EntityStatus.h'),('src/vehicles/Vehicle.h',r'enum tWheelState\s*\{.*?\};','src/vehicles/WheelState.h'),('src/vehicles/Automobile.h',r'enum \{\s*CARWHEEL_FRONT_LEFT,.*?\};','src/vehicles/WheelIndices.h'),('src/vehicles/Bike.h',r'enum \{\s*BIKEWHEEL_FRONT,.*?\};','src/vehicles/WheelIndices.h')]:
    source=(root/owner).read_text();block=re.search(pattern,source,re.S).group();blocks.append((new,block))
    source=source.replace(block,'#include "'+Path(new).name+'"')
    if not source.startswith('//+ rouz edit (ChatGPT)'):source='//+ rouz edit (ChatGPT)\n'+source+'\n//- rouz edit (ChatGPT)\n'
    write(owner,source)
for new in dict(blocks):write(new,'//+ rouz edit (ChatGPT)\n#pragma once\n\n'+'\n\n'.join(b for n,b in blocks if n==new)+'\n//- rouz edit (ChatGPT)\n')
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_ENGINE_'+name,macros)
prefix='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioPoliceGame.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "../entities/EntityStatus.h"
#include "../vehicles/WheelIndices.h"
#include "sampman.h"
#include <math.h>
#include <stddef.h>

#ifndef GTA_PS2
#define CHANNEL_PLAYER_VEHICLE_ENGINE manager->m_nActiveSamples
#endif

static uint8_t AudioVehicleEngine_Decision(uint8_t result)
{
    // Retain repeated transmission reads in floating minimum expressions
    volatile uint8_t comparison = result;
    return comparison;
}
#define AudioVehicleEngine_Min(a,b) (AudioVehicleEngine_Decision((a) < (b)) ? (a) : (b))
'''
write('src/audio/AudioVehicleEngine.c',prefix+'\nenum {\n'+m.CONSTANTS+'};\n\n'+macros+'\nuint8_t\nAudioVehicleEngine_Process(cAudioManager *manager, cVehicleParams *params)\n'+m.body(function(old,'cAudioManager::','ProcessVehicleEngine')).replace('{','{\n    // Preserve engine selection, traction arithmetic and queued sound fields',1)+'\n//- rouz edit (ChatGPT)\n')
write('src/audio/AudioLogic.cpp',m.logic(old));write('src/audio/AudioManager.h',m.header(header).replace('#pragma once','#pragma once\n#include "../vehicles/WheelState.h"',1))
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    source=(root/name).read_text()
    block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleModel.c"(?: />|>.*?</ClCompile>)',source,re.S).group()
    source=source.replace(block,block+'\n'+block.replace('AudioVehicleModel.c','AudioVehicleEngine.c'))
    for new in dict(blocks):
        reference='EntityTypes.h' if 'EntityStatus' in new else 'WheelStatus.h'
        hb=re.search(r'    <ClInclude Include="[^"\n]*'+reference+r'"(?: />|>.*?</ClInclude>)',source,re.S).group()
        # Keep the existing project path and header filter convention
        source=source.replace(hb,hb+'\n'+hb.replace(reference,Path(new).name))
    write(name,source)
prior=root/'build/audio-vehicle-model-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-vehicle-model','audio-vehicle-engine').replace('AudioVehicleModel','AudioVehicleEngine')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioVehicleEngine.c'","'src/audio/AudioVehicleModel.c','src/audio/AudioVehicleEngine.c'")
    (out/name).write_text(source,newline='\n')
source=(root/'utils/tests/test_audio_vehicle_model.ps1').read_text().replace('vehicleModel','vehicleEngine').replace('VehicleModel','VehicleEngine').replace('vehicle_model','vehicle_engine').replace('vehicle-model','vehicle-engine').replace(",'alternate'",'')
write('utils/tests/test_audio_vehicle_engine.ps1',source)
# Extend the previous pure audit normalizations to the latest migration
p=root/'utils/tests/audio_vehicle_model_migration.py';s=p.read_text();s=s.replace('import re','import re\nimport audio_vehicle_engine_migration as engine',1).replace('    return source\n\ndef header','    return engine.logic(source)\n\ndef header',1).replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')","    return engine.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')).replace('#pragma once','#pragma once\\n#include \"../vehicles/WheelState.h\"',1)")
p.write_text(s,newline='\n')
print('Converted complete vehicle engine kernel and shared enums')
