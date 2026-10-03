"""Known collision-history matching and service orchestration conversion."""
import re
import audio_collision_report_migration as report
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
void
AudioCollisionServiceHost_OneShot(cAudioManager *manager, const cAudioCollision *collision)
{
    // Retain the original collision record reference at the one-shot generator boundary
    manager->SetUpOneShotCollisionSound(*collision);
}

void
AudioCollisionServiceHost_Looping(cAudioManager *manager, const cAudioCollision *collision, uint8_t counter)
{
    // Retain the original collision record reference and history-slot counter
    manager->SetUpLoopingCollisionSound(*collision, counter);
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='void AudioCollisionService_Run(cAudioManager *manager);\nvoid AudioCollisionServiceHost_OneShot(cAudioManager *manager, const cAudioCollision *collision);\nvoid AudioCollisionServiceHost_Looping(cAudioManager *manager, const cAudioCollision *collision, uint8_t counter);\n'
def calls(source):
    # Replace only the former collision-service trampoline
    return report.calls(source.replace('AudioEffectsHost_ServiceCollisions(manager)', 'AudioCollisionService_Run(manager)'))

def owner(source):
    # Move service into C while preserving the two remaining generator boundaries
    if 'cAudioManager::ServiceCollisions(' in source:
        source=source.replace(function(source,'cAudioManager::','ServiceCollisions')+'\n','')+HOST
    if 'AudioEffectsHost_ServiceCollisions(' in source:
        source=source.replace(function(source,'AudioEffectsHost_','ServiceCollisions')+'\n','')
    return report.calls(report.owner(source))

def header(source):
    # Keep every owner field and expose actual C collision-history service
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ServiceCollisions|AudioEffectsHost_ServiceCollisions)\(',line))
    return report.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Retain ordered matching, counter mutation, partial resets and callback-visible reads
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('bool8 ','uint8_t ').replace('TRUE','1').replace('FALSE','0').replace('nil','NULL')
    source=source.replace('SetUpOneShotCollisionSound(', 'AudioCollisionServiceHost_OneShot(manager, &').replace('SetUpLoopingCollisionSound(', 'AudioCollisionServiceHost_Looping(manager, &')
    return report.calls(source)
