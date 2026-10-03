//+ rouz edit (ChatGPT)
#pragma once
#include "AudioSound.h"
#include "audio_enums.h"
#ifdef __cplusplus
extern "C" {
#endif
int8_t
AudioProvider_SetCurrent(uint8_t initialised, uint8_t which, uint8_t *activeSamples, uint8_t *activeQueue, uint8_t order[NUM_SOUND_QUEUES][NUM_CHANNELS_GENERIC], uint8_t counts[NUM_SOUND_QUEUES], tSound *active);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
