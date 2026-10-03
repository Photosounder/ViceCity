"""Compare actual C mission state and facade behavior against saved C++ methods."""
from pathlib import Path
import re
import audio_mission_data_migration as data
from audio_controls_migration import function
from audio_owner_init_migration import fields
from audio_mission_state_migration import MAP,METHODS,FACADES,calls,logic,facade,header,body
root=Path(__file__).resolve().parents[2];out=root/'build/audio-mission-state-c-tests'
old=(out/'before/src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioMissionState.c').read_text()
def normalize(source):
    # Compare executable statements without formatting or comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for name,target in MAP.items():
    actual=function(kernel,'AudioMission_',target)
    assert normalize(body(function(old,'cAudioManager::',name)))==normalize(actual[actual.index('{'):]),name
old_facade=(out/'before/src/audio/DMAudio.cpp').read_text()
for name in FACADES:
    expected=calls(function(old_facade,'DMAudio_',name))
    for a,b in [('uint8','uint8_t'),('bool8','uint8_t')]:expected=re.sub(r'\b'+a+r'\b',b,expected)
    assert normalize(expected)==normalize(function(kernel,'DMAudio_',name)),name
for name in ['AudioLogic.cpp','AudioManager.cpp','DMAudio.cpp','MusicManager.cpp']:
    saved=(out/'before/src/audio'/name).read_text();expected=logic(saved) if name=='AudioLogic.cpp' else facade(saved) if name=='DMAudio.cpp' else calls(saved)
    assert normalize(data.logic(expected) if name=='AudioLogic.cpp' else data.facade(expected) if name=='DMAudio.cpp' else expected)==normalize((root/'src/audio'/name).read_text()),name
assert normalize(data.header(header((out/'before/src/audio/AudioManager.h').read_text())))==normalize((root/'src/audio/AudioManager.h').read_text())
record=fields((root/'src/audio/AudioManager.h').read_text())
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','DMAudio.h','sampman.h'])
common=includes+'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <initializer_list>
using uint8=uint8_t;using uint32=uint32_t;using bool8=uint8_t;
#define TRUE 1
#define FALSE 0
struct cAudioManager {
'''+record+'\n'+'\n'.join(function(old,'cAudioManager::',name).split('{',1)[0].replace('cAudioManager::','').strip()+';' for name in METHODS)+'''
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
cSampleManager SampleManager;
static uint64_t digest=1469598103934665603ULL;
static cAudioManager *observed;
extern "C" void TestMissionStateC(void);
static void value(uint32_t v) {
    // Compare callback-visible state and query results in exact call order
    digest=(digest^v)*1099511628211ULL;
}
static void snapshot(const cAudioManager *state) {
    // Observe all mission state fields including coordinates and retained phone/frames values
    for(unsigned i=0;i<MISSION_AUDIO_SLOTS;++i) {
        // Keep every byte-valued flag and exact coordinate bit pattern observable
        value(state->m_nMissionAudioSampleIndex[i]);value(state->m_nMissionAudioLoadingStatus[i]);
        value(state->m_nMissionAudioPlayStatus[i]);value(state->m_bIsMissionAudioPlaying[i]);
        value(state->m_bIsMissionAudioAllowedToPlay[i]);value(state->m_bIsMissionAudio2D[i]);
        value(state->m_nMissionAudioFramesToPlay[i]);value(state->m_bIsMissionAudioPhoneCall[i]);
        uint32_t bits;
        memcpy(&bits,&state->m_vecMissionAudioPosition[i].x,4);value(bits);
        memcpy(&bits,&state->m_vecMissionAudioPosition[i].y,4);value(bits);
        memcpy(&bits,&state->m_vecMissionAudioPosition[i].z,4);value(bits);
    }
}
void SampleManager_StopStreamedFile(cSampleManager *manager,uint8_t stream) {
    // Observe state reset before the original backend stop for slot plus one
    assert(manager==&SampleManager);value(1000+stream);snapshot(observed);
}
'''
for name,target in MAP.items():
    # Declare the actual owner APIs while retaining identical fake record fields
    params='uint32_t' if target=='UsesPoliceChannel' else 'cAudioManager*,uint8_t'
    common+='extern "C" '+('void' if target in ['AllowPlay','Clear'] else 'uint8_t')+' AudioMission_'+target+'('+params+');\n'
methods='\n'.join(function(old,'cAudioManager::',name) for name in METHODS)
public='\n'.join(function(old_facade,'DMAudio_',name) for name in FACADES)
trace=(root/'utils/tests/audio_mission_state.cpp.in').read_text()
for variant in ['before','after']:
    source=common+(methods+'\n'+public if variant=='before' else '')+trace
    source=source.replace('@DUCK@','state->ShouldDuckMissionAudio(slot)' if variant=='before' else 'AudioMission_ShouldDuck(state,slot)')
    source=source.replace('@PLAYING@','state->IsMissionAudioSamplePlaying(slot)' if variant=='before' else 'AudioMission_IsPlaying(state,slot)')
    source=source.replace('@FINISHED@','state->IsMissionAudioSampleFinished(slot)' if variant=='before' else 'AudioMission_IsFinished(state,slot)')
    source=source.replace('@POLICY@','state->MissionScriptAudioUsesPoliceChannel(sound)' if variant=='before' else 'AudioMission_UsesPoliceChannel(sound)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('All seven state bodies, six facades, eight member removals, and four caller files audited')

(out/'caller.c').write_text('''#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"
#include <assert.h>
void TestMissionStateC(void) {
    // Reach all six public mission exports through the live owner from C
    assert(DMAudio_GetMissionAudioLoadingStatus(0)==LOADING_STATUS_LOADED);
    assert(DMAudio_IsMissionAudioSamplePlaying(0)==1);
    assert(DMAudio_IsMissionAudioSampleFinished(0)==0);
    AudioManager.m_bIsInitialised=1;
    AudioManager.m_nMissionAudioSampleIndex[0]=0;
    AudioManager.m_nMissionAudioLoadingStatus[0]=LOADING_STATUS_LOADED;
    DMAudio_PlayLoadedMissionAudio(0);assert(AudioManager.m_bIsMissionAudioAllowedToPlay[0]==1);
    DMAudio_SetMissionAudioLocation(0,1,2,3);assert(AudioManager.m_vecMissionAudioPosition[0].z==3);
    DMAudio_ClearMissionAudio(0);assert(AudioManager.m_nMissionAudioSampleIndex[0]==NO_SAMPLE);
}''',newline='\n')
