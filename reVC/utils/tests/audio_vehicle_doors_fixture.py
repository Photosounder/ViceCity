# Audit both generators and model predicates and generate actual C comparisons
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_doors_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-doors-c-tests'
def normalize(s):
    # Compare all production statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehicleDoors.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioVehicleDoors_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.CONSTANTS) in normalize(kernel)
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_DOORS_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('uint8_t\nAudioVehicleDoors_Process')])
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
for name,enum_name in [('Door.h','eDoorState'),('DamageManager.h','eDoorStatus')]:
    previous=(out/'before/src/vehicles'/name).read_text();enum=re.search(r'enum '+enum_name+r'\n\{.*?\n\};',previous,re.S).group()
    assert normalize(enum) in normalize((root/'src/vehicles/DoorTypes.h').read_text())
    assert normalize(previous.replace(enum,'#include "DoorTypes.h"'))==normalize((root/'src/vehicles'/name).read_text())
assert normalize((out/'before/src/vehicles/Automobile.h').read_text().replace('CDoor Doors[6];','CDoor Doors[VEHICLE_DOOR_COUNT];'))==normalize((root/'src/vehicles/Automobile.h').read_text())
assert 'enum { VEHICLE_DOOR_COUNT = 6 };' in (root/'src/vehicles/DoorTypes.h').read_text()
assert 'return m_doorStatus[door];' in function((root/'src/vehicles/DamageManager.cpp').read_text(),'CDamageManager::','GetDoorStatus')
assert normalize(migration.HOST) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
template=(root/'utils/tests/audio_vehicle_doors.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS).replace('@DOOR_TYPES@',str(root/'src/vehicles/DoorTypes.h').replace('\\','/'))
for variant in ['before','after']:
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','active->ProcessVehicleDoors(params)' if variant=='before' else 'TestVehicleDoorsC(active,&params)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nuint8_t TestVehicleDoorsC(cAudioManager *manager,cVehicleParams *params)\n{\n    // Invoke the complete generator through its actual C declaration\n    return AudioVehicleDoors_Process(manager,params);\n}\n',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-horn-c-tests'/(name+'.h')).read_bytes())
(out/'no-timescale.h').write_text('#include "config.h"\n#define AUDIO_OAL\n#undef USE_TIME_SCALE_FOR_AUDIO\n',newline='\n')
print('Complete door generator, shared enums and count, adapters, constants, callers and macros audited')
