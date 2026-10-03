"""Audit channel/crackle conversion and compare backend-visible traces."""
from pathlib import Path
import re
from audio_controls_migration import function
import audio_police_channel_migration as channel
import audio_police_state_fixture as state
root=Path(__file__).resolve().parents[2];out=root/'build/audio-police-channel-c-tests'
old=(out/'before/src/audio/PolRadio.cpp').read_text();kernel=(root/'src/audio/AudioPoliceChannel.c').read_text()
def normalize(s):
    # Compare complete bodies independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
for name,target in [('DoPoliceRadioCrackle','Crackle'),('ServicePoliceRadioChannel','ServiceChannel')]:
    actual=function(kernel,'AudioPolice_',target)
    assert normalize(channel.body(function(old,'cAudioManager::',name)))==normalize(actual[actual.index('{'):]),name
assert normalize(channel.radio(old))==normalize((root/'src/audio/PolRadio.cpp').read_text())
assert normalize(channel.header((out/'before/src/audio/AudioManager.h').read_text()))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(channel.calls((out/'before/src/audio/MusicManager.cpp').read_text()))==normalize((root/'src/audio/MusicManager.cpp').read_text())
header=(root/'src/audio/AudioManager.h').read_text();a=header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS');b=header.index('#if defined(AUDIO_MSS)',a);macros=header[a:b]
common=state.common.replace('void InitialisePoliceRadioZones();','void DoPoliceRadioCrackle();\nvoid ServicePoliceRadioChannel(uint8);\nvoid AddSampleToRequestedQueue();\nvoid InitialisePoliceRadioZones();')
common=common.replace('static void snapshot(cAudioManager *m)', 'static void stateSnapshot(cAudioManager *m)')
sound=(root/'src/audio/AudioSound.h').read_text();a=sound.index('typedef struct tSound');b=sound.index('} tSound;',a)
observe=[]
for line in sound[a:b].splitlines():
    # Observe stored sound fields using their actual conditional declarations
    if line.lstrip().startswith('#'):observe.append(line)
    match=re.match(r'\s*(\w+)\s+(m_\w+)\s*;',line)
    if match:
        kind,name=match.groups();access='m->m_sQueueSample.'+name
        if kind=='AudioSoundPosition':observe.extend('real('+access+'.'+axis+');' for axis in ('x','y','z'))
        else:observe.append(('real' if kind=='float' else 'value')+'('+access+');')
snapshot='''static void snapshot(cAudioManager *m) {
    // Compare retained sound fields and the complete relevant owner state
    stateSnapshot(m);if(m) {
        // Observe sound preparation and mutable queue submission state
'''+ '\n'.join(observe)+'''
value(m->m_nPoliceChannelEntity);value(m->m_bIsPaused);value(m->m_bWasPaused);
for(unsigned i=0;i<5;++i) {value(m->m_anRandomTable[i]);}
}
}
'''
a=common.index('uint8_t SampleManager_GetChannelUsedFlag');common=common[:a]+snapshot+common[a:]
common+='''
#define TRUE 1
extern "C" void AudioPolice_Crackle(cAudioManager *);
extern "C" void AudioPolice_ServiceChannel(cAudioManager *,uint8_t);
extern "C" void AudioRequests_Submit(cAudioManager *);
#ifdef FIX_BUGS
extern "C" uint32_t AudioPoliceHost_LogicalFramesPassed(void);
#endif
#ifdef USE_TIME_SCALE_FOR_AUDIO
extern "C" float AudioPoliceHost_TimeScale(void);
#endif
static uint8_t streamPlaying;
static uint32_t logicalFrames=1,submissions;
static float timeScale=1.0f;
struct CTimer {
#ifdef FIX_BUGS
    static uint32 GetLogicalFramesPassed() {
        // Observe the timer read location and supply unsigned wait arithmetic
        value(40);snapshot(active);return logicalFrames;
    }
#endif
    static const float &GetTimeScale() {
        // Preserve the game's reference-returning scale getter in the baseline
        value(41);snapshot(active);return timeScale;
    }
};
void cAudioManager::AddSampleToRequestedQueue() {
    // Observe the prepared sound before simulating mutable queue submission
    value(50);snapshot(this);m_sQueueSample.m_nFinalPriority=++submissions;m_sQueueSample.m_vecPos.x+=0.25f;
}
static void backend(cSampleManager *m,uint32_t event,uint32_t channel) {
    // Compare backend order, arguments, and partially written live state
    assert(m==&SampleManager);value(event);value(channel);snapshot(active);
}
uint32_t SampleManager_GetSampleBaseFrequency(cSampleManager *m,uint32_t sample) {
    // Return bounded frequencies for every tested full-width sample ID
    backend(m,4,sample);return 1000+sample%23000;
}
uint32_t SampleManager_GetSampleLoopStartOffset(cSampleManager *m,uint32_t sample) {
    // Observe the loop start read before its field assignment
    backend(m,5,sample);return 73;
}
int32_t SampleManager_GetSampleLoopEndOffset(cSampleManager *m,uint32_t sample) {
    // Preserve a signed backend loop endpoint
    backend(m,6,sample);return -37;
}
uint8_t SampleManager_InitialiseChannel(cSampleManager *m,uint32_t channel,uint32_t sample,uint8_t bank) {
    // Verify the original loop continues after backend initialization failure
    value(sample);value(bank);backend(m,7,channel);return 0;
}
void SampleManager_SetChannelFrequency(cSampleManager *m,uint32_t channel,uint32_t freq) {
    // Compare scaled-frequency conversion and the unscaled branch
    value(freq);backend(m,8,channel);
}
void SampleManager_SetChannelVolume(cSampleManager *m,uint32_t channel,uint32_t volume) {
    // Capture speech volume after frequency setup
    value(volume);backend(m,9,channel);
}
void SampleManager_SetChannelPan(cSampleManager *m,uint32_t channel,uint32_t pan) {
    // Capture the original centered radio pan
    value(pan);backend(m,10,channel);
}
void SampleManager_SetChannelLoopCount(cSampleManager *m,uint32_t channel,uint32_t count) {
    // Capture one-shot channel loop count
    value(count);backend(m,11,channel);
}
void SampleManager_SetChannelLoopPoints(cSampleManager *m,uint32_t channel,uint32_t start,int32_t end) {
    // Observe manual loop offsets only in the original non-PS2 branch
    value(start);value(end);backend(m,12,channel);
}
void SampleManager_StartChannel(cSampleManager *m,uint32_t channel) {
    // Observe complete speech configuration before playback starts
    backend(m,13,channel);
}
void SampleManager_PreloadStreamedFile(cSampleManager *m,uint32_t sample,uint8_t stream) {
    // Capture mission preloading before shared status changes
    value(sample);backend(m,14,stream);
}
void SampleManager_SetStreamedVolumeAndPan(cSampleManager *m,uint8_t volume,uint8_t pan,uint8_t effect,uint8_t stream) {
    // Capture stream volume, pan, and effect arguments
    value(volume);value(pan);value(effect);backend(m,15,stream);
}
void SampleManager_StartPreloadedStreamedFile(cSampleManager *m,uint8_t stream) {
    // Observe startup before the physical-playing flag is reset
    backend(m,16,stream);
}
uint8_t SampleManager_IsStreamPlaying(cSampleManager *m,uint8_t stream) {
    // Supply physical stream status without opening a device
    backend(m,17,stream);return streamPlaying;
}
void SampleManager_PauseStream(cSampleManager *m,uint8_t pause,uint8_t stream) {
    // Capture pause and resume at the physical stream gates
    value(pause);backend(m,18,stream);
}
'''
methods='\n'.join(function(old,'cAudioManager::',name) for name in ('DoPoliceRadioCrackle','ServicePoliceRadioChannel'))
bridges='''
extern "C" void AudioPolice_Crackle(cAudioManager *m) {
    // Drive the original member through the shared C comparison caller
    m->DoPoliceRadioCrackle();
}
extern "C" void AudioPolice_ServiceChannel(cAudioManager *m,uint8_t wanted) {
    // Keep baseline counters shared across C callers and owners
    m->ServicePoliceRadioChannel(wanted);
}
'''
trace=(root/'utils/tests/audio_police_channel.cpp.in').read_text()
for variant in ('before','after'):
    source=common+macros+(methods+bridges if variant=='before' else channel.HOST.replace('AudioPoliceHost_AddRequested','AudioRequests_Submit'))+trace
    source=source.replace('@CRACKLE@','m->DoPoliceRadioCrackle()' if variant=='before' else 'AudioPolice_Crackle(m)').replace('@SERVICE@','m->ServicePoliceRadioChannel(wanted)' if variant=='before' else 'AudioPolice_ServiceChannel(m,wanted)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','unscaled':'#define AUDIO_OAL\n#undef USE_TIME_SCALE_FOR_AUDIO\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef AUDIO_REVERB\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestPoliceChannelC(void) {
    // Reach both C operations using the live shared owner
    AudioManager.m_bIsInitialised=0;
    AudioPolice_ServiceChannel(&AudioManager,255);
    AudioPolice_Crackle(&AudioManager);
}
''',newline='\n')
print('Both bodies, three conditional host adapters, full caller/header edits and sound macros audited')
