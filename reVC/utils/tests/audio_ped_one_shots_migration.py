# Move the complete pedestrian event loop and its shared histories into C11
import re
from audio_controls_migration import function
ENUMS=('PedFightMoves','PedState','eMoveState')
QUERIES=[('CEntity *','Entity','CPed *ped','ped','Convert the pedestrian to its original entity base'),('CWeapon *','Weapon','CPed *ped','ped->GetWeapon()','Capture the original selected weapon pointer'),('int32_t','WeaponType','const CWeapon *weapon','weapon->m_eWeaponType','Read the original captured weapon type'),('uint8_t','IsPlayer','CPed *ped','ped->IsPlayer()','Query the live pedestrian player predicate'),('CVehicle *','Vehicle','const CPed *ped','ped->m_pMyVehicle','Read the live pedestrian vehicle pointer'),('uint8_t','InAir','const CPed *ped','ped->bIsInTheAir','Read the original airborne bit'),('uint8_t','Surface','const CPed *ped','ped->m_nSurfaceTouched','Read the original touched surface'),('int32_t','MoveState','const CPed *ped','ped->m_nMoveState','Read the live movement state'),('uint32_t','FightMove','const CPed *ped','ped->m_curFightMove','Read the captured fight-move field'),('int32_t','State','const CPed *ped','ped->m_nPedState','Read the original pedestrian state'),('void *','DamageEntity','const CPed *ped','ped->m_pDamageEntity','Capture the original damage entity pointer'),('uint8_t','EntityType','const void *entity','((const CEntity *)entity)->GetType()','Read the damage entity type without changing its pointer'),('CPed *','Player','void','FindPlayerPed()','Query the original player pedestrian at each step event')]
HOST='//+ rouz edit (ChatGPT)\n'
PROTOTYPES='''#ifdef __cplusplus
class CWeapon;
#else
typedef struct CWeapon CWeapon;
#endif
void AudioPedOneShots_Process(cAudioManager *manager, cPedParams *params);
'''
for result,name,parameter,expression,comment in QUERIES:
    HOST+='extern "C" '+result+'\nAudioPedOneShotsHost_'+name+'('+parameter+')\n{\n    // '+comment+'\n    return '+expression+';\n}\n'
    PROTOTYPES+=result+' AudioPedOneShotsHost_'+name+'('+parameter+');\n'
HOST+='//- rouz edit (ChatGPT)\n'
def constants(source):
    # Extract every private pedestrian event constant in its original declaration order
    return ''.join(line+'\n' for line in source.splitlines() if re.match(r'\s*PED_ONE_SHOT_\w+\s*=',line))
def calls(source):
    # Route the C pedestrian dispatcher directly into its C event loop
    return source.replace('AudioPedHost_OneShots(', 'AudioPedOneShots_Process(')
def logic(source):
    # Remove the entire owning member, private constants and obsolete trampoline
    if 'cAudioManager::ProcessPedOneShots(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessPedOneShots')+'\n','')+HOST
    if 'AudioPedHost_OneShots(' in source:source=source.replace('extern "C" void\n'+function(source,'AudioPedHost_','OneShots')+'\n','')
    for line in constants(source).splitlines():source=source.replace(line+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_ped_speech_migration
    return audio_ped_speech_migration.logic(source)
def header(source):
    # Replace owning declarations while retaining every owner field
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessPedOneShots|AudioPedHost_OneShots)\(',line))
    import audio_ped_speech_migration
    return audio_ped_speech_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Preserve event branches, captured pointers, live queries and callback order
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('params->m_pPed->m_pMyVehicle->GetModelIndex()','AudioVehicleReverseHost_Model(AudioPedOneShotsHost_Vehicle(params->m_pPed))').replace('params->m_pPed->m_pMyVehicle','AudioPedOneShotsHost_Vehicle(params->m_pPed)').replace('((CPed*)damageEntity)->m_curFightMove','AudioPedOneShotsHost_FightMove((CPed *)damageEntity)')
    for variable in ['params->m_pPed','ped']:
        for field,name in [('GetWeapon()','Weapon'),('IsPlayer()','IsPlayer'),('bIsInTheAir','InAir'),('m_nSurfaceTouched','Surface'),('m_nMoveState','MoveState'),('m_curFightMove','FightMove'),('m_nPedState','State'),('m_pDamageEntity','DamageEntity')]:source=source.replace(variable+'->'+field,'AudioPedOneShotsHost_'+name+'('+variable+')')
    source=source.replace('weapon->m_eWeaponType','AudioPedOneShotsHost_WeaponType(weapon)').replace('damageEntity->GetType()','AudioPedOneShotsHost_EntityType(damageEntity)').replace('CEntity* damageEntity','void *damageEntity').replace('FindPlayerPed()','AudioPedOneShotsHost_Player()').replace('SetupPedComments(params,','AudioVehicleOneShotsHost_PedComments(manager, params,').replace('AudioCollisionReport_Report(this, params->m_pPed, params->m_pPed,','AudioCollisionReport_Report(manager, AudioPedOneShotsHost_Entity(params->m_pPed), AudioPedOneShotsHost_Entity(params->m_pPed),').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('int32','int32_t'),('int16','int16_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=re.sub(r'SQR\((PED_ONE_SHOT_\w+)\)',r'(\1 * \1)',source)
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_PED_ONE_SHOTS_'+name,source)
    import audio_ped_speech_migration
    return audio_ped_speech_migration.calls(source)
def enumeration(source,name):
    # Preserve the complete original named enum and every implicit value
    return re.search(r'enum '+name+r'\s*\{.*?\n\};',source,re.S).group()
def types(source):
    # Share the original state enums and forward the explicit C range-pointer conversion
    import ranges_migration
    if 'enum PedFightMoves' not in source:return ranges_migration.calls(source)
    for i,name in enumerate(ENUMS):source=source.replace(enumeration(source,name),'#include "PedStates.h"' if i==0 else '')
    return ranges_migration.calls('//+ rouz edit (ChatGPT)\n'+source+'\n//- rouz edit (ChatGPT)\n')
def project(source):
    # Register the owning C11 source and shared enum header once
    for kind,base,target in [('ClCompile','audio\\AudioPed.c','audio\\AudioPedOneShots.c'),('ClInclude','peds\\Ped.h','peds\\PedStates.h')]:
        if target in source:continue
        a=source.index('    <'+kind+' Include="..\\src\\'+base+'"');first=source.index('>',a)
        b=first+1 if source[first-1]=='/' else source.index('</'+kind+'>',a)+len('</'+kind+'>')
        source=source[:b]+'\n'+source[a:b].replace(base,target)+source[b:]
    import audio_ped_speech_migration
    return audio_ped_speech_migration.project(source)
