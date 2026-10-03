//+ rouz edit (ChatGPT)
#include "AudioProvider.h"
#include "sampman.h"

int8_t
AudioProvider_SetCurrent(uint8_t initialised, uint8_t which, uint8_t *activeSamples, uint8_t *activeQueue, uint8_t order[NUM_SOUND_QUEUES][NUM_CHANNELS_GENERIC], uint8_t counts[NUM_SOUND_QUEUES], tSound *active)
{
	// Preserve channel stopping, queue reset order, and provider-result handling
#ifndef EXTERNAL_3D_SOUND
	return -1;
#else
	if (!initialised)
		return -1;
	for (uint8_t i = 0; i < (*activeSamples) + 1; i++)
		SampleManager_StopChannel(&SampleManager, i);
	AudioSoundQueue_ClearRequested(order[(*activeQueue)], &counts[(*activeQueue)], (*activeSamples));
	if ((*activeQueue) == 0)
		(*activeQueue) = 1;
	else
		(*activeQueue) = 0;
	AudioSoundQueue_ClearRequested(order[(*activeQueue)], &counts[(*activeQueue)], (*activeSamples));
	AudioSoundQueue_ClearActive(active, (*activeSamples));
	int8_t current = SampleManager_SetCurrent3DProvider(&SampleManager, which);
	if (current > 0) {
		(*activeSamples) = SampleManager_GetMaximumSupportedChannels(&SampleManager);
		if ((*activeSamples) > 1)
			(*activeSamples)--;
	}
	return current;
#endif
}
//- rouz edit (ChatGPT)
