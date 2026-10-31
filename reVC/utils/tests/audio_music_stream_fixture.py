"""Compare full streaming operations with original shared state and volume math."""
from pathlib import Path
import re
import audio_music_stream_migration as m
import audio_music_state_migration as state
from audio_owner_init_migration import fields
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-stream-c-tests'

def normalize(s):
    # Compare all statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))

old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicStream.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
assert normalize(m.STORE) in normalize(kernel) and normalize(m.HELPERS) in normalize(kernel)
for name in m.METHODS:
    # Verify the complete transplanted service including geometry replacements and native write sequencing
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicStream_',name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
assert 'inline float Sqrt(float x) { return sqrtf(x); }' in (root/'src/math/maths.h').read_text()
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\ncMusicManager();\nuint8_t ServiceAnnouncement();\nvoid ServiceTrack(CVehicle*,CPed*);\n} cMusicManager;\n'
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','Validate.h','sampman.h','DMAudio.h','AudioGeometry.h','AudioMath.h','AudioSampleConfig.h','MusicAmbience.h','soundlist.h','../core/GameAreas.h','../renderer/WeatherTypes.h'])
prototypes='''extern "C" void MusicState_InitState(cMusicManager*);
extern "C" void MusicPositions_RecordRadioStats(cMusicManager*);
extern "C" uint32_t MusicPositions_GetTrackStartPos(cMusicManager*,uint32_t);
extern "C" void MusicAmbience_ComputeAmbienceVol(cMusicManager*,uint8_t,uint8_t*);
'''
common=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <windows.h>
#include "config.h"
@HEADERS@
struct RwV3d {float x,y,z;};
#include "../../src/math/maths.h"
#include "../../src/math/Vector.h"
using uint8=uint8_t;using uint32=uint32_t;using bool8=uint8_t;
#define TRUE 1
#define FALSE 0
#define Min(a,b) ((a)<(b)?(a):(b))
#define SQR(x) ((x)*(x))
#define debug(f,...) re3_debug("[DBG]: " f)
struct CVehicle {uint8_t m_nRadioStation;};
struct CPed {uint8_t storage;};
struct CEntity {CVector position;CVector &GetPosition(){return position;}};
struct Camera {CEntity *pTargetEntity;CVector position;CVector &GetPosition(){return position;}};
static CEntity targets[2];
static Camera TheCamera;
static AudioCameraMatrix cameraMatrix;
@MUSIC@
@PROTOTYPES@
typedef struct cAudioManager { @AUDIO@ } cAudioManager;
extern "C" uint8_t AudioMission_ShouldDuck(cAudioManager*,uint8_t);
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other,*active;
cSampleManager SampleManager;
extern "C" uint8_t g_bAnnouncementReadPosAlready;
uint8_t g_bAnnouncementReadPosAlready;
extern "C" int32_t gNumRetunePresses,gRetuneCounter;
int32_t gNumRetunePresses,gRetuneCounter;
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" uint32_t TestMusicStreamC(cMusicManager*,CVehicle*,CPed*,unsigned);
static CVehicle vehicles[2],*vehicle;
static CPed ped;
static uint8_t slow;
static uint32_t pauseTick;
static unsigned seed,pass,operation,groups,callbacks,mutation,remaining,checks,stops,positions,timers,starts,volumes;
static uint8_t lastVolume,lastPan,lastEffects;
static uint64_t digest=1469598103934665603ULL;
static void value(uint32_t v) {
    // Hash deterministic scalars independently of pointer identity
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot() {
    // Observe full owners, provider state, vehicles, world inputs and stream counters
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,&AudioManager,sizeof(AudioManager));digest=TestAircraftHash(digest,&SampleManager,sizeof(SampleManager));
    value(vehicles[0].m_nRadioStation);value(vehicles[1].m_nRadioStation);digest=TestAircraftHash(digest,targets,sizeof(targets));
    digest=TestAircraftHash(digest,&TheCamera.position,sizeof(TheCamera.position));digest=TestAircraftHash(digest,&cameraMatrix,sizeof(cameraMatrix));
    value(TheCamera.pTargetEntity==nullptr?2u:(uint32_t)(TheCamera.pTargetEntity-targets));value(slow);value(g_bAnnouncementReadPosAlready);
    value((uint32_t)gNumRetunePresses);value((uint32_t)gRetuneCounter);value(pauseTick);
    value(remaining);value(checks);value(stops);value(positions);value(timers);value(starts);value(volumes);
    value(lastVolume);value(lastPan);value(lastEffects);
}
static void event(unsigned token) {
    // Mutate later live reads after the original provider result or arguments have been captured
    value(token);snapshot();callbacks++;
    if(mutation) {
        active->m_nPlayingTrack=(seed+callbacks)%NUM_RADIOS;
        active->m_nNextTrack=(callbacks%3==0)?STREAMED_SOUND_HAVANA_CITY_AMBIENT:(seed+callbacks+1)%NUM_RADIOS;
        active->m_nFrontendTrack=(callbacks%3==0)?STREAMED_SOUND_CITY_AMBIENT:(seed+callbacks+2)%NUM_RADIOS;
        active->m_nAnnouncement=(seed+callbacks+3)%TOTAL_STREAMED_SOUNDS;
        active->m_bTrackChangeStarted=(callbacks%2)?255:0;active->m_bVerifyNextTrackStartedToPlay=(callbacks%3)?0:255;
        active->m_bIsInitialised=(callbacks%3)?255:0;active->m_nCurrentVolume=(uint8_t)callbacks;active->m_nMaxVolume=255;
        active->m_nLastTrackServiceTime=-1.0f;g_bAnnouncementReadPosAlready=(callbacks%3)?255:0;
        AudioManager.m_FrameCounter+=token;AudioManager.m_bIsInitialised=(callbacks%3)?255:0;
        for(unsigned i=0;i<MISSION_AUDIO_SLOTS;i++){AudioManager.m_nMissionAudioPlayStatus[i]=(callbacks%2)?PLAY_STATUS_PLAYING:PLAY_STATUS_STOPPED;AudioManager.m_nMissionAudioSampleIndex[i]=(callbacks%3)?0:STREAMED_SOUND_MISSION_ROK2_01;}
        TheCamera.pTargetEntity=(callbacks%3==0)?nullptr:&targets[callbacks%2];
        TheCamera.position=CVector(0,0,(float)(callbacks%5));targets[0].position=CVector((float)(callbacks%50),0,20);targets[1].position=CVector(0,0,55);
        slow=(callbacks%2)?255:0;vehicles[0].m_nRadioStation=(uint8_t)callbacks;
    }
}
extern "C" uint8_t SampleManager_IsStreamPlaying(cSampleManager *manager,uint8_t stream) {
    // Retain raw playing bytes and delayed acknowledgements
    assert(manager==&SampleManager&&stream==0);uint8_t result=remaining?((seed%2)?255:1):0;checks++;value(result);event(1);return result;
}
extern "C" void SampleManager_StopStreamedFile(cSampleManager *manager,uint8_t stream) {
    // Acknowledge a stop only after the captured provider callback
    assert(manager==&SampleManager&&stream==0);stops++;event(2);if(remaining)remaining--;
}
extern "C" int32_t SampleManager_GetStreamedFilePosition(cSampleManager *manager,uint8_t stream) {
    // Return signed positions before a callback changes the destination track
    assert(manager==&SampleManager&&stream==0);int32_t result=(seed%3==0)?-1:((seed%3==1)?INT_MIN:(int32_t)(seed*127u));positions++;value((uint32_t)result);event(3);return result;
}
extern "C" void SampleManager_SetStreamedVolumeAndPan(cSampleManager *manager,uint8_t volume,uint8_t pan,uint8_t effects,uint8_t stream) {
    // Observe final narrowed volume, panning and effects arguments before latency and ramp writes
    assert(manager==&SampleManager&&stream==0);lastVolume=volume;lastPan=pan;lastEffects=effects;volumes++;value(volume);value(pan);value(effects);event(4);
}
extern "C" void SampleManager_SetStreamedFileLoopFlag(cSampleManager *manager,uint8_t flag,uint8_t stream) {
    // Observe the loop byte before the following live start-track read
    assert(manager==&SampleManager&&stream==0);value(flag);event(5);
}
extern "C" uint8_t SampleManager_StartStreamedFile(cSampleManager *manager,uint32_t track,uint32_t position,uint8_t stream) {
    // Preserve captured start arguments while delaying stream acknowledgement
    assert(manager==&SampleManager&&stream==0);value(track);value(position);starts++;event(6);remaining=(seed%2)?0:1;return (seed%3)?255:0;
}
extern "C" uint32_t MusicInitHost_PauseTime(void) {
    // Return each captured wrapping timer before later live owner and track reads
    uint32_t result=pauseTick;pauseTick+=1001u;timers++;value(result);event(7);return result;
}
extern "C" uint8_t MusicStreamHost_SlowMotion(void) {
    // Preserve the boolean slow-motion result before camera and owner inputs change
    uint8_t result=slow!=0;value(result);event(8);return result;
}
extern "C" void re3_debug(const char *format,...) {
    // Observe the original debug prefix and text at the same cleanup branch
    assert(strcmp(format,"[DBG]: m_nPlayingTrack == NO_TRACK, yet track playing - tidying up\n")==0);event(9);
}
extern "C" void AudioEntities_PlayOneShot(tAudioEntity *entities,cAudioScriptObjectManager *scripts,uint8_t initialized,int32_t index,uint16_t sound,float volume) {
    // Check the original owner storage and hurricane sound arguments before callback changes
    assert(entities==AudioManager.m_asAudioEntities&&scripts==&AudioManager.m_sAudioScriptObjectManager&&sound==SOUND_FRONTEND_HURRICANE&&volume==0.0f);value(initialized);value((uint32_t)index);value(sound);event(10);
}
extern "C" uint8_t *MusicRadioHost_Station(CVehicle *v) {assert(v==&vehicles[0]||v==&vehicles[1]);return &v->m_nRadioStation;}
extern "C" CEntity *MusicStreamHost_Target(void) {return TheCamera.pTargetEntity;}
extern "C" void MusicStreamHost_TargetPosition(CEntity *target,AudioSoundPosition *p) {assert(target);const CVector &v=target->GetPosition();p->x=v.x;p->y=v.y;p->z=v.z;}
extern "C" void AudioGeometryHost_CameraPosition(AudioSoundPosition *p) {const CVector &v=TheCamera.GetPosition();p->x=v.x;p->y=v.y;p->z=v.z;}
extern "C" void AudioGeometryHost_CameraMatrix(AudioCameraMatrix *m) {*m=cameraMatrix;m->px=TheCamera.position.x;m->py=TheCamera.position.y;m->pz=TheCamera.position.z;}
extern "C" int32_t MusicAmbienceHost_Area(void) {return AREA_MAIN_MAP;}
extern "C" uint8_t MusicAmbienceHost_RiotIntensity(void) {return 0;}
extern "C" uint8_t MusicAmbienceHost_AtBeach(void) {return 0;}
extern "C" int16_t MusicAmbienceHost_OldWeather(void) {return WEATHER_SUNNY;}
extern "C" int16_t MusicAmbienceHost_NewWeather(void) {return WEATHER_CLOUDY;}
extern "C" float MusicAmbienceHost_DistanceToWater(void) {return 150.0f;}
extern "C" float AudioWeatherHost_Wind(void) {return 0.0f;}
struct CTimer {static uint32_t GetTimeInMillisecondsPauseMode(){return MusicInitHost_PauseTime();}static bool GetIsSlowMotionActive(){return MusicStreamHost_SlowMotion()!=0;}};
static void AudioGeometry_TranslateVector(const CVector *position,CVector *translated) {
    // Use the same existing C transform as the original game vector adapter
    AudioSoundPosition p={position->x,position->y,position->z},result;AudioGeometry_Translate(&p,&result);*translated=CVector(result.x,result.y,result.z);
}
@METHODS@
static uint32_t run(unsigned op) {
    // Invoke exact preserved methods or their separately compiled C implementations
    @RUN@
}
int main() {
    // Keep failures quiet and independently verify unweighted vertical distance and null-target behavior
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    cameraMatrix={1,0,0,0,1,0,0,0,1,0,0,0};active=&MusicManager;vehicle=&vehicles[0];mutation=0;
    memset(active,0,sizeof(*active));memset(&AudioManager,0,sizeof(AudioManager));AudioManager.m_bIsInitialised=255;
    active->m_nPlayingTrack=active->m_nFrontendTrack=0;slow=255;TheCamera.pTargetEntity=&targets[0];TheCamera.position=CVector(0,0,0);
    targets[0].position=CVector(0,0,55);active->m_nCurrentVolume=50;active->m_nMaxVolume=100;active->nFramesSinceCutsceneEnded=-1;
    run(1);assert(lastVolume==0&&lastPan==63&&active->m_nCurrentVolume==56);
    targets[0].position=CVector(0,0,10);active->m_nCurrentVolume=50;run(1);assert(lastVolume==50&&lastPan==63);
    targets[0].position=CVector(0,0,20);active->m_nCurrentVolume=50;AudioManager.m_nMissionAudioPlayStatus[0]=PLAY_STATUS_PLAYING;run(1);assert(lastVolume==9);
    TheCamera.pTargetEntity=nullptr;active->m_nCurrentVolume=50;unsigned before=volumes;run(1);assert(volumes==before&&active->m_nCurrentVolume==56);
    const uint32_t tracks[]={0,NUM_RADIOS-1,STREAMED_SOUND_RADIO_MP3_PLAYER,STREAMED_SOUND_CITY_AMBIENT,STREAMED_SOUND_HAVANA_CITY_AMBIENT,STREAMED_SOUND_HAVANA_BEACH_AMBIENT,STREAMED_SOUND_MALL_AMBIENT,STREAMED_SOUND_AMBSIL_AMBIENT};
    const float distances[]={0.0f,9.99f,10.0f,10.01f,20.0f,54.99f,55.0f,55.01f,100.0f};
    for(seed=0;seed<2048;seed++)for(pass=0;pass<3;pass++)for(operation=0;operation<2;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));memset(&AudioManager,0,sizeof(AudioManager));memset(&SampleManager,0,sizeof(SampleManager));
        active=(seed+pass)%2?&other:&MusicManager;vehicle=(seed%3==0)?nullptr:&vehicles[seed%2];ped.storage=(uint8_t)seed;
        active->m_bIsInitialised=(seed%3==0)?0:255;active->m_bDisabled=255;
        active->m_nPlayingTrack=(seed%9==0)?NO_TRACK:tracks[(seed+pass)%8];active->m_nFrontendTrack=tracks[(seed/8+pass)%8];active->m_nNextTrack=tracks[(seed/64+pass)%8];
        active->m_bTrackChangeStarted=(seed%2)?255:0;active->m_bVerifyNextTrackStartedToPlay=(seed%3)?0:255;
        active->m_bAnnouncementInProgress=(seed%4==0)?255:0;active->m_nAnnouncement=seed%TOTAL_STREAMED_SOUNDS;
        active->m_nVolumeLatency=(uint8_t)(seed%12);active->m_nCurrentVolume=(uint8_t)seed;active->m_nMaxVolume=(uint8_t)(seed+7);
        active->nFramesSinceCutsceneEnded=(int8_t)seed;active->m_nLastTrackServiceTime=(seed%2)?-1.0f:4294967296.0f;
        for(unsigned i=0;i<TOTAL_STREAMED_SOUNDS;i++){active->m_aTracks[i].m_nLength=(seed%2)?17:UINT32_MAX;active->m_aTracks[i].m_nPosition=seed*123u+i;active->m_aTracks[i].m_nLastPosCheckTimer=seed*77u;}
        for(unsigned i=0;i<NUM_RADIOS;i++)active->aListenTimeArray[i]=16777216.0f+(float)i;
        AudioManager.m_FrameCounter=(pass==2)?0:seed%8;AudioManager.m_bIsInitialised=(seed%3)?255:0;AudioManager.m_nFrontEndEntity=(int32_t)seed;
        for(unsigned i=0;i<MISSION_AUDIO_SLOTS;i++){AudioManager.m_nMissionAudioPlayStatus[i]=(seed%3==i)?PLAY_STATUS_PLAYING:PLAY_STATUS_STOPPED;AudioManager.m_nMissionAudioSampleIndex[i]=(seed%5)?0:STREAMED_SOUND_MISSION_ROK2_01;}
        gRetuneCounter=(pass==2)?0:((seed%4==0)?20:0);gNumRetunePresses=(int32_t)(seed%11);active->m_bSetNextStation=(seed%7==0)?255:0;
        g_bAnnouncementReadPosAlready=(seed%2)?255:0;vehicles[0].m_nRadioStation=(uint8_t)seed;vehicles[1].m_nRadioStation=(uint8_t)(seed+1);
        slow=(seed%2)?255:0;TheCamera.position=CVector(0,0,0);TheCamera.pTargetEntity=(seed%5==0)?nullptr:&targets[seed%2];
        targets[0].position=CVector(0,0,distances[(seed/2)%9]);targets[1].position=CVector(distances[(seed/3)%9],0,0);
        remaining=(seed+pass)%5;checks=stops=positions=timers=starts=volumes=0;lastVolume=lastPan=lastEffects=0;
        mutation=pass==1;pauseTick=seed*1234567u;snapshot();uint32_t result=run(operation);value(result);snapshot();groups++;
        if(pass==2){result=run(operation);value(result);snapshot();}
        assert(checks<9&&stops<5);
        if(operation==0)assert(result==1);
    }
    printf("groups=%u callbacks=%u hash=%016llx\n",groups,callbacks,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Link real C helpers into both models while independently compiling the preserved or converted services
    if variant=='before':record=before_record;methods=state.CTOR+'\n'+'\n'.join(m.function(old,'cMusicManager::',n) for n in m.METHODS);dispatch='if(op==0)return active->ServiceAnnouncement();active->ServiceTrack(vehicle,&ped);return 0;'
    else:record='#include "MusicManager.h"';methods=state.CTOR;dispatch='return TestMusicStreamC(active,vehicle,&ped,op);'
    (out/(variant+'.cpp')).write_text(common.replace('@HEADERS@',headers).replace('@MUSIC@',record).replace('@PROTOTYPES@',prototypes).replace('@AUDIO@',fields((root/'src/audio/AudioManager.h').read_text())).replace('@METHODS@',methods).replace('@RUN@',dispatch),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
uint32_t TestMusicStreamC(cMusicManager *manager,CVehicle *vehicle,CPed *ped,unsigned op)
{
    // Enter each complete streaming service with the original captured owner and vehicle
    if(op==0)return MusicStream_ServiceAnnouncement(manager);
    MusicStream_ServiceTrack(manager,vehicle,ped);return 0;
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-radio-c-tests'/(shape+'.h')).read_bytes())
print('Two complete stream services, shared flags, unweighted geometry, three typed readers, unchanged records and all caller/project edits audited')
