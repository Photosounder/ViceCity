"""Expose music state to C and move complete independent owner operations."""
import re
from audio_controls_migration import function
import audio_music_playback_migration as playback
METHODS=['ResetTimers','SetRadioChannelByScript','PlayFrontEndTrack','StopFrontEndTrack','PlayAnnouncement','GetListenTimeArray','GetFavouriteRadioStation','CheckForMusicInterruptions']
GETTERS={'IsInitialised':('uint8_t','m_bIsInitialised'),'GetMusicMode':('uint8_t','m_nMusicMode'),'GetCurrentTrack':('uint32_t','m_nPlayingTrack')}
PROTOTYPES='''void MusicState_InitState(cMusicManager *manager);
uint8_t MusicState_IsInitialised(const cMusicManager *manager);
uint8_t MusicState_GetMusicMode(const cMusicManager *manager);
uint32_t MusicState_GetCurrentTrack(const cMusicManager *manager);
void MusicState_ResetTimers(cMusicManager *manager, uint32_t time);
void MusicState_SetRadioChannelByScript(cMusicManager *manager, uint32_t station, int32_t pos);
void MusicState_PlayFrontEndTrack(cMusicManager *manager, uint32_t track, uint8_t loopFlag);
void MusicState_StopFrontEndTrack(cMusicManager *manager);
void MusicState_PlayAnnouncement(cMusicManager *manager, uint32_t announcement);
float *MusicState_GetListenTimeArray(cMusicManager *manager);
uint32_t MusicState_GetFavouriteRadioStation(const cMusicManager *manager);
uint8_t MusicState_CheckForMusicInterruptions(const cMusicManager *manager);
'''
CTOR='cMusicManager::cMusicManager()\n{\n\t// Initialize music state at the original compiler startup callback\n\tMusicState_InitState(this);\n}'

def fields(source):
    # Extract only stored fields before the owner's remaining C++ operations
    start=source.index('class cMusicManager') if 'class cMusicManager' in source else source.index('typedef struct cMusicManager')
    end=source.index('\tcMusicManager();',start);data=source[source.index('{',start)+1:end]
    data=data.replace('public:','').replace('#ifdef __cplusplus','')
    return types(data)

def types(source):
    # Preserve fixed-width scalar representations across C and C++
    for a,b in [('bool8','uint8_t'),('uint8','uint8_t'),('int8','int8_t'),('uint32','uint32_t'),('int32','int32_t'),('FALSE','0'),('TRUE','1')]:source=re.sub(r'\b'+a+r'\b',b,source)
    return source

def calls(source):
    # Bind global member calls to the same live music owner and argument order
    for name in METHODS+list(GETTERS):
        source=source.replace('MusicManager.'+name+'()', 'MusicState_'+name+'(&MusicManager)')
        source=source.replace('MusicManager.'+name+'(', 'MusicState_'+name+'(&MusicManager, ')
    return playback.calls(source)

def logic(source):
    # Remove complete converted operations while preserving unrelated game music logic
    if 'cMusicManager::cMusicManager(' in source:
        source=source.replace(function(source,'cMusicManager::','cMusicManager'),CTOR)
        for name in METHODS:
            if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
        for name in METHODS+list(GETTERS):
            source=re.sub(r'(?<![\w.:>])'+name+r'\(\)', 'MusicState_'+name+'(this)',source)
            source=re.sub(r'(?<![\w.:>])'+name+r'\(', 'MusicState_'+name+'(this, ',source)
    return playback.logic(calls(source))

def header(source):
    # Expose unchanged owner and track layouts while guarding remaining C++ methods
    source=source.replace('#pragma once\n','#pragma once\n#include <stdint.h>\n#include "../core/config.h"\n',1)
    source=source.replace('class tStreamedSample\n{\npublic:','typedef struct tStreamedSample {').replace('\n};\n\nclass CVehicle;', '\n} tStreamedSample;\n\n#ifdef __cplusplus\nclass CVehicle;',1)
    source=source.replace('class CPed;','class CPed;\n#endif',1)
    source=source.replace('class cMusicManager\n{\npublic:','typedef struct cMusicManager {')
    source=source.replace('\npublic:\n\tcMusicManager();','\n#ifdef __cplusplus\n\tcMusicManager();',1)
    source=source.replace('\n};\n\nVALIDATE_SIZE(cMusicManager, 0x95C);','\n#endif\n} cMusicManager;\n\n#ifdef __cplusplus\nVALIDATE_SIZE(cMusicManager, 0x95C);\n#elif defined(CHECK_STRUCT_SIZES)\n_Static_assert(sizeof(cMusicManager) == 0x95C, "cMusicManager layout");\n#endif',1)
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS+list(GETTERS)))
    source=types(source)
    source=source.replace('extern cMusicManager MusicManager;', '#ifdef __cplusplus\nextern "C" {\n#endif\n'+PROTOTYPES+'extern cMusicManager MusicManager;')
    source=source.replace('float GetHeightScale();','#ifdef __cplusplus\n}\nfloat GetHeightScale();\n#endif')
    return playback.header(source)

def body(source):
    # Replace implicit fields without changing gates, arithmetic, narrowing or write order
    source=source[source.index('{'):];source=types(source)
    source=re.sub(r'(?<![\w.>])\b(m_\w+|aListenTimeArray|nFramesSinceCutsceneEnded)\b',r'manager->\1',source)
    return source.replace('IsInitialised()', 'MusicState_IsInitialised(manager)')

def project(source):
    # Register all independent music operations as a separate C11 source
    if 'MusicState.c' in source:return playback.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioOwner.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return playback.project(source[:b]+'\n'+source[a:b].replace('AudioOwner.c','MusicState.c')+source[b:])
