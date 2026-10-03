//+ rouz edit (ChatGPT)
#include "AudioCrimes.h"

void
AudioCrimes_Init(cAMCrime crimes[AUDIO_CRIME_COUNT])
{
	// Initialize every record explicitly instead of invoking its C++ constructor
	for (int32_t i = 0; i < AUDIO_CRIME_COUNT; i++) {
		// Preserve the original type, position, and timer defaults
		crimes[i].type = CRIME_NONE;
		crimes[i].position.x = 0.0f;
		crimes[i].position.y = 0.0f;
		crimes[i].position.z = 0.0f;
		crimes[i].timer = 0;
	}
}

void
AudioCrimes_Report(cAMCrime crimes[AUDIO_CRIME_COUNT], uint32_t *nextReport, enum eCrimeType type, uint32_t frame, float x, float y, float z)
{
	// Refresh the first matching active crime or remember the last empty slot
	int32_t lastCrime = AUDIO_CRIME_COUNT;
	for (int32_t i = 0; i < AUDIO_CRIME_COUNT; i++) {
		// Preserve duplicate updates without extending their existing report cooldown
		if (crimes[i].type != CRIME_NONE) {
			// Update only the matching record and leave the other records untouched
			if (crimes[i].type == type) {
				// Store the same position and reset the duplicate record's age
				crimes[i].position.x = x;
				crimes[i].position.y = y;
				crimes[i].position.z = z;
				crimes[i].timer = 0;
				return;
			}
		} else
			lastCrime = i;
	}
	if (lastCrime < AUDIO_CRIME_COUNT) {
		// Fill the original last-empty slot and apply the existing unsigned cooldown
		crimes[lastCrime].type = type;
		crimes[lastCrime].position.x = x;
		crimes[lastCrime].position.y = y;
		crimes[lastCrime].position.z = z;
		crimes[lastCrime].timer = 0;
		nextReport[type] = frame + 500;
	}
}

void
AudioCrimes_Age(cAMCrime crimes[AUDIO_CRIME_COUNT])
{
	// Preserve the original timer increment, expiration threshold, and uint16 wraparound
	for (uint8_t i = 0; i < AUDIO_CRIME_COUNT; i++) {
		// Age active records while leaving inactive positions and timers untouched
		if (crimes[i].type != CRIME_NONE) {
			// Expire the record only after its incremented timer exceeds 1200
			if (++crimes[i].timer > 1200) crimes[i].type = CRIME_NONE;
		}
	}
}
//- rouz edit (ChatGPT)
