"""Move complete comment playback and its debug producer into C11."""
import re
from audio_controls_migration import function

HOST='''//+ rouz edit (ChatGPT)
extern "C" void
AudioPedPlaybackHost_DebugPosition(PedCommentPosition *position)
{
    // Copy the original player-zero position after its live game query
    const CVector &commentPosition = CWorld::Players[0].m_pPed->GetPosition();
    position->x = commentPosition.x;
    position->y = commentPosition.y;
    position->z = commentPosition.z;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioPedPlayback_Debug(cAudioManager *manager, int32_t sound);
void AudioPedPlaybackHost_DebugPosition(PedCommentPosition *position);
'''

def extract(source):
    # Delimit the alternate conditional switch braces without changing either branch
    a=source.index('void\nPedComments_Process(')
    if '\n#pragma endregion' in source[a:]:return source[a:source.index('\n#pragma endregion',a)].rstrip()
    return source[a:source.index('//- rouz edit (ChatGPT)',a)].rstrip()

def debug_body(source):
    # Replace implicit ownership and the single typed game position boundary
    source=source[source.index('{'):]
    source=source.replace('int32 ', 'int32_t ').replace('&m_sPedComments','&manager->m_sPedComments')
    a=source.index('\tconst CVector &commentPosition = ');b=source.index('\n',source.index('pedComment.m_vecPos.z = commentPosition.z;',a))
    source=source[:a]+'\tAudioPedPlaybackHost_DebugPosition(&pedComment.m_vecPos);'+source[b:]
    return source

def playback_body(source):
    # Retain live queue indexing, shared history, backend ordering and global ownership
    # Retain original timer boundaries when upstream calls already use C names
    source=source.replace('Timer_GetIsSlowMotionActive()', 'CTimer::GetIsSlowMotionActive()')
    source=source.replace('(*Timer_GetTimeInMilliseconds())', 'CTimer::GetTimeInMilliseconds()')
    source=source[source.index('{'):]
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('bool8','uint8_t')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('ARRAY_SIZE(prevSamples)','(sizeof(prevSamples) / sizeof(prevSamples[0]))')
    return source.replace('CTimer::GetIsSlowMotionActive()','AudioRequestsHost_SlowMotion()').replace('CTimer::GetTimeInMilliseconds()','AudioPlayerEngineHost_Time()')

def logic(source):
    # Remove the owning C++ bodies and retain only the game position adapter
    import audio_explosions_migration
    if '#include "AudioManager.h"' not in source and HOST not in source:return audio_explosions_migration.logic(source)
    moved=False
    if 'cAudioManager::DebugPlayPedComment(' in source:source=source.replace(function(source,'cAudioManager::','DebugPlayPedComment')+'\n','');moved=True
    if 'void\nPedComments_Process(' in source:source=source.replace(extract(source),'');moved=True
    source=source.replace('AudioManager.DebugPlayPedComment(nDebugPlayPedComment);','AudioPedPlayback_Debug(&AudioManager, nDebugPlayPedComment);')
    if moved:source+=HOST
    if HOST in source:source=source.replace(HOST,'')+HOST
    return audio_explosions_migration.logic(source)

def header(source):
    # Replace the debug member without changing any stored owner field
    source=source.replace('\tvoid DebugPlayPedComment(int32 sound);\n','')
    import audio_explosions_migration
    return audio_explosions_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def project(source):
    # Register the playback module with the existing explicit C11 settings
    import audio_explosions_migration
    if 'AudioPedPlayback.c' in source:return audio_explosions_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioPedTalk.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_explosions_migration.project(source[:b]+'\n'+source[a:b].replace('AudioPedTalk.c','AudioPedPlayback.c')+source[b:])
