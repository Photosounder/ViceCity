"""Known police channel and crackle C migrations."""
import re
import audio_requests_migration as requests
import audio_police_reports_migration as reports
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nvoid\nAudioPoliceHost_AddRequested(cAudioManager *manager)\n{\n\t// Submit the prepared sound through the existing game queue boundary\n\tmanager->AddSampleToRequestedQueue();\n}\n\n#ifdef FIX_BUGS\nuint32_t\nAudioPoliceHost_LogicalFramesPassed(void)\n{\n\t// Read the existing game timer at the original wait-counter update point\n\treturn CTimer::GetLogicalFramesPassed();\n}\n#endif\n\n#ifdef USE_TIME_SCALE_FOR_AUDIO\nfloat\nAudioPoliceHost_TimeScale(void)\n{\n\t// Read the existing game time scale at the original frequency update point\n\treturn CTimer::GetTimeScale();\n}\n#endif\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioPolice_Crackle(cAudioManager *manager);\nvoid AudioPolice_ServiceChannel(cAudioManager *manager, uint8_t wantedLevel);\nvoid AudioPoliceHost_AddRequested(cAudioManager *manager);\n#ifdef FIX_BUGS\nuint32_t AudioPoliceHost_LogicalFramesPassed(void);\n#endif\n#ifdef USE_TIME_SCALE_FOR_AUDIO\nfloat AudioPoliceHost_TimeScale(void);\n#endif\n'

def calls(source):
    # Keep original call order and wanted-level byte conversion
    source=source.replace('AudioManager.DoPoliceRadioCrackle()', 'AudioPolice_Crackle(&AudioManager)')
    source=source.replace('ServicePoliceRadioChannel(wantedLevel);','AudioPolice_ServiceChannel(this, wantedLevel);').replace('DoPoliceRadioCrackle();','AudioPolice_Crackle(this);')
    return reports.calls(source)

def radio(source):
    # Replace the two original members with the queue and timer host boundaries
    source=source.replace(function(source,'cAudioManager::','DoPoliceRadioCrackle')+'\n','')
    source=source.replace(function(source,'cAudioManager::','ServicePoliceRadioChannel')+'\n',HOST)
    return reports.radio(calls(source))

def header(source):
    # Preserve record fields and declare the actual C operations and game callbacks
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:DoPoliceRadioCrackle|ServicePoliceRadioChannel)\(',line))
    return reports.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Qualify the same owner and expand only the original conditional sound macros
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('bool8 ','uint8_t ').replace('uint32 ','uint32_t ').replace('uint8 ','uint8_t ').replace('FALSE','0').replace('TRUE','1')
    source=source.replace('AudioPolice_Reset(this)','AudioPolice_Reset(manager)').replace('DoPoliceRadioCrackle();','AudioPolice_Crackle(manager);').replace('AddSampleToRequestedQueue();','AudioPoliceHost_AddRequested(manager);')
    source=source.replace('CTimer::GetLogicalFramesPassed()', 'AudioPoliceHost_LogicalFramesPassed()').replace('CTimer::GetTimeScale()', 'AudioPoliceHost_TimeScale()')
    source=source.replace('SET_EMITTING_VOLUME(manager->m_sQueueSample.m_nVolume);','#ifdef EXTERNAL_3D_SOUND\n\tmanager->m_sQueueSample.m_nEmittingVolume = manager->m_sQueueSample.m_nVolume;\n#endif')
    source=source.replace('SET_LOOP_OFFSETS(SFX_POLICE_RADIO_CRACKLE)','#ifndef GTA_PS2\n\tmanager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, SFX_POLICE_RADIO_CRACKLE);\n\tmanager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, SFX_POLICE_RADIO_CRACKLE);\n#endif')
    source=source.replace('SET_SOUND_REVERB(0);','#ifdef AUDIO_REVERB\n\tmanager->m_sQueueSample.m_bReverb = 0;\n#endif').replace('SET_SOUND_REFLECTION(0);','#ifdef AUDIO_REFLECTIONS\n\tmanager->m_sQueueSample.m_bReflections = 0;\n#endif')
    return requests.calls(source)
