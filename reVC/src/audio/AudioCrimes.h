#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "../core/CrimeTypes.h"

#define AUDIO_CRIME_COUNT 10

typedef struct AudioCrimePosition {
	float x, y, z;
} AudioCrimePosition;

typedef struct cAMCrime {
	int32_t type;
	AudioCrimePosition position;
	uint16_t timer;
} cAMCrime;

#ifdef __cplusplus
static_assert(sizeof(cAMCrime) == 20, "Audio crime record layout");
extern "C" {
#else
_Static_assert(sizeof(cAMCrime) == 20, "Audio crime record layout");
#endif

void AudioCrimes_Init(cAMCrime crimes[AUDIO_CRIME_COUNT]);
void AudioCrimes_Report(cAMCrime crimes[AUDIO_CRIME_COUNT], uint32_t *nextReport, enum eCrimeType type, uint32_t frame, float x, float y, float z);
void AudioCrimes_Age(cAMCrime crimes[AUDIO_CRIME_COUNT]);

#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
