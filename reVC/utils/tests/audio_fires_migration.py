"""Move the complete fire sound generator and its direct C dispatcher."""
import re
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nextern "C" uint8_t\nAudioFiresHost_Ongoing(uint8_t id)\n{\n    // Read the live ongoing flag at the original short-circuit gate\n    return gFireManager.m_aFires[id].m_bIsOngoing;\n}\nextern "C" uint8_t\nAudioFiresHost_AudioSet(uint8_t id)\n{\n    // Read the audio flag only after the ongoing gate\n    return gFireManager.m_aFires[id].m_bAudioSet;\n}\nextern "C" CEntity *\nAudioFiresHost_Entity(uint8_t id)\n{\n    // Capture the original entity pointer before type and sample queries\n    return gFireManager.m_aFires[id].m_pEntity;\n}\nextern "C" void\nAudioFiresHost_Position(uint8_t id, AudioSoundPosition *position)\n{\n    // Copy the live fire position at the original sound-vector expression\n    *position = AudioSoundPosition_FromVector(gFireManager.m_aFires[id].m_vecPos);\n}\nextern "C" uint8_t\nAudioFiresHost_Water(uint8_t id)\n{\n    // Read the live water flag after the burning sound path\n    return gFireManager.m_aFires[id].m_bExtinguishedWithWater;\n}\nextern "C" void\nAudioFiresHost_ClearWater(uint8_t id)\n{\n    // Clear the original water flag before reading its countdown\n    gFireManager.m_aFires[id].m_bExtinguishedWithWater = FALSE;\n}\nextern "C" float\nAudioFiresHost_Countdown(uint8_t id)\n{\n    // Read the countdown after clearing the original water flag\n    return gFireManager.m_aFires[id].m_fWaterExtinguishCountdown;\n}\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioFires_Process(cAudioManager *manager, int32_t id);\nuint8_t AudioFiresHost_Ongoing(uint8_t id);\nuint8_t AudioFiresHost_AudioSet(uint8_t id);\nCEntity * AudioFiresHost_Entity(uint8_t id);\nvoid AudioFiresHost_Position(uint8_t id, AudioSoundPosition *position);\nuint8_t AudioFiresHost_Water(uint8_t id);\nvoid AudioFiresHost_ClearWater(uint8_t id);\nfloat AudioFiresHost_Countdown(uint8_t id);\n'
CONSTANTS='\tFIRE_DEFAULT_MAX_DIST = 80,\n\tFIRE_DEFAULT_VOLUME = 80,\n\tFIRE_BUILDING_MAX_DIST = 80,\n\tFIRE_BUILDING_VOLUME = 100,\n\tFIRE_PED_MAX_DIST = 25,\n\tFIRE_PED_VOLUME = 60,\n\tFIRE_EXTINGUISH_VOLUME = 100,\n'
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']

def calls(source):
    # Dispatch fire audio directly from C with the original entity argument
    import audio_water_cannon_migration
    return audio_water_cannon_migration.calls(source.replace('AudioDispatchHost_ProcessFires(manager,','AudioFires_Process(manager,'))

def logic(source):
    # Remove the owning C++ body and the obsolete dispatch trampoline
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessFires(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessFires')+'\n','').replace(CONSTANTS,'')+HOST
    if 'void\nAudioDispatchHost_ProcessFires(' in source:source=source.replace(function(source,'AudioDispatchHost_','ProcessFires')+'\n','')
    import fire_migration
    for block in [HOST,fire_migration.calls(HOST)]:
        if block in source:source=source.replace(block,'')+block
    import audio_water_cannon_migration
    return audio_water_cannon_migration.logic(source)

def header(source):
    # Expose C operations without changing any owner or fire record field
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessFires|AudioDispatchHost_ProcessFires)\(',line))
    import audio_water_cannon_migration
    return audio_water_cannon_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve captured entity ownership, live fire reads and both complete sound paths
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('gFireManager.m_aFires[i].m_bExtinguishedWithWater = FALSE;','AudioFiresHost_ClearWater(i);')
    for field,name in [('m_bIsOngoing','Ongoing'),('m_bAudioSet','AudioSet'),('m_pEntity','Entity'),('m_bExtinguishedWithWater','Water'),('m_fWaterExtinguishCountdown','Countdown')]:source=source.replace('gFireManager.m_aFires[i].'+field,'AudioFiresHost_'+name+'(i)')
    source=source.replace('entity->GetType()','AudioPedOneShotsHost_EntityType(entity)')
    source=source.replace('manager->m_sQueueSample.m_vecPos = AudioSoundPosition_FromVector(gFireManager.m_aFires[i].m_vecPos);','AudioFiresHost_Position(i, &manager->m_sQueueSample.m_vecPos);')
    source=source.replace('manager->m_sQueueSample.m_vecPos = gFireManager.m_aFires[i].m_vecPos;','AudioFiresHost_Position(i, &manager->m_sQueueSample.m_vecPos);')
    source=source.replace('SQR(manager->m_sQueueSample.m_MaxDistance)','(manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)').replace('Sqrt(distSquared)','(distSquared <= 0.0f ? 0.0f : sqrtf(distSquared))').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for a,b in [('uint8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_FIRES_'+name,source)
    return source

def project(source):
    # Register the complete C11 fire generator with the existing source settings
    import audio_water_cannon_migration
    if 'AudioFires.c' in source:return audio_water_cannon_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioExplosions.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_water_cannon_migration.project(source[:b]+'\n'+source[a:b].replace('AudioExplosions.c','AudioFires.c')+source[b:])
