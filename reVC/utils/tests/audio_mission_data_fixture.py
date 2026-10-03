"""Compare complete mission label data, original case folding, and C preloading."""
from pathlib import Path
import re
import audio_mission_playback_migration as playback
from audio_controls_migration import function
from audio_owner_init_migration import fields
from audio_mission_data_migration import TABLE,calls,logic,facade,header
root=Path(__file__).resolve().parents[2];out=root/'build/audio-mission-data-c-tests'
old=(out/'before/src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioMissionData.c').read_text()
def normalize(source):
    # Compare actual data and statements independently of whitespace and comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
expected=TABLE.replace('struct MissionAudioData {','typedef struct MissionAudioData {').replace('uint32 m_nId','uint32_t m_nId').replace('};','} MissionAudioData;',1).replace('Const MissionAudioData','static const MissionAudioData').replace('{nil, 0}','{NULL, 0}')
a=kernel.index('typedef struct MissionAudioData');b=kernel.index('static uint8_t AudioMission_NameDiffers',a)
assert normalize(expected)==normalize(kernel[a:b])
general=(out/'before/src/core/General.h').read_text();a=general.index('\tstatic bool faststricmp');b=general.index('\n\tstatic bool SolveQuadratic',a);compare=general[a:b].strip()
expected=compare[compare.index('{'):].replace('return true;','return 1;')
actual=function(kernel,'AudioMission_','NameDiffers');assert normalize(expected)==normalize(actual[actual.index('{'):])
for prefix,name,target in [('', 'FindMissionAudioSfx','FindSfx'),('cAudioManager::','GetMissionAudioLoadedLabel','GetLoadedLabel'),('cAudioManager::','PreloadMissionAudio','Preload')]:
    expected=function(old,prefix,name);expected=expected[expected.index('{'):]
    expected=re.sub(r'(?<![.\w])\b(m_\w+)\b',r'manager->\1',expected)
    expected=expected.replace('uint32 ','uint32_t ').replace('nil','NULL').replace('FALSE','0').replace('TRUE','1').replace('FindMissionAudioSfx','AudioMission_FindSfx').replace('CGeneral::faststricmp','AudioMission_NameDiffers').replace('debug(','AUDIO_MISSION_DEBUG(')
    actual=function(kernel,'AudioMission_',target);assert normalize(expected)==normalize(actual[actual.index('{'):]),name
old_facade=(out/'before/src/audio/DMAudio.cpp').read_text()
for name in ['PreloadMissionAudio','GetMissionAudioLoadedLabel']:
    expected=calls(function(old_facade,'DMAudio_',name)).replace('uint8 ','uint8_t ').replace('Const char','const char')
    assert normalize(expected)==normalize(function(kernel,'DMAudio_',name)),name
assert normalize(logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(facade(old_facade))==normalize((root/'src/audio/DMAudio.cpp').read_text())
assert normalize(header((out/'before/src/audio/AudioManager.h').read_text()))==normalize((root/'src/audio/AudioManager.h').read_text())
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','DMAudio.h','sampman.h'])
record=fields((root/'src/audio/AudioManager.h').read_text())
common=includes+'''
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <initializer_list>
#include <assert.h>
using uint8=uint8_t;using uint32=uint32_t;using bool8=uint8_t;
#define Const const
#define FALSE 0
#define TRUE 1
#define nil NULL
#define debug(format,...) re3_debug("[DBG]: " format,__VA_ARGS__)
struct cAudioManager {
'''+record+'''
    const char *GetMissionAudioLoadedLabel(uint8);
    void PreloadMissionAudio(uint8,const char*);
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
cSampleManager SampleManager;
extern "C" uint8_t g_bMissionAudioLoadFailed[MISSION_AUDIO_SLOTS];
extern "C" uint32_t AudioMission_FindSfx(const char*);
extern "C" const char *AudioMission_GetLoadedLabel(cAudioManager*,uint8_t);
extern "C" void AudioMission_Preload(cAudioManager*,uint8_t,const char*);
static uint64_t digest=1469598103934665603ULL;
static int32_t duration;
static void value(uint32_t v) {
    // Observe returned IDs, duration requests, and callback-visible partial state
    digest=(digest^v)*1099511628211ULL;
}
static void string(const char *s) {
    // Compare canonical returned labels and exact missing-label diagnostic text
    for(;*s;++s)value((unsigned char)*s);value(0);
}
static void snapshot() {
    // Observe all mission arrays and the shared failure flags
    for(unsigned i=0;i<MISSION_AUDIO_SLOTS;++i) {
        // Retain coordinates, phone flags, and the untouched other slot
        value(AudioManager.m_nMissionAudioSampleIndex[i]);value(AudioManager.m_nMissionAudioLoadingStatus[i]);
        value(AudioManager.m_nMissionAudioPlayStatus[i]);value(AudioManager.m_bIsMissionAudioPlaying[i]);
        value(AudioManager.m_nMissionAudioFramesToPlay[i]);value(AudioManager.m_bIsMissionAudioAllowedToPlay[i]);
        value(AudioManager.m_bIsMissionAudio2D[i]);value(AudioManager.m_bIsMissionAudioPhoneCall[i]);
        value(g_bMissionAudioLoadFailed[i]);
        uint32_t bits;
        memcpy(&bits,&AudioManager.m_vecMissionAudioPosition[i].x,4);value(bits);
        memcpy(&bits,&AudioManager.m_vecMissionAudioPosition[i].y,4);value(bits);
        memcpy(&bits,&AudioManager.m_vecMissionAudioPosition[i].z,4);value(bits);
    }
}
extern "C" void re3_debug(const char *format,...) {
    // Capture the same format prefix and missing name without printing game diagnostics
    string(format);va_list args;va_start(args,format);string(va_arg(args,const char*));va_end(args);
}
int32_t SampleManager_GetStreamedFileLength(cSampleManager *manager,uint8_t stream) {
    // Observe legacy stream-ID narrowing and state written before duration lookup
    assert(manager==&SampleManager);value(1000+stream);snapshot();return duration;
}
'''
old_find=function(old,'','FindMissionAudioSfx')
old_get=function(old,'cAudioManager::','GetMissionAudioLoadedLabel')
old_preload=function(old,'cAudioManager::','PreloadMissionAudio')
labels=re.findall(r'\{"([^"\n]*)",\s*(\w+)\}',TABLE)
label_array='static const char *labels[]={'+','.join('"'+name+'"' for name,_ in labels)+'};\n'
ids='static const uint32_t ids[]={'+','.join(id for _,id in labels)+'};\n'
trace=(root/'utils/tests/audio_mission_data.cpp.in').read_text()
for variant in ['before','after']:
    original='struct CGeneral { '+compare+' };\n'+TABLE+'\nuint8_t g_bMissionAudioLoadFailed[MISSION_AUDIO_SLOTS];\n'+old_find+'\n'+old_get+'\n'+old_preload+'\n'+'\n'.join(function(old_facade,'DMAudio_',name) for name in ['PreloadMissionAudio','GetMissionAudioLoadedLabel']) if variant=='before' else ''
    source=common+original+label_array+ids+trace
    source=source.replace('@FIND@','FindMissionAudioSfx' if variant=='before' else 'AudioMission_FindSfx')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','locale':'#define AUDIO_OAL\n#undef ASCII_STRCMP\n','fallback':'#define AUDIO_OAL\n#define THIS_IS_STUPID\n','no-external':'#undef EXTERNAL_3D_SOUND\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete label table/comparator, find/get/preload bodies, both facades, and all ownership/caller edits audited; labels:',len(labels))
