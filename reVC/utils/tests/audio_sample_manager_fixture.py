"""Extract production state, metadata and channel methods for quiet baseline comparison."""
from pathlib import Path
import re, sys
root = Path(__file__).resolve().parents[2]
base = root / 'build/audio-sample-manager-c-tests'
baseline = '--baseline' in sys.argv
names = ['UpdateEffectsVolume','SetEffectsMasterVolume','SetMusicMasterVolume','SetMP3BoostVolume','SetEffectsFadeVolume','SetMusicFadeVolume','SetMonoMode','GetBankContainingSound','GetSampleBaseFrequency','GetSampleLoopStartOffset','GetSampleLoopEndOffset','GetSampleLength','SetChannelFrequency','SetChannelLoopPoints','SetChannelLoopCount','GetChannelUsedFlag','StartChannel','StopChannel','GetNum3DProvidersAvailable','SetNum3DProvidersAvailable','Get3DProviderName','Set3DProviderName','SetChannelEmittingVolume','SetChannelVolume','SetStreamedVolumeAndPan']
source = (base / 'before/src/audio/sampman_oal.cpp' if baseline else root / 'src/audio/sampman_oal.c').read_text()
functions = []
for name in names:
    symbol = 'cSampleManager::' + name if baseline else 'SampleManager_' + name
    match = re.search(r'(?:void|bool8|uint32|int32|int8|char\s*\*)\s*' + re.escape(symbol) + r'\([^)]*\)\s*\{', source)
    if not match:
        raise RuntimeError('Missing method ' + name)
    start = match.start(); end = source.index('{',start) + 1; depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}'); end += 1
    functions.append(source[start:end])
header = '#include "src/audio/sampman.h"'
if baseline:
    old = (base / 'before/src/audio/sampman.h').read_text().replace('class cSampleManager\n{','class cSampleManager\n{\npublic:')
    (base / 'baseline-header.h').write_text(old,newline='\n')
    header = '#include "baseline-header.h"'
prelude = """
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "src/core/config.h"
typedef uint8_t uint8; typedef uint16_t uint16; typedef uint32_t uint32;
typedef int32_t int32; typedef int8_t int8; typedef uint8_t bool8;
#define TRUE 1
#define FALSE 0
#define ASSERT assert
@HEADER@
uint32 BankStartOffset[MAX_SFX_BANKS];
static bool8 _bSampmanInitialised;
static uint8 nChannelVolume[NUM_CHANNELS];
typedef struct { unsigned volume, frequency, loops, used; int32 first, last; } TestChannel;
static TestChannel aChannel[NUM_CHANNELS];
static unsigned volume_calls;
static int test_music_mode, test_current_track, test_radio, test_interrupted;
#ifdef __cplusplus
static struct TestMusicManager {
    int GetMusicMode() { return test_music_mode; }
    int GetCurrentTrack() { return test_current_track; }
    int GetRadioInCar() { return test_radio; }
    bool CheckForMusicInterruptions() { return test_interrupted != 0; }
} MusicManager;
#endif
static int AudioSample_GetMusicMode(void) {
    // Supply game music mode through the new C callback
    return test_music_mode;
}
static int AudioSample_GetCurrentTrack(void) {
    // Supply the current track through the new C callback
    return test_current_track;
}
static int AudioSample_GetRadioInCar(void) {
    // Supply the selected station through the new C callback
    return test_radio;
}
static bool8 AudioSample_CheckForMusicInterruptions(void) {
    // Supply interruption state through the new C callback
    return test_interrupted != 0;
}
typedef struct { unsigned volume, pan, opened; } CStream;
static CStream test_streams[MAX_STREAMS];
static CStream *aStream[MAX_STREAMS];
static uint8 nStreamVolume[MAX_STREAMS], nStreamPan[MAX_STREAMS];
static bool8 CStream_IsOpened(CStream *stream) {
    // Preserve the stream-open predicate used by volume updates
    return stream->opened;
}
static void CStream_SetVolume(CStream *stream, unsigned volume) {
    // Capture production stream volume including user-track boost
    stream->volume = volume;
}
static void CStream_SetPan(CStream *stream, unsigned pan) {
    // Capture production pan clamping
    stream->pan = pan;
}

static void CChannel_SetVolume(TestChannel *channel, unsigned volume) {
    // Capture the volume forwarded by the actual manager methods
    channel->volume = volume; volume_calls++;
}
static void CChannel_SetCurrentFreq(TestChannel *channel, unsigned frequency) {
    // Capture the frequency forwarded by the actual manager methods
    channel->frequency = frequency;
}
static void CChannel_SetLoopPoints(TestChannel *channel, uint32 first, int32 last) {
    // Capture signed loop endpoints after byte-to-sample conversion
    channel->first = first; channel->last = last;
}
static void CChannel_SetLoopCount(TestChannel *channel, uint32 loops) {
    // Capture the requested loop count
    channel->loops = loops;
}
static bool8 CChannel_IsUsed(TestChannel *channel) {
    // Supply the playback predicate consumed by the production manager
    return channel->used;
}
static void CChannel_Start(TestChannel *channel) {
    // Record channel activation through the manager API
    channel->used = 1;
}
static void CChannel_Stop(TestChannel *channel) {
    // Record channel deactivation through the manager API
    channel->used = 0;
}
""".replace('@HEADER@',header)
body = """
int main(void) {
    // Compare state layout, sample metadata, bank boundaries and volume forwarding against the saved class
    static cSampleManager manager;
    printf("layout %zu %zu\\n",sizeof(manager),sizeof(tSample));
    BankStartOffset[SFX_BANK_0] = 100;
    BankStartOffset[SFX_BANK_PED_COMMENTS] = 1000;
    for(unsigned i=0; i<NUM_CHANNELS; i++) {
        // Exercise active, stopped, silent and audible channels
        aChannel[i].used = i%3 != 0; nChannelVolume[i] = i%2 ? 127 : 0;
    }
    for(unsigned step=0; step<256; step++) {
        // Sweep all byte values and both initialization predicates
        _bSampmanInitialised = step%2;
        CALL(SetEffectsMasterVolume, step);
        CALL(SetEffectsFadeVolume, 255-step);
        CALL(SetMusicMasterVolume, step);
        CALL(SetMusicFadeVolume, 255-step);
        CALL(SetMP3BoostVolume, step);
        CALL(SetMonoMode, step%2);
        assert(MUSIC() == step);
        printf("state %u %u %u %u %u %u %u\\n",step,manager.m_nEffectsVolume,manager.m_nMusicVolume,manager.m_nMP3BoostVolume,manager.m_nEffectsFadeVolume,manager.m_nMusicFadeVolume,manager.m_nMonoMode);
        for(unsigned i=0; i<NUM_CHANNELS; i++) printf("volume %u %u\\n",i,aChannel[i].volume);
    }
    for(unsigned i=0; i<TOTAL_AUDIO_SAMPLES; i++) {
        // Check every sample slot including negative loop ends
        manager.m_aSamples[i].nFrequency = 8000+i;
        manager.m_aSamples[i].nLoopStart = i*3;
        manager.m_aSamples[i].nLoopEnd = i%2 ? -1 : (int32)(i*4);
        manager.m_aSamples[i].nSize = i*5;
        printf("sample %u %u %d %u\\n",CALL(GetSampleBaseFrequency,i),CALL(GetSampleLoopStartOffset,i),CALL(GetSampleLoopEndOffset,i),CALL(GetSampleLength,i));
    }
    for(unsigned i=0; i<1100; i++) printf("bank %d\\n",CALL(GetBankContainingSound,i));
    for(unsigned i=0; i<NUM_CHANNELS; i++) {
        // Check channel forwarding and playback state for each channel
        CALL(SetChannelFrequency,i,48000+i); CALL(SetChannelLoopPoints,i,5,-1); CALL(SetChannelLoopCount,i,i);
        CALL(StartChannel,i); assert(CALL(GetChannelUsedFlag,i));
        CALL(StopChannel,i); assert(!CALL(GetChannelUsedFlag,i));
        printf("channel %u %d %d %u\\n",aChannel[i].frequency,aChannel[i].first,aChannel[i].last,aChannel[i].loops);
    }
    for(unsigned i=0; i<MAXPROVIDERS; i++) {
        // Verify provider publication and reset without changing name ownership
        char name[] = "provider"; CALL(Set3DProviderName,i,name); assert(CALL(Get3DProviderName,i) == name);
        CALL(Set3DProviderName,i,NULL); assert(CALL(Get3DProviderName,i) == NULL);
        CALL(SetNum3DProvidersAvailable,i); assert(CALL(GetNum3DProvidersAvailable) == i);
    }
    for(unsigned mode=0; mode<3; mode++) {
        // Exercise cutscene attenuation, finale silence and normal music
        test_music_mode = mode;
        for(unsigned track=0; track<2; track++) {
            // Compare special finale handling and ordinary tracks
            test_current_track = track ? STREAMED_SOUND_CUTSCENE_FINALE : 0;
            CALL(SetChannelEmittingVolume,0,255);
            CALL(SetChannelVolume,CHANNEL_POLICE_RADIO,255);
            printf("cutscene %u %u %u %u\\n",mode,track,nChannelVolume[0],nChannelVolume[CHANNEL_POLICE_RADIO]);
        }
    }
    for(unsigned step=0; step<256; step++) {
        // Sweep volume and pan clamps, user-track boost and every stream slot
        CALL(SetEffectsMasterVolume,step); CALL(SetEffectsFadeVolume,255-step);
        CALL(SetMusicMasterVolume,step); CALL(SetMusicFadeVolume,255-step); CALL(SetMP3BoostVolume,step);
        test_radio = step%2 ? USERTRACK : 0; test_interrupted = step%3 == 0;
        for(unsigned slot=0; slot<MAX_STREAMS; slot++) {
            // Compare opened and closed streams for music and effects volume paths
            aStream[slot] = &test_streams[slot]; test_streams[slot].opened = step%5 != 0;
            for(unsigned effect=0; effect<2; effect++) {
                CALL(SetStreamedVolumeAndPan,step,255-step,effect,slot);
                printf("stream %u %u %u %u %u %u\\n",slot,effect,test_streams[slot].volume,test_streams[slot].pan,nStreamVolume[slot],nStreamPan[slot]);
            }
        }
    }
    printf("volume calls %u\\n",volume_calls);
    return 0;
}
"""
if baseline:
    prelude += '\ncSampleManager::cSampleManager() {}\ncSampleManager::~cSampleManager() {}\n'
    body = re.sub(r'CALL\((\w+)(?:,([^)]*))?\)',lambda m:'manager.'+m[1]+'('+(m[2] or '')+')',body)
    body = body.replace('MUSIC()', 'manager.GetMusicVolume()')
else:
    body = re.sub(r'CALL\((\w+)(?:,([^)]*))?\)',lambda m:'SampleManager_'+m[1]+'(&manager'+(','+m[2] if m[2] else '')+')',body)
    body = body.replace('MUSIC()', 'SampleManager_GetMusicVolume(&manager)')
Path(sys.argv[1]).write_text(prelude+'\n'.join(functions)+body,newline='\n')
