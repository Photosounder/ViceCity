"""Audit complete music initialization and compare original/C clock, provider and timer traces."""
from pathlib import Path
import re
import timer_migration as timer
import audio_music_init_migration as m
import audio_music_state_migration as state
from audio_owner_init_migration import fields
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-init-c-tests'
def normalize(source):
    # Compare all statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicInit.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('src/audio/AudioManager.cpp',m.logic),('src/audio/AudioManager.h',m.header),('src/audio/AudioLifecycle.c',m.calls),('src/audio/DMAudio.cpp',m.logic),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
for name in m.METHODS:
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicInit_',name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
original=m.function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','SetStartingTrackPositions');actual=m.function(kernel,'DMAudio_','SetStartingTrackPositions')
assert normalize(m.calls(original).replace('bool8 ','uint8_t '))==normalize(actual)
assert normalize(m.timer_table(old).replace('uint32 ','uint32_t '))==normalize(kernel[kernel.index('uint32_t NewGameRadioTimers'):kernel.index('};')+2])
assert normalize(timer.calls(m.HOST)) in normalize((root/'src/audio/MusicManager.cpp').read_text())
clock='''#pragma once
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif
time_t TestMusicTime(time_t *value);
struct tm *TestMusicLocalTime(const time_t *value);
#ifdef __cplusplus
}
#endif
#define time TestMusicTime
#define localtime TestMusicLocalTime
'''
(out/'clock.h').write_text(clock,newline='\n')
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','DMAudio.h','Validate.h'])
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
record=state.fields(oldh)
common='''#ifdef _WIN32
#include <windows.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "config.h"
@HEADERS@
using uint32=uint32_t;using bool8=uint8_t;
#define FALSE 0
#define TRUE 1
@MUSIC@
struct cAudioManager {
@AUDIO@
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
cSampleManager SampleManager;
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other;
extern "C" int32_t gNumRetunePresses,gRetuneCounter;
int32_t gNumRetunePresses,gRetuneCounter;
extern "C" uint32_t NewGameRadioTimers[10];
@TABLE@
extern "C" void MusicState_InitState(cMusicManager*);
extern "C" uint8_t MusicState_IsInitialised(const cMusicManager*);
extern "C" uint32_t TestMusicInitC(cMusicManager*,unsigned,uint32_t);
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
static cMusicManager *active;
static uint64_t digest=1469598103934665603ULL;
static struct tm calendar;
static unsigned seed,pass,operation,groups,callbacks,files,timers,clocks,locals,mutation;
static uint32_t pauseTick,argument;
static time_t clockValue;
static void value(uint32_t v) {
    // Observe arguments, captured returns and counters without pointer identities
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot(void) {
    // Observe complete music/audio/provider storage and retained writable tables at callbacks
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));
    digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,&AudioManager,sizeof(AudioManager));
    digest=TestAircraftHash(digest,&SampleManager,sizeof(SampleManager));
    digest=TestAircraftHash(digest,&calendar,sizeof(calendar));
    digest=TestAircraftHash(digest,NewGameRadioTimers,sizeof(NewGameRadioTimers));
    value((uint32_t)gNumRetunePresses);value((uint32_t)gRetuneCounter);value(pauseTick);value(argument);
    value(files);value(timers);value(clocks);value(locals);
}
static int32_t randomValue(unsigned i) {
    // Keep sanitizer inputs within the original expression's defined arithmetic domain
#ifdef TEST_MUSIC_SAFE
    return (int32_t)(1+(seed+i)%3);
#else
    static const int32_t values[]={0,1,32767,65535,INT32_MAX,123,-7};
    return values[(seed+i)%7];
#endif
}
static void event(unsigned token) {
    // Mutate live callback state after its snapshot while preserving captured return values
    callbacks++;value(token);snapshot();
    if(mutation) {
        active->m_bIsInitialised=(uint8_t)((files+timers+token)%3==0?0:255);
        active->m_nRadioPosition=(int32_t)(seed+files+token);
        active->m_nPlayingTrack^=files+token;
        gNumRetunePresses=(int32_t)(files+token);gRetuneCounter=(int32_t)(timers+token);
        argument^=token;
        for(unsigned i=0;i<5;i++)AudioManager.m_anRandomTable[i]=randomValue(i+token+files);
        NewGameRadioTimers[(files+token)%10]^=token+seed;
        SampleManager.m_nMusicVolume=(uint8_t)(SampleManager.m_nMusicVolume+token);
    }
}
extern "C" time_t TestMusicTime(time_t *destination) {
    // Capture the original clock result before any callback mutation
    assert(destination==0);clocks++;time_t result=clockValue;value((uint32_t)result);event(1);return result;
}
extern "C" struct tm *TestMusicLocalTime(const time_t *valuePointer) {
    // Return the same writable broken-down time record and observe the captured timestamp
    assert(valuePointer);locals++;value((uint32_t)*valuePointer);event(2);return &calendar;
}
extern "C" int32_t SampleManager_GetStreamedFileLength(cSampleManager *manager,uint8_t stream) {
    // Observe the original byte narrowing for every logical streamed track, including indices above 255
    assert(manager==&SampleManager);assert(stream==(uint8_t)files);value(stream);
    static const int32_t lengths[]={1,17,INT32_MAX,-1,-2,1234567};
    int32_t result=lengths[(files+seed)%6];files++;value((uint32_t)result);event(3);return result;
}
static uint32_t pauseTime(void) {
    // Observe each pause-mode timer read after its length and position writes
    uint32_t result=pauseTick;timers++;value(result);event(4);pauseTick+=17;return result;
}
struct CTimer {
    static uint32 GetTimeInMillisecondsPauseMode() { return pauseTime(); }
};
extern "C" uint32_t MusicInitHost_PauseTime(void) {
    // Enter the same pause timer observer through the new C declaration
    return pauseTime();
}
@METHODS@
static uint32_t run(unsigned op) {
    // Invoke the unchanged original operation/facade or the separately compiled actual C implementation
    @RUN@
}
int main() {
#ifdef _WIN32
    // Keep sanitizer failures quiet without creating Windows fault dialogs
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif
    // Exercise each zero-field substitution, mixed masks and complete track-table passes on both owners
    static const unsigned masks[]={0,1,2,4,8,16,32,64,127,5,10,21,42,85,63,126};
    static const uint32_t table[]={948160,452150,2438150,3538230,3513100,4246050,1418050,3178240,471210,0};
    for(seed=0;seed<32;seed++)for(pass=0;pass<2;pass++)for(operation=0;operation<3;operation++) {
        // Seed every retained byte only after construction and provide nonzero file lengths
        memset((void*)&MusicManager,seed&255,sizeof(MusicManager));memset((void*)&other,(seed+1)&255,sizeof(other));
        memset(&AudioManager,0,sizeof(AudioManager));memset(&SampleManager,0,sizeof(SampleManager));memset(&calendar,0,sizeof(calendar));
        memcpy(NewGameRadioTimers,table,sizeof(table));
        active=operation==2?&MusicManager:((seed+pass+operation)%2?&other:&MusicManager);
        active->m_bIsInitialised=(uint8_t)(operation==0?(seed%3?255:0):(pass?1:seed%3?255:0));
        for(unsigned i=0;i<5;i++)AudioManager.m_anRandomTable[i]=randomValue(i);
        gNumRetunePresses=(int32_t)seed+100;gRetuneCounter=(int32_t)seed-100;
        pauseTick=UINT32_MAX-seed;argument=(seed%3==0?0:seed%3==1?1:255);
        clocks=locals=files=timers=0;mutation=pass;
        clockValue=(seed%5==0?(time_t)-1:(time_t)(1700000000U+seed));
        unsigned mask=masks[seed%16];
#ifdef TEST_MUSIC_SAFE
        int base=1+(seed%2);calendar.tm_yday=1;
        calendar.tm_sec=(mask&1)?0:base;calendar.tm_min=(mask&2)?0:base;calendar.tm_hour=(mask&4)?0:base;
        calendar.tm_mday=(mask&8)?0:base;calendar.tm_mon=(mask&16)?0:base;calendar.tm_year=(mask&32)?0:base;calendar.tm_wday=(mask&64)?0:base;
#else
        calendar.tm_yday=274;calendar.tm_sec=(mask&1)?0:56;calendar.tm_min=(mask&2)?0:34;calendar.tm_hour=(mask&4)?0:12;
        calendar.tm_mday=(mask&8)?0:2;calendar.tm_mon=(mask&16)?0:9;calendar.tm_year=(mask&32)?0:126;calendar.tm_wday=(mask&64)?0:1;
#endif
        uint32_t result=run(operation);value(result);snapshot();
        assert(files==0||files==TOTAL_STREAMED_SOUNDS);assert(timers==files);assert(clocks==(files?1U:0U));assert(locals==(files&&clockValue!=(time_t)-1?1U:0U));groups++;
    }
    printf("%u groups %u callbacks %llu\\n",groups,callbacks,(unsigned long long)digest);return 0;
}
'''.replace('@HEADERS@',headers).replace('@AUDIO@',fields((root/'src/audio/AudioManager.h').read_text()))
original_methods='\n'.join(m.function(old,'cMusicManager::',name) for name in m.METHODS)
original_facade=m.function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','SetStartingTrackPositions')
for variant in ['before','after']:
    if variant=='before':
        decl=track+'\nstruct cMusicManager {\n'+record+'\ncMusicManager();\nuint8_t Initialise();void SetStartingTrackPositions(bool8 isNewGameTimer);\n};'
        methods=state.CTOR+'\n'+original_methods+'\n'+original_facade
        run='switch(op) {case 0:return active->Initialise();case 1:active->SetStartingTrackPositions((bool8)argument);break;case 2:DMAudio_SetStartingTrackPositions((bool8)argument);break;}return 0;'
        table=m.timer_table(old)
    else:decl='#include "MusicManager.h"';methods=state.CTOR;run='return TestMusicInitC(active,op,argument);';table=''
    (out/(variant+'.cpp')).write_text(common.replace('@MUSIC@',decl).replace('@TABLE@',table).replace('@METHODS@',methods).replace('@RUN@',run),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
#include "DMAudio.h"
uint32_t TestMusicInitC(cMusicManager *manager,unsigned op,uint32_t argument)
{
    // Enter complete C initialization, selection and the public facade with original byte narrowing
    switch(op) {
    case 0:return MusicInit_Initialise(manager);
    case 1:MusicInit_SetStartingTrackPositions(manager,(uint8_t)argument);break;
    case 2:DMAudio_SetStartingTrackPositions((uint8_t)argument);break;
    }
    return 0;
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete initialization/selection, writable timer table, public facade, single pause-time adapter, direct lifecycle call and all header/project edits audited')
