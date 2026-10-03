"""Known reverb update and special sound control-flow migration."""
import re
import audio_dispatch_migration as dispatch
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
uint8_t
AudioEnvironmentHost_Replay(void)
{
    // Read replay state before deciding whether to process the player mood
    return CReplay::IsPlayingBack();
}

void
AudioEnvironmentHost_Mood(cAudioManager *manager)
{
    // Invoke player mood at its original special-service point
    manager->ProcessPlayerMood();
}

CVehicle *
AudioEnvironmentHost_Remote(void)
{
    // Capture the focused player's remote vehicle before the player lookup
    return CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle;
}

int32_t
AudioEnvironmentHost_EntityId(const CPlayerPed *player)
{
    // Read each original player audio-entity field access independently
    return player->m_audioEntityId;
}

uint8_t
AudioEnvironmentHost_Entering(CPlayerPed *player)
{
    // Query entering-car state before reading the in-vehicle flag
    return player->EnteringCar();
}

uint8_t
AudioEnvironmentHost_InVehicle(const CPlayerPed *player)
{
    // Read the live in-vehicle flag only after the entering-car test
    return player->bInVehicle;
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='void AudioEnvironment_Reverb(cAudioManager *manager);\nvoid AudioEnvironment_Special(cAudioManager *manager);\nuint8_t AudioEnvironmentHost_Replay(void);\nvoid AudioEnvironmentHost_Mood(cAudioManager *manager);\nCVehicle *AudioEnvironmentHost_Remote(void);\nint32_t AudioEnvironmentHost_EntityId(const CPlayerPed *player);\nuint8_t AudioEnvironmentHost_Entering(CPlayerPed *player);\nuint8_t AudioEnvironmentHost_InVehicle(const CPlayerPed *player);\n'
def calls(source):
    # Replace the former subsystem trampolines with their actual C operations
    return dispatch.calls(source.replace('AudioEffectsHost_ProcessReverb(manager)', 'AudioEnvironment_Reverb(manager)').replace('AudioEffectsHost_ProcessSpecial(manager)', 'AudioEnvironment_Special(manager)'))

def owner(source):
    # Remove control flow from game logic and remove the old owner trampolines
    if 'cAudioManager::ProcessReverb(' in source:
        for name in ('ProcessReverb','ProcessSpecial'):
            source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    if 'AudioEffectsHost_ProcessReverb(' in source:
        for name in ('ProcessReverb','ProcessSpecial'):
            source=source.replace(function(source,'AudioEffectsHost_',name)+'\n','')
    return dispatch.owner(calls(source))

def header(source):
    # Expose actual C operations and keep every stored owner field unchanged
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessReverb|ProcessSpecial|AudioEffectsHost_ProcessReverb|AudioEffectsHost_ProcessSpecial)\(',line))
    return dispatch.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Keep short-circuit gates, live state reads and pause-transition fade order
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('uint32 ', 'uint32_t ').replace('TRUE','1')
    source=source.replace('CReplay::IsPlayingBack()', 'AudioEnvironmentHost_Replay()').replace('ProcessPlayerMood()', 'AudioEnvironmentHost_Mood(manager)')
    source=source.replace('CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle', 'AudioEnvironmentHost_Remote()').replace('FindPlayerPed()', 'AudioPoliceHost_FindPlayer()')
    source=source.replace('playerPed->m_audioEntityId', 'AudioEnvironmentHost_EntityId(playerPed)').replace('playerPed->EnteringCar()', 'AudioEnvironmentHost_Entering(playerPed)').replace('playerPed->bInVehicle', 'AudioEnvironmentHost_InVehicle(playerPed)')
    return source
