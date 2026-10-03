"""Known audio initialization and shutdown migration."""
import re
import audio_service_migration as service
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nvoid\nAudioLifecycleHost_PreInitialise(cAudioManager *manager)\n{\n\t// Invoke the existing game callback at the original lifecycle point\n\tmanager->PreInitialiseGameSpecificSetup();\n}\n\nvoid\nAudioLifecycleHost_PostInitialise(cAudioManager *manager)\n{\n\t// Invoke the existing game callback at the original lifecycle point\n\tmanager->PostInitialiseGameSpecificSetup();\n}\n\nvoid\nAudioLifecycleHost_PreTerminate(cAudioManager *manager)\n{\n\t// Invoke the existing game callback at the original lifecycle point\n\tmanager->PreTerminateGameSpecificShutdown();\n}\n\nvoid\nAudioLifecycleHost_PostTerminate(cAudioManager *manager)\n{\n\t// Invoke the existing game callback at the original lifecycle point\n\tmanager->PostTerminateGameSpecificShutdown();\n}\n\nvoid\nAudioLifecycleHost_MusicInitialise(void)\n{\n\t// Invoke the existing music callback at the original lifecycle point\n\tMusicManager.Initialise();\n}\n\nvoid\nAudioLifecycleHost_MusicTerminate(void)\n{\n\t// Invoke the existing music callback at the original lifecycle point\n\tMusicManager.Terminate();\n}\n\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioLifecycle_Initialise(cAudioManager *manager);\nvoid AudioLifecycle_Terminate(cAudioManager *manager);\nvoid AudioLifecycleHost_PreInitialise(cAudioManager *manager);\nvoid AudioLifecycleHost_PostInitialise(cAudioManager *manager);\nvoid AudioLifecycleHost_PreTerminate(cAudioManager *manager);\nvoid AudioLifecycleHost_PostTerminate(cAudioManager *manager);\nvoid AudioLifecycleHost_MusicInitialise(void);\nvoid AudioLifecycleHost_MusicTerminate(void);\n'

def initialise(source):
    # Delimit the raw conditional function without counting mutually exclusive opening braces
    start=source.index('void\ncAudioManager::Initialise()')
    end=source.index('\nvoid\ncAudioManager::Terminate()',start)
    return source[start:end].rstrip('\n')

def calls(source):
    # Keep the existing destructor termination gate and bind the same live owner
    source=source.replace('AudioManager.Initialise()', 'AudioLifecycle_Initialise(&AudioManager)').replace('AudioManager.Terminate()', 'AudioLifecycle_Terminate(&AudioManager)')
    return re.sub(r'(?<![\w.:>])Terminate\(\)', 'AudioLifecycle_Terminate(this)',source)

def owner(source):
    # Remove only the lifecycle members and retain game/music access as callbacks
    if 'cAudioManager::Initialise(' in source:
        source=source.replace(initialise(source)+'\n','')
        source=source.replace(function(source,'cAudioManager::','Terminate')+'\n','')
        source+=HOST
    return service.owner(calls(source))

def header(source):
    # Preserve all owner fields while replacing the two method declarations
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:Initialise|Terminate)\(',line))
    return service.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def facade(source):
    # Remove only the two facade implementations now defined in C
    for name in ['Initialise','Terminate']:
        if re.search(r'(?m)^DMAudio_'+name+r'\(',source):source=source.replace(function(source,'DMAudio_',name)+'\n','')
    return service.facade(source)

def body(source):
    # Preserve the original gates, channel narrowing, partial clearing and callback sequence
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('uint32 ','uint32_t ').replace('FALSE','0').replace('AudioPolice_Init(this)', 'AudioPolice_Init(manager)')
    source=source.replace('ARRAY_SIZE(manager->m_aAudioEntityOrderList)', '(sizeof(manager->m_aAudioEntityOrderList) / sizeof(manager->m_aAudioEntityOrderList[0]))')
    for event in ['PreInitialise','PostInitialise','PreTerminate','PostTerminate']:
        method=event+('GameSpecificSetup' if 'Initialise' in event else 'GameSpecificShutdown')
        source=source.replace(method+'()', 'AudioLifecycleHost_'+event+'(manager)')
    for event in ['Initialise','Terminate']:source=source.replace('MusicManager.'+event+'()', 'AudioLifecycleHost_Music'+event+'()')
    return re.sub(r'(?<![\w.:>])Terminate\(\)', 'AudioLifecycle_Terminate(manager)',source)
