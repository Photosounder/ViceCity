"""Move complete garage and hydrant sound generators into C11."""
import re
from audio_controls_migration import function
METHODS={'ProcessGarages':'Garages','ProcessFireHydrant':'Hydrant'}
CONSTANTS='\tGARAGES_MAX_DIST = 80,\n\tGARAGES_VOLUME = 90,\n\tGARAGES_DOOR_VOLUME = 60,\n\n\tFIRE_HYDRANT_MAX_DIST = 35,\n\tFIRE_HYDRANT_VOLUME = 40,\n'
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint32_t
AudioGarageHydrantHost_Count(void)
{
    // Read the original live garage count at each loop condition
    return CGarages::NumGarages;
}
extern "C" uint8_t
AudioGarageHydrantHost_Type(uint32_t index)
{
    // Read the original live garage type at each eligibility or sample gate
    return CGarages::aGarages[index].m_eGarageType;
}
extern "C" CEntity *
AudioGarageHydrantHost_Door(uint32_t index)
{
    // Capture the original first-door pointer before position and distance queries
    return CGarages::aGarages[index].m_pDoor1;
}
extern "C" uint8_t
AudioGarageHydrantHost_State(uint32_t index)
{
    // Read the original live state without caching subsequent crusher-state queries
    return CGarages::aGarages[index].m_eGarageState;
}
extern "C" void
AudioGarageHydrantHost_Position(void *entity, AudioSoundPosition *position)
{
    // Preserve the original particle-object cast and position query for hydrants
    *position = AudioSoundPosition_FromVector(((CParticleObject *)entity)->GetPosition());
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioGarageHydrant_Garages(cAudioManager *manager);
void AudioGarageHydrant_Hydrant(cAudioManager *manager);
uint32_t AudioGarageHydrantHost_Count(void);
uint8_t AudioGarageHydrantHost_Type(uint32_t index);
CEntity *AudioGarageHydrantHost_Door(uint32_t index);
uint8_t AudioGarageHydrantHost_State(uint32_t index);
void AudioGarageHydrantHost_Position(void *entity, AudioSoundPosition *position);
'''
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']

def enumeration(source,name):
    # Capture original shared garage enum declarations without reconstructing their values
    a=source.index('enum '+name+'\n');b=source.index('};',a)+2
    return source[a:b]

def garage_header(source):
    # Share only enums while preserving all garage and stored-car fields and methods
    a=enumeration(source,'eGarageState');b=enumeration(source,'eGarageType')
    return source.replace(a,'//+ rouz edit (ChatGPT)\n#include "GarageTypes.h"\n//- rouz edit (ChatGPT)').replace(b+'\n','')

def extract(source,prefix='cAudioManager::',name='ProcessGarages'):
    # Delimit the duplicate conditional loop openings without balancing inactive branches
    if name in ['ProcessGarages','Garages']:
        match=re.search(r'(?m)^void\n'+re.escape(prefix+name)+r'\([^\n]*\)\n\{',source);assert match,name
        following='cAudioManager::ProcessFireHydrant' if name=='ProcessGarages' else 'AudioGarageHydrant_Hydrant'
        end=source.find('\nvoid\n'+following+'(',match.end())
        return source[match.start():end if end>=0 else len(source)].rstrip()
    return function(source,prefix,name)

def calls(source):
    # Enter both complete generators directly from C dispatch
    import audio_bridge_migration
    for name,target in METHODS.items():source=source.replace('AudioDispatchHost_'+name+'(manager)','AudioGarageHydrant_'+target+'(manager)')
    return audio_bridge_migration.calls(source)

def logic(source):
    # Remove complete owning methods and obsolete dispatcher trampolines
    import audio_bridge_migration
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessGarages(' in source:
        for name in METHODS:source=source.replace(extract(source,'cAudioManager::',name)+'\n','')
        source=source.replace(CONSTANTS,'')+HOST
    for name in METHODS:
        if 'void\nAudioDispatchHost_'+name+'(' in source:source=source.replace(function(source,'AudioDispatchHost_',name)+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    return audio_bridge_migration.logic(source)

def header(source):
    # Replace operation declarations while preserving every stored owner field
    import audio_bridge_migration
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessGarages|ProcessFireHydrant|AudioDispatchHost_ProcessGarages|AudioDispatchHost_ProcessFireHydrant)\(',line))
    return audio_bridge_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Retain both index widths, cached-distance decisions, event labels and shared-counter wrapping
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    for a,b in [('CGarages::NumGarages','AudioGarageHydrantHost_Count()'),('CGarages::aGarages[i].m_eGarageType','AudioGarageHydrantHost_Type(i)'),('CGarages::aGarages[i].m_pDoor1','AudioGarageHydrantHost_Door(i)'),('CGarages::aGarages[i].m_eGarageState','AudioGarageHydrantHost_State(i)'),('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')]:source=source.replace(a,b)
    for form in ['entity->GetPosition()','AudioSoundPosition_FromVector(entity->GetPosition())']:source=source.replace('manager->m_sQueueSample.m_vecPos = '+form+';','AudioReleaseHost_Position(entity, &manager->m_sQueueSample.m_vecPos);')
    pointer='manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_pEntity'
    for expr in ['((CParticleObject*)'+pointer+')->GetPosition()','AudioSoundPosition_FromVector(((CParticleObject*)'+pointer+')->GetPosition())']:source=source.replace('manager->m_sQueueSample.m_vecPos = '+expr+';','AudioGarageHydrantHost_Position('+pointer+', &manager->m_sQueueSample.m_vecPos);')
    source=re.sub(r'SQR\(([^()]+)\)',r'(\1 * \1)',source)
    for a,b in [('uint8','uint8_t'),('uint32','uint32_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_GARAGE_HYDRANT_'+name,source)
    return source

def project(source):
    # Register the complete C11 generators beside the preceding world-loop conversion
    import audio_bridge_migration
    if 'AudioGarageHydrant.c' not in source:
        a=source.index('    <ClCompile Include="..\\src\\audio\\AudioWorldLoops.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
        source=source[:b]+'\n'+source[a:b].replace('AudioWorldLoops.c','AudioGarageHydrant.c')+source[b:]
    if 'GarageTypes.h' not in source:
        a=source.index('    <ClInclude Include="..\\src\\control\\Garages.h"');b=source.index('>',a)+1
        if not source[a:b].rstrip().endswith('/>'):b=source.index('</ClInclude>',b)+len('</ClInclude>')
        source=source[:b]+'\n'+source[a:b].replace('Garages.h','GarageTypes.h')+source[b:]
    return audio_bridge_migration.project(source)
