"""Compare complete car-radio operations and facades against their preserved C++ bodies."""
from pathlib import Path
import re
import audio_music_radio_migration as m
import audio_music_state_migration as state
from audio_owner_init_migration import fields
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-radio-c-tests'

def normalize(s):
    # Compare every statement while ignoring comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))

old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicRadio.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('src/audio/DMAudio.cpp',m.logic),('src/audio/oal/AudioSampleHost.cpp',m.logic),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
for name in m.METHODS:
    # Verify that each complete body retains its original gates, arithmetic and provider calls
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicRadio_',name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
facades=[]
for name in ['GetRadioInCar','SetRadioInCar']:
    # Preserve the public byte return and unsigned station argument
    facade=m.function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_',name);facades.append(facade)
    assert normalize(m.calls(facade).replace('uint8\n','uint8_t\n').replace('uint32 ','uint32_t '))==normalize(m.function(kernel,'DMAudio_',name))
sample=m.sample_host((out/'before/src/audio/oal/AudioSampleHost.cpp').read_text())
assert normalize(sample.replace('MusicManager.GetRadioInCar()','MusicRadio_GetRadioInCar(&MusicManager)').replace('extern "C" ',''))==normalize(m.function(kernel,'AudioSample_','GetRadioInCar'))
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\ncMusicManager();\nuint8_t PlayerInCar();\nuint32_t GetRadioInCar();\nvoid SetRadioInCar(uint32_t);\nuint32_t GetNextCarTuning();\nuint32_t GetCarTuning();\nuint8_t UsesPoliceRadio(CVehicle*);\nuint8_t UsesTaxiRadio(CVehicle*);\n} cMusicManager;\n'
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','Validate.h','sampman.h','DMAudio.h','oal/AudioSampleHost.h','../modelinfo/ModelIds.h','../peds/PedStates.h','../entities/EntityStatus.h'])
common=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>
#include "config.h"
@HEADERS@
using uint8=uint8_t;using uint32=uint32_t;using int32=int32_t;using bool8=uint8_t;
#define FALSE 0
#define TRUE 1
#define nil nullptr
static void event(unsigned);
static void value(uint32_t);
struct CVehicle {
    int32_t status;
    int16_t model;
    uint8_t m_nRadioStation,siren;
    int32_t GetStatus(){int32_t result=status;value((uint32_t)result);event(3);return result;}
    int16_t GetModelIndex(){int16_t result=model;value((uint32_t)result);event(4);return result;}
    bool UsesSiren(){bool result=siren!=0;value(result);event(5);return result;}
};
struct CPed {int32_t m_nPedState;};
struct CTheScripts {static bool bPlayerHasMetDebbieHarry;};
bool CTheScripts::bPlayerHasMetDebbieHarry;
@MUSIC@
typedef struct cAudioManager { @AUDIO@ } cAudioManager;
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
extern "C" cMusicManager MusicManager;
cMusicManager MusicManager;
static cMusicManager other,*active;
cSampleManager SampleManager;
extern "C" int32_t gNumRetunePresses,gRetuneCounter;
int32_t gNumRetunePresses,gRetuneCounter;
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" uint32_t TestMusicRadioC(cMusicManager*,CVehicle*,unsigned,uint32_t);
extern "C" void MusicState_InitState(cMusicManager*);
static CVehicle vehicles[2],*selected;
static CPed ped;
static uint8_t replay,mp3;
static unsigned seed,pass,operation,groups,callbacks,finds,mutation;
static uint32_t argument;
static uint64_t digest=1469598103934665603ULL;
static void value(uint32_t v) {
    // Hash scalar results and arguments without pointer identity
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot() {
    // Observe complete owners, vehicles, provider storage and live radio inputs
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,&AudioManager,sizeof(AudioManager));digest=TestAircraftHash(digest,&SampleManager,sizeof(SampleManager));
    digest=TestAircraftHash(digest,vehicles,sizeof(vehicles));value((uint32_t)ped.m_nPedState);
    value((uint32_t)gNumRetunePresses);value((uint32_t)gRetuneCounter);value(argument);value(replay);value(mp3);
    value(CTheScripts::bPlayerHasMetDebbieHarry);value(finds);value(selected==nullptr?2u:(uint32_t)(selected-vehicles));
}
static void event(unsigned token) {
    // Mutate live inputs after the original callback has captured its result
    value(token);snapshot();callbacks++;
    if(mutation) {
        active->m_nRadioInCar=(callbacks%3==0)?NO_TRACK:callbacks*257u;
        active->m_bIsInitialised=(callbacks%3==0)?0:255;
        AudioManager.m_bIsPaused=(callbacks%2)?255:0;
        AudioManager.m_anRandomTable[2]=(callbacks%2)?INT_MIN:-1;
        replay=(callbacks%2)?255:0;mp3=(callbacks%3)?255:0;
        vehicles[0].m_nRadioStation=(uint8_t)(seed+callbacks);vehicles[1].m_nRadioStation=(uint8_t)(seed-callbacks);
        vehicles[0].model=(callbacks%3==0)?MI_KAUFMAN:MI_POLICE;vehicles[1].model=(callbacks%3==0)?MI_MRWHOOP:MI_POLMAV;
        vehicles[0].status=(callbacks%3)?STATUS_PLAYER:STATUS_WRECKED;
        vehicles[1].siren=(callbacks%2)?255:0;
        ped.m_nPedState=(callbacks%3==0)?PED_ARRESTED:PED_DRIVING;
        CTheScripts::bPlayerHasMetDebbieHarry=callbacks%2!=0;
        gNumRetunePresses=(int32_t)(callbacks%51)-25;argument^=UINT32_MAX;
        selected=(callbacks%3==0)?nullptr:&vehicles[callbacks%2];
    }
}
extern "C" CVehicle *AudioGame_FindVehicle(void) {
    // Preserve the captured vehicle even when a later lookup sees a different player vehicle
    CVehicle *result=selected;finds++;value(result==nullptr?2u:(uint32_t)(result-vehicles));event(1);return result;
}
static CPed *FindPlayerPed() {
    // Read the player state after the original live player lookup completes
    event(2);return &ped;
}
struct CReplay {
    static bool IsPlayingBack(){bool result=replay!=0;value(result);event(6);return result;}
};
extern "C" int32_t AudioPedTalkHost_PlayerState(void) {return FindPlayerPed()->m_nPedState;}
extern "C" int32_t AudioVehicleEngineHost_Status(const CVehicle *v) {return const_cast<CVehicle*>(v)->GetStatus();}
extern "C" int16_t AudioVehicleModelHost_Model(const CVehicle *v) {return const_cast<CVehicle*>(v)->GetModelIndex();}
extern "C" uint8_t AudioEnvironmentHost_Replay(void) {return CReplay::IsPlayingBack();}
extern "C" uint8_t *MusicRadioHost_Station(CVehicle *v) {
    // Expose the exact original byte field without introducing a read callback
    assert(v==&vehicles[0]||v==&vehicles[1]);return &v->m_nRadioStation;
}
extern "C" uint8_t MusicRadioHost_UsesSiren(CVehicle *v) {return v->UsesSiren();}
extern "C" uint8_t MusicRadioHost_HasMetDebbie(void) {return CTheScripts::bPlayerHasMetDebbieHarry;}
extern "C" uint8_t SampleManager_IsMP3RadioChannelAvailable(cSampleManager *manager) {
    // Observe the actual provider query before fallback reads the live random table
    assert(manager==&SampleManager);uint8_t result=mp3;value(result);event(7);return result;
}
uint8 DMAudio_IsMP3RadioChannelAvailable(void) {
    // Preserve the separate public availability query whose result is discarded during retuning
    uint8_t result=mp3;value(result);event(8);return result;
}
@METHODS@
static uint32_t workerRadio() {
    // Match the existing stream-worker query's backend compilation guard
#if defined(AUDIO_OAL) || defined(AUDIO_MSS)
    return (uint32_t)AudioSample_GetRadioInCar();
#else
    return 0;
#endif
}
static uint32_t run(unsigned op,uint32_t station) {
    // Invoke exact original operations or the separately compiled C entry points
    @RUN@
}
int main() {
    // Exercise all station bytes, owner selection, radio rules and callbacks without opening a window
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    const int16_t models[]={MI_VCNMAV,MI_POLMAV,MI_COASTG,MI_RHINO,MI_BARRACKS,MI_MRWHOOP,MI_HUNTER,MI_KAUFMAN,MI_POLICE,-1};
    const int32_t states[]={PED_DRAG_FROM_CAR,PED_EXIT_CAR,PED_ARRESTED,PED_DRIVING,-1};
    const uint32_t stations[]={0,1,255,256,UINT32_MAX,0x80000000u,1234,NO_TRACK};
    const int32_t randoms[]={INT_MIN,-1,0,1,INT_MAX};
    for(seed=0;seed<1024;seed++)for(pass=0;pass<2;pass++)for(operation=0;operation<10;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));
        memset(&AudioManager,0,sizeof(AudioManager));memset(&SampleManager,0,sizeof(SampleManager));memset(vehicles,0,sizeof(vehicles));
        active=operation>=7?&MusicManager:((seed+pass)%2?&other:&MusicManager);
        active->m_bIsInitialised=(seed%3==0)?0:((seed%3==1)?1:255);active->m_bDisabled=255;
        active->m_nRadioInCar=stations[(seed/4)%8];AudioManager.m_bIsPaused=(seed%2)?255:0;
        AudioManager.m_anRandomTable[2]=randoms[(seed/7)%5];
        for(unsigned i=0;i<2;i++){vehicles[i].m_nRadioStation=(uint8_t)(seed+i);vehicles[i].model=models[(seed/3+i)%10];vehicles[i].status=(seed%3)?STATUS_PLAYER:STATUS_WRECKED;vehicles[i].siren=(seed%2)?255:0;}
        selected=(seed%4==0)?nullptr:&vehicles[seed%2];ped.m_nPedState=states[(seed/5)%5];
        CTheScripts::bPlayerHasMetDebbieHarry=seed%2!=0;replay=(seed%3)?255:0;mp3=(seed%3)?0:255;
        gNumRetunePresses=(int32_t)(seed%513)-256;gRetuneCounter=(int32_t)(seed*123u);argument=stations[seed%8];finds=0;mutation=pass;
        uint8_t initialized=active->m_bIsInitialised;unsigned previous=callbacks;
        snapshot();uint32_t result=run(operation,argument);value(result);snapshot();groups++;
        assert(finds<=2);
        if(!initialized&&(operation==1||operation==2||operation>=7))assert(callbacks==previous);
    }
    // Independently verify byte narrowing versus signed wrapping at the station boundary
    mutation=0;active=&other;selected=&vehicles[0];vehicles[0].model=MI_MRWHOOP;
    vehicles[0].m_nRadioStation=255;gNumRetunePresses=1;finds=0;
    snapshot();uint32_t boundary=run(3,0);value(boundary);snapshot();groups++;
#ifdef RADIO_SCROLL_TO_PREV_STATION
    assert(boundary==256u%(NUM_RADIOS+1));
#else
    assert(boundary==0);
#endif
    assert(vehicles[0].m_nRadioStation==boundary&&gNumRetunePresses==0);
    printf("groups=%u callbacks=%u hash=%016llx\n",groups,callbacks,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Compile untouched original bodies against matching game boundary observers
    if variant=='before':
        record=before_record;methods=state.CTOR+'\n'+'\n'.join(m.function(old,'cMusicManager::',n) for n in m.METHODS)+'\n'+'\n'.join(facades)+'\n#if defined(AUDIO_OAL) || defined(AUDIO_MSS)\n'+sample+'\n#endif\n'
        dispatch='switch(op){case 0:return active->PlayerInCar();case 1:return active->GetRadioInCar();case 2:active->SetRadioInCar(station);return 0;case 3:return active->GetNextCarTuning();case 4:return active->GetCarTuning();case 5:return active->UsesPoliceRadio(&vehicles[seed%2]);case 6:return active->UsesTaxiRadio(&vehicles[seed%2]);case 7:return DMAudio_GetRadioInCar();case 8:DMAudio_SetRadioInCar(station);return 0;default:return workerRadio();}'
    else:record='#include "MusicManager.h"';methods=state.CTOR;dispatch='return TestMusicRadioC(active,&vehicles[seed%2],op,station);'
    (out/(variant+'.cpp')).write_text(common.replace('@HEADERS@',headers).replace('@MUSIC@',record).replace('@AUDIO@',fields((root/'src/audio/AudioManager.h').read_text())).replace('@METHODS@',methods).replace('@RUN@',dispatch),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
#include "DMAudio.h"
#include "oal/AudioSampleHost.h"
uint32_t TestMusicRadioC(cMusicManager *manager,CVehicle *vehicle,unsigned op,uint32_t station)
{
    // Enter each complete C operation and both public facades with the original captured arguments
    switch(op){case 0:return MusicRadio_PlayerInCar(manager);case 1:return MusicRadio_GetRadioInCar(manager);case 2:MusicRadio_SetRadioInCar(manager,station);return 0;case 3:return MusicRadio_GetNextCarTuning(manager);case 4:return MusicRadio_GetCarTuning(manager);case 5:return MusicRadio_UsesPoliceRadio(vehicle);case 6:return MusicRadio_UsesTaxiRadio(vehicle);case 7:return DMAudio_GetRadioInCar();case 8:DMAudio_SetRadioInCar(station);return 0;default:
#if defined(AUDIO_OAL) || defined(AUDIO_MSS)
        return (uint32_t)AudioSample_GetRadioInCar();
#else
        return 0;
#endif
    }
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-modes-c-tests'/(shape+'.h')).read_bytes())
(out/'no-previous-station.h').write_text('#include "config.h"\n#define AUDIO_OAL\n#undef RADIO_SCROLL_TO_PREV_STATION\n',newline='\n')
print('Seven complete radio bodies, unchanged owner fields, three typed accessors, two facades, stream query and all caller/project changes audited')
