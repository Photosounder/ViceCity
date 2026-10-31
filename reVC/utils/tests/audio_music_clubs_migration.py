"""Move complete club track positioning into C and remove all script-to-music wrappers."""
import re
import audio_music_modes_migration as modes
from audio_controls_migration import function
METHODS=['SetMalibuClubTrackPos','SetStripClubTrackPos']
HOSTS=['MusicMode','PlayingTrack','MalibuTrack','StripTrack']
PROTOTYPES='''void MusicClubs_SetMalibuClubTrackPos(cMusicManager *manager, uint8_t scriptObject);
void MusicClubs_SetStripClubTrackPos(cMusicManager *manager, uint8_t scriptObject);
'''

def remove_hosts(source):
    # Remove the entire explicit C-linkage declaration together with each obsolete wrapper body
    for name in HOSTS:
        if re.search(r'(?m)^AudioScriptSoundsHost_'+name+r'\(',source):
            body=function(source,'AudioScriptSoundsHost_',name)
            a=source.index(body);prefix=re.search(r'extern "C" \w+\n$',source[:a])
            if prefix:body=prefix.group()+body
            source=source.replace(body+'\n','')
    return source

def calls(source):
    # Enter complete positioning and read existing C music state at each original boundary
    source=remove_hosts(source)
    for name in METHODS:source=source.replace('MusicManager.'+name+'(', 'MusicClubs_'+name+'(&MusicManager, ')
    source=source.replace('AudioScriptSoundsHost_MusicMode()', 'MusicState_GetMusicMode(&MusicManager)').replace('AudioScriptSoundsHost_PlayingTrack()', 'MusicState_GetCurrentTrack(&MusicManager)')
    source=source.replace('AudioScriptSoundsHost_MalibuTrack(', 'MusicClubs_SetMalibuClubTrackPos(&MusicManager, ').replace('AudioScriptSoundsHost_StripTrack(', 'MusicClubs_SetStripClubTrackPos(&MusicManager, ')
    return modes.calls(source)

def logic(source):
    # Remove complete converted members and all obsolete script-to-music wrappers
    for name in METHODS:
        if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
    source=remove_hosts(source)
    return modes.logic(calls(source))

def header(source):
    # Preserve both owners' fields while replacing methods and wrapper declarations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS+['AudioScriptSoundsHost_'+n for n in HOSTS]))
    return modes.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def script(source):
    # Import the actual C music record before entering its state and positioning operations
    source=source.replace('#include "AudioManager.h"','#include "AudioManager.h"\n#include "MusicManager.h"',1)
    return calls(source)

def body(source):
    # Preserve uninitialized defaults, signed remainder, track guards and final timer writes
    source=source[source.index('{'):].replace('MusicState_IsInitialised(this)','MusicState_IsInitialised(manager)')
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source)
    return source.replace('CTimer::GetTimeInMillisecondsPauseMode()', 'MusicInitHost_PauseTime()')

def project(source):
    # Register complete club positioning as C11
    if 'MusicClubs.c' in source:return modes.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicAmbience.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return modes.project(source[:b]+'\n'+source[a:b].replace('MusicAmbience.c','MusicClubs.c')+source[b:])
