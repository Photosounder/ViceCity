"""Move the optional bank helper into C and remove unused owner method scaffolding."""
from audio_controls_migration import function
import audio_owner_lifetime_migration as lifetime
PROTOTYPES='''#ifdef GTA_PS2
void AudioBank_LoadIfNecessary(uint8_t bank);
#endif
'''

def logic(source):
    # Remove the complete member operation without changing any other owner or game code
    if 'cAudioManager::LoadBankIfNecessary(' in source:source=source.replace(function(source,'cAudioManager::','LoadBankIfNecessary')+'\n','')
    return lifetime.logic(source)

def header(source):
    # Retain only constructor and destructor declarations after all other owner operations have moved to C
    a=source.index('\tcAudioManager();');b=source.index('\n#endif\n} cAudioManager;',a)
    source=source[:a]+'\tcAudioManager();\n\t~cAudioManager();\n'+source[b:]
    return lifetime.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve the original byte parameter and both sample-provider calls without owner state
    return source[source.index('{'):]

def project(source):
    # Register the actual C11 bank helper beside the preceding complete setup conversion
    if 'AudioBank.c' in source:return lifetime.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioGameSetup.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return lifetime.project(source[:b]+'\n'+source[a:b].replace('AudioGameSetup.c','AudioBank.c')+source[b:])
