# Move complete speech routing, phrase selection and all character handlers into C11
import re
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint32_t
AudioPedTalkHost_GetUpTimer(const CPed *ped)
{
    // Read each original get-up timer expression independently
    return ped->m_getUpTimer;
}
extern "C" int16_t
AudioPedTalkHost_Model(const CPed *ped)
{
    // Read the same original signed pedestrian model index
    return ped->GetModelIndex();
}
extern "C" uint32_t *
AudioPedTalkHost_LastComment(CPed *ped)
{
    // Resolve the original writable previous-comment field before phrase selection
    return &ped->m_lastComment;
}
extern "C" int32_t
AudioPedTalkHost_PlayerState(void)
{
    // Query the original live player before reading its state
    return FindPlayerPed()->m_nPedState;
}
//- rouz edit (ChatGPT)
'''
BASE_PROTOTYPES='''uint32_t AudioPedTalk_Select(cAudioManager *manager, CPed *ped, uint16_t sound);
void AudioPedTalk_Phrase(cAudioManager *manager, uint32_t *phrase, uint32_t *prevPhrase, uint32_t sample, uint32_t maxOffset);
uint32_t AudioPedTalkHost_GetUpTimer(const CPed *ped);
int16_t AudioPedTalkHost_Model(const CPed *ped);
uint32_t *AudioPedTalkHost_LastComment(CPed *ped);
int32_t AudioPedTalkHost_PlayerState(void);
'''
def names(source):
    # Keep every original character handler in declaration order
    return list(dict.fromkeys(re.findall(r'cAudioManager::(Get\w*TalkSfx)\(',source)))
def target(name):
    # Retain the original character name in each C operation
    return name[3:-7]
def prototypes(source):
    # Expose all handlers without adding any owner field
    return BASE_PROTOTYPES+''.join('uint32_t AudioPedTalk_'+target(n)+'(cAudioManager *manager, CPed *ped, '+('int32_t model, ' if n=='GetSpecialCharacterTalkSfx' else '')+'uint16_t sound);\n' for n in names(source))
def calls(source):
    # Route speech setup directly to the complete C selector
    return source.replace('AudioPedSpeechHost_Select(', 'AudioPedTalk_Select(')
def logic(source):
    # Remove complete owning bodies and their now-unused cooldown macro
    handlers=names(source)
    moved=False
    for name in ['GetPedCommentSfx','GetPhrase']+handlers:
        if 'cAudioManager::'+name+'(' in source:source=source.replace(function(source,'cAudioManager::',name)+'\n','');moved=True
    if moved:source+=HOST
    if '#define cooldown_phrase' in source:
        a=source.index('#define cooldown_phrase');b=source.index('\n\n',a);source=source[:a]+source[b:]
    source=source.replace('#undef cooldown_phrase\n','')
    if 'AudioPedSpeechHost_Select(' in source:source=source.replace('extern "C" uint32_t\n'+function(source,'AudioPedSpeechHost_','Select')+'\n','')
    # Stabilize the surviving speech adapter block after removing its selector entry
    import audio_ped_speech_migration
    block=audio_ped_speech_migration.HOST.replace('extern "C" uint32_t\n'+function(audio_ped_speech_migration.HOST,'AudioPedSpeechHost_','Select')+'\n','')
    if block in source:source=source.replace(block,'')+block
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_ped_playback_migration
    return audio_ped_playback_migration.logic(source)
def header(source,handlers=None):
    # Replace all speech members and their reference parameters while preserving storage
    if handlers is None:handlers=list(dict.fromkeys(re.findall(r'\b(Get\w*TalkSfx)\(',source)))
    proto=BASE_PROTOTYPES+''.join('uint32_t AudioPedTalk_'+target(n)+'(cAudioManager *manager, CPed *ped, '+('int32_t model, ' if n=='GetSpecialCharacterTalkSfx' else '')+'uint16_t sound);\n' for n in handlers)
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:GetPedCommentSfx|GetPhrase|Get\w*TalkSfx|AudioPedSpeechHost_Select)\(',line))
    import audio_ped_playback_migration
    return audio_ped_playback_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',proto+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source,handlers):
    # Preserve gates, routes, phrase aliasing, random arithmetic and every cooldown site
    name=re.search(r'cAudioManager::(\w+)\(',source).group(1);source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('FindPlayerPed()->m_nPedState','AudioPedTalkHost_PlayerState()').replace('FindPlayerPed()->m_pWanted->GetWantedLevel()','AudioPoliceHost_WantedLevel(AudioPoliceHost_FindPlayer())')
    for a,b in [('ped->m_nPedState','AudioPedOneShotsHost_State(ped)'),('ped->m_getUpTimer','AudioPedTalkHost_GetUpTimer(ped)'),('ped->IsPlayer()','AudioPedOneShotsHost_IsPlayer(ped)'),('ped->GetModelIndex()','AudioPedTalkHost_Model(ped)'),('CTimer::GetTimeInMilliseconds()','AudioPlayerEngineHost_Time()')]:source=source.replace(a,b)
    source=source.replace('GetPhrase(sfx, ped->m_lastComment,','AudioPedTalk_Phrase(manager, &sfx, AudioPedTalkHost_LastComment(ped),')
    for n in handlers:source=re.sub(r'\b'+n+r'\(', 'AudioPedTalk_'+target(n)+'(manager, ',source)
    if name=='GetPhrase':
        for variable in ['phrase','prevPhrase']:source=re.sub(r'\b'+variable+r'\b','(*'+variable+')',source)
    for a,b in [('uint32','uint32_t'),('int32','int32_t'),('cooldown_phrase','AUDIO_PED_TALK_COOLDOWN')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('TRUE','1').replace('PedState objective;','enum PedState objective;')
    return source
def project(source):
    # Register the complete speech selector as C11
    import audio_ped_playback_migration
    if 'AudioPedTalk.c' in source:return audio_ped_playback_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioPedSpeech.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_ped_playback_migration.project(source[:b]+'\n'+source[a:b].replace('AudioPedSpeech.c','AudioPedTalk.c')+source[b:])
