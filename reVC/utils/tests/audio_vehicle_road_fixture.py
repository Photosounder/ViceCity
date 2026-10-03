# Audit three complete tyre and road generators and shared enum extraction
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_road_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-road-c-tests'
def normalize(s):
    # Compare complete statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioVehicleRoad.c').read_text()
for name,target in migration.NAMES.items():
    method=function(old,'cAudioManager::',name);actual=function(kernel,'AudioVehicleRoad_',target)
    assert normalize(migration.body(method))==normalize(actual[actual.index('{'):]),name
for name,transform in [('AudioLogic.cpp',migration.logic),('AudioManager.h',migration.header)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
for path,enum,new in [('src/modelinfo/VehicleModelInfo.h','eVehicleType','VehicleTypes.h'),('src/vehicles/DamageManager.h','eWheelStatus','WheelStatus.h')]:
    saved=(out/'before'/path).read_text();block=re.search(r'enum '+enum+r'\s*\{.*?\n\};',saved,re.S).group()
    assert normalize(saved.replace(block,'#include "'+new+'"'))==normalize((root/path).read_text())
    assert normalize(block)==normalize((root/Path(path).parent/new).read_text().replace('#pragma once',''))
header=(out/'before/src/audio/AudioManager.h').read_text();macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_ROAD_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('uint8_t\nAudioVehicleRoad_FlatTyre')])
for block in migration.CONSTANTS:assert normalize(block) in normalize(kernel)
assert normalize(migration.HELPER)==normalize(function(kernel,'AudioVehicleRoad_','Less'))
assert '#define AudioVehicleRoad_Min(a,b) (AudioVehicleRoad_Less((a), (b)) ? (a) : (b))' in kernel
assert 'inline float Abs(float x) { return fabsf(x); }' in (root/'src/math/maths.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_road.cpp.in').read_text().replace('../src/modelinfo/VehicleTypes.h',str(root/'src/modelinfo/VehicleTypes.h').replace('\\','/')).replace('../src/vehicles/WheelStatus.h',str(root/'src/vehicles/WheelStatus.h').replace('\\','/')).replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',''.join(migration.CONSTANTS))
methods='\n'.join(function(old,'cAudioManager::',name) for name in migration.NAMES)
for variant in ['before','after']:
    source=template.replace('@METHODS@',methods if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@RUN@','kind==0?active->ProcessVehicleFlatTyre(params):kind==1?active->ProcessVehicleRoadNoise(params):active->ProcessWetRoadNoise(params)' if variant=='before' else 'TestVehicleRoadC(active,&params,kind)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
uint8_t TestVehicleRoadC(cAudioManager *manager,cVehicleParams *params,unsigned kind)
{
    // Return the actual C tyre or road generator result to the C++ test caller
    if(kind==0) return AudioVehicleRoad_FlatTyre(manager,params);
    if(kind==1) return AudioVehicleRoad_Dry(manager,params);
    return AudioVehicleRoad_Wet(manager,params);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-rain-c-tests'/(name+'.h')).read_bytes())
print('Complete tyre/dry/wet generators, adapters, enums and macros audited')
