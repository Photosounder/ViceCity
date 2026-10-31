# Preserve complete reversing and air-brake generators and their audio-index predicates
import re
import audio_vehicle_doors_migration as doors
from audio_controls_migration import function
NAMES={'UsesReverseWarning':'UsesReverse','ProcessVehicleReverseWarning':'Reverse','ProcessAirBrakes':'AirBrakes','HasAirBrakes':'HasAirBrakes'}
PROTOTYPES='uint8_t AudioVehicleWarning_UsesReverse(uint32_t model);\nuint8_t AudioVehicleWarning_Reverse(cAudioManager *manager, cVehicleParams *params);\nuint8_t AudioVehicleWarning_AirBrakes(cAudioManager *manager, cVehicleParams *params);\nuint8_t AudioVehicleWarning_HasAirBrakes(uint32_t model);\n'
CONSTANTS='\tVEHICLE_REVERSE_WARNING_MAX_DIST = 50,\n\tVEHICLE_REVERSE_WARNING_VOLUME = 60,\n\tAIR_BRAKES_MAX_DIST = 30,\n\tAIR_BRAKES_VOLUME = 70,\n'
def logic(source):
    # Remove all four complete members and only their private constants
    for name in NAMES:
        if 'cAudioManager::'+name+'(' in source:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    for line in CONSTANTS.splitlines():source=source.replace(line+'\n','')
    for name,target in NAMES.items():
        source=re.sub(r'\b'+name+r'\(params\.m_nIndex\)', 'AudioVehicleWarning_'+target+'(params.m_nIndex)', source)
        source=re.sub(r'\b'+name+r'\(params\)', 'AudioVehicleWarning_'+target+'(this, &params)', source)
    return doors.logic(source)
def header(source):
    # Preserve stored owner state and expose explicit C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return doors.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Keep original capture timing, asymmetric velocity gates and signed frequency arithmetic
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('CAutomobile *automobile','CVehicle *automobile').replace('(CAutomobile *)params->m_pVehicle','params->m_pVehicle')
    for a,b in [('veh->bEngineOn','AudioVehicleReverseHost_Engine(veh)'),('veh->m_fGasPedal','AudioVehicleReverseHost_Gas(veh)'),('veh->bIsDrowning','AudioVehicleModelHost_Drowning(veh)'),('automobile->bEngineOn','AudioVehicleReverseHost_Engine(automobile)'),('automobile->m_fVelocityChangeForAudio','AudioPlayerEngineHost_CarVelocity(automobile)')]:source=source.replace(a,b)
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in ['VEHICLE_REVERSE_WARNING_MAX_DIST','AIR_BRAKES_MAX_DIST']:source=source.replace('SQR('+name+')','('+name+' * '+name+')')
    source=source.replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_WARNING_'+name,source)
    return source
