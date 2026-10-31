# Move complete vehicle dispatch and velocity arithmetic into C
import re
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
extern "C" void *
AudioVehicleHost_Handling(CVehicle *vehicle)
{
    // Read the current handling pointer at each original conditional access
    return vehicle->pHandling;
}
extern "C" cTransmission *
AudioVehicleHost_Transmission(void *handling)
{
    // Address the transmission of the selected handling record
    return &((tHandlingData *)handling)->Transmission;
}
extern "C" float
AudioVehicleHost_TrafficSpeed(CVehicle *vehicle)
{
    // Read the original simple-traffic speed field
    return vehicle->AutoPilot.m_fMaxTrafficSpeed;
}
extern "C" void
AudioVehicleHost_VelocityVectors(CVehicle *vehicle, AudioSoundPosition *speed, AudioSoundPosition *direction)
{
    // Resolve the forward reference before copying the current velocity vectors
    const CVector &forward = vehicle->GetForward();
    *speed = AudioSoundPosition_FromVector(vehicle->m_vecMoveSpeed);
    *direction = AudioSoundPosition_FromVector(forward);
}
extern "C" int32_t
AudioVehicleHost_Type(CVehicle *vehicle)
{
    // Read the original vehicle type before routing generators
    return vehicle->m_vehType;
}
extern "C" int32_t
AudioVehicleHost_Appearance(CVehicle *vehicle)
{
    // Query the original appearance method at the car routing gate
    return vehicle->GetVehicleAppearance();
}
extern "C" void
AudioVehicleHost_OneShots(cAudioManager *manager, cVehicleParams *params)
{
    // Invoke the remaining one-shot generator with the live parameter record
    manager->ProcessVehicleOneShots(*params);
}
extern "C" void
AudioVehicleHost_CarVelocity(CVehicle *vehicle, float velocity)
{
    // Store the final velocity on the original captured automobile
    ((CAutomobile *)vehicle)->m_fVelocityChangeForAudio = velocity;
}
extern "C" void
AudioVehicleHost_BikeVelocity(CVehicle *vehicle, float velocity)
{
    // Store the final velocity on the original captured bike
    ((CBike *)vehicle)->m_fVelocityChangeForAudio = velocity;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioVehicle_Process(cAudioManager *manager, CVehicle *vehicle);
void *AudioVehicleHost_Handling(CVehicle *vehicle);
cTransmission *AudioVehicleHost_Transmission(void *handling);
float AudioVehicleHost_TrafficSpeed(CVehicle *vehicle);
void AudioVehicleHost_VelocityVectors(CVehicle *vehicle, AudioSoundPosition *speed, AudioSoundPosition *direction);
int32_t AudioVehicleHost_Type(CVehicle *vehicle);
int32_t AudioVehicleHost_Appearance(CVehicle *vehicle);
void AudioVehicleHost_OneShots(cAudioManager *manager, cVehicleParams *params);
void AudioVehicleHost_CarVelocity(CVehicle *vehicle, float velocity);
void AudioVehicleHost_BikeVelocity(CVehicle *vehicle, float velocity);
'''
HELPER='''static float AudioVehicle_Velocity(CVehicle *vehicle)
{
    // Apply every original dot-product term to the resolved game vectors
    AudioSoundPosition speed, direction;
    AudioVehicleHost_VelocityVectors(vehicle, &speed, &direction);
    return speed.x * direction.x + speed.y * direction.y + speed.z * direction.z;
}
'''
def calls(source):
    # Route existing C physical dispatch directly into the new C vehicle operation
    import audio_ped_migration
    return audio_ped_migration.calls(re.sub(r'AudioDispatchHost_Vehicle\(manager, ([^;]+)\);',r'AudioVehicle_Process(manager, (CVehicle *)\1);',source))

def logic(source):
    # Remove both the complete member and its former physical-dispatch trampoline
    if 'cAudioManager::ProcessVehicle(' in source and 'cAudioManager::ProcessVehicleOneShots(' not in source:
        return source
    if 'cAudioManager::ProcessVehicle(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessVehicle')+'\n','')
        source+=HOST
    if 'AudioDispatchHost_Vehicle(' in source:
        source=source.replace(function(source,'AudioDispatchHost_','Vehicle')+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_vehicle_one_shots_migration
    remaining_host=audio_vehicle_one_shots_migration.logic(HOST)
    if remaining_host in source:source=source.replace(remaining_host,'')+remaining_host
    return audio_vehicle_one_shots_migration.logic(source)

def header(source):
    # Replace owner methods and trampoline declarations without changing stored fields
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessVehicle|AudioDispatchHost_Vehicle)\(',line))
    import audio_vehicle_one_shots_migration
    return audio_vehicle_one_shots_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def types(source):
    # Share the original appearance enum with both languages
    return re.sub(r'enum eVehicleAppearance\s*\{.*?\n\};','#include "../modelinfo/VehicleAppearance.h"',source,flags=re.S)

def body(source):
    # Preserve captured parameters, repeated live queries and generator ordering
    import weather_migration as weather
    source=weather.legacy_member_spelling(source)
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('this,','manager,').replace('CGame::currArea','AudioDispatchHost_Area()')
    source=source.replace('manager->m_sQueueSample.m_vecPos = AudioSoundPosition_FromVector(veh->GetPosition());','AudioReleaseHost_Position(veh, &manager->m_sQueueSample.m_vecPos);')
    source=source.replace('veh->pHandling != nil ? &veh->pHandling->Transmission : nil','AudioVehicleHost_Handling(veh) != NULL ? AudioVehicleHost_Transmission(AudioVehicleHost_Handling(veh)) : NULL')
    for a,b in [('veh->m_modelIndex','AudioVehicleReverseHost_Model(veh)'),('veh->GetStatus()','AudioVehicleEngineHost_Status(veh)'),('veh->AutoPilot.m_fMaxTrafficSpeed','AudioVehicleHost_TrafficSpeed(veh)'),('DotProduct(veh->m_vecMoveSpeed, veh->GetForward())','AudioVehicle_Velocity(veh)'),('veh->m_vehType','AudioVehicleHost_Type(veh)'),('veh->GetVehicleAppearance()','AudioVehicleHost_Appearance(veh)'),('CWeather::WetRoads','AudioVehicleRoadHost_WetRoads()'),('ProcessVehicleOneShots(params)','AudioVehicleHost_OneShots(manager, &params)'),('((CAutomobile*)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange','AudioVehicleHost_CarVelocity(veh, params.m_fVelocityChange)'),('((CBike*)veh)->m_fVelocityChangeForAudio = params.m_fVelocityChange','AudioVehicleHost_BikeVelocity(veh, params.m_fVelocityChange)'),('FALSE','0')]:source=source.replace(a,b)
    import audio_vehicle_one_shots_migration
    return audio_vehicle_one_shots_migration.calls(source)

def project(source):
    # Register the owning C11 source and shared enum exactly once
    for kind,base,target in [('ClCompile','audio\\AudioPlane.c','audio\\AudioVehicle.c'),('ClInclude','modelinfo\\VehicleTypes.h','modelinfo\\VehicleAppearance.h')]:
        if target in source:continue
        a=source.index('    <'+kind+' Include="..\\src\\'+base+'"');first=source.index('>',a)
        b=first+1 if source[first-1]=='/' else source.index('</'+kind+'>',a)+len('</'+kind+'>')
        source=source[:b]+'\n'+source[a:b].replace(base,target)+source[b:]
    import audio_vehicle_one_shots_migration
    return audio_vehicle_one_shots_migration.project(source)
