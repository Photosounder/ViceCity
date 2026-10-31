"""Move complete radio statistics and position queries into C11."""
import re
import audio_music_reload_migration as reload
from audio_controls_migration import function
METHODS=['RecordRadioStats','GetTrackStartPos','GetRadioPosition']
PROTOTYPES='''void MusicPositions_RecordRadioStats(cMusicManager *manager);
uint32_t MusicPositions_GetTrackStartPos(cMusicManager *manager, uint32_t track);
uint32_t MusicPositions_GetRadioPosition(cMusicManager *manager, uint32_t station);
'''

def calls(source):
    # Bind global operations to the same music owner
    for name in METHODS:
        source=source.replace('MusicManager.'+name+'()', 'MusicPositions_'+name+'(&MusicManager)')
        source=source.replace('MusicManager.'+name+'(', 'MusicPositions_'+name+'(&MusicManager, ')
    return reload.calls(source)

def logic(source):
    # Replace complete member bodies and their internal call sites
    owner='cMusicManager::cMusicManager(' in source
    for name in METHODS:
        if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
    if re.search(r'(?m)^DMAudio_GetRadioPosition\(',source):source=source.replace(function(source,'DMAudio_','GetRadioPosition')+'\n','')
    if owner:
        for name in METHODS:
            source=re.sub(r'(?<![\w.:>])'+name+r'\(\)', 'MusicPositions_'+name+'(this)',source)
            source=re.sub(r'(?<![\w.:>])'+name+r'\(', 'MusicPositions_'+name+'(this, ',source)
    return reload.logic(calls(source))

def header(source):
    # Retain every field while exposing the complete C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS))
    return reload.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Preserve macro re-evaluation, live timer reads and floating-point promotion
    source=source[source.index('{'):].replace('MusicState_IsInitialised(this)','MusicState_IsInitialised(manager)')
    source=re.sub(r'(?<![\w.>])\b(m_\w+|aListenTimeArray)\b',r'manager->\1',source)
    source=source.replace('CTimer::GetTimeInMillisecondsPauseMode()','MusicInitHost_PauseTime()').replace('GetTrackStartPos(station)','MusicPositions_GetTrackStartPos(manager, station)')
    return re.sub(r'\buint32\b','uint32_t',source).replace('Min(', 'MUSIC_POSITION_MIN(')

def project(source):
    # Register the C11 queries beside music initialization
    if 'MusicPositions.c' in source:return reload.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicInit.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return reload.project(source[:b]+'\n'+source[a:b].replace('MusicInit.c','MusicPositions.c')+source[b:])
