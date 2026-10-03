# Known tyre and road-noise generators with live game field adapters
import re
from audio_controls_migration import function
NAMES={'ProcessVehicleFlatTyre':'FlatTyre','ProcessVehicleRoadNoise':'Dry','ProcessWetRoadNoise':'Wet'}
HOST='''//+ rouz edit (ChatGPT)
uint8_t
AudioVehicleRoadHost_WheelStatus(CVehicle *vehicle, int32_t type, int32_t wheel)
{
    // Read each original car damage query or bike wheel status at its loop position
    if(type == VEHICLE_TYPE_CAR) return ((CAutomobile *)vehicle)->Damage.GetWheelStatus(wheel);
    return ((CBike *)vehicle)->m_wheelStatus[wheel];
}

float
AudioVehicleRoadHost_WheelTimer(CVehicle *vehicle, int32_t type, int32_t wheel)
{
    // Read the wheel contact timer only after the corresponding burst status test
    if(type == VEHICLE_TYPE_CAR) return ((CAutomobile *)vehicle)->m_aWheelTimer[wheel];
    return ((CBike *)vehicle)->m_aWheelTimer[wheel];
}

uint8_t
AudioVehicleRoadHost_WheelsOnGround(CVehicle *vehicle, int32_t type)
{
    // Read the original car or bike ground-contact byte in its selected branch
    if(type == VEHICLE_TYPE_CAR) return ((CAutomobile *)vehicle)->m_nWheelsOnGround;
    return ((CBike *)vehicle)->m_nWheelsOnGround;
}

float
AudioVehicleRoadHost_MaxVelocity(const cTransmission *transmission)
{
    // Read maximum transmission velocity at every original expression evaluation
    return transmission->fMaxVelocity;
}

uint8_t
AudioVehicleRoadHost_Surface(const CVehicle *vehicle)
{
    // Read the current vehicle surface before selecting water or road audio
    return vehicle->m_nSurfaceTouched;
}

float
AudioVehicleRoadHost_WetRoads(void)
{
    // Read current road wetness after the relative velocity calculation
    return CWeather::WetRoads;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES=''.join('uint8_t AudioVehicleRoad_'+name+'(cAudioManager *manager, cVehicleParams *params);\n' for name in NAMES.values())+'uint8_t AudioVehicleRoadHost_WheelStatus(CVehicle *vehicle, int32_t type, int32_t wheel);\nfloat AudioVehicleRoadHost_WheelTimer(CVehicle *vehicle, int32_t type, int32_t wheel);\nuint8_t AudioVehicleRoadHost_WheelsOnGround(CVehicle *vehicle, int32_t type);\nfloat AudioVehicleRoadHost_MaxVelocity(const cTransmission *transmission);\nuint8_t AudioVehicleRoadHost_Surface(const CVehicle *vehicle);\nfloat AudioVehicleRoadHost_WetRoads(void);\n'
CONSTANTS=['\tFLAT_TYRE_MAX_DIST = 60,\n\tFLAT_TYRE_VOLUME = 100,\n','\tVEHICLE_ROAD_NOISE_MAX_DIST = 95,\n\tVEHICLE_ROAD_NOISE_VOLUME = 30,\n','\tWET_ROAD_NOISE_MAX_DIST = 30,\n\tWET_ROAD_NOISE_VOLUME = 23,\n']
def logic(source):
    # Remove the three complete business members and retain only game data access
    if 'cAudioManager::ProcessVehicleFlatTyre(' in source:
        for name in NAMES: source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        for block in CONSTANTS: source=source.replace(block,'')
        source+=HOST
    if HOST in source: source=source.replace(HOST,'')+HOST
    for name,target in NAMES.items(): source=source.replace(name+'(params)', 'AudioVehicleRoad_'+target+'(this, &params)')
    return source

def header(source):
    # Keep all owner and parameter fields while exposing explicit C generator operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')

def body(source):
    # Preserve wheel short circuits, repeated scalar reads, conversions and request ordering
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source);source=re.sub(r'\bparams\.', 'params->',source)
    source=source.replace('CAutomobile* automobile;', 'CVehicle *automobile;').replace('CBike* bike;', 'CVehicle *bike;')
    # Preserve each selected ground-contact branch before removing captured wheel-scan casts
    source=source.replace('((CAutomobile*)params->m_pVehicle)->m_nWheelsOnGround','AudioVehicleRoadHost_WheelsOnGround(params->m_pVehicle, VEHICLE_TYPE_CAR)').replace('((CBike*)params->m_pVehicle)->m_nWheelsOnGround','AudioVehicleRoadHost_WheelsOnGround(params->m_pVehicle, VEHICLE_TYPE_BIKE)')
    source=source.replace('(CAutomobile*)params->m_pVehicle','params->m_pVehicle').replace('(CBike*)params->m_pVehicle','params->m_pVehicle')
    source=source.replace('automobile->Damage.GetWheelStatus(i)','AudioVehicleRoadHost_WheelStatus(automobile, VEHICLE_TYPE_CAR, i)').replace('bike->m_wheelStatus[i]','AudioVehicleRoadHost_WheelStatus(bike, VEHICLE_TYPE_BIKE, i)')
    source=source.replace('automobile->m_aWheelTimer[i]','AudioVehicleRoadHost_WheelTimer(automobile, VEHICLE_TYPE_CAR, i)').replace('bike->m_aWheelTimer[i]','AudioVehicleRoadHost_WheelTimer(bike, VEHICLE_TYPE_BIKE, i)')
    source=source.replace('params->m_pTransmission->fMaxVelocity','AudioVehicleRoadHost_MaxVelocity(params->m_pTransmission)').replace('params->m_pVehicle->m_nSurfaceTouched','AudioVehicleRoadHost_Surface(params->m_pVehicle)').replace('CWeather::WetRoads','AudioVehicleRoadHost_WetRoads()')
    for old,new in [('bool8','uint8_t'),('uint8','uint8_t'),('uint32','uint32_t'),('TRUE','1'),('FALSE','0'),('nil','NULL')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('Abs(', 'fabsf(').replace('Min(', 'AudioVehicleRoad_Min(').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    source=re.sub(r'SQR\((\w+)\)',r'(\1 * \1)',source)
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_ROAD_'+name,source)
    return source

HELPER='static uint8_t AudioVehicleRoad_Less(float left, float right)\n{\n    // Retain the conditional second transmission read when MSVC optimizes float minimum expressions\n    volatile uint8_t result = left < right;\n    return result;\n}\n'
