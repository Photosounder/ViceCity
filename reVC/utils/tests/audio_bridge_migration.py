"""Move the complete optional bridge state machine and sound generators into C11."""
import re
from audio_controls_migration import function
METHODS={'ProcessBridge':'Process','ProcessBridgeWarning':'Warning','ProcessBridgeMotor':'Motor','ProcessBridgeOneShots':'OneShots'}
CONSTANTS='\tBRIDGE_MOTOR_MAX_DIST = 400,\n\tBRIDGE_MOTOR_VOLUME = MAX_VOLUME,\n\tBRIDGE_MAX_DIST = BRIDGE_MOTOR_MAX_DIST + 50,\n\n\tBRIDGE_WARNING_VOLUME = 100,\n'
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']
HOST='''//+ rouz edit (ChatGPT)
#ifdef GTA_BRIDGE
extern "C" CEntity *
AudioBridgeHost_Road(void)
{
    // Read the original lift-road pointer at each presence or position query
    return CBridge::pLiftRoad;
}
extern "C" int32_t
AudioBridgeHost_State(void)
{
    // Read the live bridge state separately for each original state-machine gate
    return CBridge::State;
}
extern "C" int32_t
AudioBridgeHost_OldState(void)
{
    // Read the original previous state only after the matching current-state gate
    return CBridge::OldState;
}
extern "C" int32_t
AudioBridgeHost_CommercialPassed(void)
{
    // Preserve the original story-progress gate for the warning generator
    return CStats::CommercialPassed;
}
#endif
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''#ifdef GTA_BRIDGE
void AudioBridge_Process(cAudioManager *manager);
void AudioBridge_Warning(cAudioManager *manager);
void AudioBridge_Motor(cAudioManager *manager);
void AudioBridge_OneShots(cAudioManager *manager);
CEntity *AudioBridgeHost_Road(void);
int32_t AudioBridgeHost_State(void);
int32_t AudioBridgeHost_OldState(void);
int32_t AudioBridgeHost_CommercialPassed(void);
#endif
'''

def enumeration(source):
    # Capture the exact original enum declaration and its unchanged values
    a=source.index('enum bridgeStates {');b=source.index('};',a)+2
    return source[a:b]

def bridge_header(source):
    # Preserve the shared enum extraction before the complete verified bridge class removal
    import bridge_migration
    return bridge_migration.calls(source.replace(enumeration(source),'//+ rouz edit (ChatGPT)\n#include "BridgeStates.h"\n//- rouz edit (ChatGPT)'))

def calls(source):
    # Enter the complete C state machine directly from the optional dispatcher branch
    import audio_game_setup_migration
    return audio_game_setup_migration.calls(source.replace('AudioDispatchHost_ProcessBridge(manager)','AudioBridge_Process(manager)'))

def logic(source):
    # Remove complete owning generators and their obsolete dispatch trampoline
    import audio_game_setup_migration
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessBridge(' in source:
        for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source=source.replace(CONSTANTS,'')+HOST
    if 'void\nAudioDispatchHost_ProcessBridge(' in source:source=source.replace(function(source,'AudioDispatchHost_','ProcessBridge')+'\n','')
    import bridge_migration
    for host in [HOST,bridge_migration.calls(HOST)]:
        if host in source:source=source.replace(host,'')+host
    return audio_game_setup_migration.logic(source)

def header(source):
    # Replace only operation declarations while preserving the conditional owner layout
    import audio_game_setup_migration
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessBridge|ProcessBridgeWarning|ProcessBridgeMotor|ProcessBridgeOneShots|AudioDispatchHost_ProcessBridge)\(',line))
    return audio_game_setup_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve repeated live state and pointer reads, strict distance gates and scratch-write order
    import bridge_migration
    source=bridge_migration.legacy_field_spelling(source)
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    for expr in ['CBridge::pLiftRoad->GetPosition()','AudioSoundPosition_FromVector(CBridge::pLiftRoad->GetPosition())']:source=source.replace('manager->m_sQueueSample.m_vecPos = '+expr+';','AudioReleaseHost_Position(AudioBridgeHost_Road(), &manager->m_sQueueSample.m_vecPos);')
    for a,b in [('CBridge::pLiftRoad','AudioBridgeHost_Road()'),('CBridge::OldState','AudioBridgeHost_OldState()'),('CBridge::State','AudioBridgeHost_State()'),('CStats::CommercialPassed','AudioBridgeHost_CommercialPassed()'),('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')]:source=source.replace(a,b)
    for name,target in METHODS.items():source=source.replace(name+'()','AudioBridge_'+target+'(manager)')
    source=re.sub(r'SQR\(([^()]+)\)',r'(\1 * \1)',source)
    for a,b in [('bool8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_BRIDGE_'+name,source)
    return source

def project(source):
    # Register the C11 bridge generators and shared state header in the existing project
    import audio_game_setup_migration
    if 'AudioBridge.c' not in source:
        a=source.index('    <ClCompile Include="..\\src\\audio\\AudioGarageHydrant.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
        source=source[:b]+'\n'+source[a:b].replace('AudioGarageHydrant.c','AudioBridge.c')+source[b:]
    if 'BridgeStates.h' not in source:
        a=source.index('    <ClInclude Include="..\\src\\control\\Bridge.h"');b=source.index('>',a)+1
        if not source[a:b].rstrip().endswith('/>'):b=source.index('</ClInclude>',b)+len('</ClInclude>')
        source=source[:b]+'\n'+source[a:b].replace('Bridge.h','BridgeStates.h')+source[b:]
    return audio_game_setup_migration.project(source)
