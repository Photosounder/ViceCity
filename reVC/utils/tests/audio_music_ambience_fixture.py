"""Compare complete ambience C operations to preserved C++ bodies and actual vector math."""
from pathlib import Path
import re
import audio_music_ambience_migration as m
import audio_music_state_migration as state
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-ambience-c-tests'
def normalize(s):
    # Compare all statements while ignoring comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();weather=(out/'before/src/renderer/Weather.h').read_text();kernel=(root/'src/audio/MusicAmbience.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('src/renderer/Weather.h',m.weather),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
assert normalize(m.cull.calls(m.HOST)) in normalize((root/'src/audio/MusicManager.cpp').read_text())
assert normalize(m.HELPERS) in normalize(kernel)
assert 'AudioSoundPosition MusicAmbience_RiotPosition = {300.7f, -322.0f, 12.0f};' in kernel
for prefix,name in [('','GetHeightScale')]+[('cMusicManager::',n) for n in m.METHODS]:
    # Verify complete body conversion including the reference output and shared static ramp
    expected=m.body(m.function(old,prefix,name));actual=m.function(kernel,'MusicAmbience_',name)
    assert normalize(expected)==normalize(actual[actual.index('{'):]),name
a=weather.index('enum {');b=weather.index('};',a)+2
assert normalize(weather[a:b]) in normalize((root/'src/renderer/WeatherTypes.h').read_text())
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\nvoid SetUpCorrectAmbienceTrack();\nvoid ComputeAmbienceVol(uint8_t,uint8_t&);\n} cMusicManager;\n'
common=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <windows.h>
#include "config.h"
#include "audio_enums.h"
#include "Validate.h"
#include "AudioGeometry.h"
#include "AudioSampleConfig.h"
#include "../../src/core/GameAreas.h"
#include "../../src/renderer/WeatherTypes.h"
struct RwV3d {float x,y,z;};
#include "../../src/math/maths.h"
#include "../../src/math/Vector.h"
using bool8=uint8_t;using uint8=uint8_t;
#define Min(a,b) ((a) < (b) ? (a) : (b))
#define SQR(x) ((x)*(x))
@RECORD@
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other;
@RIOT@
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" uint32_t TestMusicAmbienceC(cMusicManager*,unsigned,uint8_t,uint8_t*);
static cMusicManager *active;
static AudioSoundPosition cameraPosition;
static float water,wind;
static int32_t area;
static int16_t oldWeather,newWeather;
static uint8_t riot,beach,output;
static unsigned seed,pass,operation,groups,callbacks,mutate;
static uint64_t digest=1469598103934665603ULL;
static uint8_t *target;
static void value(uint32_t v) {
    // Observe scalar values independently of pointer identity
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot() {
    // Observe complete owners, output storage, world inputs and writable riot coordinates
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,&cameraPosition,sizeof(cameraPosition));digest=TestAircraftHash(digest,&RIOT_POSITION,sizeof(RIOT_POSITION));
    digest=TestAircraftHash(digest,&water,sizeof(water));digest=TestAircraftHash(digest,&wind,sizeof(wind));
    value((uint32_t)area);value((uint32_t)oldWeather);value((uint32_t)newWeather);value(riot);value(beach);value(output);value(*target);
}
static void observe(unsigned id,uint32_t captured) {
    // Observe captured reads before changing later inputs and the pointed-to output byte
    value(id);value(captured);snapshot();callbacks++;
    if(mutate) {
        active->m_nPlayingTrack=(callbacks%3==0)?STREAMED_SOUND_HAVANA_CITY_AMBIENT:STREAMED_SOUND_CITY_AMBIENT;
        active->m_nFrontendTrack=callbacks;active->m_bIsInitialised=(callbacks%2)?255:0;
        *target=(uint8_t)(callbacks+seed);
        riot=(uint8_t)((seed+callbacks)%128);beach=(uint8_t)(callbacks%2);
        oldWeather=(callbacks%3==0)?WEATHER_HURRICANE:WEATHER_SUNNY;newWeather=(callbacks%5==0)?WEATHER_HURRICANE:WEATHER_CLOUDY;
        cameraPosition.z=(callbacks%2)?35.0f:20.0f;
        RIOT_POSITION.x+=0.25f;
    }
}
static uint32_t floatBits(float v) {
    // Capture float bit patterns without changing their numeric value
    uint32_t bits;memcpy(&bits,&v,4);return bits;
}
extern "C" int32_t MusicAmbienceHost_Area(void) {int32_t v=area;observe(1,(uint32_t)v);return v;}
extern "C" uint8_t MusicAmbienceHost_RiotIntensity(void) {uint8_t v=riot;observe(2,v);return v;}
extern "C" uint8_t MusicAmbienceHost_AtBeach(void) {uint8_t v=beach;observe(3,v);return v;}
extern "C" int16_t MusicAmbienceHost_OldWeather(void) {int16_t v=oldWeather;observe(4,(uint32_t)v);return v;}
extern "C" int16_t MusicAmbienceHost_NewWeather(void) {int16_t v=newWeather;observe(5,(uint32_t)v);return v;}
extern "C" float MusicAmbienceHost_DistanceToWater(void) {float v=water;observe(6,floatBits(v));return v;}
extern "C" float AudioWeatherHost_Wind(void) {float v=wind;observe(7,floatBits(v));return v;}
static CVector cameraRead() {
    // Return the same captured coordinates as the C geometry copy boundary
    CVector v(cameraPosition.x,cameraPosition.y,cameraPosition.z);observe(8,floatBits(v.z));return v;
}
extern "C" void AudioGeometryHost_CameraPosition(AudioSoundPosition *position) {
    // Copy the complete captured camera position without assuming the game vector layout
    CVector v=cameraRead();position->x=v.x;position->y=v.y;position->z=v.z;
}
template<typename T,T(*Read)(void)>struct Field {operator T()const{return Read();}};
struct CGame {static Field<int32_t,MusicAmbienceHost_Area> currArea;};
Field<int32_t,MusicAmbienceHost_Area> CGame::currArea;
struct CTheScripts {static Field<uint8_t,MusicAmbienceHost_RiotIntensity> RiotIntensity;};
Field<uint8_t,MusicAmbienceHost_RiotIntensity> CTheScripts::RiotIntensity;
static bool readBeach(){return MusicAmbienceHost_AtBeach()!=0;}
struct CCullZones {static Field<bool,readBeach> bAtBeachForAudio;};
Field<bool,readBeach> CCullZones::bAtBeachForAudio;
struct CWeather {static Field<int16_t,MusicAmbienceHost_OldWeather> OldWeatherType;static Field<int16_t,MusicAmbienceHost_NewWeather> NewWeatherType;static Field<float,AudioWeatherHost_Wind> Wind;};
Field<int16_t,MusicAmbienceHost_OldWeather> CWeather::OldWeatherType;Field<int16_t,MusicAmbienceHost_NewWeather> CWeather::NewWeatherType;Field<float,AudioWeatherHost_Wind> CWeather::Wind;
struct Camera {Field<float,MusicAmbienceHost_DistanceToWater> DistanceToWater;CVector GetPosition(){return cameraRead();}};
static Camera TheCamera;
@METHODS@
static uint32_t run(unsigned op,uint8_t reset) {
    // Enter the complete original operation or its C caller with identical output storage
    @RUN@
}
int main() {
    // Keep failures quiet and verify the shared ramp and riot return before reset
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    active=&MusicManager;target=&output;mutate=0;riot=0;wind=0;oldWeather=newWeather=WEATHER_SUNNY;water=150;cameraPosition={0,0,0};
    active->m_nPlayingTrack=STREAMED_SOUND_CITY_AMBIENT;
    run(2,1);assert(output==0);run(2,0);assert(output==1);
    active=&other;active->m_nPlayingTrack=STREAMED_SOUND_CITY_AMBIENT;run(2,0);assert(output==2);
    active->m_nPlayingTrack=STREAMED_SOUND_HAVANA_CITY_AMBIENT;run(2,0);assert(output==22);
    RIOT_POSITION.x=0;RIOT_POSITION.y=0;RIOT_POSITION.z=0;riot=127;run(2,255);assert(output==127);
    riot=0;active->m_nPlayingTrack=STREAMED_SOUND_CITY_AMBIENT;run(2,0);assert(output==23);run(2,1);assert(output==0);
    const float heights[]={0,19.999f,20,20.001f,35,49.999f,50,50.001f,70};
    const float waters[]={0,39.999f,40,40.001f,90,90.001f,139.999f,140,140.001f,150};
    const float winds[]={0,0.3f,0.9f,1,1.1f,2};
    const float distances[]={0,19.999f,20,20.001f,64.999f,65,65.001f,99.999f,100,100.001f};
    const uint32_t tracks[]={STREAMED_SOUND_CITY_AMBIENT,STREAMED_SOUND_HAVANA_CITY_AMBIENT,STREAMED_SOUND_HAVANA_BEACH_AMBIENT,STREAMED_SOUND_MALL_AMBIENT,STREAMED_SOUND_AMBSIL_AMBIENT,STREAMED_SOUND_AMBSIL_AMBIENT+1,UINT32_MAX};
    for(seed=0;seed<1024;seed++)for(pass=0;pass<3;pass++)for(unsigned owner=0;owner<2;owner++)for(operation=0;operation<3;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));active=owner?&other:&MusicManager;
        output=(uint8_t)seed;target=(pass==0)?&output:((pass==1)?&active->m_nCurrentVolume:&active->m_bDisabled);
        active->m_nPlayingTrack=tracks[seed%7];active->m_bIsInitialised=(seed%3==0)?0:((seed%3==1)?1:255);
        cameraPosition={distances[seed%10],0,heights[(seed/10)%9]};
        RIOT_POSITION.x=0;RIOT_POSITION.y=0;RIOT_POSITION.z=cameraPosition.z;
        water=waters[(seed/3)%10];wind=winds[(seed/7)%6];area=(int32_t)(seed%23)-2;
        oldWeather=(seed%2)?WEATHER_HURRICANE:WEATHER_SUNNY;newWeather=(seed%3)?WEATHER_CLOUDY:WEATHER_HURRICANE;
        riot=(seed%3)?(uint8_t)(seed%128):0;beach=(uint8_t)(seed%2);mutate=seed%2;
        snapshot();uint32_t result=run(operation,(seed%3==0)?0:((seed%3==1)?1:255));value(result);snapshot();groups++;
    }
    printf("groups=%u callbacks=%u hash=%016llx\n",groups,callbacks,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Compile exact preserved operations with typed read observers and actual original vector arithmetic
    if variant=='before':
        record=before_record;riot_decl='CVector vecRiotPosition(300.7f,-322.0f,12.0f);\n#define RIOT_POSITION vecRiotPosition'
        methods=m.function(old,'','GetHeightScale')+'\n'+'\n'.join(m.function(old,'cMusicManager::',n) for n in m.METHODS)
        dispatch='switch(op){case 0:active->SetUpCorrectAmbienceTrack();return 0;case 1:return floatBits(GetHeightScale());case 2:active->ComputeAmbienceVol(reset,*target);return *target;}return 0;'
    else:
        record='#include "MusicManager.h"\n#include "MusicAmbience.h"';riot_decl='#define RIOT_POSITION MusicAmbience_RiotPosition'
        methods='cMusicManager::cMusicManager(){memset(this,0,sizeof(*this));}'
        dispatch='return TestMusicAmbienceC(active,op,reset,target);'
    (out/(variant+'.cpp')).write_text(common.replace('@RECORD@',record).replace('@RIOT@',riot_decl).replace('@METHODS@',methods).replace('@RUN@',dispatch),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
#include <string.h>
uint32_t TestMusicAmbienceC(cMusicManager *manager,unsigned op,uint8_t reset,uint8_t *target)
{
    // Enter complete C selection, height and pointer-output volume operations
    switch(op){case 0:MusicAmbience_SetUpCorrectAmbienceTrack(manager);return 0;
    case 1:{float v=MusicAmbience_GetHeightScale();uint32_t bits;memcpy(&bits,&v,4);return bits;}
    case 2:MusicAmbience_ComputeAmbienceVol(manager,reset,target);return *target;}
    return 0;
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-reload-c-tests'/(shape+'.h')).read_bytes())
print('Three complete ambience bodies, reference-to-pointer output, shared ramp, scalar vector math, six typed readers, shared weather enum and all source/project edits audited')
