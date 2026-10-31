"""Audit and compare complete position/statistics operations against their original bodies."""
from pathlib import Path
import re
import audio_music_positions_migration as m
import audio_music_state_migration as state
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-positions-c-tests'
def normalize(s):
    # Ignore comments and formatting while comparing every original statement
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicPositions.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('src/audio/DMAudio.cpp',m.logic),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
for name in m.METHODS:
    # Check the complete transplanted body against the preserved member implementation
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicPositions_',name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
facade=m.function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','GetRadioPosition')
assert normalize(state.types(m.calls(facade)))==normalize(m.function(kernel,'DMAudio_','GetRadioPosition'))
assert '#define MUSIC_POSITION_MIN(a,b) ((a) < (b) ? (a) : (b))' in kernel
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'''\nvoid RecordRadioStats();
uint32_t GetTrackStartPos(uint32_t);
uint32_t GetRadioPosition(uint32_t);
} cMusicManager;
extern "C" uint8_t MusicState_IsInitialised(const cMusicManager*);
'''
common=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include "config.h"
#include "audio_enums.h"
#include "Validate.h"
using uint32=uint32_t;
using int32=int32_t;
#define Min(a,b) ((a) < (b) ? (a) : (b))
@RECORD@
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other;
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" uint32_t TestMusicPositionsC(cMusicManager*,unsigned,uint32_t);
static uint64_t digest=1469598103934665603ULL;
static unsigned seed,operation,reads,groups,callbacks,mutate;
static uint32_t argument,sequence[3];
static cMusicManager *active;
static void value(uint32_t v) {
    // Observe values independently of architecture pointer addresses
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot() {
    // Observe both complete owners, retained padding and callback counters
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));
    digest=TestAircraftHash(digest,&other,sizeof(other));
    value(reads);
}
static uint32_t pauseTime() {
    // Observe callback entry and mutate live fields after capturing the return
    assert(reads<3);
    snapshot();uint32_t result=sequence[reads++];value(result);callbacks++;
    if(mutate) {
        active->m_nPlayingTrack=(seed+reads)%NUM_RADIOS;
        active->m_nLastTrackServiceTime=(float)((seed+reads)%17);
        active->m_aTracks[argument%TOTAL_STREAMED_SOUNDS].m_nLastPosCheckTimer=(seed*13u+reads*31u);
        active->m_aTracks[argument%TOTAL_STREAMED_SOUNDS].m_nLength=(seed%2)?17u:UINT32_MAX;
        active->m_bIsInitialised=(reads%2)?255:0;
        active->aListenTimeArray[(seed+reads)%NUM_RADIOS]=(float)(reads*3u);
        argument^=0x80000000u;
    }
    return result;
}
@TIMER@
@METHODS@
static uint32_t run(unsigned op,uint32_t arg) {
    // Dispatch through the member implementation or the actual C caller
    @RUN@
}
int main() {
    // Keep sanitizer failures quiet and exercise timer, station and numeric boundaries
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    const uint32_t values[]={0,1,16,17,269999,270000,270001,INT_MAX,0x80000000u,UINT32_MAX};
    for(seed=0;seed<2048;seed++) for(unsigned owner=0;owner<2;owner++) for(operation=0;operation<4;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));
        active=(owner&&operation!=3)?&other:&MusicManager;
        uint32_t input=values[seed%10];
        argument=operation==1?seed%TOTAL_STREAMED_SOUNDS:input;
        active->m_bIsInitialised=(seed%3==0)?0:((seed%3==1)?1:255);
        active->m_bDisabled=255;
        active->m_nPlayingTrack=(seed%5==0)?UINT32_MAX:seed%(NUM_RADIOS+1);
        active->m_nLastTrackServiceTime=(seed%7==0)?-1.0f:(float)values[(seed/7)%10];
        for(unsigned i=0;i<NUM_RADIOS;i++)active->aListenTimeArray[i]=(i%2)?16777216.0f:-3.5f;
        unsigned track=argument%TOTAL_STREAMED_SOUNDS;
        active->m_aTracks[track].m_nPosition=values[(seed/10)%10];
        active->m_aTracks[track].m_nLength=(seed%3==0)?UINT32_MAX:((seed%3==1)?17:1);
        active->m_aTracks[track].m_nLastPosCheckTimer=values[(seed/100)%10];
        for(unsigned i=0;i<3;i++)sequence[i]=values[(seed/3+i)%10];
        mutate=seed%2;reads=0;value(operation);value(argument);snapshot();
        uint32_t result=run(operation,argument);value(result);snapshot();groups++;
        if(operation>=2&&input>=NUM_RADIOS)assert(reads==0&&result==0);
    }
    // Verify macro re-evaluation and strict equality against explicit expectations
    active=&MusicManager;memset(active,0,sizeof(*active));active->m_bIsInitialised=1;mutate=0;argument=0;
    active->m_aTracks[0].m_nLength=UINT32_MAX;
    sequence[0]=1;sequence[1]=269999;sequence[2]=7;reads=0;
    assert(run(1,0)==7&&reads==3);
    sequence[0]=1;sequence[1]=270000;reads=0;
    assert(run(1,0)==270000&&reads==2);
    active->m_aTracks[0].m_nPosition=17;active->m_aTracks[0].m_nLength=17;
    sequence[0]=0;sequence[1]=0;reads=0;
    assert(run(1,0)==17&&reads==2);
    sequence[0]=0;sequence[1]=UINT32_MAX;reads=0;
    assert(run(1,0)==17&&reads==2&&active->m_aTracks[0].m_nLastPosCheckTimer==UINT32_MAX);
    snapshot();printf("groups=%u callbacks=%u hash=%016llx\n",groups,callbacks,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Compile the exact preserved member bodies separately from the C implementation
    if variant=='before':
        record=before_record;methods='\n'.join(m.function(old,'cMusicManager::',n) for n in m.METHODS)+'\n'+facade
        timer='struct CTimer {static uint32_t GetTimeInMillisecondsPauseMode(){return pauseTime();}};'
        dispatch='switch(op){case 0:active->RecordRadioStats();return 0;case 1:return active->GetTrackStartPos(arg);case 2:return active->GetRadioPosition(arg);case 3:return DMAudio_GetRadioPosition(arg);}return 0;'
    else:
        record='#include "MusicManager.h"';methods='cMusicManager::cMusicManager(){memset(this,0,sizeof(*this));}'
        timer='extern "C" uint32_t MusicInitHost_PauseTime(void){return pauseTime();}'
        dispatch='return TestMusicPositionsC(active,op,arg);'
    (out/(variant+'.cpp')).write_text(common.replace('@RECORD@',record).replace('@TIMER@',timer).replace('@METHODS@',methods).replace('@RUN@',dispatch),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
#include "DMAudio.h"
uint32_t TestMusicPositionsC(cMusicManager *manager,unsigned op,uint32_t arg)
{
    // Enter each complete operation from a C translation unit
    switch(op){case 0:MusicPositions_RecordRadioStats(manager);return 0;
    case 1:return MusicPositions_GetTrackStartPos(manager,arg);
    case 2:return MusicPositions_GetRadioPosition(manager,arg);
    case 3:return DMAudio_GetRadioPosition(arg);}
    return 0;
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-init-c-tests'/(shape+'.h')).read_bytes())
print('Complete operations, public facade, unchanged fields, macro re-evaluation and project entries audited')
