# Known complete collision sound generator conversion with direct C service calls
import re
import audio_vehicle_rain_migration as rain
from audio_controls_migration import function
NAMES={'SetUpOneShotCollisionSound':'OneShot','SetUpLoopingCollisionSound':'Looping','SetLoopingCollisionRequestedSfxFreqAndGetVol':'LoopingVolume'}
PROTOTYPES='void AudioCollisionSounds_OneShot(cAudioManager *manager, const cAudioCollision *col);\nvoid AudioCollisionSounds_Looping(cAudioManager *manager, const cAudioCollision *col, uint8_t counter);\nuint32_t AudioCollisionSounds_LoopingVolume(cAudioManager *manager, const cAudioCollision *audioCollision);\n'
HELPER='''static float AudioCollisionSounds_Sqrt(float value)
{
    // Retain the original owner square root helper's nonpositive clamp
    return value <= 0.0f ? 0.0f : sqrtf(value);
}
'''
def table(source):
    # Retain every original one-shot surface sample entry
    start=source.index('static const uint32 gOneShotCol[]');end=source.index(';',start)+1
    return source[start:end].replace('uint32 ', 'uint32_t ')
def calls(source):
    # Remove the two former generator adapters from collision service
    return rain.logic(source.replace('AudioCollisionServiceHost_OneShot(', 'AudioCollisionSounds_OneShot(').replace('AudioCollisionServiceHost_Looping(', 'AudioCollisionSounds_Looping('))
def owner(source):
    # Remove all three generators, their table and the two old member adapters
    if 'cAudioManager::SetUpOneShotCollisionSound(' in source:
        source=source.replace(table(source).replace('uint32_t ', 'uint32 ')+'\n','')
        for name in NAMES:
            source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    for name in ['OneShot','Looping']:
        if 'AudioCollisionServiceHost_'+name+'(' in source:
            source=source.replace(function(source,'AudioCollisionServiceHost_',name)+'\n','')
    return rain.logic(source)

def header(source):
    # Keep owner fields while declaring actual C collision generators
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in list(NAMES)+['AudioCollisionServiceHost_OneShot','AudioCollisionServiceHost_Looping']))
    return rain.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Preserve scalar conversions, live collision reads and callback sequence
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=re.sub(r'\b(col|audioCollision)\.',r'\1->',source)
    for old,new in [('uint16','uint16_t'),('uint32','uint32_t'),('int32','int32_t'),('uint8','uint8_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0')]:
        source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('SetLoopingCollisionRequestedSfxFreqAndGetVol(col)', 'AudioCollisionSounds_LoopingVolume(manager, col)').replace('AudioRequests_Submit(this)', 'AudioRequests_Submit(manager)').replace('Sqrt(', 'AudioCollisionSounds_Sqrt(').replace('Min(', 'AudioCollisionSounds_Min(')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:
        source=re.sub(r'\b'+name+r'\b','AUDIO_COLLISION_SOUNDS_'+name,source)
    return source
