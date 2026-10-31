"""Compare complete mode/frontend operations with original provider polling and live state."""
from pathlib import Path
import re
import audio_music_modes_migration as m
import audio_music_state_migration as state
from audio_owner_init_migration import fields
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-modes-c-tests'
def normalize(s):
    # Ignore comments and formatting while comparing every original statement
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicModes.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('src/audio/DMAudio.cpp',m.logic),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
assert normalize(m.STORE) in normalize(kernel)
for name in m.METHODS:
    # Verify the complete transplanted body including polling and conditional radio pause
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicModes_',name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
facade=m.function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','ChangeMusicMode')
assert normalize(m.calls(facade).replace('uint8 ','uint8_t '))==normalize(m.function(kernel,'DMAudio_','ChangeMusicMode'))
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\ncMusicManager();\nvoid ChangeMusicMode(uint8_t);\nvoid ServiceFrontEndMode();\n} cMusicManager;\n'
prototypes='''extern "C" void MusicState_InitState(cMusicManager*);
extern "C" uint8_t MusicState_IsInitialised(const cMusicManager*);
extern "C" uint32_t MusicPositions_GetTrackStartPos(cMusicManager*,uint32_t);
extern "C" void MusicPositions_RecordRadioStats(cMusicManager*);
'''
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','Validate.h','sampman.h','DMAudio.h'])
common=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include "config.h"
@HEADERS@
using uint8=uint8_t;using uint32=uint32_t;using bool8=uint8_t;
#define TRUE 1
#define FALSE 0
#define Min(a,b) ((a) < (b) ? (a) : (b))
@MUSIC@
@PROTOTYPES@
struct cAudioManager {
@AUDIO@
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other,*active;
cSampleManager SampleManager;
extern "C" uint8_t g_bAnnouncementReadPosAlready;
uint8_t g_bAnnouncementReadPosAlready;
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" void TestMusicModesC(cMusicManager*,unsigned,uint8_t);
static uint64_t digest=1469598103934665603ULL;
static unsigned seed,pass,operation,groups,callbacks,mutation,remaining,checks,stops,positions,timers,starts;
static uint8_t argument;
static uint32_t pauseTick;
static void value(uint32_t v) {
    // Observe deterministic scalars without pointer identity
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot() {
    // Observe complete owners, provider storage, announcement flag and loop counters
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,&AudioManager,sizeof(AudioManager));digest=TestAircraftHash(digest,&SampleManager,sizeof(SampleManager));
    value(g_bAnnouncementReadPosAlready);value(remaining);value(checks);value(stops);value(positions);value(timers);value(starts);value(argument);
}
static void event(unsigned token) {
    // Observe each provider boundary before changes that affect subsequent reads and destinations
    value(token);snapshot();callbacks++;
    if(mutation) {
        active->m_bIsInitialised=(callbacks%3==0)?0:255;
        active->m_nPlayingTrack=(seed+callbacks)%NUM_RADIOS;
        active->m_nNextTrack=(seed+callbacks+1)%NUM_RADIOS;
        active->m_nFrontendTrack=(seed+callbacks+2)%NUM_RADIOS;
        active->m_nUpcomingMusicMode=(uint8_t)(callbacks%6);
        active->m_nNextLoopFlag=(callbacks%2)?255:0;active->m_bTrackChangeStarted=(callbacks%2)?255:0;
        active->m_nCurrentVolume=(uint8_t)callbacks;active->m_nMaxVolume=255;
        active->m_nLastTrackServiceTime=-1.0f;AudioManager.m_bIsPaused=(callbacks%2)?255:0;
        AudioManager.m_FrameCounter+=token;argument^=255;
        g_bAnnouncementReadPosAlready=(uint8_t)callbacks;
    }
}
extern "C" uint8_t SampleManager_IsStreamPlaying(cSampleManager *manager,uint8_t stream) {
    // Capture the raw stream byte before changing later live gates
    assert(manager==&SampleManager&&stream==0);uint8_t result=remaining?((seed%2)?1:255):0;
    checks++;value(result);event(1);return result;
}
extern "C" void SampleManager_StopStreamedFile(cSampleManager *manager,uint8_t stream) {
    // Acknowledge stops after a bounded number of original repeated polls
    assert(manager==&SampleManager&&stream==0);stops++;event(2);if(remaining)remaining--;
}
extern "C" int32_t SampleManager_GetStreamedFilePosition(cSampleManager *manager,uint8_t stream) {
    // Return signed positions while allowing the original destination track to change during the query
    assert(manager==&SampleManager&&stream==0);int32_t result=(seed%3==0)?-1:((seed%3==1)?INT_MIN:(int32_t)(seed*127u));
    positions++;value((uint32_t)result);event(3);return result;
}
extern "C" void SampleManager_SetStreamedVolumeAndPan(cSampleManager *manager,uint8_t volume,uint8_t pan,uint8_t effects,uint8_t stream) {
    // Observe captured volume and fixed pan/effects/stream arguments before later state writes
    assert(manager==&SampleManager&&pan==63&&effects==0&&stream==0);value(volume);event(4);
}
extern "C" void SampleManager_SetStreamedFileLoopFlag(cSampleManager *manager,uint8_t flag,uint8_t stream) {
    // Observe the loop byte before track and position capture at the following start
    assert(manager==&SampleManager&&stream==0);value(flag);event(5);
}
extern "C" uint8_t SampleManager_StartStreamedFile(cSampleManager *manager,uint32_t track,uint32_t position,uint8_t stream) {
    // Preserve captured start arguments while delaying backend start acknowledgement
    assert(manager==&SampleManager&&stream==0);value(track);value(position);starts++;event(6);
    remaining=(seed%2)?0:1;return (seed%3)?255:0;
}
static uint32_t pauseTime() {
    // Capture each timer value before callback changes to destination track and owner state
    uint32_t result=pauseTick;pauseTick+=1001u;timers++;value(result);event(7);return result;
}
@TIMER@
@METHODS@
static void run(unsigned op,uint8_t mode) {
    // Invoke exact preserved methods/facade or their separately compiled C implementations
    @RUN@
}
int main() {
    // Keep failures quiet and exercise gates, all mode bytes, delayed provider acknowledgements and shared state
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    const uint32_t tracks[]={0,NUM_RADIOS-1,STREAMED_SOUND_RADIO_MP3_PLAYER,STREAMED_SOUND_MISSION_COMPLETED,STREAMED_SOUND_CITY_AMBIENT,NO_TRACK};
    for(seed=0;seed<1024;seed++)for(pass=0;pass<3;pass++)for(operation=0;operation<3;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));memset(&AudioManager,0,sizeof(AudioManager));memset(&SampleManager,0,sizeof(SampleManager));
        active=operation==2?&MusicManager:((seed+pass)%2?&other:&MusicManager);
        active->m_bIsInitialised=(pass==2)?1:((seed%3==0)?0:255);active->m_bDisabled=255;
        active->m_nPlayingTrack=tracks[(seed+pass)%6];active->m_nFrontendTrack=tracks[(seed/6+pass)%6];active->m_nNextTrack=tracks[(seed/36+pass)%6];
        active->m_bTrackChangeStarted=(seed%2)?255:0;active->m_bVerifyNextTrackStartedToPlay=(seed%3)?0:255;
        active->m_bAnnouncementInProgress=(seed%4==0)?255:0;active->m_nVolumeLatency=(uint8_t)(seed%5);
        active->m_nCurrentVolume=(uint8_t)seed;active->m_nMaxVolume=(uint8_t)(seed+7);active->m_FrontendLoopFlag=(seed%2)?255:0;
        active->m_nLastTrackServiceTime=(seed%2)?-1.0f:4294967296.0f;
        for(unsigned i=0;i<TOTAL_STREAMED_SOUNDS;i++) {
            active->m_aTracks[i].m_nLength=(seed%2)?17:UINT32_MAX;active->m_aTracks[i].m_nPosition=seed*123u+i;
            active->m_aTracks[i].m_nLastPosCheckTimer=seed*77u;
        }
        for(unsigned i=0;i<NUM_RADIOS;i++)active->aListenTimeArray[i]=16777216.0f+(float)i;
        AudioManager.m_FrameCounter=(pass==2)?0:(uint32_t)(seed%8);AudioManager.m_bIsPaused=(seed%2)?255:0;
        g_bAnnouncementReadPosAlready=255;remaining=(seed+pass)%5;checks=stops=positions=timers=starts=0;
        mutation=pass==1;pauseTick=seed*1234567u;argument=(uint8_t)seed;
        unsigned beforeCallbacks=callbacks;uint8_t initialised=active->m_bIsInitialised;
        snapshot();run(operation,argument);snapshot();groups++;
        assert(checks<12&&stops<7);
        if(operation!=1&&!initialised)assert(callbacks==beforeCallbacks);
    }
    printf("groups=%u callbacks=%u hash=%016llx\n",groups,callbacks,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Link actual state/position C operations while independently compiling the complete original or C mode logic
    if variant=='before':
        record=before_record;methods=state.CTOR+'\n'+'\n'.join(m.function(old,'cMusicManager::',n) for n in m.METHODS)+'\n'+facade
        timer='extern "C" uint32_t MusicInitHost_PauseTime(void){\n// Observe the same timer boundary for original methods and actual C position helpers\nreturn pauseTime();\n}\nstruct CTimer {static uint32_t GetTimeInMillisecondsPauseMode(){return MusicInitHost_PauseTime();}};'
        dispatch='switch(op){case 0:active->ChangeMusicMode(mode);break;case 1:active->ServiceFrontEndMode();break;case 2:DMAudio_ChangeMusicMode(mode);break;}'
    else:
        record='#include "MusicManager.h"';methods=state.CTOR;timer='extern "C" uint32_t MusicInitHost_PauseTime(void){return pauseTime();}'
        dispatch='TestMusicModesC(active,op,mode);'
    (out/(variant+'.cpp')).write_text(common.replace('@HEADERS@',headers).replace('@MUSIC@',record).replace('@PROTOTYPES@',prototypes).replace('@AUDIO@',fields((root/'src/audio/AudioManager.h').read_text())).replace('@TIMER@',timer).replace('@METHODS@',methods).replace('@RUN@',dispatch),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
#include "DMAudio.h"
void TestMusicModesC(cMusicManager *manager,unsigned op,uint8_t mode)
{
    // Enter both complete C mode operations and the public mode facade
    switch(op){case 0:MusicModes_ChangeMusicMode(manager,mode);break;
    case 1:MusicModes_ServiceFrontEndMode(manager);break;
    case 2:DMAudio_ChangeMusicMode(mode);break;}
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-clubs-c-tests'/(shape+'.h')).read_bytes())
(out/'no-pause-radio.h').write_text('#include "config.h"\n#define AUDIO_OAL\n#undef PAUSE_RADIO_IN_FRONTEND\n',newline='\n')
print('Complete mode/frontend bodies, public facade, shared stats flag, conditional radio pause and all owner/header/project edits audited')
