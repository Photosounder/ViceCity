"""Known entity-kind and physical-object dispatch migration."""
import re
import audio_collision_service_migration as collision_service
from audio_controls_migration import function
INDEXED=('ProcessExplosions','ProcessFires','ProcessWeather','ProcessScriptObject','ProcessWaterCannon')
UNINDEXED=('ProcessFrontEnd','ProcessProjectiles','ProcessGarages','ProcessFireHydrant','ProcessEscalators','ProcessExtraSounds','ProcessBridge')
HOST='//+ rouz edit (ChatGPT)\n'
PROTOTYPES='void AudioDispatch_Entity(cAudioManager *manager, int32_t id);\nvoid AudioDispatch_Physical(cAudioManager *manager, int32_t id);\nint32_t AudioDispatchHost_Area(void);\nint32_t AudioDispatchHost_PhysicalType(void *entity);\nvoid AudioDispatchHost_Vehicle(cAudioManager *manager, void *entity);\nvoid AudioDispatchHost_Ped(cAudioManager *manager, void *entity);\n'
for name in INDEXED+UNINDEXED:
    parameter=', int32_t id' if name in INDEXED else ''
    argument='id' if name in INDEXED else ''
    definition='void\nAudioDispatchHost_'+name+'(cAudioManager *manager'+parameter+')\n{\n    // Invoke the remaining sound generator at the original entity routing point\n    manager->'+name+'('+argument+');\n}\n'
    declaration='void AudioDispatchHost_'+name+'(cAudioManager *manager'+parameter+');\n'
    if name=='ProcessBridge':
        definition='#ifdef GTA_BRIDGE\n'+definition+'#endif\n'
        declaration='#ifdef GTA_BRIDGE\n'+declaration+'#endif\n'
    HOST+=definition+'\n';PROTOTYPES+=declaration
HOST+='''int32_t
AudioDispatchHost_Area(void)
{
    // Read each original current-area field access independently
    return CGame::currArea;
}

int32_t
AudioDispatchHost_PhysicalType(void *entity)
{
    // Query the same base physical object used by the original type switch
    return ((CPhysical *)entity)->GetType();
}

void
AudioDispatchHost_Vehicle(cAudioManager *manager, void *entity)
{
    // Preserve the original stored-pointer cast at the vehicle generator boundary
    manager->ProcessVehicle((CVehicle *)entity);
}

void
AudioDispatchHost_Ped(cAudioManager *manager, void *entity)
{
    // Preserve the original stored-pointer cast at the pedestrian generator boundary
    manager->ProcessPed((CPhysical *)entity);
}
//- rouz edit (ChatGPT)
'''
MACROS='''#ifdef AUDIO_REVERB
#define AUDIO_DISPATCH_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_DISPATCH_REVERB(b)
#endif
'''
def enumeration(source,name):
    # Preserve the original named enum block and every explicit or implicit value
    return re.search(r'enum '+name+r'\s*\{.*?\n\};',source,re.S).group()

def entity_header(source):
    # Separate entity kinds into a header consumable by both languages
    return '//+ rouz edit (ChatGPT)\n'+source.replace(enumeration(source,'eEntityType'),'#include "EntityTypes.h"')+'\n//- rouz edit (ChatGPT)\n'

def game_header(source):
    # Separate area identifiers while retaining the existing game class and level enum
    return '//+ rouz edit (ChatGPT)\n'+source.replace(enumeration(source,'eAreaName'),'#include "GameAreas.h"')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace the former entity trampoline with actual C routing
    return collision_service.calls(source.replace('AudioEffectsHost_ProcessEntity(manager,', 'AudioDispatch_Entity(manager,'))

def owner(source):
    # Remove both switches and retain the remaining game-generator boundaries
    if 'cAudioManager::ProcessEntity(' in source and 'cAudioManager::ProcessPhysical(' in source:
        for name in ('ProcessEntity','ProcessPhysical'):
            source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    if 'AudioEffectsHost_ProcessEntity(' in source:
        source=source.replace(function(source,'AudioEffectsHost_','ProcessEntity')+'\n','')
    return collision_service.owner(calls(source))

def header(source):
    # Expose C routing without changing any stored owner field
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessEntity|ProcessPhysical|AudioEffectsHost_ProcessEntity)\(',line))
    return collision_service.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve status/pause gates, reverb writes, area short-circuiting and pointer reloads
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('TRUE','1').replace('FALSE','0').replace('SET_SOUND_REVERB(', 'AUDIO_DISPATCH_REVERB(')
    source=source.replace('ProcessPhysical(id)', 'AudioDispatch_Physical(manager, id)')
    for name in INDEXED:source=source.replace(name+'(id)', 'AudioDispatchHost_'+name+'(manager, id)')
    for name in UNINDEXED:source=source.replace(name+'()', 'AudioDispatchHost_'+name+'(manager)')
    source=source.replace('CGame::currArea','AudioDispatchHost_Area()')
    source=source.replace('CPhysical *entity = (CPhysical *)','void *entity = ')
    source=source.replace('entity->GetType()', 'AudioDispatchHost_PhysicalType(entity)')
    source=source.replace('ProcessVehicle((CVehicle *)', 'AudioDispatchHost_Vehicle(manager, ').replace('ProcessPed((CPhysical *)', 'AudioDispatchHost_Ped(manager, ')
    return source
