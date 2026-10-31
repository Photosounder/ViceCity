# Preserve the complete skid generator and both wheel-value helpers
import re
import audio_vehicle_horn_migration as horn
from audio_controls_migration import function
NAMES={'ProcessVehicleSkidding':'Process','GetVehicleDriveWheelSkidValue':'Drive','GetVehicleNonDriveWheelSkidValue':'NonDrive'}
CONSTANTS='\tVEHICLE_SKIDDING_MAX_DIST = 40,\n\tVEHICLE_SKIDDING_VOLUME = 50,\n'
HOST='''//+ rouz edit (ChatGPT)
float
AudioVehicleSkidHost_CarGas(const CVehicle *vehicle)
{
    // Copy the original automobile gas-audio value after capturing wheel states
    return ((const CAutomobile *)vehicle)->m_fGasPedalAudio;
}

float
AudioVehicleSkidHost_BikeGas(const CVehicle *vehicle)
{
    // Copy the original bike gas-audio value after capturing wheel states
    return ((const CBike *)vehicle)->m_fGasPedalAudio;
}

float
AudioVehicleSkidHost_TurnZ(const CVehicle *vehicle)
{
    // Read the original turn-speed component at every skid-value expression evaluation
    return vehicle->m_vecTurnSpeed.z;
}

void
AudioVehicleSkidHost_Unknown(int32_t type)
{
    // Retain the original unsupported vehicle diagnostic
    debug("\\n * AUDIOLOG:  ProcessVehicleSkidding() Unsupported vehicle type %d * \\n", type);
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='uint8_t AudioVehicleSkid_Process(cAudioManager *manager, cVehicleParams *params);\nfloat AudioVehicleSkidHost_CarGas(const CVehicle *vehicle);\nfloat AudioVehicleSkidHost_BikeGas(const CVehicle *vehicle);\nfloat AudioVehicleSkidHost_TurnZ(const CVehicle *vehicle);\nvoid AudioVehicleSkidHost_Unknown(int32_t type);\n'
def logic(source):
    # Remove the complete owner methods and private constants before replacing internal callers
    if 'cAudioManager::ProcessVehicleSkidding(' in source:
        for name in NAMES:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source=source.replace(CONSTANTS,'')+HOST
    if HOST in source:source=source.replace(HOST,'')+HOST
    return horn.logic(source.replace('ProcessVehicleSkidding(params)','AudioVehicleSkid_Process(this, &params)'))
def header(source):
    # Preserve all owner fields while exposing the actual C generator and narrow game access
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return horn.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Keep original captured vehicle fields, per-wheel transmission reads and skid arithmetic
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('CAutomobile *automobile','CVehicle *automobile').replace('CBike *bike','CVehicle *bike').replace('(CAutomobile*)params->m_pVehicle','params->m_pVehicle').replace('(CBike*)params->m_pVehicle','params->m_pVehicle')
    for a,b in [
      ('automobile->m_aWheelState','AudioVehicleEngineHost_CarStates(automobile)'),
      ('bike->m_aWheelState','AudioVehicleEngineHost_BikeStates(bike)'),
      ('automobile->m_nWheelsOnGround','AudioVehicleRoadHost_WheelsOnGround(automobile, VEHICLE_TYPE_CAR)'),
      ('bike->m_nWheelsOnGround','AudioVehicleRoadHost_WheelsOnGround(bike, VEHICLE_TYPE_BIKE)'),
      ('automobile->m_fGasPedalAudio','AudioVehicleSkidHost_CarGas(automobile)'),
      ('bike->m_fGasPedalAudio','AudioVehicleSkidHost_BikeGas(bike)'),
      ('transmission->nDriveType','AudioVehicleEngineHost_DriveType(transmission)'),
      ('transmission->fMaxVelocity','AudioVehicleRoadHost_MaxVelocity(transmission)'),
      ('params->m_pVehicle->m_nSurfaceTouched','AudioVehicleRoadHost_Surface(params->m_pVehicle)'),
      ('veh->m_vecTurnSpeed.z','AudioVehicleSkidHost_TurnZ(veh)'),
      ('debug("\\n * AUDIOLOG:  ProcessVehicleSkidding() Unsupported vehicle type %d * \\n", params->m_VehicleType)','AudioVehicleSkidHost_Unknown(params->m_VehicleType)')]:source=source.replace(a,b)
    for name,target in NAMES.items():source=source.replace(name+'(', 'AudioVehicleSkid_'+target+'(')
    for a,b in [('tWheelState','enum tWheelState'),('uint8','uint8_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('Min(', 'AudioVehicleSkid_Min(').replace('Max(', 'AudioVehicleSkid_Max(').replace('Abs(', 'fabsf(').replace('SQR(VEHICLE_SKIDDING_MAX_DIST)','(VEHICLE_SKIDDING_MAX_DIST * VEHICLE_SKIDDING_MAX_DIST)').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_SKID_'+name,source)
    return source
