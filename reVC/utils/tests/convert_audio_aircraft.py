from pathlib import Path
import re,shutil
import audio_aircraft_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/audio-aircraft-c-tests'
if (out/'before').exists():raise RuntimeError('Preserve the original aircraft snapshots')
for name in ['src/audio/AudioLogic.cpp','src/audio/AudioManager.h','build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    target=out/'before'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/name,target)
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
macros=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:macros=re.sub(r'\b'+name+r'\b','AUDIO_AIRCRAFT_'+name,macros)
source='''//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../modelinfo/ModelIds.h"
#include "../modelinfo/VehicleTypes.h"
#include "AudioPoliceGame.h"
#include <stddef.h>
#include "sampman.h"

enum {
'''+m.CONSTANTS+'};\n'+macros+'\n'
source+=m.HELPERS+'\n#define AUDIO_AIRCRAFT_MIN(a,b) ((a) < (b) ? (a) : (b))\n#define AUDIO_AIRCRAFT_MAX(a,b) ((a) > (b) ? (a) : (b))\n#define AUDIO_AIRCRAFT_ABS(a) (((a) < 0) ? (-(a)) : (a))\n#define AUDIO_AIRCRAFT_CLAMP(v,c,r) ((v) > (c) ? AUDIO_AIRCRAFT_MIN(v,c+r) : AUDIO_AIRCRAFT_MAX(v,c-r))\n\n'
for name,target in m.NAMES.items():
    args='cAudioManager *manager, cVehicleParams *params'
    ret='void' if target=='Cesna' else 'uint8_t'
    source+=ret+'\nAudioAircraft_'+target+'('+args+')\n'+m.body(function(old,'cAudioManager::',name)).replace('{','{\n    // Preserve the complete aircraft sound operation and original state history',1)+'\n\n'
(root/'src/audio/AudioAircraft.c').write_text(source+'//- rouz edit (ChatGPT)\n',newline='\n')
(root/'src/audio/AudioLogic.cpp').write_text(m.logic(old),newline='\n');(root/'src/audio/AudioManager.h').write_text(m.header(header),newline='\n')
for name in ['build/reVC.vcxproj','build/reVC.vcxproj.filters']:
    p=root/name;s=p.read_text();block=re.search(r'    <ClCompile Include="[^"\n]*AudioVehicleHorn.c"(?: />|>.*?</ClCompile>)',s,re.S).group();p.write_text(s.replace(block,block+'\n'+block.replace('AudioVehicleHorn.c','AudioAircraft.c')),newline='\n')
p=root/'utils/tests/audio_boat_migration.py';t=p.read_text().replace('import re','import re\nimport audio_aircraft_migration as aircraft',1).replace('    return source\n\ndef header','    return aircraft.logic(source)\n\ndef header',1).replace("    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')", "    return aircraft.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))");p.write_text(t,newline='\n')
prior=root/'build/audio-boat-c-tests'
for name in ['check_audio.ps1','check_native32.cmd','check_native64.cmd']:
    source=(prior/name).read_text().replace('audio-boat','audio-aircraft').replace('AudioBoat','AudioAircraft')
    if name.endswith('ps1'):source=source.replace("'src/audio/AudioAircraft.c'","'src/audio/AudioBoat.c','src/audio/AudioAircraft.c','src/vehicles/Heli.cpp'")
    (out/name).write_text(source,newline='\n')
p=root/'utils/tests/test_audio_aircraft.ps1';p.write_text((root/'utils/tests/test_audio_boat.ps1').read_text().replace('boat','aircraft').replace('Boat','Aircraft').replace("'AudioMath','AudioRequests'","'AudioMath','AudioGeometry','AudioRequests'").replace('"$aircraftBuild/AudioMath.o",','"$aircraftBuild/AudioMath.o","$aircraftBuild/AudioGeometry.o",').replace('"$aircraftSanBuild/AudioMath.o",','"$aircraftSanBuild/AudioMath.o","$aircraftSanBuild/AudioGeometry.o",'),newline='\n')
print('Converted complete Cessna and car-helicopter generators')
