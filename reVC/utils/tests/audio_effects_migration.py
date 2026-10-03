"""Known sound-effects orchestration and live entity traversal conversion."""
import re
import audio_release_migration as release
from audio_controls_migration import function

MEMBERS = ('ProcessReverb', 'ProcessSpecial', 'ServiceCollisions', 'AddReleasingSounds', 'ProcessActiveQueues', 'ProcessEntity')
HOST = '//+ rouz edit (ChatGPT)\n'
for name in MEMBERS:
    argument = ', int32_t sound' if name == 'ProcessEntity' else ''
    value = 'sound' if name == 'ProcessEntity' else ''
    HOST += 'void\nAudioEffectsHost_' + name + '(cAudioManager *manager' + argument + ')\n{\n    // Invoke the remaining game subsystem at its original service point\n    manager->' + name + '(' + value + ');\n}\n\n'
HOST += '//- rouz edit (ChatGPT)\n'
PROTOTYPES = 'void AudioEffects_Service(cAudioManager *manager);\nvoid AudioEffects_Interrogate(cAudioManager *manager);\n'
for name in MEMBERS:
    PROTOTYPES += 'void AudioEffectsHost_' + name + '(cAudioManager *manager' + (', int32_t sound' if name == 'ProcessEntity' else '') + ');\n'

def calls(source):
    # Replace the former service trampoline with the actual C operation
    return source.replace('AudioServiceHost_SoundEffects(manager)', 'AudioEffects_Service(manager)')

def owner(source):
    # Remove the converted methods and preserve the remaining game callbacks
    if 'cAudioManager::ServiceSoundEffects(' in source and 'cAudioManager::InterrogateAudioEntities(' in source:
        for name in ('ServiceSoundEffects', 'InterrogateAudioEntities'):
            source = source.replace(function(source, 'cAudioManager::', name) + '\n', '')
        source = source.replace(function(source, 'AudioServiceHost_', 'SoundEffects') + '\n', '')
        source += HOST
    return release.owner(calls(source))

def header(source):
    # Expose C orchestration without changing any stored owner field
    source = '\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ServiceSoundEffects|InterrogateAudioEntities|AudioServiceHost_SoundEffects)\(', line))
    return release.header(source.replace('void AudioManager_InitState(cAudioManager *manager);', PROTOTYPES + 'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Keep the original branch structure, repeated state reads and cleanup order
    source = source[source.index('{'):]
    source = re.sub(r'(?<![.>\w])\b(m_\w+)\b', r'manager->\1', source)
    source = source.replace('uint32 ', 'uint32_t ').replace('int32 ', 'int32_t ').replace('nil', 'NULL')
    source = source.replace('CTimer::GetLogicalFramesPassed()', 'AudioPoliceHost_LogicalFramesPassed()')
    source = source.replace('InterrogateAudioEntities()', 'AudioEffects_Interrogate(manager)')
    for name in MEMBERS:
        source = source.replace(name + '()', 'AudioEffectsHost_' + name + '(manager)')
    source = source.replace('ProcessEntity(', 'AudioEffectsHost_ProcessEntity(manager, ')
    source = source.replace('AudioPolice_Service(this)', 'AudioPolice_Service(manager)').replace('AudioMission_Process(this)', 'AudioMission_Process(manager)')
    return release.calls(source.replace('CPools::GetAudioScriptObjectPool()', 'CPools_GetAudioScriptObjectPool()'))
