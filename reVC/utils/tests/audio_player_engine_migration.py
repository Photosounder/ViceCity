# Preserve the complete player engine state machine and entry notifications
import re
import clock_migration as clock
import audio_vehicle_skid_migration as skid
from audio_controls_migration import function
def extract(source):
    # Preserve the paired feature branches whose alternate opening braces defeat a simple brace counter
    start=source.index('void\ncAudioManager::ProcessPlayersVehicleEngine(')
    end=source.index('bool8\ncAudioManager::ProcessVehicleSkidding(',start)
    return source[start:end].rstrip()
HOST='//+ rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioPlayerEngine_Process(cAudioManager *manager, cVehicleParams *params, CVehicle *vehicle);\nvoid AudioPlayerEngine_Entered(cAudioManager *manager);\nvoid AudioPlayerEngine_Left(cAudioManager *manager);\n'
for result,name,args,expression in [
 ('float','CarVelocity','const CVehicle *vehicle','((const CAutomobile *)vehicle)->m_fVelocityChangeForAudio'),
 ('float','BikeVelocity','const CVehicle *vehicle','((const CBike *)vehicle)->m_fVelocityChangeForAudio'),
 ('uint8_t','ExtraSpeed','const CVehicle *vehicle','((const CBike *)vehicle)->bExtraSpeed'),
 ('uint8_t','Sand','const CVehicle *vehicle','((const CAutomobile *)vehicle)->bStuckInSand'),
 ('float','SpeedZ','const CVehicle *vehicle','vehicle->m_vecMoveSpeed.z'),
 ('uint32_t','Time','void','CTimer::GetTimeInMilliseconds()')]:
    HOST+=result+'\nAudioPlayerEngineHost_'+name+'('+args+')\n{\n    // Read the original game field at the player engine access point\n    return '+expression+';\n}\n\n'
    PROTOTYPES+=result+' AudioPlayerEngineHost_'+name+'('+args+');\n'
HOST+='''#ifdef FIX_BUGS
uint8_t
AudioPlayerEngineHost_Exiting(void)
{
    // Query the original player ped only after the replay short circuit
    return FindPlayerPed()->GetPedState() == PED_EXIT_CAR;
}
#endif

void
AudioPlayerEngineHost_ChangingGear(CVehicle *vehicle)
{
    // Set the original changing-gear bit before channel initialization or cruising
    vehicle->bAudioChangingGear = TRUE;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES+='#ifdef FIX_BUGS\nuint8_t AudioPlayerEngineHost_Exiting(void);\n#endif\nvoid AudioPlayerEngineHost_ChangingGear(CVehicle *vehicle);\n'
CONSTANTS='\tVEHICLE_ENGINE_MAX_DIST = 50,\n\tPLAYER_VEHICLE_ENGINE_VOLUME = 120,\n'
def logic(source):
    # Remove all three complete members and their private source storage
    if 'cAudioManager::ProcessPlayersVehicleEngine(' in source and 'cAudioManager::ProcessVehicleSkidding(' not in source:return source
    if 'cAudioManager::ProcessPlayersVehicleEngine(' in source:
        source=source.replace(extract(source)+'\n','')
        for name in ['PlayerJustGotInCar','PlayerJustLeftCar']:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source=source.replace('bool8 bPlayerJustEnteredCar;\n','').replace('\tPLAYER_VEHICLE_ENGINE_VOLUME = 120,\n','')+HOST
    host=clock.calls(HOST) if clock.calls(HOST) in source else HOST
    if host in source:source=source.replace(host,'')+host
    if 'AudioVehicleEngineHost_Player(' in source:source=source.replace(function(source,'AudioVehicleEngineHost_','Player')+'\n','')
    return skid.logic(source)
def header(source):
    # Replace only these member declarations with explicit C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in ['ProcessPlayersVehicleEngine','PlayerJustGotInCar','PlayerJustLeftCar','AudioVehicleEngineHost_Player']))
    return skid.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def calls(source):
    # Retain every original ped and script notification with the same singleton owner
    return clock.calls(source.replace('AudioManager.PlayerJustGotInCar()','AudioPlayerEngine_Entered(&AudioManager)').replace('AudioManager.PlayerJustLeftCar()','AudioPlayerEngine_Left(&AudioManager)'))
def body(source):
    # Keep the complete state machine and replace only concrete game access and first-call initialization
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('CAutomobile *automobile','CVehicle *automobile').replace('CBike *bike','CVehicle *bike').replace('(CAutomobile*)params->m_pVehicle','params->m_pVehicle').replace('(CBike*)params->m_pVehicle','params->m_pVehicle')
    source=source.replace('static uint32 gearSoundStartTime = CTimer::GetTimeInMilliseconds();','''static uint32_t gearSoundStartTime;
    static uint8_t gearSoundTimeInitialized;
    // Initialize the original shared timer at the first engine call
    if (!gearSoundTimeInitialized) {
        gearSoundStartTime = AudioPlayerEngineHost_Time();
        gearSoundTimeInitialized = 1;
    }''')
    source=source.replace('CReplay::IsPlayingBack()', 'AudioEnvironmentHost_Replay()').replace('FindPlayerPed()->GetPedState() == PED_EXIT_CAR','AudioPlayerEngineHost_Exiting()')
    for old,new in [
      ('params->m_pVehicle->GetModelIndex()','AudioVehicleReverseHost_Model(params->m_pVehicle)'),
      ('params->m_pVehicle->m_fGasPedal','AudioVehicleReverseHost_Gas(params->m_pVehicle)'),
      ('params->m_pVehicle->m_fBrakePedal','AudioVehicleModelHost_BrakePedal(params->m_pVehicle)'),
      ('params->m_pVehicle->m_nCurrentGear','AudioVehicleReverseHost_Gear(params->m_pVehicle)'),
      ('params->m_pVehicle->bIsHandbrakeOn','AudioVehicleEngineHost_Handbrake(params->m_pVehicle)'),
      ('params->m_pVehicle->bIsDrowning','AudioVehicleModelHost_Drowning(params->m_pVehicle)'),
      ('params->m_pVehicle->m_vecMoveSpeed.z','AudioPlayerEngineHost_SpeedZ(params->m_pVehicle)'),
      ('params->m_pVehicle->bAudioChangingGear = TRUE','AudioPlayerEngineHost_ChangingGear(params->m_pVehicle)'),
      ('Pads[0].GetAccelerate()','AudioVehicleModelHost_Accelerate()'),('Pads[0].GetBrake()','AudioVehicleModelHost_Brake()'),
      ('params->m_pTransmission->nNumberOfGears','AudioVehicleEngineHost_GearCount(params->m_pTransmission)'),
      ('params->m_pTransmission->nDriveType','AudioVehicleEngineHost_DriveType(params->m_pTransmission)'),
      ('params->m_pTransmission->fMaxVelocity','AudioVehicleRoadHost_MaxVelocity(params->m_pTransmission)'),
      ('params->m_pTransmission->fMaxReverseVelocity','AudioVehicleReverseHost_MaxReverse(params->m_pTransmission)'),
      ('automobile->m_nDriveWheelsOnGroundPrev','AudioVehicleReverseHost_PreviousDriveWheels(automobile)'),
      ('automobile->m_nDriveWheelsOnGround','AudioVehicleReverseHost_DriveWheels(automobile)'),
      ('&automobile->m_fGasPedalAudio','AudioVehicleReverseHost_GasAudio(automobile)'),
      ('automobile->m_aWheelState','AudioVehicleEngineHost_CarStates(automobile)'),
      ('automobile->m_fVelocityChangeForAudio','AudioPlayerEngineHost_CarVelocity(automobile)'),
      ('bike->m_nDriveWheelsOnGroundPrev','AudioVehicleEngineHost_BikePreviousWheels(bike)'),
      ('bike->m_nDriveWheelsOnGround','AudioVehicleEngineHost_BikeWheels(bike)'),
      ('&bike->m_fGasPedalAudio','AudioVehicleEngineHost_BikeGasAudio(bike)'),
      ('bike->m_aWheelState','AudioVehicleEngineHost_BikeStates(bike)'),
      ('bike->m_fVelocityChangeForAudio','AudioPlayerEngineHost_BikeVelocity(bike)'),
      ('bike->bExtraSpeed','AudioPlayerEngineHost_ExtraSpeed(bike)'),
      ('(params->m_pVehicle)->bStuckInSand','AudioPlayerEngineHost_Sand(params->m_pVehicle)'),
      ('aVehicleSettings[params->m_nIndex].m_nBank','AudioVehicleEngineHost_Bank(params->m_nIndex)'),
      ('CTimer::GetTimeInMilliseconds()','AudioPlayerEngineHost_Time()'),('CTimer::GetTimeScale()','AudioPoliceHost_TimeScale()'),
      ('debug(" ** AUDIOLOG: Unrecognised vehicle type %d in ProcessVehicleEngine() * \\n", params->m_VehicleType)','AudioVehicleEngineHost_Unknown(params->m_VehicleType)')]:source=source.replace(old,new)
    source=source.replace('const CVector soundPosition = AudioSoundPosition_ToVector(&manager->m_sQueueSample.m_vecPos);','const AudioSoundPosition soundPosition = manager->m_sQueueSample.m_vecPos;').replace('AudioGeometry_TranslateVector(&soundPosition, &pos)','AudioGeometry_Translate(&soundPosition, &pos)').replace('CVector pos','AudioSoundPosition pos')
    for old,new in [('tWheelState','enum tWheelState'),('int32','int32_t'),('int16','int16_t'),('uint32','uint32_t'),('uint8','uint8_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('Clamp(', 'AudioPlayerEngine_Clamp(').replace('Min(', 'AudioPlayerEngine_Min(').replace('Max(', 'AudioPlayerEngine_Max(').replace('AudioVehiclePlayer_AddSample(this,','AudioVehiclePlayer_AddSample(manager,')
    return source
