# Move the pedestrian context dispatcher into C without changing the event generator
import re
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
extern "C" void
AudioPedHost_OneShots(cAudioManager *manager, cPedParams *params)
{
    // Invoke the remaining pedestrian event loop at the original dispatch point
    manager->ProcessPedOneShots(*params);
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioPed_Process(cAudioManager *manager, CPed *ped);
void AudioPedHost_OneShots(cAudioManager *manager, cPedParams *params);
'''
def calls(source):
    # Route the physical switch directly into the C pedestrian dispatcher
    import audio_explosions_migration
    return audio_explosions_migration.calls(re.sub(r'AudioDispatchHost_Ped\(manager, ([^;]+)\);',r'AudioPed_Process(manager, (CPed *)\1);',source))
def logic(source):
    # Remove the owning method and its obsolete physical-dispatch trampoline
    if 'cAudioManager::ProcessPed(' in source and 'cAudioManager::ProcessPedOneShots(' not in source:return source
    if 'cAudioManager::ProcessPed(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessPed')+'\n','')+HOST
    if 'AudioDispatchHost_Ped(' in source:source=source.replace(function(source,'AudioDispatchHost_','Ped')+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_ped_one_shots_migration
    return audio_ped_one_shots_migration.logic(source)
def header(source):
    # Replace the member and routing declaration without changing owner storage
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessPed|AudioDispatchHost_Ped)\(',line))
    import audio_ped_one_shots_migration
    return audio_ped_one_shots_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Preserve initialization, position-copy timing, distance and event-loop handoff
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('manager->m_sQueueSample.m_vecPos = AudioSoundPosition_FromVector(ped->GetPosition());','AudioReleaseHost_Position(ped, &manager->m_sQueueSample.m_vecPos);').replace('(CPed *)ped','ped').replace('FALSE','0').replace('ProcessPedOneShots(params)','AudioPedHost_OneShots(manager, &params)')
    import audio_ped_one_shots_migration
    return audio_ped_one_shots_migration.calls(source)
def project(source):
    # Register the C11 source once in both Visual Studio compilation lists
    if 'AudioPed.c' in source:return source
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioTrain.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    import audio_ped_one_shots_migration
    return audio_ped_one_shots_migration.project(source[:b]+'\n'+source[a:b].replace('AudioTrain.c','AudioPed.c')+source[b:])
