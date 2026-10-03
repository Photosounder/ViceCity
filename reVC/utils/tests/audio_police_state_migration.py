"""Known police state, initialization and direct C caller migrations."""
import re
import audio_geometry_migration as geometry
import audio_game_entries_migration as entries
import audio_police_reports_migration as reports
import audio_police_channel_migration as channel
from audio_controls_migration import function
STORAGE='struct tPoliceRadioZone {\n\tchar m_aName[8];\n\tuint32 m_nSampleIndex;\n\tint32 field_12;\n};\n\ntPoliceRadioZone ZoneSfx[NUMAUDIOZONES];\n\nuint32 g_nMissionAudioSfx = TOTAL_AUDIO_SAMPLES;\nint8 g_nMissionAudioPlayingStatus = PLAY_STATUS_FINISHED;\nbool8 gSpecialSuspectLastSeenReport;\nuint32 gMinTimeToNextReport[NUM_CRIME_TYPES];\n'
PROTOTYPES='void AudioPolice_Init(cAudioManager *manager);\nvoid AudioPolice_Reset(cAudioManager *manager);\n'

MAP={'InitialisePoliceRadioZones':'InitZones','InitialisePoliceRadio':'Init','ResetPoliceRadio':'Reset','SetMissionScriptPoliceAudio':'SetMission','GetMissionScriptPoliceAudioPlayingStatus':'GetMissionStatus'}
def calls(source):
    # Preserve owner identity and initialization gates at existing game callers
    source=source.replace('AudioManager.ResetPoliceRadio()', 'AudioPolice_Reset(&AudioManager)')
    for name,target in [('InitialisePoliceRadioZones','InitZones'),('InitialisePoliceRadio','Init'),('ResetPoliceRadio','Reset')]:
        source=re.sub(r'(?<![\w.:>])'+name+r'\(\)', 'AudioPolice_'+target+'('+('' if target=='InitZones' else 'this')+')',source)
    return channel.calls(source)

def radio(source):
    # Remove exactly the migrated storage and five original methods
    source=source.replace(STORAGE+'\n','')
    for name in MAP:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    return channel.radio(calls(source))

def logic(source):
    # Remove the former police bridges now that playback can call C directly
    for name in ('SetPolice','PoliceStatus'):source=source.replace(function(source,'AudioMissionHost_',name)+'\n','')
    return entries.logic(source)

def header(source):
    # Keep the owner record while removing obsolete methods and host declarations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in list(MAP)+['AudioMissionHost_SetPolice','AudioMissionHost_PoliceStatus']))
    return channel.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def facade(source):
    # Move the public radio reset implementation into C unchanged at its boundary
    return entries.facade(reports.facade(source.replace(function(source,'DMAudio_','ResetPoliceRadio')+'\n','')))

def playback(source):
    # Replace only the police bridges with direct C state operations
    return geometry.calls(source.replace('AudioMissionHost_SetPolice(manager, ', 'AudioPolice_SetMission(manager->m_bIsInitialised, ').replace('AudioMissionHost_PoliceStatus(manager)', 'AudioPolice_GetMissionStatus()'))

def body(source,name):
    # Retain reset order, partial writes, and the original backend conditionals
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.\w])\b(m_\w+|m_FrameCounter)\b',r'manager->\1',source)
    source=source.replace('int32 ', 'int32_t ').replace('FALSE','0')
    if name=='SetMissionScriptPoliceAudio':source=source.replace('manager->m_bIsInitialised','initialized')
    return source.replace('InitialisePoliceRadio();','AudioPolice_Init(manager);')
