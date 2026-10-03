"""Known requested sound submission and reflection C migration."""
import re
import audio_reflections_migration as reflections
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\n#ifdef AUDIO_REFLECTIONS\nuint8_t\nAudioRequestsHost_InRoom(void)\n{\n    // Read the existing camera room flag at the original sound eligibility check\n    return CCullZones::InRoomForAudio();\n}\n#ifndef USE_TIME_SCALE_FOR_AUDIO\nuint8_t\nAudioRequestsHost_SlowMotion(void)\n{\n    // Read slow motion at each original reflection frequency and delay decision\n    return CTimer::GetIsSlowMotionActive();\n}\n#endif\n#endif\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioRequests_Submit(cAudioManager *manager);\n#ifdef AUDIO_REFLECTIONS\nvoid AudioRequests_AddReflections(cAudioManager *manager);\nuint8_t AudioRequestsHost_InRoom(void);\n#ifndef USE_TIME_SCALE_FOR_AUDIO\nuint8_t AudioRequestsHost_SlowMotion(void);\n#endif\n#endif\n'

def calls(source):
    # Submit the same live owner and retain the original reflection recursion point
    source=source.replace('AudioPoliceHost_AddRequested(manager)', 'AudioRequests_Submit(manager)')
    for old,new in [('AddSampleToRequestedQueue','Submit'),('AddReflectionsToRequestedQueue','AddReflections')]:
        source=source.replace('AudioManager.'+old+'()', 'AudioRequests_'+new+'(&AudioManager)')
        source=re.sub(r'(?<![\w.:>])'+old+r'\(\)', 'AudioRequests_'+new+'(this)',source)
    return reflections.owner(source)

def owner(source):
    # Remove exactly the migrated owner methods and add only the game scalar callbacks
    if 'cAudioManager::AddSampleToRequestedQueue(' in source:
        for name in ('AddSampleToRequestedQueue','AddReflectionsToRequestedQueue'):
            source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    return calls(source)

def radio(source):
    # Remove the obsolete police queue wrapper now that channel processing calls C
    if re.search(r'(?m)^AudioPoliceHost_AddRequested\(',source):
        source=source.replace(function(source,'AudioPoliceHost_','AddRequested')+'\n','')
    return calls(source)

def header(source):
    # Remove only obsolete method and callback declarations and expose C operations
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:AddSampleToRequestedQueue|AddReflectionsToRequestedQueue|AudioPoliceHost_AddRequested)\(',line))
    return reflections.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve all gates, mutations, numeric conversions and recursive call timing
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    for old,new in [('uint32','uint32_t'),('uint8','uint8_t'),('int32','int32_t'),('bool8','uint8_t'),('FALSE','0'),('TRUE','1')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('CCullZones::InRoomForAudio()', 'AudioRequestsHost_InRoom()').replace('CTimer::GetIsSlowMotionActive()', 'AudioRequestsHost_SlowMotion()')
    source=source.replace('AddSampleToRequestedQueue()', 'AudioRequests_Submit(manager)').replace('AddReflectionsToRequestedQueue()', 'AudioRequests_AddReflections(manager)')
    source=source.replace('CVector oldPos = AudioSoundPosition_ToVector(&manager->m_sQueueSample.m_vecPos);','AudioSoundPosition oldPos = manager->m_sQueueSample.m_vecPos;').replace('AudioSoundPosition_FromVector(oldPos)','oldPos')
    source=source.replace('ARRAY_SIZE(manager->m_afReflectionsDistances)', '(sizeof(manager->m_afReflectionsDistances) / sizeof(manager->m_afReflectionsDistances[0]))')
    source=source.replace('SET_EMITTING_VOLUME(emittingVolume);','#ifdef EXTERNAL_3D_SOUND\n\t\t\t\tmanager->m_sQueueSample.m_nEmittingVolume = emittingVolume;\n#endif')
    return source
