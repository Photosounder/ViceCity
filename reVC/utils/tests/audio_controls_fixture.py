"""Compare actual C controls and facade calls against saved C++ implementations."""
from pathlib import Path
import re
import audio_mission_data_migration as data
import audio_mission_state_migration as mission
import audio_owner_entities_migration as entities
import audio_owner_init_migration as init
import audio_provider_migration as provider
from audio_controls_migration import VOLUMES, CONTROLS, OTHER, function, owner, header
root=Path(__file__).resolve().parents[2]
out=root/'build/audio-controls-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text()
facade=(out/'before/src/audio/DMAudio.cpp').read_text()
kernel=(root/'src/audio/AudioControls.c').read_text()
def normalize(source):
    # Compare real statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
def types(source):
    # Apply only fixed-width C type and null substitutions
    for a,b in [('uint8','uint8_t'),('bool8','uint8_t'),('int8','int8_t'),('int32','int32_t'),('nil','NULL'),('FALSE','0')]:
        source=re.sub(r'\b'+a+r'\b',b,source)
    return source
expected_facade=facade.replace('#include "sampman.h"','#include "sampman.h"\n#include "AudioControls.h"')
for name in VOLUMES:
    original=function(facade,'DMAudio_',name)
    delegated=function(old,'cAudioManager::',name)
    target=re.search(r'SampleManager_\w+\(&SampleManager, volume\);',delegated).group().replace('volume','vol')
    expected=types(original.replace('AudioManager.'+name+'(vol);',target))
    assert normalize(expected)==normalize(function(kernel,'DMAudio_',name)),name
    expected_facade=expected_facade.replace(original+'\n','')
for name in CONTROLS:
    original=function(old,'cAudioManager::',name)
    expected=types(original[original.index('{'):].replace('m_bIsInitialised','initialised').replace('Clamp(','AUDIO_CONTROL_CLAMP('))
    actual=function(kernel,'AudioControls_',name)
    assert normalize(expected)==normalize(actual[actual.index('{'):]),name
    gated=name not in ['SetSpeakerConfig','CheckForAnAudioFileOnCD']
    args=('AudioManager.m_bIsInitialised'+(', id' if name=='Get3DProviderName' else '') if gated else ('config' if name=='SetSpeakerConfig' else ''))
    expected_facade=re.sub(r'AudioManager\.'+name+r'\([^)]*\)','AudioControls_'+name+'('+args+')',expected_facade)
expected_facade=expected_facade.replace('AudioManager.SetDynamicAcousticModelingStatus(status);','AudioManager.m_bDynamicAcousticModelingStatus = status;').replace('AudioManager.IsAudioInitialised()','AudioManager.m_bIsInitialised')
assert normalize(data.facade(mission.facade(entities.facade(provider.facade(expected_facade)))))==normalize((root/'src/audio/DMAudio.cpp').read_text())
assert normalize(mission.calls(entities.owner(init.owner(provider.owner(owner(old))))))==normalize((root/'src/audio/AudioManager.cpp').read_text())
assert normalize(data.header(mission.header(entities.header(init.header(provider.header(header((out/'before/src/audio/AudioManager.h').read_text())))))))==normalize((root/'src/audio/AudioManager.h').read_text())
old_host=(out/'before/src/audio/oal/AudioSampleHost.cpp').read_text()
assert normalize(provider.facade(old_host.replace('AudioManager.IsAudioInitialised()','AudioManager.m_bIsInitialised'), host=True))==normalize((root/'src/audio/oal/AudioSampleHost.cpp').read_text())
macro=next(line for line in (root/'src/core/common.h').read_text().splitlines() if line.startswith('#define Clamp('))
assert macro.replace('Clamp(','AUDIO_CONTROL_CLAMP(') in kernel
common='''#include "DMAudio.h"
#include "AudioControls.h"
#include "sampman.h"
#include <stdio.h>
#include <assert.h>
using uint8=uint8_t; using int8=int8_t; using int32=int32_t; using bool8=uint8_t;
#define nil NULL
#define FALSE 0
'''+macro+'\n'
methods=[function(old,'cAudioManager::',name) for name in VOLUMES+CONTROLS+OTHER]
class_before='struct cAudioManager { uint8 m_bIsInitialised; uint8 m_bDynamicAcousticModelingStatus;\n'+'\n'.join(method[:method.index('{')].replace('cAudioManager::','').strip()+';' for method in methods)+'\n};\nstatic cAudioManager AudioManager;\n'+'\n'.join(methods)+'\n'
class_after='struct {uint8 m_bIsInitialised; uint8 m_bDynamicAcousticModelingStatus;} AudioManager;\n'
functions_before='\n'.join(function(facade,'DMAudio_',name) for name in VOLUMES+CONTROLS+OTHER)
current=(root/'src/audio/DMAudio.cpp').read_text()
functions_after='\n'.join(function(current,'DMAudio_',name) for name in CONTROLS+OTHER)
mocks='''
static uint64_t digest=1469598103934665603ULL;
static uint32_t providers;
static int8_t currentProvider;
static char providerName[]="mock";
static void record(uint32_t value) {
    // Record both backend call ordering and returned state
    digest=(digest^value)*1099511628211ULL;
}
cSampleManager SampleManager;
'''
backend=(root/'src/audio/sampman.h').read_text()
names=list(dict.fromkeys(re.findall(r'\b(SampleManager_\w+)\(',kernel)))
for index,name in enumerate(names):
    declaration=next(line.strip() for line in backend.splitlines() if re.search(r'\b'+name+r'\(',line))
    arguments=declaration[declaration.index('(')+1:declaration.index(')')].split(',')
    body='assert(manager==&SampleManager); record('+str(index+1)+');'
    for arg in arguments[1:]:
        body+='record((uint32_t)'+arg.strip().split()[-1]+');'
    if name.endswith('GetNum3DProvidersAvailable'): body+='return providers;'
    elif name.endswith('Get3DProviderName'):body+='return providerName;'
    elif name.endswith('GetCurrent3DProviderIndex') or name.endswith('AutoDetect3DProviders'):body+='return currentProvider;'
    elif name.endswith('IsMP3RadioChannelAvailable') or name.endswith('CheckForAnAudioFileOnCD'):body+='return 255;'
    elif name.endswith('GetCDAudioDriveLetter'):body+="return 'Z';"
    mocks+=declaration[:-1]+' {\n    // Model backend responses while observing every call and argument\n    '+body+'\n}\n'
trace='''
int main() {
    // Check every byte volume against the unchanged facade clamping
    for (unsigned volume=0;volume<256;++volume) {
        // Invoke all five exports through their existing public C declarations
        DMAudio_SetMP3BoostVolume(volume); DMAudio_SetEffectsMasterVolume(volume);
        DMAudio_SetMusicMasterVolume(volume); DMAudio_SetEffectsFadeVol(volume); DMAudio_SetMusicFadeVol(volume);
    }
    for (unsigned flag: {0u,1u,2u,255u}) {
        // Cover all initialization gates without normalizing stored byte state
        AudioManager.m_bIsInitialised=flag;
        record(DMAudio_IsAudioInitialised());
        DMAudio_ReleaseDigitalHandle(); DMAudio_ReacquireDigitalHandle();
        record(DMAudio_CheckForAnAudioFileOnCD()); record(DMAudio_GetCDAudioDriveLetter());
        record(DMAudio_IsMP3RadioChannelAvailable());
        for (uint32_t count: {0u,1u,3u,255u,256u,513u}) {
            // Exercise zero providers, byte truncation, and both provider validation policies
            providers=count;record(DMAudio_GetNum3DProvidersAvailable());
            for (unsigned id=0;id<256;++id) {
                // Compare provider lookup results together with exact backend query counts
                char *name=DMAudio_Get3DProviderName(id);record(name?name[0]:0);
            }
        }
        for (int value: {-128,-1,0,1,127}) {
            // Retain signed provider sentinel values and speaker configuration arguments
            currentProvider=value;record((uint8_t)DMAudio_GetCurrent3DProviderIndex());
            record((uint8_t)DMAudio_AutoDetect3DProviders()); DMAudio_SetSpeakerConfig(value);
        }
        DMAudio_SetDynamicAcousticModelingStatus(flag); record(AudioManager.m_bDynamicAcousticModelingStatus);
    }
    printf("%llu\\n",(unsigned long long)digest);
}
'''
for variant in ['before','after']:
    source=common+'#include <initializer_list>\n'+mocks+(class_before+functions_before if variant=='before' else class_after+functions_after)+trace
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,defines in {'oal':'#define AUDIO_OAL\n','mss':'#define AUDIO_MSS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reflections':'#define AUDIO_OAL\n#undef AUDIO_REFLECTIONS\n'}.items():
    # Load the actual game configuration before selecting conditional fixture branches
    (out/(name+'.h')).write_text('#include "config.h"\n'+defines,newline='\n')
print('All fifteen C operations, seventeen removed methods, facade edits and owner lifecycle audited')
