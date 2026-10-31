"""Move complete saved-radio storage, helpers and reload reset into C11."""
import re
import clock_migration as clock
import audio_music_ambience_migration as ambience
from audio_controls_migration import function
PROTOTYPES='void MusicReload_ResetMusicAfterReload(cMusicManager *manager);\nextern uint32_t RadioStaticTimer;\n'
SAVED_METHODS=['InitRadioStationPositionList','GetSavedRadioStationPosition','PopulateRadioStationPositionList']

def calls(source):
    # Enter the complete C reset at the original audio reload boundary
    source=source.replace('MusicManager.ResetMusicAfterReload()', 'MusicReload_ResetMusicAfterReload(&MusicManager)')
    return ambience.calls(source.replace('AudioReloadHost_ResetMusic()', 'MusicReload_ResetMusicAfterReload(&MusicManager)'))

def logic(source):
    # Remove the complete reset and its obsolete C++ callback
    if 'cMusicManager::ResetMusicAfterReload(' in source:
        source=source.replace(function(source,'cMusicManager::','ResetMusicAfterReload')+'\n','')
        source=source.replace('uint32 RadioStaticTimer;\n','')
    if 'void\nAudioReloadHost_ResetMusic(' in source:source=source.replace(function(source,'AudioReloadHost_','ResetMusic')+'\n','')
    return ambience.logic(calls(source))

def header(source):
    # Retain owner fields while replacing the member and obsolete callback declaration
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ResetMusicAfterReload|AudioReloadHost_ResetMusic)\(',line))
    return ambience.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def reload(source):
    # Include the C music owner before entering its reset directly
    source=source.replace('#include "AudioManager.h"','#include "AudioManager.h"\n#include "MusicManager.h"',1)
    return calls(source)

def stats(source):
    # Replace class-owned radio storage and the complete getter with C storage and access
    source=source.replace('float CStats::FavoriteRadioStationList[NUM_RADIOS];\n','')
    if 'CStats::GetFavoriteRadioStationList(' in source:source=source.replace(function(source,'CStats::','GetFavoriteRadioStationList')+'\n','')
    source=source.replace('CStats::FavoriteRadioStationList','MusicSavedData_FavoriteRadioStationList')
    source=re.sub(r'\bFavoriteRadioStationList\b','MusicSavedData_FavoriteRadioStationList',source)
    return clock.calls(source)

def stats_header(source):
    # Expose the C radio data instead of static class members
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:FavoriteRadioStationList|GetFavoriteRadioStationList)\b',line))
    return '//+ rouz edit (ChatGPT)\n'+source.replace('#include "audio_enums.h"','#include "audio_enums.h"\n#include "../audio/MusicSavedData.h"',1)+'\n//- rouz edit (ChatGPT)\n'

def storage(source):
    # Move the complete saved-position storage and its three helper bodies into C
    source=source.replace('int32 RadioStationPosition[NUM_RADIOS];\n','')
    for name in SAVED_METHODS:source=source.replace(function(source,'',name)+'\n','')
    return clock.calls(source)

def storage_header(source):
    # Preserve the remaining save interfaces while importing the C helper declarations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+n+r'\(',line) for n in SAVED_METHODS))
    return '//+ rouz edit (ChatGPT)\n'+source.replace('#pragma once','#pragma once\n#include "../audio/MusicSavedData.h"',1)+'\n//- rouz edit (ChatGPT)\n'

def body(source):
    # Preserve signed/unsigned conversions, saved-time capture and live callback order
    source=source[source.index('{'):]
    source=re.sub(r'(?<![\w.>])\b(m_\w+|aListenTimeArray|nFramesSinceCutsceneEnded)\b',r'manager->\1',source)
    source=source.replace('CStats::GetFavoriteRadioStationList','MusicSavedData_GetFavoriteRadioStationList').replace('CTimer::GetTimeInMillisecondsPauseMode()','MusicInitHost_PauseTime()')
    source=source.replace('debug(', 'MUSIC_RELOAD_DEBUG(').replace('FALSE','0').replace('TRUE','1')
    return re.sub(r'\bbool8\b','uint8_t',re.sub(r'\bint32\b','int32_t',source))

def project(source):
    # Compile the complete saved-radio helpers and reset as C11
    for name in ['MusicSavedData','MusicReload']:
        if name+'.c' in source:continue
        a=source.index('    <ClCompile Include="..\\src\\audio\\MusicPositions.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
        source=source[:b]+'\n'+source[a:b].replace('MusicPositions.c',name+'.c')+source[b:]
    return ambience.project(source)
