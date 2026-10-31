"""Move complete script-object routing and both sound generators into C11."""
import re
import audio_music_clubs_migration as clubs
from audio_controls_migration import function

METHODS={'ProcessScriptObject':'Process','ProcessOneShotScriptObject':'OneShot','ProcessLoopingScriptObject':'Looping'}
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint8_t
AudioScriptSoundsHost_MusicMode(void)
{
    // Read the original music mode before accessing the script entity
    return MusicManager.m_nMusicMode;
}
extern "C" uint32_t
AudioScriptSoundsHost_PlayingTrack(void)
{
    // Read the live playing track at each club sound gate
    return MusicManager.m_nPlayingTrack;
}
extern "C" void
AudioScriptSoundsHost_MalibuTrack(uint8_t sound)
{
    // Update the original club track position before querying sample frequency
    MusicManager.SetMalibuClubTrackPos(sound);
}
extern "C" void
AudioScriptSoundsHost_StripTrack(uint8_t sound)
{
    // Update the original strip-club track position before querying sample frequency
    MusicManager.SetStripClubTrackPos(sound);
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioScriptSounds_Process(cAudioManager *manager, int32_t id);
void AudioScriptSounds_OneShot(cAudioManager *manager, uint8_t sound);
void AudioScriptSounds_Looping(cAudioManager *manager, uint8_t sound);
uint8_t AudioScriptSoundsHost_MusicMode(void);
uint32_t AudioScriptSoundsHost_PlayingTrack(void);
void AudioScriptSoundsHost_MalibuTrack(uint8_t sound);
void AudioScriptSoundsHost_StripTrack(uint8_t sound);
'''
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']

def constants(source):
    # Capture the original private constants including their existing unused entries
    a=source.index('\tSCRIPT_OBJECT_SEAPLANE_LOW_FUEL_MAX_DIST =');b=source.index('\n\tFRONTEND_VOLUME',a)
    return source[a:b]

def calls(source):
    # Enter complete script audio directly from the C dispatcher
    import audio_weather_frontend_migration
    return audio_weather_frontend_migration.calls(source.replace('AudioDispatchHost_ProcessScriptObject(manager,','AudioScriptSounds_Process(manager,'))

def logic(source):
    # Remove the complete owning functions while preserving older partial-body audits
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessScriptObject(' in source:
        source=source.replace(constants(source),'')
        for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    if 'void\nAudioDispatchHost_ProcessScriptObject(' in source:source=source.replace(function(source,'AudioDispatchHost_','ProcessScriptObject')+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_weather_frontend_migration
    return audio_weather_frontend_migration.logic(source)

def header(source):
    # Replace only operation declarations without changing any stored owner field
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:'+ '|'.join(METHODS)+r'|AudioDispatchHost_ProcessScriptObject)\(',line))
    import audio_weather_frontend_migration
    return audio_weather_frontend_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Retain switch decisions, narrow conversions, callback order and shared counter lifetime
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('MusicManager.m_nMusicMode','AudioScriptSoundsHost_MusicMode()').replace('MusicManager.m_nPlayingTrack','AudioScriptSoundsHost_PlayingTrack()')
    source=source.replace('MusicManager.SetMalibuClubTrackPos(','AudioScriptSoundsHost_MalibuTrack(').replace('MusicManager.SetStripClubTrackPos(','AudioScriptSoundsHost_StripTrack(')
    source=source.replace('CPlayerPed *playerPed;','CPed *playerPed;').replace('FindPlayerPed()','AudioPedOneShotsHost_Player()').replace('playerPed->m_nSurfaceTouched','AudioPedOneShotsHost_Surface(playerPed)')
    source=source.replace('ProcessOneShotScriptObject(','AudioScriptSounds_OneShot(manager, ').replace('ProcessLoopingScriptObject(','AudioScriptSounds_Looping(manager, ')
    source=source.replace('SQR(manager->m_sQueueSample.m_MaxDistance)','(manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)').replace('Sqrt(distSquared)','(distSquared <= 0.0f ? 0.0f : sqrtf(distSquared))')
    source=source.replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)').replace('AudioPedSpeech_Setup(this,','AudioPedSpeech_Setup(manager,')
    for a,b in [('uint8','uint8_t'),('int32','int32_t'),('TRUE','1'),('FALSE','0'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_SCRIPT_SOUNDS_'+name,source)
    return clubs.calls(source)

def project(source):
    # Register the C11 script generators beside the preceding water-cannon conversion
    import audio_weather_frontend_migration
    if 'AudioScriptSounds.c' in source:return audio_weather_frontend_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioWaterCannon.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_weather_frontend_migration.project(source[:b]+'\n'+source[a:b].replace('AudioWaterCannon.c','AudioScriptSounds.c')+source[b:])
