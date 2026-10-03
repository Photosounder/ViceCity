"""Known active-channel queue matching, update and startup conversion."""
import re
import audio_environment_migration as environment
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
uint8_t
AudioActiveHost_CameraSwitched(void)
{
    // Read the camera switch flag at the original doppler calculation point
    return TheCamera.Get_Just_Switched_Status();
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='void AudioActive_Process(cAudioManager *manager);\nuint8_t AudioActiveHost_CameraSwitched(void);\n'
MACROS='''#define AudioActive_Min(a,b) ((a) < (b) ? (a) : (b))
#define AudioActive_Max(a,b) ((a) > (b) ? (a) : (b))
#define AudioActive_Clamp(v,center,radius) ((v) > (center) ? AudioActive_Min(v, center + radius) : AudioActive_Max(v, center - radius))
'''
def extract(source,signature='void\ncAudioManager::ProcessActiveQueues()'):
    # Delimit the raw conditional body without counting mutually exclusive braces twice
    start=source.index(signature)
    marker='\n#undef WORKING_VOLUME_FIELD\n}'
    end=source.index(marker,start)+len(marker)
    return source[start:end]

def calls(source):
    # Replace the former game trampoline with actual C channel processing
    return environment.calls(source.replace('AudioEffectsHost_ProcessActiveQueues(manager)', 'AudioActive_Process(manager)'))

def owner(source):
    # Remove the complete business member and retain only camera game access
    if 'cAudioManager::ProcessActiveQueues(' in source and 'AudioEffectsHost_ProcessActiveQueues(' in source:
        source=source.replace(extract(source)+'\n','')
        source=source.replace(function(source,'AudioEffectsHost_','ProcessActiveQueues')+'\n','')+HOST
    return environment.owner(calls(source))

def header(source):
    # Keep stored owner fields while exposing the actual C channel operation
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:ProcessActiveQueues|AudioEffectsHost_ProcessActiveQueues)\(',line))
    return environment.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve every channel index, conditional branch and callback-visible state update
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=re.sub(r'(#define WORKING_VOLUME_FIELD )manager->',r'\1',source)
    source=source.replace('tSound &sample = ', 'tSound *sample = &')
    source=re.sub(r'\bsample\.', 'sample->',source)
    source=source.replace('&sample, sizeof(tSound)', 'sample, sizeof(tSound)')
    for old,new in [('bool8','uint8_t'),('uint8','uint8_t'),('int8','int8_t'),('uint32','uint32_t'),('int32','int32_t')]:
        source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('FALSE','0').replace('TRUE','1').replace('CVector position;', 'AudioSoundPosition position;')
    source=re.sub(r'const CVector soundPosition = AudioSoundPosition_ToVector\(&([^;]+)\);',r'const AudioSoundPosition soundPosition = \1;',source)
    source=source.replace('AudioGeometry_TranslateVector(', 'AudioGeometry_Translate(')
    source=source.replace('CTimer::GetTimeScale()', 'AudioPoliceHost_TimeScale()').replace('TheCamera.Get_Just_Switched_Status()', 'AudioActiveHost_CameraSwitched()')
    source=source.replace('Clamp2(', 'AudioActive_Clamp(').replace('Min(', 'AudioActive_Min(')
    return source
