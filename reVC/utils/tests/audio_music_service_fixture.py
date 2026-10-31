"""Compare full music dispatch/gameplay service with original control queries and live state."""
from pathlib import Path
import ast,re
import audio_music_service_migration as m
import audio_music_state_migration as state
import messages_migration as messages
from audio_owner_init_migration import fields
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-service-c-tests'

def normalize(s):
    # Compare every statement while ignoring comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))

old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicService.c').read_text()
changes={'src/audio/MusicManager.cpp':m.logic,'src/audio/MusicManager.h':m.header,'src/audio/AudioManager.cpp':m.calls,'src/audio/AudioManager.h':m.header,'src/audio/AudioService.c':m.audio,'src/skel/skeleton.h':m.skeleton,'build/reVC.vcxproj':m.project,'build/reVC.vcxproj.filters':m.project}
for name,transform in changes.items():assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
for original,target in [('Service','Run'),('ServiceGameMode','GameMode')]:
    # Verify both complete implementations rather than testing only newly introduced entry points
    actual=m.function(kernel,'MusicService_',target)
    assert normalize(m.body(m.function(old,'cMusicManager::',original)))==normalize(actual[actual.index('{'):]),original
assert normalize(m.GLOBALS) in normalize(kernel) and normalize(m.STORE) in normalize(kernel) and normalize(m.HELPER) in normalize(kernel)
assert normalize(m.keys((out/'before/src/skel/skeleton.h').read_text()))==normalize((root/'src/skel/RsKeys.h').read_text().replace('#pragma once',''))
assert 'bool DyingOrDead(void) { return m_nPedState == PED_DIE || m_nPedState == PED_DEAD; }' in (root/'src/peds/Ped.h').read_text()
tree=ast.parse((root/'utils/tests/audio_music_stream_fixture.py').read_text())
common=next(ast.literal_eval(node.value) for node in tree.body if isinstance(node,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='common' for t in node.targets))
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
before_record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\ncMusicManager();\nvoid Service();\nvoid ServiceGameMode();\n} cMusicManager;\n'
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','Validate.h','sampman.h','DMAudio.h','AudioGeometry.h','AudioMath.h','AudioSampleConfig.h','MusicAmbience.h','soundlist.h','../core/GameAreas.h','../renderer/WeatherTypes.h','../peds/PedStates.h','../entities/EntityStatus.h','../modelinfo/ModelIds.h','../skel/RsKeys.h'])
controller=(root/'src/core/ControllerConfig.h').read_text()
original_controller=(root/'build/messages-c-tests/before/src/core/ControllerConfig.h').read_text()
action_header=(root/'src/core/ControllerActions.h').read_text()
assert messages.calls(original_controller)==controller
assert messages.actions_header(original_controller)==action_header
enums=''
for name in ['eControllerType','e_ControllerAction']:
    # Use the exact existing controller enum values in original adapter argument checks
    definitions=action_header if name=='e_ControllerAction' else controller
    a=definitions.index('enum '+name+'\n');b=definitions.index('};',a)+2;enums+=definitions[a:b]+'\n'
prototypes='''extern "C" void MusicState_InitState(cMusicManager*);
extern "C" void MusicPositions_RecordRadioStats(cMusicManager*);
extern "C" uint32_t MusicPositions_GetTrackStartPos(cMusicManager*,uint32_t);
extern "C" void MusicAmbience_SetUpCorrectAmbienceTrack(cMusicManager*);
extern "C" void MusicAmbience_ComputeAmbienceVol(cMusicManager*,uint8_t,uint8_t*);
extern "C" void MusicModes_ServiceFrontEndMode(cMusicManager*);
extern "C" uint8_t MusicRadio_PlayerInCar(cMusicManager*);
extern "C" uint8_t MusicRadio_UsesPoliceRadio(CVehicle*);
extern "C" uint8_t MusicRadio_UsesTaxiRadio(CVehicle*);
extern "C" uint32_t MusicRadio_GetCarTuning(cMusicManager*);
extern "C" uint32_t MusicRadio_GetNextCarTuning(cMusicManager*);
extern "C" uint8_t MusicStream_ServiceAnnouncement(cMusicManager*);
extern "C" void MusicStream_ServiceTrack(cMusicManager*,CVehicle*,CPed*);
'''
common=common.replace('struct CVehicle {uint8_t m_nRadioStation;};','''static void event(unsigned);
static void value(uint32_t);
struct CVehicle {
    int32_t status;int16_t model;uint8_t m_nRadioStation,siren;
    int32_t GetStatus(){int32_t result=status;value((uint32_t)result);event(14);return result;}
    int16_t GetModelIndex(){int16_t result=model;value((uint32_t)result);event(13);return result;}
    bool UsesSiren(){bool result=siren!=0;value(result);event(15);return result;}
};''').replace('struct CPed {uint8_t storage;};','struct CPed {int32_t m_nPedState;bool DyingOrDead(){return m_nPedState==PED_DIE||m_nPedState==PED_DEAD;}};')
common=common.replace('\nuint8_t g_bAnnouncementReadPosAlready;\n','\n@GLOBALS@\n',1).replace('\nint32_t gNumRetunePresses,gRetuneCounter;\n','\n',1)
common=common.replace('uint32_t TestMusicStreamC(cMusicManager*,CVehicle*,CPed*,unsigned)','void TestMusicServiceC(cMusicManager*,unsigned)')
common=common.replace('using bool8=uint8_t;','using bool8=uint8_t;using int32=int32_t;\n#define nil nullptr')
common=common.replace('static CPed ped;','''static CPed peds[2];
static CPed *mainPed;
static unsigned playerFinds,vehicleFinds;
static uint8_t controls[6],mp3,replay;
static int32_t area,mouseKey;
static uint32_t gameTick;
extern "C" uint32_t RadioStaticTimer;
uint32_t RadioStaticTimer;
extern "C" uint8_t RadioStaticCounter;''')
common=common.replace('value(vehicles[0].m_nRadioStation);value(vehicles[1].m_nRadioStation);','digest=TestAircraftHash(digest,vehicles,sizeof(vehicles));digest=TestAircraftHash(digest,peds,sizeof(peds));')
common=common.replace('active->m_bTrackChangeStarted=(callbacks%2)?255:0;', 'active->m_bTrackChangeStarted=(active->m_bAnnouncementInProgress||callbacks%2==0)?0:255;')
common=common.replace('value(lastVolume);value(lastPan);value(lastEffects);','''value(lastVolume);value(lastPan);value(lastEffects);
    for(unsigned i=0;i<6;i++)value(controls[i]);value(mp3);value(replay);value((uint32_t)area);value((uint32_t)mouseKey);value(gameTick);
    value(playerFinds);value(vehicleFinds);value(RadioStaticCounter);value(RadioStaticTimer);
    value(vehicle==nullptr?2u:(uint32_t)(vehicle-vehicles));value(mainPed==nullptr?2u:(uint32_t)(mainPed-peds));''')
common=common.replace('slow=(callbacks%2)?255:0;vehicles[0].m_nRadioStation=(uint8_t)callbacks;','''slow=(callbacks%2)?255:0;vehicles[0].m_nRadioStation=(uint8_t)callbacks;
        vehicle=(callbacks%3==0)?nullptr:&vehicles[callbacks%2];
        vehicles[0].model=(callbacks%3)?MI_POLICE:MI_KAUFMAN;vehicles[1].model=MI_MRWHOOP;
        peds[0].m_nPedState=(callbacks%3)?PED_DRIVING:PED_DIE;peds[1].m_nPedState=PED_DRIVING;
        controls[callbacks%6]=(callbacks%2)?255:0;mp3=(callbacks%2)?255:0;
        area=(callbacks%3)?AREA_MAIN_MAP:AREA_HOTEL;mouseKey=(callbacks%2)?rsMOUSEWHEELUPBUTTON:rsMOUSEWHEELDOWNBUTTON;
        active->m_bGameplayAllowsRadio=(callbacks%3)?255:0;active->m_bRadioSetByScript=(callbacks%2)?255:0;
        active->m_nRadioStationScript=(uint8_t)(callbacks%NUM_RADIOS);
        gRetuneCounter=(int32_t)(callbacks%21);gNumRetunePresses=(int32_t)(callbacks%11)-5;
        RadioStaticCounter=(uint8_t)(callbacks%6);RadioStaticTimer+=token;''')
common=common.replace('&&sound==SOUND_FRONTEND_HURRICANE&&volume==0.0f','&&volume==0.0f')
common=common.replace('extern "C" int32_t MusicAmbienceHost_Area(void) {return AREA_MAIN_MAP;}','extern "C" int32_t MusicAmbienceHost_Area(void) {return area;}')
extra=r'''
static CPed *FindPlayerPed() {
    // Preserve an optional first captured pedestrian while later car checks receive valid player storage
    CPed *result=playerFinds++==0?mainPed:&peds[playerFinds%2];value(result==nullptr?2u:(uint32_t)(result-peds));event(11);return result;
}
extern "C" CPed *AudioPedOneShotsHost_Player(void) {return FindPlayerPed();}
extern "C" int32_t AudioPedOneShotsHost_State(const CPed *p) {return p->m_nPedState;}
extern "C" int32_t AudioPedTalkHost_PlayerState(void) {return FindPlayerPed()->m_nPedState;}
extern "C" CVehicle *AudioGame_FindVehicle(void) {
    // Preserve the captured vehicle and keep a missing main vehicle on the original guarded startup path
    CVehicle *result=vehicle;vehicleFinds++;value(result==nullptr?2u:(uint32_t)(result-vehicles));event(12);
    if(vehicleFinds==1&&!result)active->m_bGameplayAllowsRadio=0;
    return result;
}
extern "C" int32_t AudioVehicleEngineHost_Status(const CVehicle *v) {return const_cast<CVehicle*>(v)->GetStatus();}
extern "C" int16_t AudioVehicleModelHost_Model(const CVehicle *v) {return const_cast<CVehicle*>(v)->GetModelIndex();}
extern "C" uint8_t MusicRadioHost_UsesSiren(CVehicle *v) {return v->UsesSiren();}
extern "C" uint8_t MusicRadioHost_HasMetDebbie(void) {return controls[5]!=0;}
extern "C" uint8_t AudioEnvironmentHost_Replay(void) {uint8_t result=replay!=0;value(result);event(16);return result;}
extern "C" uint8_t SampleManager_IsMP3RadioChannelAvailable(cSampleManager *manager) {assert(manager==&SampleManager);uint8_t result=mp3;value(result);event(17);return result;}
uint8_t DMAudio_IsMP3RadioChannelAvailable(void) {return SampleManager_IsMP3RadioChannelAvailable(&SampleManager);}
struct CPad {
    static CPad *GetPad(int32_t index){assert(index==0);event(18);static CPad pad;return &pad;}
    bool ChangeStationJustDown(){bool result=controls[0]!=0;value(result);event(19);return result;}
    bool ArePlayerControlsDisabled(){bool result=controls[1]!=0;value(result);event(20);return result;}
    bool GetMouseWheelDownJustDown(){bool result=controls[2]!=0;value(result);event(21);return result;}
    bool GetMouseWheelUpJustDown(){bool result=controls[3]!=0;value(result);event(22);return result;}
};
@CONTROL_ENUMS@
struct ControlManager {
    bool GetIsKeyboardKeyJustDown(RsKeyCodes key){assert(key==rsF9);bool result=controls[4]!=0;value((uint32_t)key);value(result);event(23);return result;}
    int32_t GetControllerKeyAssociatedWithAction(e_ControllerAction action,eControllerType type){assert(action==VEHICLE_CHANGE_RADIO_STATION&&type==MOUSE);int32_t result=mouseKey;value((uint32_t)result);event(24);return result;}
    bool IsAnyVehicleActionAssignedToMouseKey(int32_t key){assert(key==rsMOUSEWHEELUPBUTTON||key==rsMOUSEWHEELDOWNBUTTON);bool result=controls[5]!=0;value((uint32_t)key);value(result);event(25);return result;}
};
static ControlManager ControlsManager;
struct CGame {static int32_t &currArea;};
int32_t &CGame::currArea=area;
extern "C" uint32_t AudioPlayerEngineHost_Time(void) {uint32_t result=gameTick;gameTick+=101u;value(result);event(26);return result;}
extern "C" void AudioPolice_Crackle(cAudioManager *manager) {assert(manager==&AudioManager);event(27);}
@CONTROL_HOSTS@
'''
common=common.replace('struct CTimer {static uint32_t GetTimeInMillisecondsPauseMode()', 'struct CTimer {static uint32_t GetTimeInMilliseconds(){return AudioPlayerEngineHost_Time();}static uint32_t GetTimeInMillisecondsPauseMode()')
common=common.replace('struct CTimer {','extern "C" uint32_t AudioPlayerEngineHost_Time(void);\nstruct CTimer {',1)
common=common.replace('@METHODS@',extra+'\n@METHODS@')
main=common.index('int main()');common=common[:main]+r'''int main() {
    // Verify original scalar initialization and timer reset before initialization/disabled gates
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);assert(RadioStaticCounter==5);
    active=&MusicManager;memset(active,0,sizeof(*active));active->m_bDisabled=255;active->m_bResetTimers=255;active->m_nResetTime=UINT32_MAX;
    unsigned initialCallbacks=callbacks;run(0);assert(!active->m_bResetTimers&&active->m_nLastTrackServiceTime==4294967296.0f&&callbacks==initialCallbacks);
    const uint32_t tracks[]={0,NUM_RADIOS-1,STREAMED_SOUND_RADIO_MP3_PLAYER,STREAMED_SOUND_CITY_AMBIENT,STREAMED_SOUND_HAVANA_CITY_AMBIENT,STREAMED_SOUND_MALL_AMBIENT};
    const int32_t areas[]={AREA_MAIN_MAP,AREA_HOTEL,AREA_MALL,AREA_STRIP_CLUB,AREA_DIRT,AREA_MALIBU_CLUB,AREA_MANSION};
    const int16_t models[]={MI_POLICE,MI_MRWHOOP,MI_KAUFMAN,MI_POLMAV};
    for(seed=0;seed<1024;seed++)for(pass=0;pass<3;pass++)for(operation=0;operation<2;operation++) {
        memset(&MusicManager,0xA5,sizeof(MusicManager));memset(&other,0x5A,sizeof(other));memset(&AudioManager,0,sizeof(AudioManager));memset(&SampleManager,0,sizeof(SampleManager));
        active=(seed+pass)%2?&other:&MusicManager;mainPed=(seed%5==0)?nullptr:&peds[seed%2];vehicle=(seed%4==0)?nullptr:&vehicles[seed%2];
        for(unsigned i=0;i<2;i++){peds[i].m_nPedState=(seed%7==0)?PED_DEAD:PED_DRIVING;vehicles[i].status=(seed%6==0)?STATUS_WRECKED:STATUS_PLAYER;vehicles[i].model=models[(seed/4+i)%4];vehicles[i].m_nRadioStation=(uint8_t)(seed+i);vehicles[i].siren=(seed%2)?255:0;}
        active->m_bIsInitialised=(seed%3)?255:0;active->m_bDisabled=(seed%7==0)?255:0;active->m_bResetTimers=(seed%2)?255:0;active->m_nResetTime=seed*1234567u;
        active->m_nMusicMode=pass==0?(uint8_t)seed:(uint8_t)(seed%6);active->m_nUpcomingMusicMode=(seed%3)?active->m_nMusicMode:(uint8_t)((seed+1)%6);
        active->m_nMusicModeToBeSet=(uint8_t)(seed/3);active->m_bMusicModeChangeStarted=pass==2?0:((seed%2)?255:0);
        active->m_nPlayingTrack=seed%7==0?NO_TRACK:tracks[(seed+pass)%6];active->m_nFrontendTrack=tracks[(seed/6+pass)%6];active->m_nNextTrack=tracks[(seed/36+pass)%6];
        active->m_bTrackChangeStarted=(seed%2)?255:0;active->m_bVerifyNextTrackStartedToPlay=(seed%3)?0:255;
        active->m_bAnnouncementInProgress=(seed%5==0)?255:0;active->m_nAnnouncement=(seed%3)?seed%TOTAL_STREAMED_SOUNDS:NO_TRACK;
        // Avoid the original out-of-bounds next-track path when announcement cleanup clears the sentinel
        if(active->m_bAnnouncementInProgress)active->m_bTrackChangeStarted=0;
        active->m_bGameplayAllowsRadio=(seed%2)?255:0;active->m_bRadioStreamReady=255;active->m_bUserResumedGame=(seed%8==0)?255:0;active->m_bEarlyFrontendTrack=(seed%2)?255:0;
        active->m_bRadioSetByScript=(seed%3==1)?255:0;active->m_nRadioStationScript=(uint8_t)seed;active->m_nRadioPosition=seed%3==0?-1:(seed%3==1?INT_MIN:(int32_t)(seed*127u));
        active->m_nVolumeLatency=(uint8_t)(seed%12);active->m_nCurrentVolume=(uint8_t)seed;active->m_nMaxVolume=(uint8_t)(seed+7);active->nFramesSinceCutsceneEnded=(int8_t)seed;active->m_nLastTrackServiceTime=-1.0f;
        for(unsigned i=0;i<TOTAL_STREAMED_SOUNDS;i++){active->m_aTracks[i].m_nLength=seed%2?17:UINT32_MAX;active->m_aTracks[i].m_nPosition=seed*123u+i;active->m_aTracks[i].m_nLastPosCheckTimer=seed*77u;}
        for(unsigned i=0;i<NUM_RADIOS;i++)active->aListenTimeArray[i]=16777216.0f+(float)i;
        AudioManager.m_FrameCounter=pass==2?0:seed%8;AudioManager.m_bIsInitialised=255;AudioManager.m_bIsPaused=(seed%2)?255:0;AudioManager.m_bWasPaused=(seed%3==0)?255:0;AudioManager.m_nFrontEndEntity=(int32_t)seed;
        SampleManager.m_nMusicVolume=(seed%3)?255:0;AudioManager.m_anRandomTable[2]=(seed%2)?INT_MIN:-1;
        for(unsigned i=0;i<MISSION_AUDIO_SLOTS;i++){AudioManager.m_nMissionAudioPlayStatus[i]=(seed%3==i)?PLAY_STATUS_PLAYING:PLAY_STATUS_STOPPED;AudioManager.m_nMissionAudioSampleIndex[i]=(seed%5)?0:STREAMED_SOUND_MISSION_ROK2_01;}
        for(unsigned i=0;i<6;i++)controls[i]=(seed&(1u<<i))?255:0;
        area=areas[(seed/3)%7];mouseKey=seed%3==0?rsMOUSEWHEELUPBUTTON:(seed%3==1?rsMOUSEWHEELDOWNBUTTON:-1);mp3=(seed%3)?0:255;replay=(seed%2)?255:0;
        gNumRetunePresses=(int32_t)(seed%11)-5;gRetuneCounter=seed%4==0?0:(int32_t)(seed%21);RadioStaticCounter=(uint8_t)(seed%6);RadioStaticTimer=seed*1001u;g_bAnnouncementReadPosAlready=(seed%2)?255:0;
        slow=(seed%2)?255:0;TheCamera.position=CVector(0,0,0);TheCamera.pTargetEntity=(seed%5==0)?nullptr:&targets[seed%2];targets[0].position=CVector(0,0,20);targets[1].position=CVector(10,0,0);
        cameraMatrix={1,0,0,0,1,0,0,0,1,0,0,0};remaining=(seed+pass)%5;checks=stops=positions=timers=starts=volumes=0;lastVolume=lastPan=lastEffects=0;playerFinds=vehicleFinds=0;
        mutation=pass==1;pauseTick=seed*1234567u;gameTick=seed%3==0?UINT32_MAX:seed*1200u;snapshot();run(operation);snapshot();groups++;
        if(pass==2){run(operation);snapshot();}
        assert(checks<20&&stops<12);
    }
    printf("groups=%u callbacks=%u hash=%016llx\n",groups,callbacks,(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    # Link all converted child services while comparing complete original dispatcher/gameplay bodies
    if variant=='before':record=before_record;methods=state.CTOR+'\n'+'\n'.join(m.function(old,'cMusicManager::',n) for n in m.METHODS);dispatch='if(op==0)active->Service();else active->ServiceGameMode();return 0;';globals='extern "C" {\n'+m.GLOBALS+'}\n'
    else:record='#include "MusicManager.h"';methods=state.CTOR;dispatch='TestMusicServiceC(active,op);return 0;';globals=''
    text=common.replace('@HEADERS@',headers).replace('@MUSIC@',record).replace('@PROTOTYPES@',prototypes).replace('@AUDIO@',fields((root/'src/audio/AudioManager.h').read_text())).replace('@CONTROL_ENUMS@',enums).replace('@CONTROL_HOSTS@',m.HOST).replace('@METHODS@',methods).replace('@RUN@',dispatch).replace('@GLOBALS@',globals)
    (out/(variant+'.cpp')).write_text(text,newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
void TestMusicServiceC(cMusicManager *manager,unsigned op)
{
    // Enter each complete C music service with the original owner
    if(op==0)MusicService_Run(manager);else MusicService_GameMode(manager);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape in ['default','vanilla','no-external','ps2']:(out/(shape+'.h')).write_bytes((root/'build/audio-music-stream-c-tests'/(shape+'.h')).read_bytes())
(out/'no-previous-station.h').write_bytes((root/'build/audio-music-radio-c-tests/no-previous-station.h').read_bytes())
print('Both complete music service bodies, four C globals, direct audio entry, removed wrapper, exact shared key enum, seven control readers and unchanged records audited')
