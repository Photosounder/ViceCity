"""Known entity-member removals and direct C caller substitutions."""
import re
from audio_controls_migration import function
METHODS=['CreateEntity','DestroyEntity','GetEntityStatus','SetEntityStatus','GetEntityPointer','PlayOneShot']
FACADES=['CreateEntity','DestroyEntity','GetEntityStatus','SetEntityStatus','PlayOneShot']
def calls(source):
    # Bind each former implicit or global member call to its original owner fields
    for name in METHODS:
        for prefix, context in [('AudioManager.', 'AudioManager.'),('', '')]:
            if name=='CreateEntity':target='AudioManager_CreateEntity('+('&AudioManager' if prefix else 'this')+', '
            else:
                target={'DestroyEntity':'AudioEntities_Destroy('+context+'m_asAudioEntities, '+context+'m_aAudioEntityOrderList, &'+context+'m_nAudioEntitiesCount, '+context+'m_bIsInitialised, ', 'GetEntityStatus':'AudioEntities_GetStatus('+context+'m_asAudioEntities, '+context+'m_bIsInitialised, ', 'SetEntityStatus':'AudioEntities_SetStatus('+context+'m_asAudioEntities, '+context+'m_bIsInitialised, ', 'GetEntityPointer':'AudioEntities_GetPointer('+context+'m_asAudioEntities, '+context+'m_bIsInitialised, ', 'PlayOneShot':'AudioEntities_PlayOneShot('+context+'m_asAudioEntities, &'+context+'m_sAudioScriptObjectManager, '+context+'m_bIsInitialised, '}[name]
            pattern=re.escape(prefix+name)+r'\('
            if not prefix:pattern=r'(?<![\w.:>])'+pattern
            source=re.sub(pattern,lambda m:target,source)
    return source
def owner(source):
    # Remove only the six separately verified member implementations
    for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    return calls(source)
def header(source):
    # Replace the six member declarations with the actual C creation API
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in METHODS))
    return source.replace('void AudioManager_InitState(cAudioManager *manager);','int32_t AudioManager_CreateEntity(cAudioManager *manager, enum eAudioType type, void *entity);\nvoid AudioManager_InitState(cAudioManager *manager);')
def facade(source):
    # Move these five complete public operations to the C module
    for name in FACADES:source=source.replace(function(source,'DMAudio_',name)+'\n','')
    return calls(source)

def creation_body(source):
    # Convert only reference access, owner qualification, and fixed-width C scalars
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.\w])\b(m_[A-Za-z_0-9]+)\b',r'manager->\1',source)
    source=source.replace('tSound &sound =','tSound *sound = &').replace('sound.','sound->')
    for a,b in [('int32','int32_t'),('uint32','uint32_t'),('uint8','uint8_t'),('bool','uint8_t'),('false','0'),('true','1')]:source=re.sub(r'\b'+a+r'\b',b,source)
    return source
