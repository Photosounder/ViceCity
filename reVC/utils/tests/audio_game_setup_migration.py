"""Move complete game-specific audio setup and shutdown into C11."""
import re
from audio_controls_migration import function
METHODS={name+('GameSpecificSetup' if 'Initialise' in name else 'GameSpecificShutdown'):name for name in ['PreInitialise','PostInitialise','PreTerminate','PostTerminate']}
HOST='''//+ rouz edit (ChatGPT)
extern "C" void *
AudioGameSetupHost_FireManager(void)
{
    // Return the original global fire-manager address used by entity creation
    return &gFireManager;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioGameSetup_PreInitialise(cAudioManager *manager);
void AudioGameSetup_PostInitialise(cAudioManager *manager);
void AudioGameSetup_PreTerminate(cAudioManager *manager);
void AudioGameSetup_PostTerminate(cAudioManager *manager);
void *AudioGameSetupHost_FireManager(void);
'''

def calls(source):
    # Enter each complete C operation directly at the original lifecycle point
    for target in METHODS.values():source=source.replace('AudioLifecycleHost_'+target+'(manager)','AudioGameSetup_'+target+'(manager)')
    import audio_music_playback_migration as playback
    return playback.lifecycle(source) if '#include "AudioManager.h"' in source and 'AudioLifecycle_Initialise(' in source else playback.calls(source)

def logic(source):
    # Remove game-specific owning methods and the four obsolete lifecycle trampolines
    import audio_bank_migration
    if '#include "AudioManager.h"' in source and 'cAudioManager::PreInitialiseGameSpecificSetup(' in source:
        for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    for target in METHODS.values():
        if 'void\nAudioLifecycleHost_'+target+'(' in source:source=source.replace(function(source,'AudioLifecycleHost_',target)+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    return audio_bank_migration.logic(source)

def header(source):
    # Replace operations while retaining every owner field and its conditional layout
    import audio_bank_migration
    names=list(METHODS)+['AudioLifecycleHost_'+target for target in METHODS.values()]
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in names))
    return audio_bank_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve bank writes, entity ordering, live initialization gates, mission resets and timer sequencing
    unused=any('cAudioManager::'+name+'(' in source for name in ['PreInitialiseGameSpecificSetup','PostTerminateGameSpecificShutdown'])
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('&gFireManager','AudioGameSetupHost_FireManager()').replace('CTimer::GetTimeInMilliseconds()','AudioPlayerEngineHost_Time()')
    source=re.sub(r'\bthis\b','manager',source)
    for a,b in [('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    if unused:source=source.replace('{','{\n\t(void)manager;',1)
    return source

def project(source):
    # Register all four complete C11 operations beside the preceding bridge conversion
    import audio_bank_migration
    if 'AudioGameSetup.c' in source:return audio_bank_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioBridge.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_bank_migration.project(source[:b]+'\n'+source[a:b].replace('AudioBridge.c','AudioGameSetup.c')+source[b:])

def observers():
    # Preserve the older lifecycle fixture's isolated game-callback observations at the new C boundary
    return ''.join('extern "C" void AudioGameSetup_'+target+'(cAudioManager *manager) {\n    // Observe the original game callback at its unchanged lifecycle point\n    manager->'+name+'();\n}\n' for name,target in METHODS.items())
