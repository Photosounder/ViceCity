"""Known releasing-sound carryover, attachment and fading conversion."""
import re
import audio_active_migration as active
from audio_controls_migration import function
HOST='''//+ rouz edit (ChatGPT)
float
AudioReleaseHost_TimeStep(void)
{
    // Read the original fixed time step at each fading expression
    return CTimer::GetTimeStepFix();
}

void
AudioReleaseHost_Position(void *entity, AudioSoundPosition *position)
{
    // Copy the current base entity position into the C sound record
    *position = AudioSoundPosition_FromVector(((CEntity *)entity)->GetPosition());
}

uint8_t
AudioReleaseHost_LineClear(const AudioSoundPosition *position)
{
    // Retain the original camera ray and world collision category flags
    return CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), AudioSoundPosition_ToVector(position), true, false, false, false, false, false);
}
//- rouz edit (ChatGPT)
'''
PROTOTYPES='void AudioRelease_Add(cAudioManager *manager);\nfloat AudioReleaseHost_TimeStep(void);\nvoid AudioReleaseHost_Position(void *entity, AudioSoundPosition *position);\nuint8_t AudioReleaseHost_LineClear(const AudioSoundPosition *position);\n'
HELPERS='''static float AudioRelease_Square(float value)
{
    // Preserve the original float square helper and its argument conversion
    return value * value;
}

static float AudioRelease_Sqrt(float value)
{
    // Retain the owner helper's nonpositive clamp before the square root
    return value <= 0.0f ? 0.0f : sqrtf(value);
}
'''

def calls(source):
    # Replace only the former releasing-sound trampoline
    return active.calls(source.replace('AudioEffectsHost_AddReleasingSounds(manager)', 'AudioRelease_Add(manager)'))

def owner(source):
    # Remove the business method and its former callback from the complete owner
    if 'cAudioManager::AddReleasingSounds(' in source and 'AudioEffectsHost_AddReleasingSounds(' in source:
        source=source.replace(function(source,'cAudioManager::','AddReleasingSounds')+'\n','')
        source=source.replace(function(source,'AudioEffectsHost_','AddReleasingSounds')+'\n','')+HOST
    return active.owner(calls(source))

def header(source):
    # Expose actual C operations while keeping every owner field unchanged
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:AddReleasingSounds|AudioEffectsHost_AddReleasingSounds)\(',line))
    return active.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve loop bounds, fading expressions and live callback reads
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('tSound &sample = ', 'tSound *sample = &')
    source=re.sub(r'\bsample\.', 'sample->',source)
    source=source.replace('&sample, sizeof(tSound)', 'sample, sizeof(tSound)')
    source=re.sub(r'\b(?:bool8|uint8)\b','uint8_t',source).replace('FALSE','0').replace('TRUE','1')
    source=source.replace('CEntity* entity = (CEntity*)', 'void *entity = ')
    source=source.replace('sample->m_vecPos = AudioSoundPosition_FromVector(entity->GetPosition());','AudioReleaseHost_Position(entity, &sample->m_vecPos);')
    source=source.replace('CWorld::GetIsLineOfSightClear(TheCamera.GetPosition(), AudioSoundPosition_ToVector(&sample->m_vecPos), true, false, false, false, false, false)', 'AudioReleaseHost_LineClear(&sample->m_vecPos)')
    source=source.replace('Sqrt(', 'AudioRelease_Sqrt(').replace('sq(', 'AudioRelease_Square(')
    source=source.replace('Min(MAX_VOLUME, newVolume)', '(MAX_VOLUME < newVolume ? MAX_VOLUME : newVolume)')
    source=source.replace('CTimer::GetTimeStepFix()', 'AudioReleaseHost_TimeStep()').replace('AudioRequests_Submit(this)', 'AudioRequests_Submit(manager)')
    return source
