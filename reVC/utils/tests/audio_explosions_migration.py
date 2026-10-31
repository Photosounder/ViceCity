"""Move the complete explosion sound generator and its direct C dispatcher."""
import re
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint8_t
AudioExplosionsHost_MakesSound(uint8_t id)
{
    // Query the original explosion sound flag before its active counter
    return CExplosion::DoesExplosionMakeSound(id);
}
extern "C" int8_t
AudioExplosionsHost_ActiveCounter(uint8_t id)
{
    // Retain the original signed active-counter result
    return CExplosion::GetExplosionActiveCounter(id);
}
extern "C" void
AudioExplosionsHost_ResetCounter(uint8_t id)
{
    // Consume the original active counter before type and distance queries
    CExplosion::ResetExplosionActiveCounter(id);
}
extern "C" uint8_t
AudioExplosionsHost_Type(uint8_t id)
{
    // Retain the original byte-sized explosion type result
    return CExplosion::GetExplosionType(id);
}
extern "C" void
AudioExplosionsHost_Position(uint8_t id, AudioSoundPosition *position)
{
    // Copy the same original explosion position into the C sound record
    *position = AudioSoundPosition_FromVector(*CExplosion::GetExplosionPosition(id));
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='''void AudioExplosions_Process(cAudioManager *manager, int32_t id);
uint8_t AudioExplosionsHost_MakesSound(uint8_t id);
int8_t AudioExplosionsHost_ActiveCounter(uint8_t id);
void AudioExplosionsHost_ResetCounter(uint8_t id);
uint8_t AudioExplosionsHost_Type(uint8_t id);
void AudioExplosionsHost_Position(uint8_t id, AudioSoundPosition *position);
'''
CONSTANTS='\tEXPLOSION_DEFAULT_MAX_DIST = 200,\n\tEXPLOSION_MOLOTOV_MAX_DIST = 150,\n\tEXPLOSION_MINE_MAX_DIST = 200,\n'
MACRO_NAMES=['RESET_LOOP_OFFSETS','SET_EMITTING_VOLUME','SET_SOUND_REFLECTION','SET_SOUND_REVERB']

def calls(source):
    # Dispatch explosion audio directly from C with the original entity argument
    import audio_fires_migration
    return audio_fires_migration.calls(source.replace('AudioDispatchHost_ProcessExplosions(manager,','AudioExplosions_Process(manager,'))

def logic(source):
    # Remove the owning C++ body and the obsolete dispatch trampoline
    if '#include "AudioManager.h"' in source and 'cAudioManager::ProcessExplosions(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessExplosions')+'\n','').replace(CONSTANTS,'')+HOST
    if 'void\nAudioDispatchHost_ProcessExplosions(' in source:source=source.replace(function(source,'AudioDispatchHost_','ProcessExplosions')+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_fires_migration
    return audio_fires_migration.logic(source)

def header(source):
    # Expose C operations without changing any owner or explosion record field
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessExplosions|AudioDispatchHost_ProcessExplosions)\(',line))
    import audio_fires_migration
    return audio_fires_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve selection, reset timing, repeated scratch reads and each conditional write
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('ARRAY_SIZE(gaExplosion)','NUM_EXPLOSIONS')
    for name,target in [('DoesExplosionMakeSound','MakesSound'),('GetExplosionActiveCounter','ActiveCounter'),('ResetExplosionActiveCounter','ResetCounter'),('GetExplosionType','Type')]:source=source.replace('CExplosion::'+name+'(', 'AudioExplosionsHost_'+target+'(')
    source=source.replace('manager->m_sQueueSample.m_vecPos = AudioSoundPosition_FromVector(*CExplosion::GetExplosionPosition(i));','AudioExplosionsHost_Position(i, &manager->m_sQueueSample.m_vecPos);')
    source=source.replace('manager->m_sQueueSample.m_vecPos = *CExplosion::GetExplosionPosition(i);','AudioExplosionsHost_Position(i, &manager->m_sQueueSample.m_vecPos);')
    source=source.replace('SQR(manager->m_sQueueSample.m_MaxDistance)','(manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)').replace('Sqrt(distSquared)','(distSquared <= 0.0f ? 0.0f : sqrtf(distSquared))').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for a,b in [('uint8','uint8_t'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in MACRO_NAMES:source=re.sub(r'\b'+name+r'\b','AUDIO_EXPLOSIONS_'+name,source)
    return source

def enumeration(source):
    # Preserve every original explicit and implicit explosion type value
    return re.search(r'enum eExplosionType\s*\{.*?\n\};',source,re.S).group()

def types(source):
    # Share the original identifiers while retaining the original game class
    return '//+ rouz edit (ChatGPT)\n'+source.replace(enumeration(source),'#include "ExplosionTypes.h"')+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register the complete C11 generator and shared type header
    for kind,base,target in [('ClCompile','audio\\AudioPedPlayback.c','audio\\AudioExplosions.c'),('ClInclude','weapons\\Explosion.h','weapons\\ExplosionTypes.h')]:
        if target in source:continue
        a=source.index('    <'+kind+' Include="..\\src\\'+base+'"');first=source.index('>',a)
        b=first+1 if source[first-1]=='/' else source.index('</'+kind+'>',a)+len('</'+kind+'>')
        source=source[:b]+'\n'+source[a:b].replace(base,target)+source[b:]
    import audio_fires_migration
    return audio_fires_migration.project(source)
