"""Compare police state, zone initialization, and reset callback order with saved members."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_police_state_migration as police
root=Path(__file__).resolve().parents[2];out=root/'build/audio-police-state-c-tests'
old=(out/'before/src/audio/PolRadio.cpp').read_text();kernel=(root/'src/audio/AudioPoliceState.c').read_text()
def normalize(s):
    # Compare all statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
for name,target in police.MAP.items():
    expected=police.body(function(old,'cAudioManager::',name),name);actual=function(kernel,'AudioPolice_',target)
    assert normalize(expected)==normalize(actual[actual.index('{'):]),name
storage=police.STORAGE;structure=storage[:storage.index('\n\n')]
expected=structure.replace('struct tPoliceRadioZone {','typedef struct tPoliceRadioZone {').replace('uint32 ','uint32_t ').replace('int32 ','int32_t ').replace('};','} tPoliceRadioZone;')
h=(root/'src/audio/AudioPoliceState.h').read_text();a=h.index('typedef struct');b=h.index('#ifdef __cplusplus',a)
assert normalize(expected)==normalize(h[a:b])
expected=storage[storage.index('tPoliceRadioZone ZoneSfx'):].replace('uint32 ','uint32_t ').replace('int8 ','int8_t ').replace('bool8 ','uint8_t ')
a=kernel.index('tPoliceRadioZone ZoneSfx');b=kernel.index('void\nAudioPolice_InitZones',a)
assert normalize(expected)==normalize(kernel[a:b])
for file,transform in [('PolRadio.cpp',police.radio),('AudioLogic.cpp',police.logic),('AudioManager.cpp',police.calls),('AudioManager.h',police.header),('AudioMissionPlayback.c',police.playback),('DMAudio.cpp',police.facade),('MusicManager.cpp',police.calls)]:
    assert normalize(transform((out/'before/src/audio'/file).read_text()))==normalize((root/'src/audio'/file).read_text()),file
assert normalize((out/'before/src/audio/PolRadio.h').read_text().replace('#include "AudioCrimes.h"','#include "AudioCrimes.h"\n#include "AudioPoliceState.h"'))==normalize((root/'src/audio/PolRadio.h').read_text())
expected=police.calls(function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','ResetPoliceRadio')).replace('DMAudio_ResetPoliceRadio()','DMAudio_ResetPoliceRadio(void)')
assert normalize(expected)==normalize(function(kernel,'DMAudio_','ResetPoliceRadio'))
record=fields((root/'src/audio/AudioManager.h').read_text())
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','AudioPoliceState.h','DMAudio.h','sampman.h'])
common=includes+"""
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <initializer_list>
using uint32=uint32_t;using int32=int32_t;using uint8=uint8_t;using int8=int8_t;using bool8=uint8_t;
#define FALSE 0
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
struct cAudioManager {
"""+record+"""
void InitialisePoliceRadioZones();
void InitialisePoliceRadio();
void ResetPoliceRadio();
void SetMissionScriptPoliceAudio(uint32);
int8 GetMissionScriptPoliceAudioPlayingStatus();
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
cAudioManager other;
cSampleManager SampleManager;
extern "C" void AudioPolice_Init(cAudioManager *);
extern "C" void AudioPolice_Reset(cAudioManager *);
static cAudioManager *active;
static uint8_t channelUsed;
static uint64_t digest=1469598103934665603ULL;
static void value(uint32_t v) {
    // Hash scalar values without padding or pointer representations
    digest=(digest^v)*1099511628211ULL;
}
static void real(float v) {
    // Compare retained crime coordinates using defined float bits
    uint32_t bits;memcpy(&bits,&v,4);value(bits);
}
static void snapshot(cAudioManager *m) {
    // Compare global state, queue storage, and all crime fields at every backend boundary
    value(g_nMissionAudioSfx);value((int32_t)g_nMissionAudioPlayingStatus);value(gSpecialSuspectLastSeenReport);
    for(unsigned i=0;i<NUM_CRIME_TYPES;++i) {value(gMinTimeToNextReport[i]);}
    for(unsigned i=0;i<NUMAUDIOZONES;++i) {
        // Retain the unused zone field and complete eight-byte names
        for(unsigned j=0;j<8;++j) {value((uint8_t)ZoneSfx[i].m_aName[j]);}
        value(ZoneSfx[i].m_nSampleIndex);value(ZoneSfx[i].field_12);
    }
    if(m) {
        // Detect unrelated resets of queue samples, crime positions, timers, and owner gates
        value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_FrameCounter);
        value(m->m_sPoliceRadioQueue.m_nSamplesInQueue);value(m->m_sPoliceRadioQueue.m_nAddOffset);value(m->m_sPoliceRadioQueue.m_nRemoveOffset);
        for(unsigned i=0;i<POLICE_RADIO_QUEUE_MAX_SAMPLES;++i) {value(m->m_sPoliceRadioQueue.m_aSamples[i]);}
        for(unsigned i=0;i<AUDIO_CRIME_COUNT;++i) {
            // Observe the original type-only crime reset
            value(m->m_aCrimes[i].type);value(m->m_aCrimes[i].timer);
            real(m->m_aCrimes[i].position.x);real(m->m_aCrimes[i].position.y);real(m->m_aCrimes[i].position.z);
        }
    }
}
uint8_t SampleManager_GetChannelUsedFlag(cSampleManager *m,uint32_t channel) {
    // Observe the usage query before any reset writes
    assert(m==&SampleManager);value(1);value(channel);snapshot(active);return channelUsed;
}
void SampleManager_StopChannel(cSampleManager *m,uint32_t channel) {
    // Observe channel stop before queue and crime initialization
    assert(m==&SampleManager);value(2);value(channel);snapshot(active);
}
void SampleManager_SetChannelReverbFlag(cSampleManager *m,uint32_t channel,uint8_t reverb) {
    // Observe the partial reset state before report history is cleared
    assert(m==&SampleManager);value(3);value(channel);value(reverb);snapshot(active);
}
"""
methods='\n'.join(function(old,'cAudioManager::',name) for name in police.MAP)
facade=function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','ResetPoliceRadio')
bridges="""
extern "C" void AudioPolice_InitZones(void) {
    // Drive the original initializer through the same C ABI test caller
    AudioManager.InitialisePoliceRadioZones();
}
extern "C" void AudioPolice_Init(cAudioManager *m) {
    // Retain owner identity in the baseline initialization bridge
    m->InitialisePoliceRadio();
}
extern "C" void AudioPolice_Reset(cAudioManager *m) {
    // Retain owner identity in the baseline reset bridge
    m->ResetPoliceRadio();
}
extern "C" void AudioPolice_SetMission(uint8_t initialized,uint32_t sfx) {
    // Supply only the gate used by the original setter without mutating the live owner
    cAudioManager m;m.m_bIsInitialised=initialized;m.SetMissionScriptPoliceAudio(sfx);
}
extern "C" int8_t AudioPolice_GetMissionStatus(void) {
    // Read the original shared mission status through its baseline member
    return AudioManager.GetMissionScriptPoliceAudioPlayingStatus();
}
"""
trace=(root/'utils/tests/audio_police_state.cpp.in').read_text()
for variant in ('before','after'):
    source=common+(storage[storage.index('tPoliceRadioZone ZoneSfx'):]+'\n'+methods+'\n'+facade+'\n'+bridges if variant=='before' else '')+trace
    source=source.replace('@ZONES@','m->InitialisePoliceRadioZones()' if variant=='before' else 'AudioPolice_InitZones()').replace('@INIT@','m->InitialisePoliceRadio()' if variant=='before' else 'AudioPolice_Init(m)').replace('@RESET@','m->ResetPoliceRadio()' if variant=='before' else 'AudioPolice_Reset(m)').replace('@SET@','m->SetMissionScriptPoliceAudio(sfx)' if variant=='before' else 'AudioPolice_SetMission(m->m_bIsInitialised,sfx)').replace('@GET@','m->GetMissionScriptPoliceAudioPlayingStatus()' if variant=='before' else 'AudioPolice_GetMissionStatus()')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef AUDIO_REVERB\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
(out/'caller.c').write_text("""#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"
#include <assert.h>
void TestPoliceStateC(void) {
    // Check C-owned startup state before any fixture seeding
    assert(g_nMissionAudioSfx==TOTAL_AUDIO_SAMPLES);
    assert(AudioPolice_GetMissionStatus()==PLAY_STATUS_FINISHED);
    assert(gSpecialSuspectLastSeenReport==0);
    for(unsigned i=0;i<NUMAUDIOZONES;++i) {
        // Confirm the zone storage starts in the original zero state
        assert(ZoneSfx[i].m_aName[0]==0 && ZoneSfx[i].field_12==0);
    }
    AudioPolice_InitZones();
    assert(ZoneSfx[0].m_nSampleIndex==SFX_POLICE_RADIO_VICE_CITY);
    AudioManager.m_bIsInitialised=1;AudioManager.m_FrameCounter=0xffffffffu;
    AudioPolice_SetMission(255,0xffffffffu);
    assert(g_nMissionAudioSfx==0xffffffffu && AudioPolice_GetMissionStatus()==PLAY_STATUS_STOPPED);
    g_nMissionAudioPlayingStatus=PLAY_STATUS_PLAYING;
    AudioPolice_SetMission(1,7);assert(g_nMissionAudioSfx==0xffffffffu);
    AudioPolice_Init(&AudioManager);
    assert(gMinTimeToNextReport[0]==0xffffffffu);
    DMAudio_ResetPoliceRadio();
    assert(AudioManager.m_sPoliceRadioQueue.m_nSamplesInQueue==0);
    AudioManager.m_bIsInitialised=0;AudioPolice_Reset(&AudioManager);
}
""",newline='\n')
print('Five state/initialization bodies, one facade, all globals and zone fields, and eight caller/header edits audited')
