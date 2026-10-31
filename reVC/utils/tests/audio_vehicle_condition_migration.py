# Move complete engine-damage and bomb-tick generators with live vehicle reads
import re
import audio_boat_migration as boat
from audio_controls_migration import function
NAMES={'ProcessEngineDamage':'Damage','ProcessCarBombTick':'BombTick'}
CONSTANTS='\tENGINE_DAMAGE_MAX_DIST = 40,\n\tCAR_BOMB_TICK_MAX_DIST = 40,\n'
C_CONSTANTS=CONSTANTS+'\tCAR_BOMB_TICK_VOLUME = 60,\n'
PROTOTYPES='uint8_t AudioVehicleCondition_Damage(cAudioManager *manager, cVehicleParams *params);\nuint8_t AudioVehicleCondition_BombTick(cAudioManager *manager, cVehicleParams *params);\nfloat AudioVehicleConditionHost_Health(const CVehicle *vehicle);\nuint8_t AudioVehicleConditionHost_Bomb(const CVehicle *vehicle);\nvoid AudioVehicleConditionHost_Unknown(int32_t type);\n'
HOST='//+ rouz edit (ChatGPT)\nextern "C" float\nAudioVehicleConditionHost_Health(const CVehicle *vehicle)\n{\n\t// Read health at each original condition evaluation\n\treturn vehicle->m_fHealth;\n}\nextern "C" uint8_t\nAudioVehicleConditionHost_Bomb(const CVehicle *vehicle)\n{\n\t// Read the original three-bit bomb type on the current vehicle\n\treturn vehicle->m_bombType;\n}\nextern "C" void\nAudioVehicleConditionHost_Unknown(int32_t type)\n{\n\t// Retain the original unsupported vehicle diagnostic\n\tdebug("\\n * AUDIOLOG:  ProcessCarBombTick()  Unsupported vehicle type %d * \\n", type);\n}\n//- rouz edit (ChatGPT)\n'
def logic(source):
    # Remove both complete members and only constants without remaining C++ consumers
    if 'cAudioManager::ProcessEngineDamage(' in source:
        for name in NAMES:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    for line in CONSTANTS.splitlines():source=source.replace(line+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    for name,target in NAMES.items():source=source.replace(name+'(params)','AudioVehicleCondition_'+target+'(this, &params)')
    return boat.logic(source)

def header(source):
    # Replace only the two owning member declarations with C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return boat.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def types(source):
    # Follow the exact shared bomb enum extraction in older vehicle header audits
    import audio_vehicle_migration
    return audio_vehicle_migration.types(re.sub(r'enum eBombType\s*\{.*?\n\};','#include "VehicleBombTypes.h"',source,flags=re.S))

def body(source):
    # Preserve every live vehicle read, short circuit, threshold and sound request field
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    for a,b in [('params->m_pVehicle->m_modelIndex','AudioVehicleReverseHost_Model(params->m_pVehicle)'),('params->m_pVehicle->GetStatus()','AudioVehicleEngineHost_Status(params->m_pVehicle)'),('params->m_pVehicle->m_fHealth','AudioVehicleConditionHost_Health(params->m_pVehicle)'),('params->m_pVehicle->bIsDrowning','AudioVehicleModelHost_Drowning(params->m_pVehicle)'),('params->m_pVehicle->bEngineOn','AudioVehicleReverseHost_Engine(params->m_pVehicle)'),('params->m_pVehicle->m_bombType','AudioVehicleConditionHost_Bomb(params->m_pVehicle)')]:source=source.replace(a,b)
    source=source.replace('debug("\\n * AUDIOLOG:  ProcessCarBombTick()  Unsupported vehicle type %d * \\n", params->m_VehicleType)','AudioVehicleConditionHost_Unknown(params->m_VehicleType)')
    for a,b in [('uint8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in ['ENGINE_DAMAGE_MAX_DIST','CAR_BOMB_TICK_MAX_DIST']:source=source.replace('SQR('+name+')','('+name+' * '+name+')')
    source=source.replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_CONDITION_'+name,source)
    return source
