# Preserve horn patterns, siren classification and complete horn/alarm generators
import re
import audio_vehicle_warning_migration as warning
from audio_controls_migration import function
NAMES={'ProcessVehicleHorn':'Horn','UsesSiren':'Uses','UsesSirenSwitching':'Switching','ProcessVehicleSirenOrAlarm':'Siren'}
CONSTANTS='\tVEHICLE_HORN_MAX_DIST = 40,\n\tVEHICLE_HORN_VOLUME = 80,\n\n\tVEHICLE_SIREN_MAX_DIST = 110,\n\tVEHICLE_SIREN_VOLUME = 80,\n'
def pattern(source):
    # Preserve every original pattern bit and both dimensions
    return re.search(r'Const static bool8 HornPattern\[8\]\[44\] = \{.*?\};',source,re.S).group()
def models(source):
    # Preserve the original audio table index enum independently of model IDs
    return re.search(r'enum eVehicleModel \{.*?\n\};',source,re.S).group()
HOST='//+ rouz edit (ChatGPT)\n'
PROTOTYPES='uint8_t AudioVehicleHorn_Horn(cAudioManager *manager, cVehicleParams *params);\nuint8_t AudioVehicleHorn_Siren(cAudioManager *manager, cVehicleParams *params);\n'
for result,name,args,expr in [
 ('uint8_t','Enabled','const CVehicle *vehicle','vehicle->m_bSirenOrAlarm'),
 ('uint8_t','Alarm','CVehicle *vehicle','vehicle->IsAlarmOn()'),
 ('uint8_t','Uses','CVehicle *vehicle','vehicle->UsesSiren()'),
 ('uint32_t','Timer','const CVehicle *vehicle','vehicle->m_nCarHornTimer'),
 ('uint8_t','Pattern','const CVehicle *vehicle','vehicle->m_nCarHornPattern'),
 ('int32_t','HornSample','uint32_t index','aVehicleSettings[index].m_nHornSample'),
 ('int32_t','HornFrequency','uint32_t index','aVehicleSettings[index].m_nHornFrequency'),
 ('uint8_t','SirenSample','uint32_t index','aVehicleSettings[index].m_nSirenOrAlarmSample'),
 ('int32_t','SirenFrequency','uint32_t index','aVehicleSettings[index].m_nSirenOrAlarmFrequency')]:
    HOST+=result+'\nAudioVehicleHornHost_'+name+'('+args+')\n{\n    // Read the original vehicle predicate or shared sound setting at its access point\n    return '+expr+';\n}\n\n'
    PROTOTYPES+=result+' AudioVehicleHornHost_'+name+'('+args+');\n'
for name,typ,field in [('Timer','uint32_t','m_nCarHornTimer'),('Pattern','uint8_t','m_nCarHornPattern')]:
    HOST+='void\nAudioVehicleHornHost_Set'+name+'(CVehicle *vehicle, '+typ+' value)\n{\n    // Update the original vehicle field at the horn or alarm assignment point\n    vehicle->'+field+' = value;\n}\n\n'
    PROTOTYPES+='void AudioVehicleHornHost_Set'+name+'(CVehicle *vehicle, '+typ+' value);\n'
HOST+='//- rouz edit (ChatGPT)\n'
def logic(source):
    # Remove all four complete members and move the pattern and audio indices into C-compatible code
    if 'cAudioManager::ProcessVehicleHorn(' in source:
        for name in NAMES:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source=source.replace(pattern(source)+'\n','').replace(models(source),'#include "AudioVehicleModels.h"').replace(CONSTANTS,'')+HOST
    if HOST in source:source=source.replace(HOST,'')+HOST
    return warning.logic(source.replace('ProcessVehicleHorn(params)','AudioVehicleHorn_Horn(this, &params)').replace('ProcessVehicleSirenOrAlarm(params)','AudioVehicleHorn_Siren(this, &params)'))
def header(source):
    # Preserve owner state and replace the former member declarations with C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return warning.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Preserve short circuits, live parameter reads, captured siren vehicle and timer arithmetic
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('params->m_pVehicle->m_nCarHornTimer = Min(44, params->m_pVehicle->m_nCarHornTimer);','''// Resolve the original assignment target after computing its clamped right-hand value
                uint32_t clampedTimer = Min(44, AudioVehicleHornHost_Timer(params->m_pVehicle));
                AudioVehicleHornHost_SetTimer(params->m_pVehicle, clampedTimer);''')
    source=source.replace('params->m_pVehicle->m_nCarHornPattern = (manager->m_FrameCounter + manager->m_sQueueSample.m_nEntityIndex) & 7','AudioVehicleHornHost_SetPattern(params->m_pVehicle, (manager->m_FrameCounter + manager->m_sQueueSample.m_nEntityIndex) & 7)')
    source=source.replace('veh->m_nCarHornTimer = CTimer::GetTimeInMilliseconds() + 750','AudioVehicleHornHost_SetTimer(veh, AudioPlayerEngineHost_Time() + 750)')
    for a,b in [
      ('params->m_pVehicle->m_bSirenOrAlarm','AudioVehicleHornHost_Enabled(params->m_pVehicle)'),
      ('params->m_pVehicle->UsesSiren()','AudioVehicleHornHost_Uses(params->m_pVehicle)'),
      ('params->m_pVehicle->IsAlarmOn()','AudioVehicleHornHost_Alarm(params->m_pVehicle)'),
      ('params->m_pVehicle->m_nCarHornTimer','AudioVehicleHornHost_Timer(params->m_pVehicle)'),
      ('params->m_pVehicle->m_nCarHornPattern','AudioVehicleHornHost_Pattern(params->m_pVehicle)'),
      ('params->m_pVehicle->GetModelIndex()','AudioVehicleReverseHost_Model(params->m_pVehicle)'),
      ('params->m_pVehicle->GetStatus()','AudioVehicleEngineHost_Status(params->m_pVehicle)'),
      ('params->m_pVehicle->bIsDrowning','AudioVehicleModelHost_Drowning(params->m_pVehicle)'),
      ('veh->m_bSirenOrAlarm','AudioVehicleHornHost_Enabled(veh)'),
      ('veh->IsAlarmOn()','AudioVehicleHornHost_Alarm(veh)'),
      ('veh->m_nCarHornTimer','AudioVehicleHornHost_Timer(veh)'),
      ('veh->bIsDrowning','AudioVehicleModelHost_Drowning(veh)'),
      ('CTimer::GetTimeInMilliseconds()','AudioPlayerEngineHost_Time()'),
      ('aVehicleSettings[params->m_nIndex].m_nHornSample','AudioVehicleHornHost_HornSample(params->m_nIndex)'),
      ('aVehicleSettings[params->m_nIndex].m_nHornFrequency','AudioVehicleHornHost_HornFrequency(params->m_nIndex)'),
      ('aVehicleSettings[params->m_nIndex].m_nSirenOrAlarmSample','AudioVehicleHornHost_SirenSample(params->m_nIndex)'),
      ('aVehicleSettings[params->m_nIndex].m_nSirenOrAlarmFrequency','AudioVehicleHornHost_SirenFrequency(params->m_nIndex)')]:source=source.replace(a,b)
    for name,target in NAMES.items():source=source.replace(name+'(params)','AudioVehicleHorn_'+target+'(params)')
    for a,b in [('uint8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('Min(', 'AudioVehicleHorn_Min(').replace('SQR(VEHICLE_HORN_MAX_DIST)','(VEHICLE_HORN_MAX_DIST * VEHICLE_HORN_MAX_DIST)').replace('SQR(VEHICLE_SIREN_MAX_DIST)','(VEHICLE_SIREN_MAX_DIST * VEHICLE_SIREN_MAX_DIST)').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_HORN_'+name,source)
    return source
