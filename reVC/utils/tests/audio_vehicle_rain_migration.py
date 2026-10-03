# Known rain-on-vehicle generator migration with unchanged counter storage
import re
import audio_vehicle_road_migration as road
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
float
AudioVehicleRainHost_Rain(void)
{
    // Read rainfall at each original gate and emitting-volume query
    return CWeather::Rain;
}

uint8_t
AudioVehicleRainHost_CameraNoRain(void)
{
    // Query the existing camera rain exclusion at its short-circuit point
    return CCullZones::CamNoRain();
}

uint8_t
AudioVehicleRainHost_PlayerNoRain(void)
{
    // Query the existing player rain exclusion only when the camera excludes rain
    return CCullZones::PlayerNoRain();
}

void
AudioVehicleRainHost_Counters(CVehicle *vehicle, uint8_t **audioCounter, uint8_t **sampleCounter)
{
    // Expose the original byte counters without copying or changing the vehicle layout
    *audioCounter = &vehicle->m_bRainAudioCounter;
    *sampleCounter = &vehicle->m_bRainSamplesCounter;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='void AudioVehicleRain_Process(cAudioManager *manager, cVehicleParams *params);\nfloat AudioVehicleRainHost_Rain(void);\nuint8_t AudioVehicleRainHost_CameraNoRain(void);\nuint8_t AudioVehicleRainHost_PlayerNoRain(void);\nvoid AudioVehicleRainHost_Counters(CVehicle *vehicle, uint8_t **audioCounter, uint8_t **sampleCounter);\n'
CONSTANTS='\tRAIN_ON_VEHICLE_MAX_DIST = 22,\n\tRAIN_ON_VEHICLE_VOLUME = 30,\n'
def logic(source):
    # Remove only this generator and its now-private constants from the full game source
    if 'cAudioManager::ProcessRainOnVehicle(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessRainOnVehicle')+'\n','').replace(CONSTANTS,'')+HOST
    # Keep this newest game adapter block after earlier migration adapters
    if HOST in source: source=source.replace(HOST,'')+HOST
    return road.logic(source.replace('ProcessRainOnVehicle(params);','AudioVehicleRain_Process(this, &params);'))
def header(source):
    # Keep owner and vehicle parameter fields while exposing the C generator
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bProcessRainOnVehicle\(',line))
    return road.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Retain the ordered gates, captured vehicle, live rainfall and byte increments
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=re.sub(r'\bparams\.', 'params->',source)
    source=source.replace('SQR(RAIN_ON_VEHICLE_MAX_DIST)','(RAIN_ON_VEHICLE_MAX_DIST * RAIN_ON_VEHICLE_MAX_DIST)').replace('CWeather::Rain','AudioVehicleRainHost_Rain()').replace('CCullZones::CamNoRain()','AudioVehicleRainHost_CameraNoRain()').replace('CCullZones::PlayerNoRain()','AudioVehicleRainHost_PlayerNoRain()')
    source=source.replace('CVehicle *veh = params->m_pVehicle;', 'CVehicle *veh = params->m_pVehicle;\n        uint8_t *audioCounter, *sampleCounter;\n        AudioVehicleRainHost_Counters(veh, &audioCounter, &sampleCounter);')
    source=source.replace('veh->m_bRainAudioCounter','(*audioCounter)').replace('veh->m_bRainSamplesCounter','(*sampleCounter)')
    source=re.sub(r'\buint8\b','uint8_t',source).replace('FALSE','0').replace('TRUE','1').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:
        source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_RAIN_'+name,source)
    return source
