"""Compare complete saved-radio helpers and reload behavior with preserved sources."""
from pathlib import Path
import re
import audio_music_reload_migration as m
import audio_music_state_migration as state
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-reload-c-tests'
def normalize(s):
    # Compare every statement while ignoring formatting and comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
changes={'src/audio/MusicManager.cpp':m.logic,'src/audio/MusicManager.h':m.header,'src/audio/AudioManager.cpp':m.logic,'src/audio/AudioManager.h':m.header,'src/audio/AudioReload.c':m.reload,'src/core/Stats.cpp':m.stats,'src/core/Stats.h':m.stats_header,'src/core/Frontend.cpp':m.stats,'src/save/GenericGameStorage.cpp':m.storage,'src/save/GenericGameStorage.h':m.storage_header,'build/reVC.vcxproj':m.project,'build/reVC.vcxproj.filters':m.project}
for name,transform in changes.items():assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();saved=(out/'before/src/save/GenericGameStorage.cpp').read_text();stats=(out/'before/src/core/Stats.cpp').read_text();kernel=(root/'src/audio/MusicReload.c').read_text();data=(root/'src/audio/MusicSavedData.c').read_text()
original=m.function(old,'cMusicManager::','ResetMusicAfterReload');actual=m.function(kernel,'MusicReload_','ResetMusicAfterReload')
assert normalize(m.body(original))==normalize(actual[actual.index('{'):])
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
for name in m.SAVED_METHODS:
    # Verify the complete C saved-position helper against the original implementation
    expected=m.function(saved,'',name).replace('int32','int32_t').replace('PositionList()','PositionList(void)')
    assert normalize(expected)==normalize(m.function(data,'',name)),name
getter=m.function(stats,'CStats::','GetFavoriteRadioStationList')
expected=getter.replace('CStats::GetFavoriteRadioStationList','MusicSavedData_GetFavoriteRadioStationList').replace('int32','int32_t').replace('FavoriteRadioStationList[station]','MusicSavedData_FavoriteRadioStationList[station]')
assert normalize(expected)==normalize(m.function(data,'MusicSavedData_','GetFavoriteRadioStationList'))
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\nvoid ResetMusicAfterReload();\n} cMusicManager;\n'
common=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <windows.h>
#include "config.h"
#include "audio_enums.h"
#include "Validate.h"
using uint32=uint32_t;using int32=int32_t;using bool8=uint8_t;
#define FALSE 0
#define TRUE 1
@RECORD@
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other;
@DATA@
extern "C" int32_t gNumRetunePresses,gRetuneCounter;
int32_t gNumRetunePresses,gRetuneCounter;
@STATIC_TIMER@
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" uint32_t TestMusicReloadC(cMusicManager*,unsigned,int32_t);
static uint64_t digest=1469598103934665603ULL;
static unsigned seed,operation,groups,callbacks,debugs,timers,facades,mutate;
static cMusicManager *active;
static uint32_t pauseTick;
static void value(uint32_t v) {
    // Observe scalar values without pointer addresses
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot() {
    // Observe both complete owners and the actual writable saved-radio arrays
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));
    digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,FAVORITES,sizeof(FAVORITES));
    digest=TestAircraftHash(digest,RadioStationPosition,sizeof(RadioStationPosition));
    value(RadioStaticTimer);value((uint32_t)gNumRetunePresses);value((uint32_t)gRetuneCounter);
}
static void change() {
    // Change live positions and lengths while leaving captured listening times distinguishable
    if(!mutate)return;
    for(unsigned i=0;i<NUM_RADIOS;i++) {
        FAVORITES[i]=(float)(seed+i+callbacks);
        RadioStationPosition[i]=(i%3==0)?-1:(int32_t)(seed+i+callbacks);
        active->m_aTracks[i].m_nLength=(callbacks%2)?17:UINT32_MAX;
    }
    gNumRetunePresses=(int32_t)callbacks;gRetuneCounter=-(int32_t)callbacks;
    RadioStaticTimer=callbacks*31u;active->m_bIsInitialised=255;
    active->aListenTimeArray[(seed+callbacks)%NUM_RADIOS]=-7.0f;
}
extern "C" void re3_debug(const char *format,...) {
    // Observe exact original diagnostic text and arguments before mutating live state
    assert(strcmp(format,"[DBG]: Radio Track %d saved position is %d, Length is only %d\n")==0);
    va_list args;va_start(args,format);int station=va_arg(args,int);int32_t position=va_arg(args,int32_t);uint32_t length=va_arg(args,uint32_t);va_end(args);
    value(1);value((uint32_t)station);value((uint32_t)position);value(length);snapshot();callbacks++;debugs++;change();
}
static uint32_t pauseTime() {
    // Capture the pause timer before callback changes and the following per-track write
    uint32_t result=pauseTick;pauseTick+=0x80000001u;
    value(2);value(result);snapshot();callbacks++;timers++;change();return result;
}
extern "C" int32_t DMAudio_GetRadioPosition(uint32_t station) {
    // Supply signed facade returns and observe partial writes during list population
    assert(station<NUM_RADIOS);value(3);value(station);snapshot();callbacks++;facades++;
    int32_t result=(int32_t)(station*0x80000001u+seed);
    change();return result;
}
@TIMER@
@METHODS@
static uint32_t run(unsigned op,int32_t arg) {
    // Dispatch through the preserved complete implementation or the actual C caller
    @RUN@
}
int main() {
    // Keep failures quiet and exercise the reset and all four saved-radio helpers
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    const int32_t positions[]={-1,-2,INT_MIN,INT_MAX,0,1,16,17,18,123456789};
    const uint32_t floatBits[]={0,0x80000000u,0x3F800000u,0xBF800000u,0x7F800000u,0xFF800000u,0x7FC01234u,1};
    for(seed=0;seed<1024;seed++) for(unsigned owner=0;owner<2;owner++) for(operation=0;operation<5;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));active=owner?&other:&MusicManager;
        for(unsigned i=0;i<NUM_RADIOS;i++) {
            uint32_t bits=(seed%4==0)?floatBits[i%2]:floatBits[(seed+i)%8];memcpy(&FAVORITES[i],&bits,4);
            RadioStationPosition[i]=positions[(seed+i)%10];
            active->m_aTracks[i].m_nLength=(seed%3==0)?1:((seed%3==1)?17:UINT32_MAX);
            active->aListenTimeArray[i]=-3.5f;
        }
        active->m_bIsInitialised=(seed%3==0)?0:((seed%3==1)?1:255);
        RadioStaticTimer=UINT32_MAX;gNumRetunePresses=INT_MIN;gRetuneCounter=INT_MAX;
        pauseTick=seed*123456789u;mutate=seed%2;unsigned oldTimers=timers,oldDebugs=debugs;
        value(operation);snapshot();uint32_t result=run(operation,(int32_t)(seed%NUM_RADIOS));value(result);snapshot();groups++;
        if(operation==0&&seed%4==0) {
            assert(timers==oldTimers&&debugs==oldDebugs);
            assert(RadioStaticTimer==0&&gNumRetunePresses==0&&gRetuneCounter==0);
            for(unsigned i=0;i<NUM_RADIOS;i++)assert(active->aListenTimeArray[i]==-3.5f);
        }
        if(operation==1)for(unsigned i=0;i<NUM_RADIOS;i++)assert(RadioStationPosition[i]==-1);
    }
    printf("groups=%u callbacks=%u debug=%u timers=%u facades=%u hash=%016llx\n",groups,callbacks,debugs,timers,facades,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Compare the original complete operations to linked production C functions and storage
    if variant=='before':
        record=before_record;data_decl='struct CStats {static float FavoriteRadioStationList[NUM_RADIOS];static float GetFavoriteRadioStationList(int32);};\nfloat CStats::FavoriteRadioStationList[NUM_RADIOS];\nint32 RadioStationPosition[NUM_RADIOS];\n#define FAVORITES CStats::FavoriteRadioStationList\n'
        timer_decl='uint32 RadioStaticTimer;';timer='struct CTimer {static uint32 GetTimeInMillisecondsPauseMode(){return pauseTime();}};\n#define debug(format, ...) re3_debug("[DBG]: " format, __VA_ARGS__)\n'
        methods=getter+'\n'+'\n'.join(m.function(saved,'',n) for n in m.SAVED_METHODS)+'\n'+original
        dispatch='switch(op){case 0:active->ResetMusicAfterReload();return 0;case 1:InitRadioStationPositionList();return 0;case 2:return GetSavedRadioStationPosition(arg);case 3:PopulateRadioStationPositionList();return 0;case 4:{float v=CStats::GetFavoriteRadioStationList(arg);uint32_t bits;memcpy(&bits,&v,4);return bits;}}return 0;'
    else:
        record='#include "MusicManager.h"\n#include "MusicSavedData.h"';data_decl='#define FAVORITES MusicSavedData_FavoriteRadioStationList\n';timer_decl=''
        timer='extern "C" uint32_t MusicInitHost_PauseTime(void){return pauseTime();}'
        methods='cMusicManager::cMusicManager(){memset(this,0,sizeof(*this));}'
        dispatch='return TestMusicReloadC(active,op,arg);'
    (out/(variant+'.cpp')).write_text(common.replace('@RECORD@',record).replace('@DATA@',data_decl).replace('@STATIC_TIMER@',timer_decl).replace('@TIMER@',timer).replace('@METHODS@',methods).replace('@RUN@',dispatch),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
#include "MusicSavedData.h"
#include <string.h>
uint32_t TestMusicReloadC(cMusicManager *manager,unsigned op,int32_t arg)
{
    // Exercise the complete C reset and saved-radio helpers from C
    switch(op){case 0:MusicReload_ResetMusicAfterReload(manager);return 0;
    case 1:InitRadioStationPositionList();return 0;
    case 2:return GetSavedRadioStationPosition(arg);
    case 3:PopulateRadioStationPositionList();return 0;
    case 4:{float v=MusicSavedData_GetFavoriteRadioStationList(arg);uint32_t bits;memcpy(&bits,&v,4);return bits;}}
    return 0;
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-positions-c-tests'/(shape+'.h')).read_bytes())
print('Complete reset, four saved-radio helpers, storage, stats/frontend/save consumers and direct audio reload entry audited')
