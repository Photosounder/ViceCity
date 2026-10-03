#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void AudioMath_CalculateDistance(uint8_t *calculated, float *distance, float squaredDistance);
uint8_t AudioMath_ComputeVolume(uint8_t emittingVolume, float maxDistance, float distance);
uint8_t AudioMath_ComputeEmittingVolume(uint8_t emittingVolume, float maxDistance, float distance);
int32_t AudioMath_ComputePan(float dist, float x);
int32_t AudioMath_ComputeFrontRearMix(float dist, float y);
uint32_t AudioMath_ComputeDopplerFrequency(uint8_t cameraSwitched, uint8_t timeSpent, float speedOfSound, uint32_t oldFreq, float position1, float position2, float speedMultiplier);
int32_t AudioMath_RandomDisplacement(const int32_t randomTable[5], uint32_t seed);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
