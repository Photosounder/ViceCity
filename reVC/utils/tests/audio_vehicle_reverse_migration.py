# Known reverse-gear sound conversion with original captured and live vehicle reads
import re
import audio_vehicle_model_migration as model
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
int16_t
AudioVehicleReverseHost_Model(const CVehicle *vehicle)
{
    // Query the original model index before the caddy exclusion
    return vehicle->GetModelIndex();
}

uint8_t
AudioVehicleReverseHost_Engine(const CVehicle *vehicle)
{
    // Read the engine bit on the initially captured vehicle
    return vehicle->bEngineOn;
}

float
AudioVehicleReverseHost_Gas(const CVehicle *vehicle)
{
    // Read the gas pedal at each original captured or live vehicle check
    return vehicle->m_fGasPedal;
}

uint8_t
AudioVehicleReverseHost_Gear(const CVehicle *vehicle)
{
    // Read the gear only when the captured vehicle's gas pedal did not select reverse
    return vehicle->m_nCurrentGear;
}

uint8_t
AudioVehicleReverseHost_DriveWheels(const CVehicle *vehicle)
{
    // Read current drive-wheel contact from the later automobile capture
    return ((const CAutomobile *)vehicle)->m_nDriveWheelsOnGround;
}

uint8_t
AudioVehicleReverseHost_PreviousDriveWheels(const CVehicle *vehicle)
{
    // Read previous drive-wheel contact only on the airborne branch
    return ((const CAutomobile *)vehicle)->m_nDriveWheelsOnGroundPrev;
}

float *
AudioVehicleReverseHost_GasAudio(CVehicle *vehicle)
{
    // Expose the original float field so its decay and subsequent read run in C
    return &((CAutomobile *)vehicle)->m_fGasPedalAudio;
}

float
AudioVehicleReverseHost_MaxReverse(const cTransmission *transmission)
{
    // Read the original reverse velocity limit after the drive-wheel contact test
    return transmission->fMaxReverseVelocity;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='uint8_t AudioVehicleReverse_Process(cAudioManager *manager, cVehicleParams *params);\nint16_t AudioVehicleReverseHost_Model(const CVehicle *vehicle);\nuint8_t AudioVehicleReverseHost_Engine(const CVehicle *vehicle);\nfloat AudioVehicleReverseHost_Gas(const CVehicle *vehicle);\nuint8_t AudioVehicleReverseHost_Gear(const CVehicle *vehicle);\nuint8_t AudioVehicleReverseHost_DriveWheels(const CVehicle *vehicle);\nuint8_t AudioVehicleReverseHost_PreviousDriveWheels(const CVehicle *vehicle);\nfloat *AudioVehicleReverseHost_GasAudio(CVehicle *vehicle);\nfloat AudioVehicleReverseHost_MaxReverse(const cTransmission *transmission);\n'
CONSTANTS='\tREVERSE_GEAR_MAX_DIST = 30,\n\tREVERSE_GEAR_VOLUME = 24,\n'
def logic(source):
    # Remove only the full reverse generator and move its constants into C
    if 'cAudioManager::ProcessReverseGear(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessReverseGear')+'\n','').replace(CONSTANTS,'')+HOST
    if HOST in source:source=source.replace(HOST,'')+HOST
    return model.logic(source.replace('ProcessReverseGear(params)', 'AudioVehicleReverse_Process(this, &params)'))
def header(source):
    # Keep stored owner and parameter fields while exposing the actual C operation
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bProcessReverseGear\(',line))
    return model.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Preserve each original vehicle capture, short-circuit read and gas-audio field mutation
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source);source=re.sub(r'\bparams\.', 'params->',source)
    source=source.replace('CAutomobile *automobile;', 'CVehicle *automobile;').replace('(CAutomobile*)params->m_pVehicle','params->m_pVehicle')
    source=source.replace('veh->GetModelIndex()', 'AudioVehicleReverseHost_Model(veh)').replace('veh->bEngineOn','AudioVehicleReverseHost_Engine(veh)').replace('veh->m_fGasPedal','AudioVehicleReverseHost_Gas(veh)').replace('veh->m_nCurrentGear','AudioVehicleReverseHost_Gear(veh)')
    source=source.replace('automobile->m_nDriveWheelsOnGroundPrev','AudioVehicleReverseHost_PreviousDriveWheels(automobile)').replace('automobile->m_nDriveWheelsOnGround','AudioVehicleReverseHost_DriveWheels(automobile)')
    source=source.replace('automobile->m_fGasPedalAudio', '(*AudioVehicleReverseHost_GasAudio(automobile))').replace('params->m_pTransmission->fMaxReverseVelocity','AudioVehicleReverseHost_MaxReverse(params->m_pTransmission)').replace('params->m_pVehicle->m_fGasPedal','AudioVehicleReverseHost_Gas(params->m_pVehicle)')
    for old,new in [('bool8','uint8_t'),('uint8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('ABS(', 'AudioVehicleReverse_Abs(').replace('SQR(REVERSE_GEAR_MAX_DIST)','(REVERSE_GEAR_MAX_DIST * REVERSE_GEAR_MAX_DIST)').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_REVERSE_'+name,source)
    return source
