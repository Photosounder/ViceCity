"""Extract complete owner cleanup while retaining proven compiler startup registration."""
from audio_controls_migration import function
import audio_music_state_migration as music
import audio_music_playback_migration as playback
PROTOTYPES='void AudioOwner_Destroy(cAudioManager *manager);\n'
DTOR='cAudioManager::~cAudioManager()\n{\n\t// Delegate the complete owner cleanup to its C implementation\n\tAudioOwner_Destroy(this);\n}'
def logic(source):
    # Retain the original compiler initialization and exit registration sequence
    if 'cAudioManager::~cAudioManager(' in source:source=source.replace(function(source,'cAudioManager::','~cAudioManager'),DTOR)
    return music.logic(source)

def header(source):
    # Expose C cleanup without altering the remaining startup declarations
    return playback.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def project(source):
    # Register the complete C11 owner destruction operation
    if 'AudioOwner.c' in source:return music.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioBank.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return music.project(source[:b]+'\n'+source[a:b].replace('AudioBank.c','AudioOwner.c')+source[b:])
