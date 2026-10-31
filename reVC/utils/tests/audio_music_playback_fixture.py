import weather_migration as _weather_conversion
"""Audit complete playback operations and generate C++/C provider callback comparisons."""
from pathlib import Path
import re
import audio_music_playback_migration as m
import audio_music_state_migration as state
from audio_owner_init_migration import fields
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-playback-c-tests'
def normalize(source):
    # Apply the explicit verified weather rename before comparing every original statement
    source=_weather_conversion.calls(source)
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text();current=(root/'src/audio/MusicManager.h').read_text();kernel=(root/'src/audio/MusicPlayback.c').read_text()
for name,transform in [('src/audio/MusicManager.cpp',m.logic),('src/audio/MusicManager.h',m.header),('src/audio/AudioManager.cpp',m.logic),('src/audio/AudioManager.h',m.header),('src/audio/AudioLifecycle.c',m.lifecycle),('src/audio/DMAudio.cpp',m.logic),('build/reVC.vcxproj',m.project),('build/reVC.vcxproj.filters',m.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(state.fields(oldh))==normalize(state.fields(current))
for name in m.METHODS:
    original=m.function(old,'cMusicManager::',name);actual=m.function(kernel,'MusicPlayback_',name)
    assert normalize(m.body(original))==normalize(actual[actual.index('{'):]),name
facade=(out/'before/src/audio/DMAudio.cpp').read_text()
for name in m.FACADES:
    original=m.function(facade,'DMAudio_',name);actual=m.function(kernel,'DMAudio_',name)
    assert normalize(m.calls(original).replace('uint32 ','uint32_t '))==normalize(actual),name
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','AudioPoliceState.h','sampman.h','DMAudio.h','Validate.h'])
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
record=state.fields(oldh)
common='''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
static cMusicManager *active;
static uint64_t digest=1469598103934665603ULL;
static unsigned groups,callbacks,seed,mutation,remaining,checks,stops;
static uint32_t capturedTrack;
extern "C" uint64_t TestAircraftHash(uint64_t,const void*,uint32_t);
extern "C" void AudioPolice_Reset(cAudioManager*);
extern "C" void MusicState_InitState(cMusicManager*);
extern "C" uint8_t MusicState_IsInitialised(const cMusicManager*);
extern "C" void TestPlaybackC(cMusicManager*,unsigned,uint32_t);
@METHODS@
static void value(uint32_t v) {
    // Record scalar arguments and captured provider results in order
    digest=TestAircraftHash(digest,&v,sizeof(v));
}
static void snapshot(void) {
    // Observe complete owners, provider storage and police globals at every callback boundary
    digest=TestAircraftHash(digest,&MusicManager,sizeof(MusicManager));
    digest=TestAircraftHash(digest,&other,sizeof(other));
    digest=TestAircraftHash(digest,&AudioManager,sizeof(AudioManager));
    digest=TestAircraftHash(digest,&SampleManager,sizeof(SampleManager));
    digest=TestAircraftHash(digest,ZoneSfx,sizeof(ZoneSfx));
    digest=TestAircraftHash(digest,gMinTimeToNextReport,sizeof(gMinTimeToNextReport));
    value(gSpecialSuspectLastSeenReport);value(g_nMissionAudioSfx);value((uint8_t)g_nMissionAudioPlayingStatus);
    value(remaining);value(checks);value(stops);value(capturedTrack);
}
static void event(unsigned token) {
    // Mutate live state after observing the callback without changing captured arguments
    callbacks++;value(token);snapshot();
    SampleManager.m_nMusicVolume=(uint8_t)(SampleManager.m_nMusicVolume+token);
    AudioManager.m_FrameCounter+=token;
    if(mutation) {
        active->m_nPlayingTrack^=seed+token;
        active->m_bIsInitialised=(uint8_t)((seed+token)%3==0?0:255);
        active->m_bDisabled=(uint8_t)((seed+token)%2);
        active->m_nMusicMode=(uint8_t)((seed+token)%6);
        capturedTrack+=token;
    }
}
extern "C" uint8_t SampleManager_GetChannelUsedFlag(cSampleManager *manager,uint32_t channel) {
    // Preserve the captured police-channel gate while exposing subsequent state mutation
    assert(manager==&SampleManager);value(channel);uint8_t result=(seed%3==0?0:255);value(result);event(1);return result;
}
extern "C" void SampleManager_StopChannel(cSampleManager *manager,uint32_t channel) {
    // Observe police reset before stream polling starts
    assert(manager==&SampleManager);value(channel);event(2);
}
extern "C" void SampleManager_SetChannelReverbFlag(cSampleManager *manager,uint32_t channel,uint8_t enabled) {
    // Observe the existing conditional police initialization callback and frame capture
    assert(manager==&SampleManager);value(channel);value(enabled);event(3);
}
extern "C" uint8_t SampleManager_IsStreamPlaying(cSampleManager *manager,uint8_t stream) {
    // Capture the raw byte result before mutating state for the next loop iteration
    assert(manager==&SampleManager);assert(stream==0);uint8_t result=(remaining?((seed%2)?1:255):0);checks++;value(result);event(4);return result;
}
extern "C" void SampleManager_StopStreamedFile(cSampleManager *manager,uint8_t stream) {
    // Delay stop acknowledgement across bounded repeated queries as a backend may do
    assert(manager==&SampleManager);assert(stream==0);stops++;event(5);if(remaining)remaining--;
}
extern "C" void SampleManager_PreloadStreamedFile(cSampleManager *manager,uint32_t track,uint8_t stream) {
    // Observe the originally captured track despite live mutations during police reset and polling
    assert(manager==&SampleManager);assert(stream==0);value(track);event(6);
}
extern "C" void SampleManager_SetStreamedVolumeAndPan(cSampleManager *manager,uint8_t volume,uint8_t pan,uint8_t effects,uint8_t stream) {
    // Observe exact preload volume, pan, effect flag and stream arguments before track assignment
    assert(manager==&SampleManager);value(volume);value(pan);value(effects);value(stream);event(7);
}
extern "C" void SampleManager_StartPreloadedStreamedFile(cSampleManager *manager,uint8_t stream) {
    // Observe the gated start callback without launching an audio device
    assert(manager==&SampleManager);assert(stream==0);event(8);
}
static void run(unsigned op,uint32_t track) {
    // Execute complete original operations and facades or their actual C counterparts
    @RUN@
}
int main() {
    // Seed zero padding and retained music, police and provider state for repeatable comparisons
    static const uint32_t tracks[]={0,1,TOTAL_STREAMED_SOUNDS-1,TOTAL_STREAMED_SOUNDS,UINT32_MAX,STREAMED_SOUND_MISSION_COMPLETED,STREAMED_SOUND_CUTSCENE_FINALE};
    for(seed=0;seed<1024;seed++) {
        memset((void*)&MusicManager,seed&255,sizeof(MusicManager));
        memset((void*)&other,(seed+1)&255,sizeof(other));
        memset(&AudioManager,0,sizeof(AudioManager));memset(&SampleManager,0,sizeof(SampleManager));
        memset(ZoneSfx,0,sizeof(ZoneSfx));memset(gMinTimeToNextReport,0,sizeof(gMinTimeToNextReport));
        gSpecialSuspectLastSeenReport=(uint8_t)seed;g_nMissionAudioSfx=seed;g_nMissionAudioPlayingStatus=(int8_t)(seed%4);
        AudioManager.m_bIsInitialised=(uint8_t)(seed%3?255:0);
        AudioManager.m_FrameCounter=UINT32_MAX-seed;
        AudioManager.m_sPoliceRadioQueue.m_nSamplesInQueue=(uint8_t)(seed%POLICE_RADIO_QUEUE_MAX_SAMPLES);
        for(unsigned i=0;i<AUDIO_CRIME_COUNT;i++)AudioManager.m_aCrimes[i].type=(int32_t)(seed+i);
        for(unsigned pass=0;pass<3;pass++) {
            // Mix closed gates, active cutscene playback and live mutation across both owners
            for(unsigned op=0;op<7;op++) {
                active=(op>=4?&MusicManager:((seed+op+pass)%2?&other:&MusicManager));
                active->m_bIsInitialised=(uint8_t)(pass==2?1:seed%3?255:0);
                active->m_bDisabled=(uint8_t)(pass==2?0:seed%4?0:255);
                active->m_nMusicMode=(uint8_t)(pass==2?MUSICMODE_CUTSCENE:seed%6);
                active->m_nPlayingTrack=tracks[(seed+pass+op)%7];
                remaining=(seed+op+pass)%5;checks=stops=0;mutation=(pass==1);
                capturedTrack=tracks[(seed+op)%7];
                run(op,capturedTrack);snapshot();assert(checks<=5);assert(stops<=4);groups++;
            }
        }
    }
    printf("%u groups %u callbacks %llu\\n",groups,callbacks,(unsigned long long)digest);return 0;
}
'''.replace('@HEADERS@',headers).replace('@AUDIO@',fields((root/'src/audio/AudioManager.h').read_text()))
original_methods='\n'.join(m.function(old,'cMusicManager::',name) for name in m.METHODS)
original_facades='\n'.join(m.function(facade,'DMAudio_',name) for name in m.FACADES)
for variant in ['before','after']:
    if variant=='before':
        decl=track+'\nstruct cMusicManager {\n'+record+'\ncMusicManager();\nvoid Terminate();void PreloadCutSceneMusic(uint32 track);void PlayPreloadedCutSceneMusic(void);void StopCutSceneMusic(void);\n};'
        methods=state.CTOR+'\n'+original_methods+'\n'+original_facades
        run='switch(op) {case 0:active->Terminate();break;case 1:active->PreloadCutSceneMusic(track);break;case 2:active->PlayPreloadedCutSceneMusic();break;case 3:active->StopCutSceneMusic();break;case 4:DMAudio_PreloadCutSceneMusic(track);break;case 5:DMAudio_PlayPreloadedCutSceneMusic();break;case 6:DMAudio_StopCutSceneMusic();break;}'
    else:decl='#include "MusicManager.h"';methods=state.CTOR;run='TestPlaybackC(active,op,track);'
    (out/(variant+'.cpp')).write_text(common.replace('@MUSIC@',decl).replace('@METHODS@',methods).replace('@RUN@',run),newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
#include "DMAudio.h"
void TestPlaybackC(cMusicManager *manager,unsigned op,uint32_t track)
{
    // Enter every complete C operation and public cutscene facade through its production declaration
    switch(op) {
    case 0:MusicPlayback_Terminate(manager);break;
    case 1:MusicPlayback_PreloadCutSceneMusic(manager,track);break;
    case 2:MusicPlayback_PlayPreloadedCutSceneMusic(manager);break;
    case 3:MusicPlayback_StopCutSceneMusic(manager);break;
    case 4:DMAudio_PreloadCutSceneMusic(track);break;
    case 5:DMAudio_PlayPreloadedCutSceneMusic();break;
    case 6:DMAudio_StopCutSceneMusic();break;
    }
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','ps2-no-reverb':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n#undef AUDIO_REVERB\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Four complete playback operations, three facades, direct lifecycle cleanup, all caller/header/project edits and unchanged music fields audited')
