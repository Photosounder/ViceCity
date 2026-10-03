"""Known top-level service and deferred timer reset migration."""
import re
import audio_collision_report_migration as collision_report
import audio_effects_migration as effects
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nvoid\nAudioServiceHost_ResetMusicTimers(uint32_t time)\n{\n    // Read the music timer argument at the original deferred reset point\n    MusicManager.ResetTimers(time);\n}\n\nuint8_t\nAudioServiceHost_UserPaused(void)\n{\n    // Query the existing user pause flag after retaining the previous pause state\n    return CTimer::GetIsUserPaused();\n}\n\nvoid\nAudioServiceHost_SoundEffects(cAudioManager *manager)\n{\n    // Invoke sound effects at the original service point after reflection updates\n    manager->ServiceSoundEffects();\n}\n\nvoid\nAudioServiceHost_Music(void)\n{\n    // Invoke music after the sound effects service callback completes\n    MusicManager.Service();\n}\n//- rouz edit (ChatGPT)\n'
PED_HOST='//+ rouz edit (ChatGPT)\nuint8_t\nAudioServiceHost_IsPed(CPed *ped)\n{\n    // Preserve the original game type test on each physical entity\n    return ped->IsPed();\n}\n\nvoid\nAudioServiceHost_PedLastStart(CPed *ped, uint32_t time)\n{\n    // Write the previous sound start before evaluating the randomized next start\n    ped->m_lastSoundStart = time;\n}\n\nvoid\nAudioServiceHost_PedSoundStart(CPed *ped, uint32_t time)\n{\n    // Write the randomized sound start using the original unsigned timer arithmetic\n    ped->m_soundStart = time;\n}\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioService_Run(cAudioManager *manager);\nvoid AudioService_ResetLogicTimers(cAudioManager *manager, uint32_t time);\nvoid AudioServiceHost_ResetMusicTimers(uint32_t time);\nuint8_t AudioServiceHost_UserPaused(void);\nvoid AudioServiceHost_SoundEffects(cAudioManager *manager);\nvoid AudioServiceHost_Music(void);\nuint8_t AudioServiceHost_IsPed(CPed *ped);\nvoid AudioServiceHost_PedLastStart(CPed *ped, uint32_t time);\nvoid AudioServiceHost_PedSoundStart(CPed *ped, uint32_t time);\n'

def calls(source):
    # Retain the original post-setup timer reset point and owner
    source=re.sub(r'(?<![\w.:>])ResetAudioLogicTimers\(', 'AudioService_ResetLogicTimers(this, ',source)
    return source.replace('AudioManager.Service()', 'AudioService_Run(&AudioManager)')

def owner(source):
    # Replace the service and game-timer members with C control flow and narrow callbacks
    if 'cAudioManager::Service(' in source:
        source=source.replace(function(source,'cAudioManager::','Service')+'\n','')+HOST
    if 'cAudioManager::ResetAudioLogicTimers(' in source:
        source=source.replace(function(source,'cAudioManager::','ResetAudioLogicTimers')+'\n','')+PED_HOST
    return effects.owner(calls(source))

def header(source):
    # Preserve every owner field while exposing the actual C service operations
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:Service|ResetAudioLogicTimers)\(',line))
    return effects.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def facade(source):
    # Move the public service facade into the C translation unit
    if re.search(r'(?m)^DMAudio_Service\(',source):source=source.replace(function(source,'DMAudio_','Service')+'\n','')
    return collision_report.facade(effects.calls(source))

def body(source):
    # Preserve unconditional RNG draws, live state reads and deferred reset order
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('uint32 ','uint32_t ').replace('FALSE','0')
    source=source.replace('ResetAudioLogicTimers(', 'AudioService_ResetLogicTimers(manager, ').replace('MusicManager.ResetTimers(', 'AudioServiceHost_ResetMusicTimers(')
    source=source.replace('CTimer::GetIsUserPaused()', 'AudioServiceHost_UserPaused()').replace('ServiceSoundEffects()', 'AudioServiceHost_SoundEffects(manager)').replace('MusicManager.Service()', 'AudioServiceHost_Music()')
    source=source.replace('AudioReflections_Update(this)', 'AudioReflections_Update(manager)').replace('AudioMission_Clear(this,', 'AudioMission_Clear(manager,')
    source=source.replace('ped->IsPed()', 'AudioServiceHost_IsPed(ped)').replace('ped->m_lastSoundStart = timer;', 'AudioServiceHost_PedLastStart(ped, timer);').replace('ped->m_soundStart = timer + manager->m_anRandomTable[0] % 3000;', 'AudioServiceHost_PedSoundStart(ped, timer + manager->m_anRandomTable[0] % 3000);')
    return effects.calls(source)
