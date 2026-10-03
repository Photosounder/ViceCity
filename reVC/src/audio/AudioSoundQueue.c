//+ rouz edit (ChatGPT)
#include "AudioSound.h"
#include <string.h>

void
AudioSoundQueue_InsertOrder(tSound *requested, uint8_t *order, uint8_t activeSamples, uint8_t sample)
{
	// Preserve the original physical-slot bound and stable priority insertion
	uint32_t i = 0;
	if (sample > 0) {
		for (; i < sample; i++) {
			if (requested[order[i]].m_nFinalPriority >
			    requested[sample].m_nFinalPriority)
				break;
		}
		if (i < sample) 
			memmove(&order[i + 1], &order[i], activeSamples - i - 1);
	}
	order[i] = sample;
}
//- rouz edit (ChatGPT)
