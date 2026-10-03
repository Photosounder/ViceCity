"""Known audio reload and game-created entity cleanup conversion."""
import re
import audio_lifecycle_migration as lifecycle
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nvoid\nAudioReloadHost_ResetMusic(void)\n{\n    // Reset music at the original reload point before clearing the player speech flag\n    MusicManager.ResetMusicAfterReload();\n}\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioReload_ResetTimers(cAudioManager *manager, uint32_t time);\nvoid AudioReload_DestroyEntities(cAudioManager *manager);\nvoid AudioReloadHost_ResetMusic(void);\n'

def owner(source):
    # Remove the three owner methods and retain only the music manager callback
    if 'cAudioManager::DestroyAllGameCreatedEntities(' in source:
        for name in ('SetOutputMode','ResetTimers','DestroyAllGameCreatedEntities'):
            source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    return lifecycle.owner(source)

def header(source):
    # Preserve all stored fields and expose the C reload operations
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:SetOutputMode|ResetTimers|DestroyAllGameCreatedEntities)\(',line))
    return lifecycle.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def facade(source):
    # Remove only the three facade definitions now owned by the C module
    for name in ('SetOutputMode','ResetTimers','DestroyAllGameCreatedEntities'):
        if re.search(r'(?m)^DMAudio_'+name+r'\(',source):
            source=source.replace(function(source,'DMAudio_',name)+'\n','')
    return lifecycle.facade(source)

def body(source):
    # Keep the original initialization gates, callback order and entity type switch
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('uint32 ','uint32_t ').replace('TRUE','1').replace('FALSE','0').replace('nil','NULL')
    source=source.replace('AudioMission_Clear(this,','AudioMission_Clear(manager,').replace('MusicManager.ResetMusicAfterReload()', 'AudioReloadHost_ResetMusic()').replace('CPools::GetAudioScriptObjectPool()', 'CPools_GetAudioScriptObjectPool()')
    return source
