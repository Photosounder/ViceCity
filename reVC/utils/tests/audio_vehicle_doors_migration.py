# Move the complete door generator while preserving repeated reads and reflection submission
import re
import audio_vehicle_condition_migration as condition
from audio_controls_migration import function
NAMES={'ProcessVehicleDoors':'Process'}
CONSTANTS='\tVEHICLE_DOORS_MAX_DIST = 40,\n\tVEHICLE_DOORS_VOLUME = 100,\n'
PROTOTYPES='uint8_t AudioVehicleDoors_Process(cAudioManager *manager, cVehicleParams *params);\nint32_t AudioVehicleDoorsHost_Status(CVehicle *vehicle, uint8_t door);\nint8_t AudioVehicleDoorsHost_State(const CVehicle *vehicle, uint8_t door);\nfloat AudioVehicleDoorsHost_Velocity(const CVehicle *vehicle, uint8_t door);\n'
HOST='//+ rouz edit (ChatGPT)\nextern "C" int32_t\nAudioVehicleDoorsHost_Status(CVehicle *vehicle, uint8_t door)\n{\n\t// Query the original damage manager on the captured automobile\n\treturn ((CAutomobile *)vehicle)->Damage.GetDoorStatus(door);\n}\nextern "C" int8_t\nAudioVehicleDoorsHost_State(const CVehicle *vehicle, uint8_t door)\n{\n\t// Read the original signed door state\n\treturn ((const CAutomobile *)vehicle)->Doors[door].m_nDoorState;\n}\nextern "C" float\nAudioVehicleDoorsHost_Velocity(const CVehicle *vehicle, uint8_t door)\n{\n\t// Read each original angular velocity expression\n\treturn ((const CAutomobile *)vehicle)->Doors[door].m_fAngVel;\n}\n//- rouz edit (ChatGPT)\n'
def logic(source):
    # Remove the complete member and its constants before changing internal calls
    if 'cAudioManager::ProcessVehicleDoors(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessVehicleDoors')+'\n','')+HOST
    for line in CONSTANTS.splitlines():source=source.replace(line+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    return condition.logic(source.replace('ProcessVehicleDoors(params)','AudioVehicleDoors_Process(this, &params)'))
def header(source):
    # Replace only the owning member declaration with C operations
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bProcessVehicleDoors\(',line))
    return condition.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Preserve original array order, captures, velocity comparisons and queue writes
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('CAutomobile *automobile','CVehicle *automobile').replace('(CAutomobile *)params->m_pVehicle','params->m_pVehicle')
    source=source.replace('ARRAY_SIZE(automobile->Doors)','VEHICLE_DOOR_COUNT').replace('automobile->Damage.GetDoorStatus(i)','AudioVehicleDoorsHost_Status(automobile, i)').replace('automobile->Doors[i].m_nDoorState','AudioVehicleDoorsHost_State(automobile, i)').replace('automobile->Doors[i].m_fAngVel','AudioVehicleDoorsHost_Velocity(automobile, i)')
    for a,b in [('int8','int8_t'),('uint8','uint8_t'),('TRUE','1'),('FALSE','0'),('Abs','fabsf'),('Min','AUDIO_VEHICLE_DOORS_MIN')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('SQR(VEHICLE_DOORS_MAX_DIST)','(VEHICLE_DOORS_MAX_DIST * VEHICLE_DOORS_MAX_DIST)').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_DOORS_'+name,source)
    return source

def types(source):
    # Follow the exact shared enum extraction and fixed array bound in older header audits
    for name in ['eDoorState','eDoorStatus']:
        source=re.sub(r'enum '+name+r'\s*\{.*?\n\};','#include "DoorTypes.h"',source,flags=re.S)
    return condition.types(source.replace('CDoor Doors[6];','CDoor Doors[VEHICLE_DOOR_COUNT];'))
