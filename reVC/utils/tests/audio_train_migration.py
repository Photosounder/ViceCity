# Move the complete optional train audio generator into C11
import re
from audio_controls_migration import function
CONSTANTS='\tTRAIN_NOISE_FAR_MAX_DIST = 300,\n\tTRAIN_NOISE_NEAR_MAX_DIST = 70,\n\tTRAIN_NOISE_VOLUME = 75,\n'
HOST='''#ifdef GTA_TRAIN
//+ rouz edit (ChatGPT)
extern "C" float
AudioTrainHost_Speed(const CVehicle *vehicle)
{
    // Read each original speed expression on the captured train
    return ((const CTrain *)vehicle)->m_fSpeed;
}
extern "C" float
AudioTrainHost_WagonPosition(const CVehicle *vehicle)
{
    // Read the original captured wagon position for the far-layer gate
    return ((const CTrain *)vehicle)->m_fWagonPosition;
}
//- rouz edit (ChatGPT)
#endif
'''
PROTOTYPES='''#ifdef GTA_TRAIN
uint8_t AudioTrain_Process(cAudioManager *manager, cVehicleParams *params);
float AudioTrainHost_Speed(const CVehicle *vehicle);
float AudioTrainHost_WagonPosition(const CVehicle *vehicle);
#endif
'''
def logic(source):
    # Remove only the optional owning member and its private constants
    if 'cAudioManager::ProcessTrainNoise(' in source:
        source=source.replace(function(source,'cAudioManager::','ProcessTrainNoise')+'\n','')
        source+=HOST
    for line in CONSTANTS.splitlines():source=source.replace(line+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_ped_migration
    return audio_ped_migration.logic(source)

def header(source):
    # Replace the guarded member declaration without changing owner storage
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bProcessTrainNoise\(',line))
    import audio_ped_migration
    return audio_ped_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve original gates, repeated speed reads, frequency arithmetic and request ordering
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('CTrain *train','CVehicle *train').replace('(CTrain *)params->m_pVehicle','params->m_pVehicle').replace('train->m_fSpeed','AudioTrainHost_Speed(train)').replace('train->m_fWagonPosition','AudioTrainHost_WagonPosition(train)')
    for a,b in [('uint8','uint8_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0'),('Min','AUDIO_TRAIN_MIN')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in ['TRAIN_NOISE_FAR_MAX_DIST','TRAIN_NOISE_NEAR_MAX_DIST']:source=source.replace('SQR('+name+')','('+name+' * '+name+')')
    source=source.replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_TRAIN_'+name,source)
    return source

def project(source):
    # Register the guarded C11 source once in either Visual Studio project list
    if 'AudioTrain.c' in source:return source
    a=source.index('    <ClCompile Include="..\\src\\audio\\AudioVehicleOneShots.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    import audio_ped_migration
    return audio_ped_migration.project(source[:b]+'\n'+source[a:b].replace('AudioVehicleOneShots.c','AudioTrain.c')+source[b:])