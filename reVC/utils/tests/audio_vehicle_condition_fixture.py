# Audit both generators and model predicates and generate actual C comparisons
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_condition_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-condition-c-tests'
def normalize(s):
    # Compare all production statements independently of comments and formatting
    import fire_migration
    s=fire_migration.calls(s)
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehicleCondition.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioVehicleCondition_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.C_CONSTANTS) in normalize(kernel)
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_CONDITION_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('uint8_t\nAudioVehicleCondition_Damage')])
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
vehicle=(out/'before/src/vehicles/Vehicle.h').read_text();bomb=re.search(r'enum eBombType\s*\{.*?\n\};',vehicle,re.S).group()
assert normalize(bomb)==normalize((root/'src/vehicles/VehicleBombTypes.h').read_text().replace('#pragma once',''))
assert normalize(migration.types(vehicle))==normalize((root/'src/vehicles/Vehicle.h').read_text())
assert normalize(migration.HOST) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert '\tCAR_BOMB_TICK_VOLUME = 60,' in old and '\tCAR_BOMB_TICK_VOLUME = 60,' in (root/'src/audio/AudioVehicleOneShots.c').read_text()
assert 'int16 GetModelIndex(void) const { return m_modelIndex; }' in (root/'src/entities/Entity.h').read_text()
template=(root/'utils/tests/audio_vehicle_condition.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.C_CONSTANTS)
for marker,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('VEHICLE_TYPES','src/modelinfo/VehicleTypes.h'),('ENTITY_STATUS','src/entities/EntityStatus.h'),('BOMB_TYPES','src/vehicles/VehicleBombTypes.h')]:template=template.replace('@'+marker+'@',str(root/path).replace('\\','/'))
for variant in ['before','after']:
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','kind==0?active->ProcessEngineDamage(params):active->ProcessCarBombTick(params)' if variant=='before' else 'TestVehicleConditionC(active,&params,kind)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nuint8_t TestVehicleConditionC(cAudioManager *manager,cVehicleParams *params,unsigned kind)\n{\n    // Invoke both complete generators through their actual C declarations\n    if(kind==0)return AudioVehicleCondition_Damage(manager,params);\n    return AudioVehicleCondition_BombTick(manager,params);\n}\n',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','no-timescale','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-doors-c-tests'/(name+'.h')).read_bytes())
print('Complete condition generators, bomb enum, adapters, constants, callers and macros audited')
