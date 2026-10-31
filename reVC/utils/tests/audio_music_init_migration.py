"""Move complete music initialization and starting-track selection into C11."""
import re
import audio_music_positions_migration as positions
from audio_controls_migration import function
METHODS=['Initialise','SetStartingTrackPositions']
PROTOTYPES='''uint8_t MusicInit_Initialise(cMusicManager *manager);
void MusicInit_SetStartingTrackPositions(cMusicManager *manager, uint8_t isNewGameTimer);
uint32_t MusicInitHost_PauseTime(void);
extern int32_t gNumRetunePresses;
extern int32_t gRetuneCounter;
extern uint32_t NewGameRadioTimers[10];
'''
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint32_t
MusicInitHost_PauseTime(void)
{
    // Read the original pause-mode timer after each streamed-file length and position update
    return CTimer::GetTimeInMillisecondsPauseMode();
}
//- rouz edit (ChatGPT)
'''
def timer_table(source):
    # Extract the original writable starting-position table without changing its values
    a=source.index('uint32 NewGameRadioTimers[10]');b=source.index('};',a)+2
    return source[a:b]

def calls(source):
    # Enter complete C initialization and selection at the existing global call sites
    source=source.replace('MusicManager.Initialise()', 'MusicInit_Initialise(&MusicManager)').replace('MusicManager.SetStartingTrackPositions(', 'MusicInit_SetStartingTrackPositions(&MusicManager, ')
    return positions.calls(source.replace('AudioLifecycleHost_MusicInitialise()', 'MusicInit_Initialise(&MusicManager)'))

def facade(source):
    # Remove the complete public starting-track facade now compiled in C
    if re.search(r'(?m)^DMAudio_SetStartingTrackPositions\(',source):source=source.replace(function(source,'DMAudio_','SetStartingTrackPositions')+'\n','')
    return positions.logic(calls(source))

def logic(source):
    # Remove converted methods, the table storage and the obsolete initialization trampoline
    if 'cMusicManager::SetStartingTrackPositions(' in source:
        for name in METHODS:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
        source=source.replace(timer_table(source)+'\n','')+HOST
    if 'void\nAudioLifecycleHost_MusicInitialise(' in source:source=source.replace(function(source,'AudioLifecycleHost_','MusicInitialise')+'\n','')
    return facade(source)

def header(source):
    # Preserve all record fields while replacing methods and the obsolete callback declaration
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS+['AudioLifecycleHost_MusicInitialise']))
    return positions.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Preserve original time/random arithmetic, live reads, narrowing, timer order and partial writes
    source=source[source.index('{'):].replace('MusicState_IsInitialised(this)','MusicState_IsInitialised(manager)').replace('tm* pTm','struct tm* pTm')
    source=re.sub(r'(?<![\w.>])\b(m_\w+|aListenTimeArray|nFramesSinceCutsceneEnded)\b',r'manager->\1',source)
    source=source.replace('SetStartingTrackPositions(FALSE)','MusicInit_SetStartingTrackPositions(manager, 0)').replace('CTimer::GetTimeInMillisecondsPauseMode()','MusicInitHost_PauseTime()')
    return source.replace('FALSE','0').replace('TRUE','1')

def project(source):
    # Register complete C initialization and track selection beside C playback
    if 'MusicInit.c' in source:return positions.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicPlayback.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return positions.project(source[:b]+'\n'+source[a:b].replace('MusicPlayback.c','MusicInit.c')+source[b:])
