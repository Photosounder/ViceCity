"""Move complete ambience selection, height and reference-output volume into C11."""
import re
import cull_zones_migration as cull
import audio_music_clubs_migration as clubs
from audio_controls_migration import function
METHODS=['SetUpCorrectAmbienceTrack','ComputeAmbienceVol']
PROTOTYPES='''void MusicAmbience_SetUpCorrectAmbienceTrack(cMusicManager *manager);
void MusicAmbience_ComputeAmbienceVol(cMusicManager *manager, uint8_t reset, uint8_t *outVolume);
float MusicAmbience_GetHeightScale(void);
int32_t MusicAmbienceHost_Area(void);
uint8_t MusicAmbienceHost_RiotIntensity(void);
uint8_t MusicAmbienceHost_AtBeach(void);
int16_t MusicAmbienceHost_OldWeather(void);
int16_t MusicAmbienceHost_NewWeather(void);
float MusicAmbienceHost_DistanceToWater(void);
'''
HOST='''//+ rouz edit (ChatGPT)
extern "C" int32_t
MusicAmbienceHost_Area(void)
{
    // Read the current area at the original ambience selection point
    return CGame::currArea;
}
extern "C" uint8_t
MusicAmbienceHost_RiotIntensity(void)
{
    // Read the live riot intensity without caching across branches
    return CTheScripts::RiotIntensity;
}
extern "C" uint8_t
MusicAmbienceHost_AtBeach(void)
{
    // Read the existing beach flag through its boolean representation
    return CCullZones::bAtBeachForAudio;
}
extern "C" int16_t
MusicAmbienceHost_OldWeather(void)
{
    // Read the old weather type at the original condition
    return CWeather::OldWeatherType;
}
extern "C" int16_t
MusicAmbienceHost_NewWeather(void)
{
    // Read the new weather type only when the original condition needs it
    return CWeather::NewWeatherType;
}
extern "C" float
MusicAmbienceHost_DistanceToWater(void)
{
    // Read the live camera distance at each original threshold or calculation
    return TheCamera.DistanceToWater;
}
//- rouz edit (ChatGPT)
'''
HELPERS='''static float
MusicAmbience_CameraHeight(void)
{
    // Read the camera coordinates through the existing C geometry boundary
    AudioSoundPosition camera;
    AudioGeometryHost_CameraPosition(&camera);
    return camera.z;
}

static float
MusicAmbience_RiotDistanceSquared(void)
{
    // Subtract each riot coordinate before preserving the original sum of float squares
    AudioSoundPosition camera;
    float x, y, z;
    AudioGeometryHost_CameraPosition(&camera);
    x = camera.x - MusicAmbience_RiotPosition.x;
    y = camera.y - MusicAmbience_RiotPosition.y;
    z = camera.z - MusicAmbience_RiotPosition.z;
    return x*x + y*y + z*z;
}
'''

def calls(source):
    # Replace complete owner operations and pass the original output lvalue by address
    source=source.replace('MusicManager.SetUpCorrectAmbienceTrack()', 'MusicAmbience_SetUpCorrectAmbienceTrack(&MusicManager)')
    if 'cMusicManager::cMusicManager(' in source:
        source=source.replace('SetUpCorrectAmbienceTrack();','MusicAmbience_SetUpCorrectAmbienceTrack(this);')
        source=re.sub(r'(?<![\w:])ComputeAmbienceVol\((TRUE|FALSE), volume\);',r'MusicAmbience_ComputeAmbienceVol(this, \1, &volume);',source)
    return clubs.calls(source)

def logic(source):
    # Remove every complete converted body and the vector constructor before adding typed field readers
    if 'cMusicManager::ComputeAmbienceVol(' in source:
        for name in METHODS:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
        source=source.replace(function(source,'','GetHeightScale')+'\n','')
        source=source.replace('CVector vecRiotPosition(300.7f, -322.0f, 12.0f);\n','')+HOST
    # Match the existing readers after cull-zone class removal
    import weather_migration as weather
    transformed=cull.calls(HOST)
    host=next((candidate for candidate in [HOST,transformed,weather.legacy_member_spelling(transformed),weather.calls(HOST)] if candidate in source),HOST)
    if host in source:
        # Keep the new readers after adapters inserted by preceding migration stages
        source=source.replace(host,'')+host
    return clubs.logic(calls(source))

def header(source):
    # Expose the complete C operations and remove the output reference and old height declaration
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+n+r'\(',line) for n in METHODS+['GetHeightScale']))
    return clubs.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Preserve shared static state, early returns, repeated reads and numeric expression order
    import weather_migration as weather
    source=weather.legacy_member_spelling(source)
    source=source[source.index('{'):]
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('(TheCamera.GetPosition() - vecRiotPosition).MagnitudeSqr()', 'MusicAmbience_RiotDistanceSquared()')
    source=source.replace('TheCamera.GetPosition().z','MusicAmbience_CameraHeight()').replace('GetHeightScale()', 'MusicAmbience_GetHeightScale()')
    for a,b in [('CGame::currArea','MusicAmbienceHost_Area()'),('CTheScripts::RiotIntensity','MusicAmbienceHost_RiotIntensity()'),('CCullZones::bAtBeachForAudio','MusicAmbienceHost_AtBeach()'),('CWeather::OldWeatherType','MusicAmbienceHost_OldWeather()'),('CWeather::NewWeatherType','MusicAmbienceHost_NewWeather()'),('CWeather::Wind','AudioWeatherHost_Wind()'),('TheCamera.DistanceToWater','MusicAmbienceHost_DistanceToWater()')]:source=source.replace(a,b)
    source=re.sub(r'\boutVolume\b','(*outVolume)',source)
    return source.replace('Min(', 'MUSIC_AMBIENCE_MIN(').replace('SQR(', 'MUSIC_AMBIENCE_SQUARE(').replace('Sqrt(', 'sqrtf(')

def weather(source):
    # Share the exact original weather enum with C without duplicating values
    a=source.index('enum {');b=source.index('};',a)+2
    import weather_migration as controller
    return controller.calls('//+ rouz edit (ChatGPT)\n'+source[:a]+'#include "WeatherTypes.h"'+source[b:]+'\n//- rouz edit (ChatGPT)\n')

def project(source):
    # Register complete ambience operations as a C11 translation unit
    if 'MusicAmbience.c' in source:return clubs.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicReload.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    source=source[:b]+'\n'+source[a:b].replace('MusicReload.c','MusicAmbience.c')+source[b:]
    for original,new in [('..\\src\\audio\\MusicManager.h','..\\src\\audio\\MusicAmbience.h'),('..\\src\\renderer\\Weather.h','..\\src\\renderer\\WeatherTypes.h')]:
        a=source.index('    <ClInclude Include="'+original+'"');end=source.index('>',a)+1
        if source[a:end].endswith('/>'):b=end
        else:b=source.index('</ClInclude>',a)+len('</ClInclude>')
        source=source[:b]+'\n'+source[a:b].replace(original,new)+source[b:]
    return clubs.project(source)
