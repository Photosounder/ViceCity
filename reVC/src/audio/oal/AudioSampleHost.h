#pragma once
//+ rouz edit (ChatGPT)
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "AudioReflectionTypes.h"
#ifdef __cplusplus
extern "C" {
#endif
bool AudioSample_IsAudioInitialised(void);
bool AudioSample_IsMusicInitialised(void);
bool AudioSample_IsCodePaused(void);
int AudioSample_GetMusicMode(void);
int AudioSample_GetCurrentTrack(void);
int AudioSample_GetRadioInCar(void);
bool AudioSample_CheckForMusicInterruptions(void);
int32_t AudioSample_GetRandomValue(unsigned index);
uint32_t AudioSample_GetFrameCounter(void);
#ifdef AUDIO_REFLECTIONS
const float *AudioSample_GetReflectionDistances(void);
#endif
#ifdef AUDIO_MSS
void AudioSample_SetCurrent3DProvider(uint8_t index);
bool AudioSample_WaitForUserCD(void);
void AudioSample_ApplyCDVolume(bool available);
#endif
void re3_trace(const char *filename, unsigned int lineno, const char *func, const char *format, ...);
void re3_usererror(const char *format, ...);
#ifndef _WIN32
#include <limits.h>
#include <time.h>
typedef struct AudioSampleFindData {
    char extension[32];
    char folder[PATH_MAX];
    char cFileName[256];
    time_t ftLastWriteTime;
} AudioSampleFindData;
void *AudioSample_FindFirstFile(const char *path, AudioSampleFindData *data);
bool AudioSample_FindNextFile(void *handle, AudioSampleFindData *data);
void AudioSample_FindClose(void *handle);
FILE *AudioSample_CaseOpen(const char *filename, const char *mode);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
