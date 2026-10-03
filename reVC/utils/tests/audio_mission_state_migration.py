"""Known mission-state member removals and direct C call substitutions."""
import re
import audio_police_state_migration as police
import audio_mission_playback_migration as playback
from audio_controls_migration import function
MAP={'GetMissionAudioLoadingStatus':'GetLoadingStatus','PlayLoadedMissionAudio':'AllowPlay','ShouldDuckMissionAudio':'ShouldDuck','IsMissionAudioSamplePlaying':'IsPlaying','IsMissionAudioSampleFinished':'IsFinished','ClearMissionAudio':'Clear','MissionScriptAudioUsesPoliceChannel':'UsesPoliceChannel'}
METHODS=list(MAP)+['SetMissionAudioLocation']
FACADES=['GetMissionAudioLoadingStatus','SetMissionAudioLocation','PlayLoadedMissionAudio','IsMissionAudioSamplePlaying','IsMissionAudioSampleFinished','ClearMissionAudio']
def calls(source, context='this'):
    # Preserve the original call sites while binding explicit live owner state
    for name,target in MAP.items():
        for prefix,owner in [('AudioManager.','&AudioManager'),('',context)]:
            pattern=re.escape(prefix+name)+r'\('
            if not prefix:pattern=r'(?<![\w.:>])'+pattern
            replacement='AudioMission_'+target+'('+('' if target=='UsesPoliceChannel' else owner+', ')
            source=re.sub(pattern,lambda m:replacement,source)
    source=source.replace('AudioManager.SetMissionAudioLocation(', 'AudioMissionPosition_Set(AudioManager.m_vecMissionAudioPosition, AudioManager.m_bIsMissionAudio2D, AudioManager.m_bIsInitialised, ')
    return police.calls(playback.calls(source))
def logic(source):
    # Remove the eight members whose actual C implementations are separately verified
    for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    return calls(source)
def facade(source):
    # Move the complete public mission-state operations into C
    for name in FACADES:source=source.replace(function(source,'DMAudio_',name)+'\n','')
    return calls(source)
PROTOTYPES='uint8_t\nAudioMission_GetLoadingStatus(cAudioManager *manager, uint8_t slot);\nvoid\nAudioMission_AllowPlay(cAudioManager *manager, uint8_t slot);\nuint8_t\nAudioMission_ShouldDuck(cAudioManager *manager, uint8_t slot);\nuint8_t\nAudioMission_IsPlaying(cAudioManager *manager, uint8_t slot);\nuint8_t\nAudioMission_IsFinished(cAudioManager *manager, uint8_t slot);\nvoid\nAudioMission_Clear(cAudioManager *manager, uint8_t slot);\nuint8_t\nAudioMission_UsesPoliceChannel(uint32_t soundMission);\n'
def header(source):
    # Retain all record fields while removing only the converted method declarations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS))
    return source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);')
def body(source):
    # Qualify stored fields and preserve original gates, static history, and write order
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.\w])\b(m_[A-Za-z_0-9]+)\b',r'manager->\1',source)
    source=calls(source,'manager')
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('FALSE','0'),('TRUE','1')]:source=re.sub(r'\b'+a+r'\b',b,source)
    return source
