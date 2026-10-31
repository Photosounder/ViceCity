# Move speech setup and player mood control into C11
import re
from audio_controls_migration import function
NAMES={'SetPedTalkingStatus':'SetTalking','SetPlayersMood':'SetMood','ProcessPlayerMood':'Mood','SetupPedComments':'Setup'}
HOST='''//+ rouz edit (ChatGPT)
extern "C" void
AudioPedSpeechHost_SetTalking(CPed *ped, uint8_t status)
{
    // Write the original boolean talking permission
    ped->m_canTalk = status;
}
extern "C" uint8_t
AudioPedSpeechHost_CanTalk(const CPed *ped)
{
    // Read talking permission before selecting speech
    return ped->m_canTalk;
}
extern "C" uint32_t
AudioPedSpeechHost_Select(cAudioManager *manager, CPed *ped, uint16_t sound)
{
    // Invoke the remaining speech selector with the captured pedestrian
    return manager->GetPedCommentSfx(ped, sound);
}
extern "C" uint8_t
AudioPedSpeechHost_LineClear(const AudioSoundPosition *position)
{
    // Keep both vector arguments and the original visibility flags in the game expression
    return CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), AudioSoundPosition_ToVector(position), true, false, false, false, false, false);
}
extern "C" uint32_t
AudioPedSpeechHost_MissionTime(void)
{
    // Read each original mission-time expression independently
    return CTheScripts::GetLastMissionPassedTime();
}
extern "C" void
AudioPedSpeechHost_SetMissionTime(uint32_t time)
{
    // Preserve the original reference-returning mission-time assignment
    CTheScripts::GetLastMissionPassedTime() = time;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioPedSpeech_SetTalking(CPed *ped, uint8_t status);
void AudioPedSpeech_SetMood(cAudioManager *manager, uint8_t mood, uint32_t time);
void AudioPedSpeech_Mood(cAudioManager *manager);
void AudioPedSpeech_Setup(cAudioManager *manager, cPedParams *params, uint16_t sound);
void AudioPedSpeechHost_SetTalking(CPed *ped, uint8_t status);
uint8_t AudioPedSpeechHost_CanTalk(const CPed *ped);
uint32_t AudioPedSpeechHost_Select(cAudioManager *manager, CPed *ped, uint16_t sound);
uint8_t AudioPedSpeechHost_LineClear(const AudioSoundPosition *position);
uint32_t AudioPedSpeechHost_MissionTime(void);
void AudioPedSpeechHost_SetMissionTime(uint32_t time);
'''
def calls(source):
    # Replace the former speech and mood trampolines with direct C operations
    return source.replace('AudioVehicleOneShotsHost_PedComments(', 'AudioPedSpeech_Setup(').replace('AudioEnvironmentHost_Mood(', 'AudioPedSpeech_Mood(')
def logic(source):
    # Remove complete owner operations and the obsolete service/event bridges
    if 'cAudioManager::SetupPedComments(' in source and 'cAudioManager::GetPedCommentSfx(' not in source:return source
    moved=False
    for name in NAMES:
        if 'cAudioManager::'+name+'(' in source:source=source.replace(function(source,'cAudioManager::',name)+'\n','');moved=True
    if moved:source+=HOST
    for prefix,name,extern in [('AudioEnvironmentHost_','Mood',''),('AudioVehicleOneShotsHost_','PedComments','extern "C" void\n')]:
        if prefix+name+'(' in source:source=source.replace(extern+function(source,prefix,name)+'\n','')
    source=source.replace('SetupPedComments(pedParams,','AudioPedSpeech_Setup(this, &pedParams,')
    # Keep the surviving adapter blocks in a stable order after removing their obsolete entries
    import audio_environment_migration, audio_vehicle_one_shots_migration
    for module,prefix,name,extern in [(audio_environment_migration,'AudioEnvironmentHost_','Mood',''),(audio_vehicle_one_shots_migration,'AudioVehicleOneShotsHost_','PedComments','extern "C" void\n')]:
        block=module.HOST.replace(extern+function(module.HOST,prefix,name)+'\n','')
        if block in source:source=source.replace(block,'')+block
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_ped_talk_migration
    return audio_ped_talk_migration.logic(source)
def header(source):
    # Replace declarations while retaining the complete owner layout
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:'+ '|'.join(NAMES)+r'|AudioEnvironmentHost_Mood|AudioVehicleOneShotsHost_PedComments)\(',line))
    import audio_ped_talk_migration
    return audio_ped_talk_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def facade(source):
    # Move the two public wrappers into the owning C module
    for name in ['SetPedTalkingStatus','SetPlayersMood']:
        if 'DMAudio_'+name+'(' in source:source=source.replace(function(source,'DMAudio_',name)+'\n','')
    import audio_music_state_migration as music
    import audio_music_playback_migration as playback
    return playback.facade(music.calls(source))
def body(source):
    # Preserve gates, timer arithmetic, repeated game reads and comment insertion order
    # Retain original timer boundaries when upstream calls already use C names
    source=source.replace('(*Timer_GetTimeInMilliseconds())', 'CTimer::GetTimeInMilliseconds()')
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('ped->m_canTalk = status','AudioPedSpeechHost_SetTalking(ped, status)').replace('ped->m_canTalk','AudioPedSpeechHost_CanTalk(ped)').replace('GetPedCommentSfx(ped, sound)','AudioPedSpeechHost_Select(manager, ped, sound)').replace('CTimer::GetTimeInMilliseconds()','AudioPlayerEngineHost_Time()').replace('FindPlayerPed()','AudioPoliceHost_FindPlayer()').replace('playerPed->m_pWanted->GetWantedLevel()','AudioPoliceHost_WantedLevel(playerPed)').replace('CTheScripts::GetLastMissionPassedTime() = curTime','AudioPedSpeechHost_SetMissionTime(curTime)').replace('CTheScripts::GetLastMissionPassedTime()','AudioPedSpeechHost_MissionTime()')
    source=source.replace('CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), AudioSoundPosition_ToVector(&manager->m_sQueueSample.m_vecPos), true, false, false, false, false, false)','AudioPedSpeechHost_LineClear(&manager->m_sQueueSample.m_vecPos)').replace('const CVector &commentPosition = AudioSoundPosition_ToVector(&manager->m_sQueueSample.m_vecPos);','const AudioSoundPosition commentPosition = manager->m_sQueueSample.m_vecPos;').replace('SQR(maxDist)','(maxDist * maxDist)')
    # Follow the earlier separately audited vector-boundary form of this producer
    source=source.replace('const CVector &commentPosition = manager->m_sQueueSample.m_vecPos;','const AudioSoundPosition commentPosition = manager->m_sQueueSample.m_vecPos;').replace('CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), manager->m_sQueueSample.m_vecPos, true, false, false, false, false, false)','AudioPedSpeechHost_LineClear(&manager->m_sQueueSample.m_vecPos)')
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('TRUE','1'),('FALSE','0'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    import audio_ped_talk_migration
    return audio_ped_talk_migration.calls(source)
def project(source):
    # Register the owning speech C11 module once
    if 'AudioPedSpeech.c' in source:return source
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioPedOneShots.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    import audio_ped_talk_migration
    return audio_ped_talk_migration.project(source[:b]+'\n'+source[a:b].replace('AudioPedOneShots.c','AudioPedSpeech.c')+source[b:])
