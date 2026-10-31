"""Move complete weather and front-end sound generators into C11."""
import re
import wind_migration as wind
from audio_controls_migration import function
METHODS={'ProcessWeather':'AudioWeather_Process','ProcessFrontEnd':'AudioFrontEnd_Process'}
CONSTANTS='\tFRONTEND_VOLUME = 127,\n'
HOST='''//+ rouz edit (ChatGPT)
extern "C" float
AudioWeatherHost_SoundDistUp(void)
{
    // Read the original camera ceiling distance at the thunder-volume gate
    return TheCamera.SoundDistUp;
}
extern "C" void
AudioWeatherHost_WindModifier(float *x, float *y)
{
    // Query the original camera position and wind modifier after rain processing
    CWindModifiers::FindWindModifier(TheCamera.GetPosition(), x, y);
}
extern "C" float
AudioWeatherHost_Wind(void)
{
    // Read the live wind value at each original comparison or volume expression
    return CWeather::Wind;
}
extern "C" float
AudioWeatherHost_NearestTree(void)
{
    // Read the live tree distance at the original gate or volume expression
    return CObject::fDistToNearestTree;
}
extern "C" void
AudioWeatherHost_ResetNearestTree(void)
{
    // Reset the original tree distance only after submitting the wind sound
    CObject::fDistToNearestTree = 999999.9f;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioWeather_Process(cAudioManager *manager, int32_t id);
void AudioFrontEnd_Process(cAudioManager *manager);
float AudioWeatherHost_SoundDistUp(void);
void AudioWeatherHost_WindModifier(float *x, float *y);
float AudioWeatherHost_Wind(void);
float AudioWeatherHost_NearestTree(void);
void AudioWeatherHost_ResetNearestTree(void);
'''
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']

def calls(source):
    # Dispatch directly into both complete C sound generators
    import audio_world_loops_migration
    return audio_world_loops_migration.calls(source.replace('AudioDispatchHost_ProcessWeather(manager,','AudioWeather_Process(manager,').replace('AudioDispatchHost_ProcessFrontEnd(manager)','AudioFrontEnd_Process(manager)'))

def logic(source):
    # Remove complete owners and their obsolete routing trampolines
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessWeather(' in source:
        for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source=source.replace(CONSTANTS,'')+HOST
    for name in METHODS:
        if 'void\nAudioDispatchHost_'+name+'(' in source:source=source.replace(function(source,'AudioDispatchHost_',name)+'\n','')
    current_host=wind.calls(HOST)
    import weather_migration as weather
    host=next((candidate for candidate in [HOST,current_host,weather.legacy_member_spelling(current_host),weather.calls(HOST)] if candidate in source),HOST)
    if host in source:source=source.replace(host,'')+host
    import audio_world_loops_migration
    return audio_world_loops_migration.logic(source)

def header(source):
    # Replace operation declarations while preserving all stored owner fields
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessWeather|ProcessFrontEnd|AudioDispatchHost_ProcessWeather|AudioDispatchHost_ProcessFrontEnd)\(',line))
    import audio_world_loops_migration
    return audio_world_loops_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve original arithmetic, repeated live reads, shared statics and queue ordering
    import weather_migration as weather
    source=weather.legacy_member_spelling(source)
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('CObject::fDistToNearestTree = 999999.9f;','AudioWeatherHost_ResetNearestTree();')
    for a,b in [('TheCamera.SoundDistUp','AudioWeatherHost_SoundDistUp()'),('CWeather::Rain','AudioVehicleRainHost_Rain()'),('CCullZones::CamNoRain()','AudioVehicleRainHost_CameraNoRain()'),('CCullZones::PlayerNoRain()','AudioVehicleRainHost_PlayerNoRain()'),('CWindModifiers::FindWindModifier(TheCamera.GetPosition(), &x, &y)','AudioWeatherHost_WindModifier(&x, &y)'),('CWeather::Wind','AudioWeatherHost_Wind()'),('CObject::fDistToNearestTree','AudioWeatherHost_NearestTree()'),('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')]:source=source.replace(a,b)
    for a,b in [('uint8','uint8_t'),('uint32','uint32_t'),('int16','int16_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0'),('Max','AUDIO_WEATHER_FRONTEND_MAX'),('Min','AUDIO_WEATHER_FRONTEND_MIN'),('Abs','fabsf')]:source=re.sub(r'\b'+a+r'\b',b,source)
    # Keep the original comparison read separate from the selected-value read under native optimization
    source=source.replace('wind = AUDIO_WEATHER_FRONTEND_MAX(AudioWeatherHost_Wind(), modifier);','volatile float comparedWind = AudioWeatherHost_Wind();\n\tif (comparedWind > modifier)\n\t\twind = AudioWeatherHost_Wind();\n\telse\n\t\twind = modifier;')
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_WEATHER_FRONTEND_'+name,source)
    return source

def project(source):
    # Register the complete C11 generators beside the script sound conversion
    import audio_world_loops_migration
    if 'AudioWeatherFrontend.c' in source:return audio_world_loops_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioScriptSounds.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_world_loops_migration.project(source[:b]+'\n'+source[a:b].replace('AudioScriptSounds.c','AudioWeatherFrontend.c')+source[b:])
