"""Move complete music mode changes and frontend streaming service into C11."""
import re
import audio_music_radio_migration as radio
from audio_controls_migration import function
METHODS=['ChangeMusicMode','ServiceFrontEndMode']
PROTOTYPES='''void MusicModes_ChangeMusicMode(cMusicManager *manager, uint8_t mode);
void MusicModes_ServiceFrontEndMode(cMusicManager *manager);
'''
STORE='''#if defined(_MSC_VER) && !defined(__clang__)
#define MUSIC_MODES_STORE_TRACK_VALUE(manager, field, expression) do { \\
    /* Capture the provider result before selecting the live destination track */ \\
    uint32_t result = (expression); \\
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = result; \\
} while (0)
#else
#define MUSIC_MODES_STORE_TRACK_VALUE(manager, field, expression) \\
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = (expression)
#endif
'''

def calls(source):
    # Route complete mode operations through the same live music owner
    source=source.replace('MusicManager.ChangeMusicMode(', 'MusicModes_ChangeMusicMode(&MusicManager, ')
    if 'cMusicManager::cMusicManager(' in source:
        source=source.replace('ServiceFrontEndMode();','MusicModes_ServiceFrontEndMode(this);')
        source=re.sub(r'(?<![\w.:>])ChangeMusicMode\(', 'MusicModes_ChangeMusicMode(this, ',source)
    return radio.calls(source)

def logic(source):
    # Remove complete members and the public facade now compiled in C
    for name in METHODS:
        if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
    if re.search(r'(?m)^DMAudio_ChangeMusicMode\(',source):source=source.replace(function(source,'DMAudio_','ChangeMusicMode')+'\n','')
    return radio.logic(calls(source))

def header(source):
    # Preserve stored fields while replacing converted method declarations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+n+r'\(',line) for n in METHODS))
    return radio.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Retain stream polling, live destination reads, shared statistics state and branch ordering
    source=source[source.index('{'):]
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('MusicState_IsInitialised(this)','MusicState_IsInitialised(manager)').replace('MusicPositions_GetTrackStartPos(this,','MusicPositions_GetTrackStartPos(manager,').replace('MusicPositions_RecordRadioStats(this)','MusicPositions_RecordRadioStats(manager)')
    source=source.replace('ChangeMusicMode(MUSICMODE_GAME)','MusicModes_ChangeMusicMode(manager, MUSICMODE_GAME)').replace('CTimer::GetTimeInMillisecondsPauseMode()','MusicInitHost_PauseTime()')
    for a,b in [('uint32','uint32_t'),('bool8','uint8_t'),('FALSE','0'),('TRUE','1')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('Min(', 'MUSIC_MODES_MIN(')
    for field,expression in [('m_nPosition','SampleManager_GetStreamedFilePosition(&SampleManager, 0)'),('m_nLastPosCheckTimer','MusicInitHost_PauseTime()')]:
        source=source.replace('manager->m_aTracks[manager->m_nPlayingTrack].'+field+' = '+expression+';', 'MUSIC_MODES_STORE_TRACK_VALUE(manager, '+field+', '+expression+');')
    return source

def project(source):
    # Compile both complete operations and the public facade as C11
    if 'MusicModes.c' in source:return radio.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicClubs.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return radio.project(source[:b]+'\n'+source[a:b].replace('MusicClubs.c','MusicModes.c')+source[b:])
