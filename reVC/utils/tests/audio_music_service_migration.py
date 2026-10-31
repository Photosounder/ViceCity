"""Move the complete music dispatcher and gameplay service into C11."""
import re
import audio_music_display_migration as display
from audio_controls_migration import function
METHODS=['Service','ServiceGameMode']
GLOBALS='''int32_t gNumRetunePresses;
int32_t gRetuneCounter;
uint8_t g_bAnnouncementReadPosAlready;
uint8_t RadioStaticCounter = 5;
'''
PROTOTYPES='''void MusicService_Run(cMusicManager *manager);
void MusicService_GameMode(cMusicManager *manager);
extern uint8_t RadioStaticCounter;
#ifdef GTA_PC
uint8_t MusicServiceHost_F9(void);
#endif
uint8_t MusicServiceHost_ChangeStation(void);
#ifdef RADIO_SCROLL_TO_PREV_STATION
uint8_t MusicServiceHost_ControlsDisabled(void);
uint8_t MusicServiceHost_WheelDown(void);
uint8_t MusicServiceHost_WheelUp(void);
int32_t MusicServiceHost_NextMouseKey(void);
uint8_t MusicServiceHost_MouseAssigned(int32_t key);
#endif
'''
HOST='''//+ rouz edit (ChatGPT)
#ifdef GTA_PC
extern "C" uint8_t
MusicServiceHost_F9(void)
{
    // Query the original keyboard key after the MP3 and station gates
    return ControlsManager.GetIsKeyboardKeyJustDown(rsF9);
}
#endif
extern "C" uint8_t
MusicServiceHost_ChangeStation(void)
{
    // Retain the original pad lookup before its station-change query
    return CPad::GetPad(0)->ChangeStationJustDown();
}
#ifdef RADIO_SCROLL_TO_PREV_STATION
extern "C" uint8_t
MusicServiceHost_ControlsDisabled(void)
{
    // Retain the original pad lookup before checking disabled controls
    return CPad::GetPad(0)->ArePlayerControlsDisabled();
}
extern "C" uint8_t
MusicServiceHost_WheelDown(void)
{
    // Retain the original repeated pad lookup before the downward wheel query
    return CPad::GetPad(0)->GetMouseWheelDownJustDown();
}
extern "C" uint8_t
MusicServiceHost_WheelUp(void)
{
    // Retain the original repeated pad lookup before the upward wheel query
    return CPad::GetPad(0)->GetMouseWheelUpJustDown();
}
extern "C" int32_t
MusicServiceHost_NextMouseKey(void)
{
    // Read the original radio-station mouse binding without normalizing its key code
    return ControlsManager.GetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, MOUSE);
}
extern "C" uint8_t
MusicServiceHost_MouseAssigned(int32_t key)
{
    // Check the captured opposite wheel key at the original vehicle-action gate
    return ControlsManager.IsAnyVehicleActionAssignedToMouseKey(key);
}
#endif
//- rouz edit (ChatGPT)
'''
STORE='''#if defined(_MSC_VER) && !defined(__clang__)
#define MUSIC_SERVICE_STORE_TRACK_VALUE(manager, index, field, expression) do { \\
    /* Capture the provider result before selecting the live destination track */ \\
    uint32_t result = (expression); \\
    (manager)->m_aTracks[(manager)->index].field = result; \\
} while (0)
#else
#define MUSIC_SERVICE_STORE_TRACK_VALUE(manager, index, field, expression) \\
    (manager)->m_aTracks[(manager)->index].field = (expression)
#endif
'''
HELPER='''static uint8_t
MusicService_PedDyingOrDead(const CPed *ped)
{
    // Preserve the original short circuit between the two live pedestrian-state reads
    return AudioPedOneShotsHost_State(ped) == PED_DIE || AudioPedOneShotsHost_State(ped) == PED_DEAD;
}
'''

def remove_host(source):
    # Remove the complete obsolete audio-to-music wrapper
    if re.search(r'(?m)^AudioServiceHost_Music\(',source):source=source.replace(function(source,'AudioServiceHost_','Music')+'\n','')
    return display.calls(source)

def calls(source):
    # Enter complete C music service at each original audio or music dispatch boundary
    source=remove_host(source).replace('AudioServiceHost_Music()', 'MusicService_Run(&MusicManager)').replace('MusicManager.Service()', 'MusicService_Run(&MusicManager)')
    source=source.replace('MusicManager.ServiceGameMode()', 'MusicService_GameMode(&MusicManager)')
    if 'cMusicManager::cMusicManager(' in source:source=re.sub(r'(?<![\w.:>])ServiceGameMode\(\)', 'MusicService_GameMode(this)',source)
    return display.calls(source)

def logic(source):
    # Remove both complete methods and move scalar music storage to C
    owner='cMusicManager::cMusicManager(' in source
    for name in METHODS:
        if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
    if owner:
        for line in ['int32 gNumRetunePresses;','int32 gRetuneCounter;','bool8 g_bAnnouncementReadPosAlready;','uint8 RadioStaticCounter = 5;']:source=source.replace(line+'\n','')
    source=calls(source)
    if owner and 'MusicServiceHost_ChangeStation(void)' not in source:source+='\n'+HOST
    if HOST in source:
        # Keep new control readers after adapters inserted by preceding migration stages
        source=source.replace(HOST,'')+HOST
    return display.logic(source)

def header(source):
    # Preserve record fields while replacing the dispatcher and obsolete wrapper declarations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+n+r'\(',line) for n in METHODS+['AudioServiceHost_Music']))
    if 'void MusicState_InitState(cMusicManager *manager);' in source:source=source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);')
    return display.header(source)

def audio(source):
    # Keep music service after sound effects while entering the C owner directly
    return calls(source.replace('#include "AudioManager.h"','#include "AudioManager.h"\n#include "MusicManager.h"',1))

def body(source):
    # Preserve all gates, repeated control queries, live fields and original retune arithmetic
    source=source[source.index('{'):]
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('nFramesSinceCutsceneEnded','manager->nFramesSinceCutsceneEnded')
    source=source.replace('ServiceGameMode()', 'MusicService_GameMode(manager)')
    source=source.replace('FindPlayerPed()', 'AudioPedOneShotsHost_Player()').replace('ped->DyingOrDead()', 'MusicService_PedDyingOrDead(ped)').replace('CGame::currArea','MusicAmbienceHost_Area()')
    source=source.replace('vehicle->m_nRadioStation','(*MusicRadioHost_Station(vehicle))')
    source=source.replace('ControlsManager.GetIsKeyboardKeyJustDown(rsF9)', 'MusicServiceHost_F9()')
    for original,converted in [('ChangeStationJustDown','ChangeStation'),('ArePlayerControlsDisabled','ControlsDisabled'),('GetMouseWheelDownJustDown','WheelDown'),('GetMouseWheelUpJustDown','WheelUp')]:source=source.replace('CPad::GetPad(0)->'+original+'()', 'MusicServiceHost_'+converted+'()')
    source=source.replace('ControlsManager.GetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, MOUSE)','MusicServiceHost_NextMouseKey()').replace('ControlsManager.IsAnyVehicleActionAssignedToMouseKey(scrollPrev)','MusicServiceHost_MouseAssigned(scrollPrev)')
    source=source.replace('CTimer::GetTimeInMillisecondsPauseMode()','MusicInitHost_PauseTime()').replace('CTimer::GetTimeInMilliseconds()','AudioPlayerEngineHost_Time()')
    source=source.replace('(this', '(manager')
    for a,b in [('uint32','uint32_t'),('int32','int32_t'),('bool8','uint8_t'),('FALSE','0'),('TRUE','1'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for index in ['m_nPlayingTrack','m_nFrontendTrack']:
        for field,expression in [('m_nPosition','SampleManager_GetStreamedFilePosition(&SampleManager, 0)'),('m_nLastPosCheckTimer','MusicInitHost_PauseTime()')]:
            source=source.replace('manager->m_aTracks[manager->'+index+'].'+field+' = '+expression+';', 'MUSIC_SERVICE_STORE_TRACK_VALUE(manager, '+index+', '+field+', '+expression+');')
    return source

def keys(source):
    # Share the exact original key enum and typedef with C without copying numeric definitions
    a=source.index('enum RsKeyCodes\n');b=source.index('typedef enum RsKeyCodes RsKeyCodes;',a)+len('typedef enum RsKeyCodes RsKeyCodes;')
    return source[a:b]

def skeleton(source):
    # Replace the existing key enum block with its shared C header
    return '//+ rouz edit (ChatGPT)\n'+source.replace(keys(source),'#include "RsKeys.h"')+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register the complete C service and shared key definitions
    if 'MusicService.c' in source:return display.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicStream.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    source=source[:b]+'\n'+source[a:b].replace('MusicStream.c','MusicService.c')+source[b:]
    a=source.index('    <ClInclude Include="..\\src\\skel\\skeleton.h"');end=source.index('>',a)+1
    b=end if source[a:end].endswith('/>') else source.index('</ClInclude>',a)+len('</ClInclude>')
    return display.project(source[:b]+'\n'+source[a:b].replace('skeleton.h','RsKeys.h')+source[b:])
