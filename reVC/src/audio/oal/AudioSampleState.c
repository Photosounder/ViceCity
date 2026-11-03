//+ rouz edit (ChatGPT)
#include "config.h"
#if defined(AUDIO_OAL) || defined(AUDIO_MSS)
#include "AudioManager.h"
#include "AudioProvider.h"
#include "MusicManager.h"
#include "Timer.h"
#include "AudioSampleHost.h"
#include "AudioStreamHost.h"

bool AudioStream_IsCutscene(void)
{
    // Preserve the stream worker's direct music-mode query
    return MusicManager.m_nMusicMode == MUSICMODE_CUTSCENE;
}

bool AudioSample_IsAudioInitialised(void)
{
    // Query game state at the C audio boundary
    return AudioManager.m_bIsInitialised;
}

bool AudioSample_IsMusicInitialised(void)
{
    // Query game state at the C audio boundary
    return MusicState_IsInitialised(&MusicManager);
}

bool AudioSample_IsCodePaused(void)
{
    // Query game state at the C audio boundary
    return Timer_GetIsCodePaused();
}

int AudioSample_GetMusicMode(void)
{
    // Query game state at the C audio boundary
    return MusicState_GetMusicMode(&MusicManager);
}

int AudioSample_GetCurrentTrack(void)
{
    // Query game state at the C audio boundary
    return MusicState_GetCurrentTrack(&MusicManager);
}


bool AudioSample_CheckForMusicInterruptions(void)
{
    // Query game state at the C audio boundary
    return MusicState_CheckForMusicInterruptions(&MusicManager);
}

int32_t AudioSample_GetRandomValue(unsigned index)
{
    // Query game state at the C audio boundary
    return AudioManager.m_anRandomTable[index];
}

uint32_t AudioSample_GetFrameCounter(void)
{
    // Query game state at the C audio boundary
    return AudioManager.m_FrameCounter;
}

#ifdef AUDIO_REFLECTIONS
const float * AudioSample_GetReflectionDistances(void)
{
    // Query game state at the C audio boundary
    return AudioManager.m_afReflectionsDistances;
}
#endif

#ifdef AUDIO_MSS
void AudioSample_SetCurrent3DProvider(uint8_t index)
{
    // Preserve provider changes through the game audio manager
    AudioProvider_SetCurrent(AudioManager.m_bIsInitialised, index, &AudioManager.m_nActiveSamples, &AudioManager.m_nActiveQueue, AudioManager.m_aRequestedOrderList, AudioManager.m_nRequestedCount, AudioManager.m_asActiveSamples);
}
#endif
#endif
//- rouz edit (ChatGPT)
