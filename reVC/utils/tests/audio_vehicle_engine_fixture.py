# Audit the complete engine conversion and generate before/after runtime fixtures
from pathlib import Path
import re
import audio_vehicle_doors_migration as doors
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_engine_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-engine-c-tests'
def normalize(s):
    # Compare production statements independently of comments and whitespace
    import fire_migration
    s=fire_migration.calls(s)
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehicleEngine.c').read_text()
method=function(old,'cAudioManager::','ProcessVehicleEngine');actual=function(kernel,'AudioVehicleEngine_','Process')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header).replace('#pragma once','#pragma once\n#include "../vehicles/WheelState.h"',1))==normalize((root/'src/audio/AudioManager.h').read_text())
for owner,pattern,new in [('src/entities/Entity.h',r'enum eEntityStatus\s*\{.*?\};','src/entities/EntityStatus.h'),('src/vehicles/Vehicle.h',r'enum tWheelState\s*\{.*?\};','src/vehicles/WheelState.h'),('src/vehicles/Automobile.h',r'enum \{\s*CARWHEEL_FRONT_LEFT,.*?\};','src/vehicles/WheelIndices.h'),('src/vehicles/Bike.h',r'enum \{\s*BIKEWHEEL_FRONT,.*?\};','src/vehicles/WheelIndices.h')]:
    before=(out/'before'/owner).read_text();block=re.search(pattern,before,re.S).group()
    assert normalize(block) in normalize((root/new).read_text())
    assert normalize(doors.types(before.replace(block,'#include "'+Path(new).name+'"')))==normalize((root/owner).read_text())
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_ENGINE_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2\n#define AUDIO_VEHICLE_ENGINE_RESET'):kernel.index('uint8_t\nAudioVehicleEngine_Process')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_engine.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS)
# Use absolute includes for the standalone generated fixture paths
template=re.sub(r'#include "(\.\./[^"\n]+)"',lambda m:'#include "'+str((root/'src/audio'/m[1]).resolve()).replace('\\','/')+'"',template)
for variant in ['before','after']:
    source=template.replace('@METHODS@',method if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@RUN@','active->ProcessVehicleEngine(params)' if variant=='before' else 'TestVehicleEngineC(active,&params)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
uint8_t TestVehicleEngineC(cAudioManager *manager,cVehicleParams *params)
{
    // Invoke the complete main vehicle engine generator through its C declaration
    return AudioVehicleEngine_Process(manager,params);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-model-c-tests'/(name+'.h')).read_bytes())
print('Complete engine generator, adapters, enum values, constants and queue macros audited')
