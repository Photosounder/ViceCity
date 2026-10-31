"""Move the complete water-cannon sound generator into C11."""
import re
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
extern "C" int32_t
AudioWaterCannonHost_Id(uint32_t index)
{
    // Read the original live cannon identifier at its eligibility gate
    return CWaterCannons::aCannons[index].m_nId;
}
extern "C" void
AudioWaterCannonHost_Position(uint32_t index, AudioSoundPosition *position)
{
    // Preserve the original cannon-zero position and live segment-index expression
    *position = AudioSoundPosition_FromVector(CWaterCannons::aCannons[0].m_avecPos[CWaterCannons::aCannons[index].m_nCur]);
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioWaterCannon_Process(cAudioManager *manager, int32_t id);
int32_t AudioWaterCannonHost_Id(uint32_t index);
void AudioWaterCannonHost_Position(uint32_t index, AudioSoundPosition *position);
'''
CONSTANTS='\tWATER_CANNON_MAX_DIST = 30,\n\tWATER_CANNON_VOLUME = 50,\n'
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']

def calls(source):
    # Dispatch directly into the C generator with the original entity argument
    import audio_script_sounds_migration
    return audio_script_sounds_migration.calls(source.replace('AudioDispatchHost_ProcessWaterCannon(manager,','AudioWaterCannon_Process(manager,'))

def logic(source):
    # Remove the owning member and obsolete dispatcher trampoline
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessWaterCannon(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessWaterCannon')+'\n','').replace(CONSTANTS,'')+HOST
    if 'void\nAudioDispatchHost_ProcessWaterCannon(' in source:source=source.replace(function(source,'AudioDispatchHost_','ProcessWaterCannon')+'\n','')
    import water_cannon_migration
    current_host=water_cannon_migration.calls(HOST)
    if HOST in source:source=source.replace(HOST,'')+HOST
    elif current_host in source:source=source.replace(current_host,'')+current_host
    import audio_script_sounds_migration
    return audio_script_sounds_migration.logic(source)

def header(source):
    # Replace declarations without changing any stored owner or cannon field
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessWaterCannon|AudioDispatchHost_ProcessWaterCannon)\(',line))
    import audio_script_sounds_migration
    return audio_script_sounds_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve live index selection, strict distance gates and scratch/request ordering
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    # Follow either historical class storage or its separately audited C relocation
    for storage in ['CWaterCannons::aCannons','WaterCannons_aCannons']:
        source=source.replace(storage+'[i].m_nId','AudioWaterCannonHost_Id(i)')
        position=storage+'[0].m_avecPos['+storage+'[i].m_nCur]'
        for expr in ['AudioSoundPosition_FromVector('+position+')',position]:source=source.replace('manager->m_sQueueSample.m_vecPos = '+expr+';','AudioWaterCannonHost_Position(i, &manager->m_sQueueSample.m_vecPos);')
    source=source.replace('SQR(WATER_CANNON_MAX_DIST)','(WATER_CANNON_MAX_DIST * WATER_CANNON_MAX_DIST)').replace('Sqrt(distSquared)','(distSquared <= 0.0f ? 0.0f : sqrtf(distSquared))').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for a,b in [('uint32','uint32_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_WATER_CANNON_'+name,source)
    return source

def project(source):
    # Register the complete C11 generator with the existing source settings
    import audio_script_sounds_migration
    if 'AudioWaterCannon.c' in source:return audio_script_sounds_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioFires.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_script_sounds_migration.project(source[:b]+'\n'+source[a:b].replace('AudioFires.c','AudioWaterCannon.c')+source[b:])
