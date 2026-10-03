#pragma once
//+ rouz edit (ChatGPT)
#include "AudioSound.h"
#define MISSION_AUDIO_SLOTS (2)

#ifdef __cplusplus
extern "C" {
#endif
void AudioMissionPosition_Set(AudioSoundPosition *positions, uint8_t *is2D, uint8_t initialized, uint8_t slot, float x, float y, float z);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
