# Preserve the complete main engine generator while replacing game field access
import re
import audio_vehicle_player_migration as player
from audio_controls_migration import function
NAMES={'ProcessVehicleEngine':'Process'}
CONSTANTS='\tVEHICLE_ENGINE_MAX_DIST = 50,\n\tVEHICLE_ENGINE_BASE_VOLUME = 75,\n\tVEHICLE_ENGINE_FULL_VOLUME = 90,\n'
HOST='//+ rouz edit (ChatGPT)\n'
PROTOTYPES='uint8_t AudioVehicleEngine_Process(cAudioManager *manager, cVehicleParams *params);\n'
for result,name,args,expression in [
 ('int32_t','Status','const CVehicle *vehicle','vehicle->GetStatus()'),
 ('uint8_t','Handbrake','const CVehicle *vehicle','vehicle->bIsHandbrakeOn'),
 ('int8_t','GearCount','const cTransmission *transmission','transmission->nNumberOfGears'),
 ('char','DriveType','const cTransmission *transmission','transmission->nDriveType'),
 ('float','ShiftDown','const cTransmission *transmission, uint8_t gear','transmission->Gears[gear].fShiftDownVelocity'),
 ('float','ShiftUp','const cTransmission *transmission, uint8_t gear','transmission->Gears[gear].fShiftUpVelocity'),
 ('uint8_t','BikeWheels','const CVehicle *vehicle','((const CBike *)vehicle)->m_nDriveWheelsOnGround'),
 ('uint8_t','BikePreviousWheels','const CVehicle *vehicle','((const CBike *)vehicle)->m_nDriveWheelsOnGroundPrev'),
 ('float *','BikeGasAudio','CVehicle *vehicle','&((CBike *)vehicle)->m_fGasPedalAudio'),
 ('enum tWheelState *','CarStates','CVehicle *vehicle','((CAutomobile *)vehicle)->m_aWheelState'),
 ('enum tWheelState *','BikeStates','CVehicle *vehicle','((CBike *)vehicle)->m_aWheelState'),
 ('uint8_t','Bank','int32_t index','aVehicleSettings[index].m_nBank'),
 ('int32_t','Acceleration','int32_t index','aVehicleSettings[index].m_nAccelerationSampleIndex')]:
    HOST+=result+'\nAudioVehicleEngineHost_'+name+'('+args+')\n{\n    // Read the original field at the engine generator access point\n    return '+expression+';\n}\n\n'
    PROTOTYPES+=result+' AudioVehicleEngineHost_'+name+'('+args+');\n'
HOST+='''void
AudioVehicleEngineHost_Player(cAudioManager *manager, cVehicleParams *params, CVehicle *vehicle)
{
    // Forward the existing player engine operation with its original parameter aliases
    manager->ProcessPlayersVehicleEngine(*params, vehicle);
}

void
AudioVehicleEngineHost_Unknown(int32_t type)
{
    // Retain the original diagnostic for unsupported vehicle types
    debug(" ** AUDIOLOG: Unrecognised vehicle type %d in ProcessVehicleEngine() * \\n", type);
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES+='void AudioVehicleEngineHost_Player(cAudioManager *manager, cVehicleParams *params, CVehicle *vehicle);\nvoid AudioVehicleEngineHost_Unknown(int32_t type);\n'
def logic(source):
    # Replace the whole member and its two callers
    if 'cAudioManager::ProcessVehicleEngine(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessVehicleEngine')+'\n','')+HOST
    if HOST in source:source=source.replace(HOST,'')+HOST
    without_player=HOST.replace(function(HOST,'AudioVehicleEngineHost_','Player')+'\n','')
    if without_player in source:source=source.replace(without_player,'')+HOST
    return player.logic(source.replace('ProcessVehicleEngine(params)','AudioVehicleEngine_Process(this, &params)'))
def header(source):
    # Keep all stored state and replace the member declaration with C operations
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bProcessVehicleEngine\(',line))
    return player.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Retain every original branch and arithmetic expression using narrow field adapters
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('params.','params->').replace('CAutomobile *automobile','CVehicle *automobile').replace('CBike *bike','CVehicle *bike')
    source=source.replace('(CAutomobile*)params->m_pVehicle','params->m_pVehicle').replace('(CBike*)params->m_pVehicle','params->m_pVehicle')
    replacements={
      'FindPlayerVehicle()':'AudioPoliceHost_PlayerVehicle()',
      'params->m_pVehicle->GetStatus()':'AudioVehicleEngineHost_Status(params->m_pVehicle)',
      'params->m_pVehicle->GetModelIndex()':'AudioVehicleReverseHost_Model(params->m_pVehicle)',
      'params->m_pVehicle->bEngineOn':'AudioVehicleReverseHost_Engine(params->m_pVehicle)',
      'params->m_pVehicle->m_nCurrentGear':'AudioVehicleReverseHost_Gear(params->m_pVehicle)',
      'params->m_pVehicle->m_fGasPedal':'AudioVehicleReverseHost_Gas(params->m_pVehicle)',
      'params->m_pVehicle->bIsHandbrakeOn':'AudioVehicleEngineHost_Handbrake(params->m_pVehicle)',
      'params->m_pVehicle->bIsDrowning':'AudioVehicleModelHost_Drowning(params->m_pVehicle)',
      'ProcessPlayersVehicleEngine(params, params->m_pVehicle)':'AudioPlayerEngine_Process(manager, params, params->m_pVehicle)',
      'transmission->nNumberOfGears':'AudioVehicleEngineHost_GearCount(transmission)',
      'transmission->nDriveType':'AudioVehicleEngineHost_DriveType(transmission)',
      'transmission->fMaxVelocity':'AudioVehicleRoadHost_MaxVelocity(transmission)',
      'transmission->fMaxReverseVelocity':'AudioVehicleReverseHost_MaxReverse(transmission)',
      'transmission->Gears[currentGear].fShiftDownVelocity':'AudioVehicleEngineHost_ShiftDown(transmission, currentGear)',
      'transmission->Gears[0].fShiftDownVelocity':'AudioVehicleEngineHost_ShiftDown(transmission, 0)',
      'transmission->Gears[1].fShiftUpVelocity':'AudioVehicleEngineHost_ShiftUp(transmission, 1)',
      'automobile->m_nDriveWheelsOnGroundPrev':'AudioVehicleReverseHost_PreviousDriveWheels(automobile)',
      'automobile->m_nDriveWheelsOnGround':'AudioVehicleReverseHost_DriveWheels(automobile)',
      'automobile->m_aWheelState':'AudioVehicleEngineHost_CarStates(automobile)',
      '&automobile->m_fGasPedalAudio':'AudioVehicleReverseHost_GasAudio(automobile)',
      'bike->m_nDriveWheelsOnGroundPrev':'AudioVehicleEngineHost_BikePreviousWheels(bike)',
      'bike->m_nDriveWheelsOnGround':'AudioVehicleEngineHost_BikeWheels(bike)',
      'bike->m_aWheelState':'AudioVehicleEngineHost_BikeStates(bike)',
      '&bike->m_fGasPedalAudio':'AudioVehicleEngineHost_BikeGasAudio(bike)',
      'aVehicleSettings[params->m_nIndex].m_nBank':'AudioVehicleEngineHost_Bank(params->m_nIndex)',
      'aVehicleSettings[params->m_nIndex].m_nAccelerationSampleIndex':'AudioVehicleEngineHost_Acceleration(params->m_nIndex)',
      'debug(" ** AUDIOLOG: Unrecognised vehicle type %d in ProcessVehicleEngine() * \\n", params->m_VehicleType)':'AudioVehicleEngineHost_Unknown(params->m_VehicleType)'}
    for a,b in replacements.items():source=source.replace(a,b)
    for a,b in [('tWheelState','enum tWheelState'),('bool8','uint8_t'),('uint8','uint8_t'),('uint32','uint32_t'),('TRUE','1'),('FALSE','0'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('Min(', 'AudioVehicleEngine_Min(').replace('Abs(', 'fabsf(').replace('SQR(VEHICLE_ENGINE_MAX_DIST)','(VEHICLE_ENGINE_MAX_DIST * VEHICLE_ENGINE_MAX_DIST)').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_ENGINE_'+name,source)
    source=source.replace('fabsf(params->m_fVelocityChange / AudioVehicleRoadHost_MaxVelocity(transmission) > 1.0f)', 'fabsf((float)(params->m_fVelocityChange / AudioVehicleRoadHost_MaxVelocity(transmission) > 1.0f))')
    return source
