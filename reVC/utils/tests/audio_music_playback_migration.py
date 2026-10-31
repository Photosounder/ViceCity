"""Move complete music stream termination and cutscene playback into C11."""
import re
from audio_controls_migration import function
import audio_music_init_migration as init
METHODS=['Terminate','PreloadCutSceneMusic','PlayPreloadedCutSceneMusic','StopCutSceneMusic']
FACADES=METHODS[1:]
PROTOTYPES='''void MusicPlayback_Terminate(cMusicManager *manager);
void MusicPlayback_PreloadCutSceneMusic(cMusicManager *manager, uint32_t track);
void MusicPlayback_PlayPreloadedCutSceneMusic(cMusicManager *manager);
void MusicPlayback_StopCutSceneMusic(cMusicManager *manager);
'''
def calls(source):
    # Bind complete operations to the original global music owner at each boundary
    for name in METHODS:
        source=source.replace('MusicManager.'+name+'()', 'MusicPlayback_'+name+'(&MusicManager)')
        source=source.replace('MusicManager.'+name+'(', 'MusicPlayback_'+name+'(&MusicManager, ')
    return init.calls(source.replace('AudioLifecycleHost_MusicTerminate()', 'MusicPlayback_Terminate(&MusicManager)'))

def facade(source):
    # Remove the complete cutscene facades now compiled with their C operations
    for name in FACADES:
        if re.search(r'(?m)^DMAudio_'+name+r'\(',source):source=source.replace(function(source,'DMAudio_',name)+'\n','')
    return init.facade(calls(source))

def logic(source):
    # Remove converted owner methods and the obsolete music shutdown trampoline
    for name in METHODS:
        if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
    if 'void\nAudioLifecycleHost_MusicTerminate(' in source:source=source.replace(function(source,'AudioLifecycleHost_','MusicTerminate')+'\n','')
    return init.logic(facade(source))

def header(source):
    # Keep every record field while removing converted methods and obsolete callback declarations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS+['AudioLifecycleHost_MusicTerminate']))
    return init.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Preserve initialized gates, stream checks, callback order and retained field writes
    source=source[source.index('{'):].replace('MusicState_IsInitialised(this)','MusicState_IsInitialised(manager)')
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source)
    return source.replace('FALSE','0').replace('TRUE','1')

def lifecycle(source):
    # Enter complete C music cleanup before the existing audio entity and bank clearing
    if '#include "MusicManager.h"' not in source:source=source.replace('#include "AudioManager.h"','#include "AudioManager.h"\n#include "MusicManager.h"',1)
    return calls(source)

def project(source):
    # Register all complete playback operations and public facades as C11
    if 'MusicPlayback.c' in source:return init.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicState.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return init.project(source[:b]+'\n'+source[a:b].replace('MusicState.c','MusicPlayback.c')+source[b:])
