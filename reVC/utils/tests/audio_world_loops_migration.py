"""Move complete projectile, escalator and arcade generators into C11."""
import re
from audio_controls_migration import function
METHODS={'ProcessProjectiles':'Projectiles','ProcessEscalators':'Escalators','ProcessExtraSounds':'ExtraSounds'}
ARRAY='CVector aVecExtraSoundPosition[] = { CVector(-1042.546f, 88.794f, 11.324f), CVector(-1004.476f, 181.697f, 11.324f) };'
C_ARRAY='AudioSoundPosition aVecExtraSoundPosition[2] = { {-1042.546f, 88.794f, 11.324f}, {-1004.476f, 181.697f, 11.324f} };'
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint8_t
AudioWorldLoopsHost_ProjectileInUse(uint8_t index)
{
    // Read the original projectile eligibility flag at each scan step
    return CProjectileInfo::GetProjectileInfo(index)->m_bInUse;
}
extern "C" int32_t
AudioWorldLoopsHost_ProjectileWeapon(uint8_t index)
{
    // Query the original projectile info again at the weapon-selection point
    return CProjectileInfo::GetProjectileInfo(index)->m_eWeaponType;
}
extern "C" void
AudioWorldLoopsHost_ProjectilePosition(uint8_t index, AudioSoundPosition *position)
{
    // Read the live projectile pointer and position after sample-frequency queries
    *position = AudioSoundPosition_FromVector(CProjectileInfo::ms_apProjectile[index]->GetPosition());
}
extern "C" int32_t
AudioWorldLoopsHost_EscalatorCount(void)
{
    // Read the live signed escalator count at every loop condition
    return CEscalators::NumEscalators;
}
extern "C" uint8_t
AudioWorldLoopsHost_EscalatorActive(int32_t index)
{
    // Query the original escalator activation gate
    return CEscalators::GetEscalator(index).IsActive();
}
extern "C" void
AudioWorldLoopsHost_EscalatorPosition(int32_t index, AudioSoundPosition *position)
{
    // Query the escalator again before copying its live midpoint
    *position = AudioSoundPosition_FromVector(CEscalators::GetEscalator(index).GetPosition());
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioWorldLoops_Projectiles(cAudioManager *manager);
void AudioWorldLoops_Escalators(cAudioManager *manager);
void AudioWorldLoops_ExtraSounds(cAudioManager *manager);
uint8_t AudioWorldLoopsHost_ProjectileInUse(uint8_t index);
int32_t AudioWorldLoopsHost_ProjectileWeapon(uint8_t index);
void AudioWorldLoopsHost_ProjectilePosition(uint8_t index, AudioSoundPosition *position);
int32_t AudioWorldLoopsHost_EscalatorCount(void);
uint8_t AudioWorldLoopsHost_EscalatorActive(int32_t index);
void AudioWorldLoopsHost_EscalatorPosition(int32_t index, AudioSoundPosition *position);
extern AudioSoundPosition aVecExtraSoundPosition[2];
'''
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']

def constants(source):
    # Capture the original private constants for all three complete generators
    a=source.index('\tPROJECTILE_ROCKET_MAX_DIST =');b=source.index('\n\tGARAGES_MAX_DIST',a)
    return source[a:b]

def calls(source):
    # Dispatch directly into the complete C generators
    for name,target in METHODS.items():source=source.replace('AudioDispatchHost_'+name+'(manager)','AudioWorldLoops_'+target+'(manager)')
    import audio_garage_hydrant_migration
    return audio_garage_hydrant_migration.calls(source)

def logic(source):
    # Remove owning members and move the writable arcade positions into C storage
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessProjectiles(' in source:
        source=source.replace(constants(source),'').replace(ARRAY+'\n','')
        for name in METHODS:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    for name in METHODS:
        if 'void\nAudioDispatchHost_'+name+'(' in source:source=source.replace(function(source,'AudioDispatchHost_',name)+'\n','')
    import escalators_migration
    for block in [HOST,escalators_migration.calls(HOST)]:
        if block in source:source=source.replace(block,'')+block
    import audio_garage_hydrant_migration
    return audio_garage_hydrant_migration.logic(source)

def header(source):
    # Replace operation declarations while preserving all stored owner and game fields
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:'+ '|'.join(list(METHODS)+['AudioDispatchHost_'+n for n in METHODS])+r')\(',line))
    import audio_garage_hydrant_migration
    return audio_garage_hydrant_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve live gates, positions, original frequency arithmetic and queue setup order
    import projectile_migration
    source=projectile_migration.legacy_member_spelling(source)
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    for a,b in [('CProjectileInfo::GetProjectileInfo(i)->m_bInUse','AudioWorldLoopsHost_ProjectileInUse(i)'),('CProjectileInfo::GetProjectileInfo(i)->m_eWeaponType','AudioWorldLoopsHost_ProjectileWeapon(i)'),('CEscalators::NumEscalators','AudioWorldLoopsHost_EscalatorCount()'),('CEscalators::GetEscalator(i).IsActive()','AudioWorldLoopsHost_EscalatorActive(i)'),('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')]:source=source.replace(a,b)
    for kind,expr in [('Projectile','CProjectileInfo::ms_apProjectile[i]->GetPosition()'),('Escalator','CEscalators::GetEscalator(i).GetPosition()')]:
        for form in [expr,'AudioSoundPosition_FromVector('+expr+')']:source=source.replace('manager->m_sQueueSample.m_vecPos = '+form+';','AudioWorldLoopsHost_'+kind+'Position(i, &manager->m_sQueueSample.m_vecPos);')
    for form in ['aVecExtraSoundPosition[i]','AudioSoundPosition_FromVector(aVecExtraSoundPosition[i])']:source=source.replace('manager->m_sQueueSample.m_vecPos = '+form+';','manager->m_sQueueSample.m_vecPos = aVecExtraSoundPosition[i];')
    source=source.replace('ARRAY_SIZE(aVecExtraSoundPosition)','(sizeof(aVecExtraSoundPosition) / sizeof(aVecExtraSoundPosition[0]))')
    source=re.sub(r'SQR\(([^()]+)\)',r'(\1 * \1)',source)
    for name in ['distSquared','distance']:source=source.replace('Sqrt('+name+')','('+name+' <= 0.0f ? 0.0f : sqrtf('+name+'))')
    for a,b in [('uint8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_WORLD_LOOPS_'+name,source)
    return source

def project(source):
    # Register all three complete C11 generators beside the previous audio conversion
    import audio_garage_hydrant_migration
    if 'AudioWorldLoops.c' in source:return audio_garage_hydrant_migration.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioWeatherFrontend.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return audio_garage_hydrant_migration.project(source[:b]+'\n'+source[a:b].replace('AudioWeatherFrontend.c','AudioWorldLoops.c')+source[b:])
