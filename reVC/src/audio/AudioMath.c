//+ rouz edit (ChatGPT)
#include "AudioMath.h"
#include <math.h>

#define AUDIO_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AUDIO_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AUDIO_SQUARE(x) ((x) * (x))
#define AUDIO_ABS_INT(a)  (((a) < 0) ? (-(a)) : (a))
#define AUDIO_CLAMP_CENTER(v, center, radius) ((v) > (center) ? AUDIO_MIN(v, center + radius) : AUDIO_MAX(v, center - radius))

static const uint8_t PanTable[64] = { 0,  3,  8, 12, 16, 19, 22, 24, 26, 28, 30, 31, 33, 34, 36, 37, 39, 40, 41, 42, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 53, 53,
								   54, 55, 55, 56, 56, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 61, 61, 61, 61, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63};

uint8_t AudioMath_ComputeVolume(uint8_t emittingVolume, float maxDistance, float distance)
{
	// Preserve the original formula, gates, and numeric conversions
	float minDistance;
	uint8_t newEmittingVolume;

	if (maxDistance <= 0.0f)
		return 0;

	minDistance = maxDistance / 5.0f;
	if (minDistance > distance)
		return emittingVolume;

	newEmittingVolume = emittingVolume * AUDIO_SQUARE((maxDistance - minDistance - (distance - minDistance))
		/ (maxDistance - minDistance));
	return AUDIO_MIN(127, newEmittingVolume);
}

uint8_t AudioMath_ComputeEmittingVolume(uint8_t emittingVolume, float maxDistance, float distance)
{
	// Preserve the original formula, gates, and numeric conversions
	float minDistance = maxDistance / 4.0f;
	float diffDistance = maxDistance - minDistance;
	if (distance > diffDistance)
		return (minDistance - (distance - diffDistance)) * (float)emittingVolume / minDistance;
	return emittingVolume;
}

int32_t AudioMath_ComputePan(float dist, float x)
{
	// Preserve the original formula, gates, and numeric conversions
	int32_t index = x / (dist / 64.0f);
	index = AUDIO_MIN(63, AUDIO_ABS_INT(index));

	if (x > 0.0f)
		return AUDIO_MAX(20, 63 - (int8_t)PanTable[index]);
	return AUDIO_MIN(107, PanTable[index] + 63);
}

int32_t AudioMath_ComputeFrontRearMix(float dist, float y)
{
	// Preserve the original formula, gates, and numeric conversions
	int32_t index = y / (dist / 64.0f);
	index = AUDIO_MIN(63, AUDIO_ABS_INT(index));

	if (y > 0.0f)
		return AUDIO_MAX(0, 63 - (int8_t)PanTable[index]);
	return AUDIO_MIN(127, PanTable[index] + 63);
}

uint32_t AudioMath_ComputeDopplerFrequency(uint8_t cameraSwitched, uint8_t timeSpent, float speedOfSound, uint32_t oldFreq, float position1, float position2, float speedMultiplier)
{
	// Preserve the original formula, gates, and numeric conversions
	uint32_t newFreq = oldFreq;
	if (!cameraSwitched && speedMultiplier != 0.0f) {
		float dist = position2 - position1;
		if (dist != 0.0f) {
			float speedOfSource = (dist / timeSpent) * speedMultiplier;
			if (speedOfSound > fabsf(speedOfSource)) {
				speedOfSource = AUDIO_CLAMP_CENTER(speedOfSource, 0.0f, 1.5f);
				newFreq = (oldFreq * speedOfSound) / (speedOfSource + speedOfSound);
			}
		}
	}
	return newFreq;
}

int32_t AudioMath_RandomDisplacement(const int32_t randomTable[5], uint32_t seed)
{
	// Preserve the original formula, gates, and numeric conversions
	int32_t value;

	static uint8_t bPos = 1;
	static uint32_t Adjustment = 0;

	if (seed == 0)
		return 0;

	value = randomTable[(Adjustment + seed) % 5] % seed;
	Adjustment += value;

	if (value % 2)
		bPos = !bPos;

	if (!bPos)
		value = -value;
	return value;
}

void AudioMath_CalculateDistance(uint8_t *calculated, float *distance, float squaredDistance)
{
	// Calculate the cached distance once using the original nonpositive clamp
	if (!*calculated) {
		*distance = squaredDistance <= 0.0f ? 0.0f : sqrtf(squaredDistance);
		*calculated = 1;
	}
}
//- rouz edit (ChatGPT)
