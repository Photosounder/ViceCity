# Move the entire vehicle one-shot event loop and its shared histories into C
import re
from audio_controls_migration import function
CONSTANTS='\tCAR_BOMB_TICK_VOLUME = 60,\n\tVEHICLE_ONE_SHOT_HELI_BLADE_MAX_DIST = 35,\n\tVEHICLE_ONE_SHOT_HELI_BLADE_VOLUME = 70,\n\tVEHICLE_ONE_SHOT_CAR_TYRE_POP_MAX_DIST = 60,\n\tVEHICLE_ONE_SHOT_CAR_TYRE_POP_VOLUME = 117,\n\tVEHICLE_ONE_SHOT_DOOR_MAX_DIST = 50,\n\tVEHICLE_ONE_SHOT_DOOR_OPEN_VOLUME = 122,\n\tVEHICLE_ONE_SHOT_DOOR_CLOSE_VOLUME = 117,\n\tVEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST = 40,\n\tVEHICLE_ONE_SHOT_WINDSHIELD_CRACK_VOLUME = 80,\n\tVEHICLE_ONE_SHOT_CAR_JUMP_MAX_DIST = 35,\n\tVEHICLE_ONE_SHOT_CAR_JUMP_VOLUME = 80,\n\tVEHICLE_ONE_SHOT_CAR_ENGINE_START_MAX_DIST = 40,\n\tVEHICLE_ONE_SHOT_CAR_ENGINE_START_VOLUME = 60,\n\tVEHICLE_ONE_SHOT_CAR_LIGHT_BREAK_VOLUME = 30,\n\tVEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST = 35,\n\tVEHICLE_ONE_SHOT_CAR_HYDRAULIC_VOLUME = 55,\n\tVEHICLE_ONE_SHOT_CAR_SPLASH_MAX_DIST = 60,\n\tVEHICLE_ONE_SHOT_CAR_SPLASH_VOLUME = 35,\n\tVEHICLE_ONE_SHOT_BOAT_SLOWDOWN_MAX_DIST = 50,\n\tVEHICLE_ONE_SHOT_TRAIN_DOOR_MAX_DIST = 35,\n\tVEHICLE_ONE_SHOT_TRAIN_DOOR_VOLUME = 70,\n\tVEHICLE_ONE_SHOT_CAR_TANK_TURRET_MAX_DIST = 40,\n\tVEHICLE_ONE_SHOT_CAR_TANK_TURRET_VOLUME = 90,\n\tVEHICLE_ONE_SHOT_CAR_BOMB_TICK_MAX_DIST = 30,\n\tVEHICLE_ONE_SHOT_CAR_BOMB_TICK_VOLUME = CAR_BOMB_TICK_VOLUME,\n\tVEHICLE_ONE_SHOT_PLANE_ON_GROUND_MAX_DIST = 180,\n\tVEHICLE_ONE_SHOT_PLANE_ON_GROUND_VOLUME = 75,\n\tVEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST = 120,\n\tVEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_VOLUME = 65,\n\tVEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_MAX_DIST = 40,\n\tVEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_VOLUME = 90,\n\tVEHICLE_ONE_SHOT_BOMB_ARMED_MAX_DIST = 50,\n\tVEHICLE_ONE_SHOT_BOMB_ARMED_VOLUME = 50,\n\tVEHICLE_ONE_SHOT_WATER_FALL_MAX_DIST = 40,\n\tVEHICLE_ONE_SHOT_WATER_FALL_VOLUME = 90,\n\tVEHICLE_ONE_SHOT_SPLATTER_MAX_DIST = 40,\n\tVEHICLE_ONE_SHOT_SPLATTER_VOLUME = 55,\n\tVEHICLE_ONE_SHOT_CAR_PED_COLLISION_MAX_DIST = 40,\n'
DOOR_ENUM='enum\n{\n\tOLD_DOOR = 0,\n\tNEW_DOOR,\n\tTRUCK_DOOR,\n\tBUS_DOOR,\n};'
HOST='//+ rouz edit (ChatGPT)\nextern "C" uint8_t\nAudioVehicleOneShotsHost_DoorType(uint32_t index)\n{\n    // Read the original audio-table door type at each event\'s setup point\n    return aVehicleSettings[index].m_bDoorType;\n}\nextern "C" int16_t\nAudioVehicleOneShotsHost_HandBrake(void)\n{\n    // Query the original first pad handbrake value\n    return Pads[0].GetHandBrake();\n}\nextern "C" CPed *\nAudioVehicleOneShotsHost_Driver(const CVehicle *vehicle)\n{\n    // Capture the current driver pointer for the original weapon branch\n    return vehicle->pDriver;\n}\nextern "C" uint8_t\nAudioVehicleOneShotsHost_HasWeapon(CPed *ped, uint8_t slot)\n{\n    // Query the original captured driver\'s weapon-slot predicate\n    return ped->HasWeaponSlot(slot);\n}\nextern "C" uint8_t\nAudioVehicleOneShotsHost_DriveBy(CPed *ped)\n{\n    // Query the original captured driver\'s drive-by predicate\n    return ped->IsPedDoingDriveByShooting();\n}\nextern "C" int32_t\nAudioVehicleOneShotsHost_Weapon(CPed *ped, uint8_t slot)\n{\n    // Read the weapon type returned by the original selected slot query\n    return ped->GetWeapon(slot).m_eWeaponType;\n}\nextern "C" void\nAudioVehicleOneShotsHost_PedComments(cAudioManager *manager, cPedParams *params, uint16_t sound)\n{\n    // Invoke the remaining pedestrian-comment generator with the retained local context\n    manager->SetupPedComments(*params, sound);\n}\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioVehicleOneShots_Process(cAudioManager *manager, cVehicleParams *params);\nuint8_t AudioVehicleOneShotsHost_DoorType(uint32_t index);\nint16_t AudioVehicleOneShotsHost_HandBrake(void);\nCPed *AudioVehicleOneShotsHost_Driver(const CVehicle *vehicle);\nuint8_t AudioVehicleOneShotsHost_HasWeapon(CPed *ped, uint8_t slot);\nuint8_t AudioVehicleOneShotsHost_DriveBy(CPed *ped);\nint32_t AudioVehicleOneShotsHost_Weapon(CPed *ped, uint8_t slot);\nvoid AudioVehicleOneShotsHost_PedComments(cAudioManager *manager, cPedParams *params, uint16_t sound);\n'

def extract(source, signature='void\ncAudioManager::ProcessVehicleOneShots'):
    # Delimit the function without counting mutually exclusive preprocessor braces
    start=source.index(signature)
    end=source.index('\n#ifdef GTA_TRAIN\nbool8\ncAudioManager::ProcessTrainNoise',start) if 'cAudioManager::ProcessTrainNoise' in source[start:] else source.index('\n//- rouz edit (ChatGPT)',start)
    return source[start:end].rstrip()

def calls(source):
    # Route the C vehicle dispatcher directly into the owning event loop
    return source.replace('AudioVehicleHost_OneShots(', 'AudioVehicleOneShots_Process(')

def logic(source):
    # Remove the complete member, private constants and obsolete one-shot trampoline
    if 'cAudioManager::ProcessVehicleOneShots(' in source:
        source=source.replace(extract(source)+'\n','')
        source+=HOST
    if 'AudioVehicleHost_OneShots(' in source:
        source=source.replace('extern "C" void\n'+function(source,'AudioVehicleHost_','OneShots')+'\n','')
    for line in CONSTANTS.splitlines():source=source.replace(line+'\n','')
    source=source.replace(DOOR_ENUM,'#include "AudioVehicleDoorTypes.h"')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_train_migration
    return audio_train_migration.logic(source)

def header(source):
    # Replace owning declarations and the trampoline without changing stored fields
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessVehicleOneShots|AudioVehicleHost_OneShots)\(',line))
    import audio_train_migration
    return audio_train_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve complete event branches, shared counters and live scratch/entity accesses
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    for a,b in [('((CAutomobile*)params->m_pVehicle)->m_aWheelSpeed[1]','AudioAircraftHost_WheelSpeed(params->m_pVehicle)'),('aVehicleSettings[params->m_nIndex].m_bDoorType','AudioVehicleOneShotsHost_DoorType(params->m_nIndex)'),('params->m_pVehicle->GetVehicleAppearance()','AudioVehicleHost_Appearance(params->m_pVehicle)'),('params->m_pVehicle->m_modelIndex','AudioVehicleReverseHost_Model(params->m_pVehicle)'),('params->m_pVehicle->GetModelIndex()','AudioVehicleReverseHost_Model(params->m_pVehicle)'),('Pads[0].GetHandBrake()','AudioVehicleOneShotsHost_HandBrake()'),('params->m_pVehicle->pDriver','AudioVehicleOneShotsHost_Driver(params->m_pVehicle)'),('pPed->HasWeaponSlot(WEAPONSLOT_SUBMACHINEGUN)','AudioVehicleOneShotsHost_HasWeapon(pPed, WEAPONSLOT_SUBMACHINEGUN)'),('pPed->IsPedDoingDriveByShooting()','AudioVehicleOneShotsHost_DriveBy(pPed)'),('pPed->GetWeapon(WEAPONSLOT_SUBMACHINEGUN).m_eWeaponType','AudioVehicleOneShotsHost_Weapon(pPed, WEAPONSLOT_SUBMACHINEGUN)'),('SetupPedComments(pedParams,','AudioVehicleOneShotsHost_PedComments(manager, &pedParams,'),('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')]:source=source.replace(a,b)
    for a,b in [('uint32','uint32_t'),('uint16','uint16_t'),('uint8','uint8_t'),('int16','int16_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0'),('Min','AUDIO_VEHICLE_ONE_SHOTS_MIN'),('Max','AUDIO_VEHICLE_ONE_SHOTS_MAX'),('ARRAY_SIZE','AUDIO_VEHICLE_ONE_SHOTS_COUNT')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=re.sub(r'SQR\((VEHICLE_ONE_SHOT_\w+)\)',r'(\1 * \1)',source)
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_ONE_SHOTS_'+name,source)
    import audio_ped_speech_migration
    return audio_ped_speech_migration.calls(source)

def project(source):
    # Register the new owning C11 source and shared audio-table door identifiers
    for kind,base,target in [('ClCompile','audio\\AudioVehicle.c','audio\\AudioVehicleOneShots.c'),('ClInclude','audio\\AudioVehicleModels.h','audio\\AudioVehicleDoorTypes.h')]:
        if target in source:continue
        a=source.index('    <'+kind+' Include="..\\src\\'+base+'"');first=source.index('>',a)
        b=first+1 if source[first-1]=='/' else source.index('</'+kind+'>',a)+len('</'+kind+'>')
        source=source[:b]+'\n'+source[a:b].replace(base,target)+source[b:]
    import audio_train_migration
    return audio_train_migration.project(source.rstrip()+'\n')
