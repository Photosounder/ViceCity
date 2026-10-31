# Audit the complete one-shot event loop and compare its actual original bodies
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_one_shots_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-one-shots-c-tests'
def normalize(source):
    # Compare all production statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehicleOneShots.c').read_text();method=migration.extract(old);actual=migration.extract(kernel,'void\nAudioVehicleOneShots_Process')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(__import__('audio_ped_speech_migration').logic(migration.HOST)) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.CONSTANTS) in normalize(kernel)
assert normalize(migration.DOOR_ENUM) in normalize((root/'src/audio/AudioVehicleDoorTypes.h').read_text())
assert normalize(migration.calls((out/'before/src/audio/AudioVehicle.c').read_text()))==normalize((root/'src/audio/AudioVehicle.c').read_text())
for name in ['reVC.vcxproj','reVC.vcxproj.filters']:assert migration.project((out/'before/build'/name).read_text())==(root/'build'/name).read_text()
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_ONE_SHOTS_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioVehicleOneShots_Process')])
# Preserve shared histories and every original event label across all conditional branches
for line in re.findall(r'static (?:uint8|uint32) \w+ = \d+;',method):assert normalize(line.replace('uint8','uint8_t').replace('uint32','uint32_t')) in normalize(kernel)
events=list(dict.fromkeys(re.findall(r'case (SOUND_\w+):',method)));events.append('SOUND_BOAT_SLOWDOWN')
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
a=old.index('struct tVehicleSampleData {');b=old.index(' };',old.index('const tVehicleSampleData aVehicleSettings',a))+3;table=old[a:b]
# Instrument only the immutable door-field read without changing table values
original_table=table;table=table.replace('uint8 m_bDoorType;','DoorValue m_bDoorType;')
assert original_table in old
prototypes=['extern "C" '+line for line in migration.PROTOTYPES.strip().split('\n')]
for name in ['AudioAircraftHost_WheelSpeed','AudioVehicleHost_Appearance','AudioVehicleReverseHost_Model']:
 prototype=re.search(r'^[^\n{}]+\b'+name+r'\([^\n]*\);',header,re.M).group();prototypes.append('extern "C" '+prototype)
prototypes.append('extern "C" CVehicle *AudioGame_FindVehicle(void);')
template=(root/'utils/tests/audio_vehicle_one_shots.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS).replace('@TABLE@',table).replace('@PROTOTYPES@','\n'.join(prototypes)).replace('@EVENTS@','static const int events[]={'+','.join(events)+'};')
for marker,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('VEHICLE_TYPES','src/modelinfo/VehicleTypes.h'),('APPEARANCE','src/modelinfo/VehicleAppearance.h'),('WEAPON_TYPES','src/weapons/WeaponType.h')]:template=template.replace('@'+marker+'@',str(root/path).replace('\\','/'))
for variant in ['before','after']:
 source=template.replace('@METHOD@',method if variant=='before' else '').replace('@RUN@','active->ProcessVehicleOneShots(params)' if variant=='before' else 'TestVehicleOneShotsC(active,&params)')
 (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestVehicleOneShotsC(cAudioManager *manager,cVehicleParams *params)\n{\n    // Invoke the complete event loop through its actual C declaration\n    AudioVehicleOneShots_Process(manager,params);\n}\n',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','no-timescale','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-plane-c-tests'/(name+'.h')).read_bytes())
for name,flag in [('train','#define GTA_TRAIN\n'),('event-reload','#define THIS_IS_STUPID\n')]:
 (out/(name+'.h')).write_text('#include "config.h"\n#define AUDIO_OAL\n'+flag,newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Complete one-shot event loop, seven histories, constants, door enum, macros, adapters, direct caller and '+str(len(events)-1)+' event labels audited')
