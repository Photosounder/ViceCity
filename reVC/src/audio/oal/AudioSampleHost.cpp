//+ rouz edit (ChatGPT)
#ifdef _WIN32
#include <windows.h>
#endif
#include "common.h"
#if defined(AUDIO_OAL) || defined(AUDIO_MSS)
#include "AudioManager.h"
#include "AudioProvider.h"
#include "MusicManager.h"
#include "Timer.h"
#ifdef AUDIO_MSS
#include "Frontend.h"
#include "DMAudio.h"
#endif
#include "crossplatform.h"
#include "AudioSampleHost.h"
#include "AudioStreamHost.h"

extern "C" bool AudioStream_IsCutscene(void)
{
    // Preserve the stream worker's direct music-mode query
    return MusicManager.m_nMusicMode == MUSICMODE_CUTSCENE;
}

extern "C" bool AudioSample_IsAudioInitialised(void)
{
    // Query game state at the C audio boundary
    return AudioManager.m_bIsInitialised;
}

extern "C" bool AudioSample_IsMusicInitialised(void)
{
    // Query game state at the C audio boundary
    return MusicManager.IsInitialised();
}

extern "C" bool AudioSample_IsCodePaused(void)
{
    // Query game state at the C audio boundary
    return CTimer::GetIsCodePaused();
}

extern "C" int AudioSample_GetMusicMode(void)
{
    // Query game state at the C audio boundary
    return MusicManager.GetMusicMode();
}

extern "C" int AudioSample_GetCurrentTrack(void)
{
    // Query game state at the C audio boundary
    return MusicManager.GetCurrentTrack();
}

extern "C" int AudioSample_GetRadioInCar(void)
{
    // Query game state at the C audio boundary
    return MusicManager.GetRadioInCar();
}

extern "C" bool AudioSample_CheckForMusicInterruptions(void)
{
    // Query game state at the C audio boundary
    return MusicManager.CheckForMusicInterruptions();
}

extern "C" int32_t AudioSample_GetRandomValue(unsigned index)
{
    // Query game state at the C audio boundary
    return AudioManager.m_anRandomTable[index];
}

extern "C" uint32_t AudioSample_GetFrameCounter(void)
{
    // Query game state at the C audio boundary
    return AudioManager.m_FrameCounter;
}

#ifdef AUDIO_REFLECTIONS
extern "C" const float * AudioSample_GetReflectionDistances(void)
{
    // Query game state at the C audio boundary
    return AudioManager.m_afReflectionsDistances;
}
#endif

#ifdef AUDIO_MSS
extern "C" void AudioSample_SetCurrent3DProvider(uint8_t index)
{
    // Preserve provider changes through the game audio manager
    AudioProvider_SetCurrent(AudioManager.m_bIsInitialised, index, &AudioManager.m_nActiveSamples, &AudioManager.m_nActiveQueue, AudioManager.m_aRequestedOrderList, AudioManager.m_nRequestedCount, AudioManager.m_asActiveSamples);
}
extern "C" bool AudioSample_WaitForUserCD(void)
{
    // Run the existing CD prompt and report the resulting quit decision
    FrontEndMenuManager.WaitForUserCD();
    return FrontEndMenuManager.m_bQuitGameNoCD;
}
extern "C" void AudioSample_ApplyCDVolume(bool available)
{
    // Restore menu volume preferences or silence audio before the next service pass
    DMAudio_SetMusicMasterVolume(available ? FrontEndMenuManager.m_PrefsMusicVolume : 0);
    DMAudio_SetEffectsMasterVolume(available ? FrontEndMenuManager.m_PrefsSfxVolume : 0);
    DMAudio_Service();
}
#endif

#ifndef _WIN32
extern "C" char *AudioStream_CasePath(const char *filename)
{
    // Retain path correction and its caller-owned allocation
    return casepath(filename);
}
extern "C" FILE *AudioSample_CaseOpen(const char *filename, const char *mode)
{
    // Retain case-insensitive file lookup through the platform implementation
    return fcaseopen(filename, mode);
}
static void CopyFindData(AudioSampleFindData *out, const WIN32_FIND_DATA *data)
{
    // Copy the existing directory search state without adding allocations
    memcpy(out->extension, data->extension, sizeof(out->extension));
    memcpy(out->folder, data->folder, sizeof(out->folder));
    memcpy(out->cFileName, data->cFileName, sizeof(out->cFileName));
    out->ftLastWriteTime = data->ftLastWriteTime;
}
extern "C" void *AudioSample_FindFirstFile(const char *path, AudioSampleFindData *out)
{
    // Use the platform's original wildcard enumeration and directory handle
    WIN32_FIND_DATA data = {};
    HANDLE handle = FindFirstFile(path, &data);
    CopyFindData(out, &data);
    return handle;
}
extern "C" bool AudioSample_FindNextFile(void *handle, AudioSampleFindData *data)
{
    // Restore search state and return the platform's next directory entry
    WIN32_FIND_DATA native = {};
    memcpy(native.extension, data->extension, sizeof(native.extension));
    memcpy(native.folder, data->folder, sizeof(native.folder));
    memcpy(native.cFileName, data->cFileName, sizeof(native.cFileName));
    native.ftLastWriteTime = data->ftLastWriteTime;
    bool found = FindNextFile(handle, &native);
    CopyFindData(data, &native);
    return found;
}
extern "C" void AudioSample_FindClose(void *handle)
{
    // Close the original platform directory handle
    FindClose(handle);
}
#endif
#endif

//- rouz edit (ChatGPT)
