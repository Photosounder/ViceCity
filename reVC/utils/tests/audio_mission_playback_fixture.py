"""Audit mission playback statements and compare original/C callback-visible traces."""
from pathlib import Path
import re
import audio_police_state_migration as police
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_mission_playback_migration as playback
root=Path(__file__).resolve().parents[2];out=root/'build/audio-mission-playback-c-tests'
old=(out/'before/src/audio/AudioLogic.cpp').read_text();new=(root/'src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioMissionPlayback.c').read_text()
def normalize(s):
    # Compare all actual statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
for name,target in [('ProcessMissionAudioSlot','ProcessSlot'),('ProcessMissionAudio','Process')]:
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioMission_',target)
    assert normalize(playback.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(police.logic(playback.logic(old)))==normalize(new)
for file,transform in [('AudioManager.cpp',playback.calls),('AudioManager.h',playback.header)]:
    assert normalize((police.header if file=='AudioManager.h' else police.calls)(transform((out/'before/src/audio'/file).read_text())))==normalize((root/'src/audio'/file).read_text()),file
for name in ('MISSION_AUDIO_MAX_DIST','MISSION_AUDIO_VOLUME'):
    assert re.search(name+r'\s*=\s*80\b',old) and re.search(name+r'\s*=\s*80\b',kernel)
assert normalize((root/'src/math/Maths.h').read_text().split('inline float Sqrt(float x)')[1].split('\n')[0])==normalize('{ return sqrtf(x); }')
record=fields((root/'src/audio/AudioManager.h').read_text())
includes='\n'.join('#include "'+n+'"' for n in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
common=includes+"""
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <initializer_list>
using uint8=uint8_t;using uint32=uint32_t;using int8=int8_t;
#define TRUE 1
#define FALSE 0
#define Const const
#define SQR(x) ((x)*(x))
inline float Sqrt(float x) {return sqrtf(x);}
enum {MISSION_AUDIO_MAX_DIST=80, MISSION_AUDIO_VOLUME=80};
struct CVector {float x,y,z; CVector() {} CVector(float a,float b,float c):x(a),y(b),z(c) {}};
static inline CVector AudioSoundPosition_ToVector(const AudioSoundPosition *p) {return CVector(p->x,p->y,p->z);}
static inline AudioSoundPosition AudioSoundPosition_FromVector(const CVector &p) {return {p.x,p.y,p.z};}
struct cAudioManager {
"""+record+"""
void ProcessMissionAudioSlot(uint8);
void ProcessMissionAudio();
float GetDistanceSquared(const CVector &);
void TranslateEntity(const CVector *,CVector *);
void SetMissionScriptPoliceAudio(uint32);
int8 GetMissionScriptPoliceAudioPlayingStatus();
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
cAudioManager other;
cSampleManager SampleManager;
extern "C" uint8_t g_bMissionAudioLoadFailed[MISSION_AUDIO_SLOTS];
uint8_t g_bMissionAudioLoadFailed[MISSION_AUDIO_SLOTS];
extern "C" void AudioMission_Clear(cAudioManager *,uint8_t);
extern "C" uint8_t AudioMission_UsesPoliceChannel(uint32_t);
extern "C" void AudioMission_ProcessSlot(cAudioManager *,uint8_t);
extern "C" void AudioMission_Process(cAudioManager *);
extern "C" uint8_t AudioMath_ComputeVolume(uint8_t,float,float);
extern "C" int32_t AudioMath_ComputePan(float,float);
extern "C" float AudioGeometry_DistanceSquared(const AudioSoundPosition *);
extern "C" void AudioGeometry_Translate(const AudioSoundPosition *,AudioSoundPosition *);
extern "C" void AudioMissionHost_SetPolice(cAudioManager *,uint32_t);
extern "C" int8_t AudioMissionHost_PoliceStatus(cAudioManager *);
#include "AudioPoliceState.h"
static uint64_t digest=1469598103934665603ULL;
static cAudioManager *active;
static float distanceSquared,panX;
static uint8_t streamPlaying;
static void value(uint32_t v) {
    // Observe backend arguments and exact live state during callbacks
    digest=(digest^v)*1099511628211ULL;
}
static void real(float v) {
    // Hash defined floating point bits without struct padding
    uint32_t bits;memcpy(&bits,&v,4);value(bits);
}
static void snapshot(cAudioManager *m) {
    // Observe both slots and fields which participate in mission transitions
    value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_bIsPaused);value(m->m_bWasPaused);
    value(m->m_bTimerJustReset);value(m->m_nGlobalSfxVolumeMultiplier);
    for(unsigned i=0;i<MISSION_AUDIO_SLOTS;++i) {
        // Retain coordinates, phone flags, and the untouched other slot
        value(m->m_nMissionAudioSampleIndex[i]);value(m->m_nMissionAudioLoadingStatus[i]);
        value(m->m_nMissionAudioPlayStatus[i]);value(m->m_bIsMissionAudioPlaying[i]);
        value(m->m_nMissionAudioFramesToPlay[i]);value(m->m_bIsMissionAudioAllowedToPlay[i]);
        value(m->m_bIsMissionAudio2D[i]);value(m->m_bIsMissionAudioPhoneCall[i]);value(g_bMissionAudioLoadFailed[i]);
        real(m->m_vecMissionAudioPosition[i].x);real(m->m_vecMissionAudioPosition[i].y);real(m->m_vecMissionAudioPosition[i].z);
    }
}
static void callback(cSampleManager *m,uint32_t event,uint8_t stream) {
    // Record backend order and the partial owner state visible to each operation
    assert(m==&SampleManager);value(event);value(stream);snapshot(active);
}
void SampleManager_PreloadStreamedFile(cSampleManager *m,uint32_t file,uint8_t stream) {
    // Observe the full preload ID without narrowing it to the duration API's byte
    value(file);callback(m,1,stream);
}
void SampleManager_StopStreamedFile(cSampleManager *m,uint8_t stream) {
    // Capture every stop including the original duplicate reset stops
    callback(m,2,stream);
}
void SampleManager_PauseStream(cSampleManager *m,uint8_t pause,uint8_t stream) {
    // Compare pause and resume ordering
    value(pause);callback(m,3,stream);
}
void SampleManager_StartPreloadedStreamedFile(cSampleManager *m,uint8_t stream) {
    // Observe state before the transition to playing
    callback(m,4,stream);
}
void SampleManager_SetStreamedVolumeAndPan(cSampleManager *m,uint8_t volume,uint8_t pan,uint8_t effect,uint8_t stream) {
    // Compare positional attenuation, stereo pan, and effect arguments
    value(volume);value(pan);value(effect);callback(m,5,stream);
}
uint8_t SampleManager_IsStreamPlaying(cSampleManager *m,uint8_t stream) {
    // Return a controlled stream status at the original query point
    callback(m,6,stream);return streamPlaying;
}
uint8_t SampleManager_GetChannelUsedFlag(cSampleManager *m,uint32_t channel) {
    // Retain a controlled backend boundary for the C police module linked by this fixture
    value(channel);callback(m,11,0);return 0;
}
void SampleManager_StopChannel(cSampleManager *m,uint32_t channel) {
    // Observe any police reset channel stop if a fixture invokes it
    value(channel);callback(m,12,0);
}
void SampleManager_SetChannelReverbFlag(cSampleManager *m,uint32_t channel,uint8_t flag) {
    // Keep the actual police initializer linkable without an audio device
    value(channel);value(flag);callback(m,13,0);
}
float cAudioManager::GetDistanceSquared(const CVector &v) {
    // Drive distance branches while recording the supplied game coordinates
    value(7);real(v.x);real(v.y);real(v.z);snapshot(this);return distanceSquared;
}
void cAudioManager::TranslateEntity(const CVector *in,CVector *out) {
    // Supply bounded camera-space pan while observing the copied input coordinates
    value(8);real(in->x);real(in->y);real(in->z);snapshot(this);*out=CVector(panX,3,-7);
}
void cAudioManager::SetMissionScriptPoliceAudio(uint32 sfx) {
    // Preserve the host shape even though the production police policy returns false
    value(9);value(sfx);snapshot(this);
}
int8 cAudioManager::GetMissionScriptPoliceAudioPlayingStatus() {
    // Preserve the unreachable production policy branch's return type
    value(10);snapshot(this);return PLAY_STATUS_FINISHED;
}
"""
trace=(root/'utils/tests/audio_mission_playback.cpp.in').read_text()
methods='\n'.join(function(old,'cAudioManager::',name) for name in ('ProcessMissionAudioSlot','ProcessMissionAudio'))
for variant in ('before','after'):
    bridges="""
extern "C" void AudioMission_ProcessSlot(cAudioManager *m,uint8_t slot) {
    // Drive the original member from the same C caller used for the converted module
    m->ProcessMissionAudioSlot(slot);
}
extern "C" void AudioMission_Process(cAudioManager *m) {
    // Drive the original ordered service from the C ABI comparison caller
    m->ProcessMissionAudio();
}
""" if variant=='before' else ''
    controlled=playback.HOST
    for name in ('SetPolice','PoliceStatus'):
        controlled=controlled.replace(function(controlled,'AudioMissionHost_',name)+'\n','')
    controlled=controlled.replace('AudioMissionHost_GetDistanceSquared(cAudioManager *manager, const AudioSoundPosition *position)', 'AudioGeometry_DistanceSquared(const AudioSoundPosition *position)').replace('AudioMissionHost_Translate(cAudioManager *manager, const AudioSoundPosition *position, AudioSoundPosition *translated)', 'AudioGeometry_Translate(const AudioSoundPosition *position, AudioSoundPosition *translated)').replace('manager->GetDistanceSquared(', 'active->GetDistanceSquared(').replace('manager->TranslateEntity(', 'active->TranslateEntity(')
    source=common+(methods if variant=='before' else controlled)+bridges+trace
    source=source.replace('@SLOT@','m->ProcessMissionAudioSlot(slot)' if variant=='before' else 'AudioMission_ProcessSlot(m,slot)').replace('@PROCESS@','m->ProcessMissionAudio()' if variant=='before' else 'AudioMission_Process(m)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Both playback bodies, four host adapters, full owner/header/caller edits, and constants audited')

(out/'caller.c').write_text("""#include "AudioManager.h"
#include <assert.h>
void TestMissionPlaybackC(void) {
    // Exercise the live owner and both playback operations from an actual C caller
    AudioManager.m_bIsInitialised=0;
    AudioMission_Process(&AudioManager);
    AudioManager.m_bIsInitialised=1;
    AudioManager.m_nMissionAudioSampleIndex[0]=NO_SAMPLE;
    AudioManager.m_nMissionAudioSampleIndex[1]=NO_SAMPLE;
    AudioManager.m_nGlobalSfxVolumeMultiplier=80;
    AudioMission_ProcessSlot(&AudioManager,0);
    AudioMission_Process(&AudioManager);
    assert(AudioManager.m_nGlobalSfxVolumeMultiplier==85);
    AudioManager.m_bIsMissionAudioPhoneCall[1]=255;
    AudioMission_Process(&AudioManager);
    assert(AudioManager.m_nGlobalSfxVolumeMultiplier==64);
}
""",newline='\n')
