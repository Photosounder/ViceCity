//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
uint8_t
AudioControls_GetNum3DProvidersAvailable(uint8_t initialised);
char *
AudioControls_Get3DProviderName(uint8_t initialised, uint8_t id);
int8_t
AudioControls_GetCurrent3DProviderIndex(uint8_t initialised);
int8_t
AudioControls_AutoDetect3DProviders(uint8_t initialised);
void
AudioControls_SetSpeakerConfig(int32_t conf);
uint8_t
AudioControls_IsMP3RadioChannelAvailable(uint8_t initialised);
void
AudioControls_ReleaseDigitalHandle(uint8_t initialised);
void
AudioControls_ReacquireDigitalHandle(uint8_t initialised);
uint8_t
AudioControls_CheckForAnAudioFileOnCD(void);
char
AudioControls_GetCDAudioDriveLetter(uint8_t initialised);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
