"""Audit complete club operations and compare original/C owner state and timer observations."""
from pathlib import Path
import re
import audio_music_clubs_migration as m
import audio_music_state_migration as state
from audio_owner_init_migration import fields
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-clubs-c-tests'
def normalize(s):
    # Ignore comments and whitespace while comparing complete statements
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();audioh=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/MusicClubs.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('src/audio/AudioLogic.cpp',m.logic),('src/audio/AudioManager.h',m.header),('src/audio/AudioScriptSounds.c',m.script),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
assert normalize(fields(audioh))==normalize(fields((root/'src/audio/AudioManager.h').read_text()))
for name in m.METHODS:
    # Verify each complete operation against its exact preserved member body
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicClubs_',name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\nvoid SetMalibuClubTrackPos(uint8_t);\nvoid SetStripClubTrackPos(uint8_t);\n} cMusicManager;\nextern "C" uint8_t MusicState_IsInitialised(const cMusicManager*);\nextern "C" uint8_t MusicState_GetMusicMode(const cMusicManager*);\nextern "C" uint32_t MusicState_GetCurrentTrack(const cMusicManager*);\n'
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','Validate.h'])
common=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include "config.h"
#include "soundlist.h"
@HEADERS@
using uint8=uint8_t;
@MUSIC@
struct cAudioManager {
@AUDIO@
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other,*active;
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" void TestMusicClubsC(cMusicManager*,unsigned,uint8_t);
static uint64_t digest=1469598103934665603ULL;
static unsigned seed,operation,groups,callbacks,mutate,reads;
static uint8_t argument;
static uint32_t pauseTick;
static void value(uint32_t v) {
    // Observe scalar values independently of pointer addresses
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot() {
    // Observe both complete music owners and the complete audio owner including retained bytes
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,&AudioManager,sizeof(AudioManager));value(argument);value(reads);
}
static uint32_t pauseTime() {
    // Capture the pause timer after position writes and change state before its final assignment
    uint32_t result=pauseTick;value(result);snapshot();callbacks++;reads++;
    if(mutate) {
        active->m_bIsInitialised=255;active->m_nNextTrack=STREAMED_SOUND_MALIBU_AMBIENT;active->m_nPlayingTrack=STREAMED_SOUND_STRIPCLUB_AMBIENT;
        active->m_aTracks[STREAMED_SOUND_MALIBU_AMBIENT].m_nPosition=UINT32_MAX;
        active->m_aTracks[STREAMED_SOUND_STRIPCLUB_AMBIENT].m_nPosition=0x80000000u;
        active->m_aTracks[STREAMED_SOUND_MALIBU_AMBIENT].m_nLastPosCheckTimer=0;
        active->m_aTracks[STREAMED_SOUND_STRIPCLUB_AMBIENT].m_nLastPosCheckTimer=1;
        AudioManager.m_anRandomTable[0]=INT_MIN;argument^=255;pauseTick+=0x80000001u;
    }
    return result;
}
@TIMER@
@METHODS@
static void run(unsigned op,uint8_t arg) {
    // Use preserved complete methods or the real C caller with captured byte arguments
    @RUN@
}
int main() {
    // Keep failures quiet and exercise every script byte, both owners and all guard combinations
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    const int32_t randoms[]={INT_MIN,INT_MAX,-129,-128,-127,-1,0,1,127,128,129};
    for(seed=0;seed<4096;seed++)for(unsigned owner=0;owner<2;owner++)for(operation=0;operation<2;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));memset(&AudioManager,0x3C,sizeof(AudioManager));active=owner?&other:&MusicManager;
        uint32_t track=operation?STREAMED_SOUND_STRIPCLUB_AMBIENT:STREAMED_SOUND_MALIBU_AMBIENT;
        active->m_bIsInitialised=(seed%3==0)?0:((seed%3==1)?1:255);
        active->m_nNextTrack=(seed&256)?track:UINT32_MAX;
        active->m_nPlayingTrack=(seed&512)?track:STREAMED_SOUND_CITY_AMBIENT;
        active->m_nMusicMode=(uint8_t)seed;AudioManager.m_anRandomTable[0]=randoms[(seed/256)%11];
        argument=(uint8_t)seed;pauseTick=seed*123456789u;mutate=(seed&1024)!=0;reads=0;
        uint32_t beforePosition=active->m_aTracks[track].m_nPosition,beforeTimer=active->m_aTracks[track].m_nLastPosCheckTimer;
        uint8_t initialised=active->m_bIsInitialised;bool blocked=(seed&768)!=0;
        value(operation);value(MusicState_GetMusicMode(active));value(MusicState_GetCurrentTrack(active));snapshot();run(operation,argument);snapshot();groups++;
        assert(reads==(blocked?0u:1u));
        if(blocked) {
            assert(active->m_aTracks[track].m_nLastPosCheckTimer==beforeTimer);
            assert(active->m_aTracks[track].m_nPosition==(initialised?beforePosition:(operation?0u:8640u)));
        } else assert(active->m_aTracks[track].m_nLastPosCheckTimer==seed*123456789u);
    }
    printf("groups=%u callbacks=%u hash=%016llx\n",groups,callbacks,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Link the actual C state getters in both variants while comparing complete position operations
    if variant=='before':
        record=before_record;methods='\n'.join(m.function(old,'cMusicManager::',n) for n in m.METHODS);timer='struct CTimer {static uint32_t GetTimeInMillisecondsPauseMode(){return pauseTime();}};'
        dispatch='if(op)active->SetStripClubTrackPos(arg);else active->SetMalibuClubTrackPos(arg);'
    else:
        record='#include "MusicManager.h"';methods='cMusicManager::cMusicManager(){memset(this,0,sizeof(*this));}';timer='extern "C" uint32_t MusicInitHost_PauseTime(void){return pauseTime();}'
        dispatch='TestMusicClubsC(active,op,arg);'
    (out/(variant+'.cpp')).write_text(common.replace('@HEADERS@',headers).replace('@MUSIC@',record).replace('@AUDIO@',fields(audioh)).replace('@TIMER@',timer).replace('@METHODS@',methods).replace('@RUN@',dispatch),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
void TestMusicClubsC(cMusicManager *manager,unsigned op,uint8_t argument)
{
    // Enter each complete club operation from C using the captured script byte
    if(op)MusicClubs_SetStripClubTrackPos(manager,argument);
    else MusicClubs_SetMalibuClubTrackPos(manager,argument);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-ambience-c-tests'/(shape+'.h')).read_bytes())
print('Complete club bodies, actual state getters, unchanged owner fields, four removed wrappers, direct script calls and project registration audited')
