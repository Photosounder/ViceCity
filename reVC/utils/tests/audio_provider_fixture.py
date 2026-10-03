"""Audit and execute the original provider switch and ordered-volume pass."""
from pathlib import Path
import re
import audio_mission_data_migration as data
import audio_mission_state_migration as mission
import audio_owner_entities_migration as entities
import audio_owner_init_migration as init
from audio_controls_migration import function
from audio_provider_migration import owner, header, facade
root=Path(__file__).resolve().parents[2]
out=root/'build/audio-provider-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text()
current=(root/'src/audio/AudioManager.cpp').read_text()
def normalize(source):
    # Compare executable statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
switch=function(old,'cAudioManager::','SetCurrent3DProvider')
adjust=function(old,'cAudioManager::','AdjustSamplesVolume')
expected=switch[switch.index('{'):]
for a,b in [('m_bIsInitialised','initialised'),('m_nActiveSamples','(*activeSamples)'),('m_nActiveQueue','(*activeQueue)'),('m_aRequestedOrderList','order'),('m_nRequestedCount','counts'),('m_asActiveSamples','active'),('uint8','uint8_t'),('int8','int8_t')]:
    expected=re.sub(r'\b'+a+r'\b',b,expected)
actual=function((root/'src/audio/AudioProvider.c').read_text(),'AudioProvider_','SetCurrent')
assert normalize(expected)==normalize(actual[actual.index('{'):])
expected=adjust[adjust.index('{'):].replace('m_nRequestedCount[m_nActiveQueue]','count').replace('m_aRequestedQueue[m_nActiveQueue]','requested').replace('m_aRequestedOrderList[m_nActiveQueue]','order').replace('uint8 ','uint8_t ')
actual=function((root/'src/audio/AudioSoundVolume.c').read_text(),'AudioSoundQueue_','AdjustVolume')
assert normalize(expected)==normalize(actual[actual.index('{'):])
assert normalize(mission.calls(entities.owner(init.owner(owner(old)))))==normalize(current)
assert normalize(data.header(mission.header(entities.header(init.header(header((out/'before/src/audio/AudioManager.h').read_text()))))))==normalize((root/'src/audio/AudioManager.h').read_text())
for path,host in [('src/audio/DMAudio.cpp',False),('src/audio/oal/AudioSampleHost.cpp',True)]:
    assert normalize(facade((out/'before'/path).read_text(),host) if host else data.facade(mission.facade(entities.facade(facade((out/'before'/path).read_text(),host)))))==normalize((root/path).read_text())
common='''#include "AudioProvider.h"
#include "AudioMath.h"
#include "sampman.h"
#include <stdio.h>
#include <string.h>
#include <initializer_list>
#include <assert.h>
using uint8=uint8_t;using int8=int8_t;
struct cAudioManager {
    uint8 m_bIsInitialised,m_nActiveSamples,m_nActiveQueue;
    uint8 m_aRequestedOrderList[NUM_SOUND_QUEUES][NUM_CHANNELS_GENERIC];
    uint8 m_nRequestedCount[NUM_SOUND_QUEUES];
    tSound m_asActiveSamples[NUM_CHANNELS_GENERIC];
    tSound m_aRequestedQueue[NUM_SOUND_QUEUES][NUM_CHANNELS_GENERIC];
    int8 SetCurrent3DProvider(uint8);
    void AdjustSamplesVolume();
};
static cAudioManager state;
cSampleManager SampleManager;
static int8_t providerResult;
static uint32_t channelResult;
static uint64_t digest=1469598103934665603ULL;
static void bytes(const void *ptr,size_t size) {
    // Observe initialized record bytes and tails after every operation
    const unsigned char *data=(const unsigned char*)ptr;
    for(size_t i=0;i<size;++i) digest=(digest^data[i])*1099511628211ULL;
}
static void value(uint32_t v) {
    // Observe backend call order, arguments, and callback-visible owner state
    bytes(&v,sizeof(v));
}
void SampleManager_StopChannel(cSampleManager *manager,uint32_t channel) {
    // Confirm every stop precedes queue reset and reaches the same backend object
    assert(manager==&SampleManager);value(1);value(channel);
    value(state.m_nActiveSamples);value(state.m_nActiveQueue);
    value(state.m_nRequestedCount[0]);value(state.m_nRequestedCount[1]);
}
int8_t SampleManager_SetCurrent3DProvider(cSampleManager *manager,uint8_t which) {
    // Inspect the cleared queues before returning the controlled provider index
    assert(manager==&SampleManager);value(2);value(which);
    value(state.m_nActiveQueue);bytes(state.m_aRequestedOrderList,sizeof(state.m_aRequestedOrderList));
    bytes(state.m_nRequestedCount,sizeof(state.m_nRequestedCount));
    bytes(state.m_asActiveSamples,sizeof(state.m_asActiveSamples));return providerResult;
}
uint32_t SampleManager_GetMaximumSupportedChannels(cSampleManager *manager) {
    // Observe the query that occurs only for positive provider results
    assert(manager==&SampleManager);value(3);return channelResult;
}
static void seed(uint8_t flag,uint8_t count,uint8_t queue) {
    // Define every byte before checking retained records, padding, and inactive tails
    memset(&state,0x3a,sizeof(state));state.m_bIsInitialised=flag;
    state.m_nActiveSamples=count;state.m_nActiveQueue=queue;
    state.m_nRequestedCount[0]=NUM_CHANNELS_GENERIC;
    state.m_nRequestedCount[1]=NUM_CHANNELS_GENERIC;
}
static void snapshot() {
    // Compare all queue storage and owner control fields after provider changes
    bytes(&state,sizeof(state));
}
'''
trace=(root/'utils/tests/audio_provider.cpp.in').read_text()
for variant in ['before','after']:
    methods=switch+'\n#ifdef EXTERNAL_3D_SOUND\n'+adjust+'\n#endif\n' if variant=='before' else ''
    source=common+methods+trace
    source=source.replace('@SWITCH@','state.SetCurrent3DProvider(which)' if variant=='before' else 'AudioProvider_SetCurrent(state.m_bIsInitialised,which,&state.m_nActiveSamples,&state.m_nActiveQueue,state.m_aRequestedOrderList,state.m_nRequestedCount,state.m_asActiveSamples)')
    source=source.replace('@ADJUST@','state.AdjustSamplesVolume()' if variant=='before' else 'AudioSoundQueue_AdjustVolume(state.m_aRequestedQueue[state.m_nActiveQueue],state.m_aRequestedOrderList[state.m_nActiveQueue],state.m_nRequestedCount[state.m_nActiveQueue])')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reflections':'#define AUDIO_OAL\n#undef AUDIO_REFLECTIONS\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Both C bodies, all owner/header edits, and both provider boundary callers audited')
