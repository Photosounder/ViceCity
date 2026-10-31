# Known RC model vehicle audio conversion with shared function-static history
import re
import audio_vehicle_engine_migration as engine
from audio_controls_migration import function
NAMES={'ProcessModelHeliVehicle':'Heli','ProcessModelVehicle':'Car'}
HOST='''//+ rouz edit (ChatGPT)
int16_t
AudioVehicleModelHost_Model(const CVehicle *vehicle)
{
    // Read the original model field before selecting the RC bandit branch
    return vehicle->m_modelIndex;
}

float
AudioVehicleModelHost_BrakePedal(const CVehicle *vehicle)
{
    // Read the current vehicle brake pedal after the non-player gas query
    return vehicle->m_fBrakePedal;
}

uint8_t
AudioVehicleModelHost_Drowning(const CVehicle *vehicle)
{
    // Read the original drowning bit before emitting-volume attenuation
    return vehicle->bIsDrowning;
}

int16_t
AudioVehicleModelHost_Brake(void)
{
    // Query the original primary pad brake at each model-specific control point
    return Pads[0].GetBrake();
}

int16_t
AudioVehicleModelHost_Accelerate(void)
{
    // Query the original primary pad accelerator at every macro evaluation
    return Pads[0].GetAccelerate();
}

int16_t
AudioVehicleModelHost_GunUpDown(void)
{
    // Query the original primary pad gun axis at every helicopter input evaluation
    return Pads[0].GetCarGunUpDown();
}

#ifdef THIS_IS_STUPID
void
AudioVehicleModelHost_Up(CVehicle *vehicle, AudioSoundPosition *up)
{
    // Copy the original up vector for the alternate C dot-product expression
    *up = AudioSoundPosition_FromVector(vehicle->GetUp());
}
#else
float
AudioVehicleModelHost_UpY(CVehicle *vehicle)
{
    // Read only the original up-vector Y component on the normal branch
    return vehicle->GetUp().y;
}
#endif
//- rouz edit (ChatGPT)
'''
PROTOTYPES='void AudioVehicleModel_Heli(cAudioManager *manager, cVehicleParams *params);\nvoid AudioVehicleModel_Car(cAudioManager *manager, cVehicleParams *params);\nint16_t AudioVehicleModelHost_Model(const CVehicle *vehicle);\nfloat AudioVehicleModelHost_BrakePedal(const CVehicle *vehicle);\nuint8_t AudioVehicleModelHost_Drowning(const CVehicle *vehicle);\nint16_t AudioVehicleModelHost_Brake(void);\nint16_t AudioVehicleModelHost_Accelerate(void);\nint16_t AudioVehicleModelHost_GunUpDown(void);\n#ifdef THIS_IS_STUPID\nvoid AudioVehicleModelHost_Up(CVehicle *vehicle, AudioSoundPosition *up);\n#else\nfloat AudioVehicleModelHost_UpY(CVehicle *vehicle);\n#endif\n'
CONSTANTS='\tMODEL_CAR_ENGINE_MAX_DIST = 35,\n\tMODEL_CAR_ENGINE_VOLUME = 90,\n\tMODEL_HELI_ENGINE_VOLUME = 70,\n'
HELPERS='''static uint8_t AudioVehicleModel_Decision(uint8_t result)
{
    // Preserve conditional repeated pad queries when MSVC recognizes a floating maximum
    volatile uint8_t comparison = result;
    return comparison;
}

#ifdef THIS_IS_STUPID
static float AudioVehicleModel_UpDot(CVehicle *vehicle)
{
    // Retain the original alternate dot-product terms instead of replacing them with Y
    AudioSoundPosition up;
    AudioVehicleModelHost_Up(vehicle, &up);
    return up.x * 0.0f + up.y * 1.0f + up.z * 0.0f;
}
#endif
'''
def logic(source):
    # Remove exactly both model generators and preserve current adapter ordering
    if 'cAudioManager::ProcessModelHeliVehicle(' in source:
        for name in NAMES:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source=source.replace(CONSTANTS,'')+HOST
    if HOST in source:source=source.replace(HOST,'')+HOST
    for name,target in NAMES.items():source=source.replace(name+'(params)','AudioVehicleModel_'+target+'(this, &params)')
    return engine.logic(source)

def header(source):
    # Keep every owner field and expose two C model generators with narrow game access
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return engine.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')).replace('#pragma once','#pragma once\n#include "../vehicles/WheelState.h"',1)

def body(source):
    # Preserve independent statics, control order, narrowing, clamping and branch-specific history updates
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source);source=re.sub(r'\bparams\.', 'params->',source)
    source=source.replace('FindPlayerVehicle()','AudioPoliceHost_PlayerVehicle()').replace('CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle','AudioEnvironmentHost_Remote()')
    source=source.replace('Max(Pads[0].GetAccelerate(), Abs(Pads[0].GetCarGunUpDown()) * 2)', 'AudioVehicleModel_InputMax(AudioVehicleModelHost_Accelerate(), fabsf((float)AudioVehicleModelHost_GunUpDown()) * 2)')
    source=source.replace('Pads[0].GetAccelerate()','AudioVehicleModelHost_Accelerate()').replace('Pads[0].GetBrake()','AudioVehicleModelHost_Brake()')
    source=source.replace('params->m_pVehicle->m_modelIndex','AudioVehicleModelHost_Model(params->m_pVehicle)').replace('((CAutomobile*)params->m_pVehicle)->m_nDriveWheelsOnGround','AudioVehicleReverseHost_DriveWheels(params->m_pVehicle)')
    source=source.replace('params->m_pVehicle->m_fGasPedal','AudioVehicleReverseHost_Gas(params->m_pVehicle)').replace('params->m_pVehicle->m_fBrakePedal','AudioVehicleModelHost_BrakePedal(params->m_pVehicle)')
    source=source.replace('DotProduct(params->m_pVehicle->GetUp(), CVector(0.0f, 1.0f, 0.0f))','AudioVehicleModel_UpDot(params->m_pVehicle)').replace('params->m_pVehicle->GetUp().y','AudioVehicleModelHost_UpY(params->m_pVehicle)').replace('params->m_pVehicle->bIsDrowning','AudioVehicleModelHost_Drowning(params->m_pVehicle)')
    for old,new in [('uint32','uint32_t'),('uint8','uint8_t'),('int16','int16_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0'),('nil','NULL')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('Clamp2(', 'AudioVehicleModel_Clamp2(').replace('Min(', 'AudioVehicleModel_Min(').replace('Abs(', 'fabsf(').replace('SQR(MODEL_CAR_ENGINE_MAX_DIST)','(MODEL_CAR_ENGINE_MAX_DIST * MODEL_CAR_ENGINE_MAX_DIST)').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    source=re.sub(r'\bMax\(', 'AudioVehicleModel_Max(',source)
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_MODEL_'+name,source)
    return source
