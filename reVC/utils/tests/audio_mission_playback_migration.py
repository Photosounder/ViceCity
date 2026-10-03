"""Known mission playback removals and camera/police host boundaries."""
import re
import audio_police_state_migration as police
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nfloat\nAudioMissionHost_GetDistanceSquared(cAudioManager *manager, const AudioSoundPosition *position)\n{\n\t// Pass plain coordinates to the existing game camera distance calculation\n\treturn manager->GetDistanceSquared(AudioSoundPosition_ToVector(position));\n}\n\nvoid\nAudioMissionHost_Translate(cAudioManager *manager, const AudioSoundPosition *position, AudioSoundPosition *translated)\n{\n\t// Copy coordinates through the existing game camera transform\n\tconst CVector input = AudioSoundPosition_ToVector(position);\n\tCVector output;\n\tmanager->TranslateEntity(&input, &output);\n\t*translated = AudioSoundPosition_FromVector(output);\n}\n\nvoid\nAudioMissionHost_SetPolice(cAudioManager *manager, uint32_t sfx)\n{\n\t// Retain the existing police radio state boundary\n\tmanager->SetMissionScriptPoliceAudio(sfx);\n}\n\nint8_t\nAudioMissionHost_PoliceStatus(cAudioManager *manager)\n{\n\t// Read the existing police radio playback state\n\treturn manager->GetMissionScriptPoliceAudioPlayingStatus();\n}\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioMission_ProcessSlot(cAudioManager *manager, uint8_t slot);\nvoid AudioMission_Process(cAudioManager *manager);\nfloat AudioMissionHost_GetDistanceSquared(cAudioManager *manager, const AudioSoundPosition *position);\nvoid AudioMissionHost_Translate(cAudioManager *manager, const AudioSoundPosition *position, AudioSoundPosition *translated);\nvoid AudioMissionHost_SetPolice(cAudioManager *manager, uint32_t sfx);\nint8_t AudioMissionHost_PoliceStatus(cAudioManager *manager);\n'

def calls(source):
    # Bind service and per-slot operations to the original live owner
    source=source.replace('AudioManager.ProcessMissionAudio()', 'AudioMission_Process(&AudioManager)')
    source=re.sub(r'(?<![\w.:>])ProcessMissionAudio\(\)', 'AudioMission_Process(this)',source)
    return source

def logic(source):
    # Replace precisely the two former members with the required game host operations
    for name in ('ProcessMissionAudioSlot','ProcessMissionAudio'):
        source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    return source.replace('#pragma endregion All the mission audio stuff',HOST+'#pragma endregion All the mission audio stuff')

def header(source):
    # Remove only playback declarations and expose actual C functions and host callbacks
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessMissionAudioSlot|ProcessMissionAudio)\(',line))
    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')

def body(source):
    # Retain statement ordering and replace C++ coordinates and implicit owner access
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('uint8 ','uint8_t ').replace('CVector vec;', 'AudioSoundPosition vec;')
    source=source.replace('TRUE','1').replace('FALSE','0').replace('AudioMission_Clear(this,','AudioMission_Clear(manager,')
    source=source.replace('GetDistanceSquared(AudioSoundPosition_ToVector(&manager->m_vecMissionAudioPosition[slot]))','AudioMissionHost_GetDistanceSquared(manager, &manager->m_vecMissionAudioPosition[slot])')
    source=source.replace('const CVector missionPosition = AudioSoundPosition_ToVector(&manager->m_vecMissionAudioPosition[slot]);','const AudioSoundPosition missionPosition = manager->m_vecMissionAudioPosition[slot];').replace('TranslateEntity(&missionPosition, &vec);','AudioMissionHost_Translate(manager, &missionPosition, &vec);')
    source=source.replace('SetMissionScriptPoliceAudio(', 'AudioMissionHost_SetPolice(manager, ').replace('GetMissionScriptPoliceAudioPlayingStatus()', 'AudioMissionHost_PoliceStatus(manager)')
    source=source.replace('ProcessMissionAudioSlot(i);','AudioMission_ProcessSlot(manager, i);').replace('Sqrt(', 'sqrtf(')
    return police.playback(source)
