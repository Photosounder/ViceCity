#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "AudioSamples.h"

#define POLICE_RADIO_QUEUE_MAX_SAMPLES 60

typedef struct cPoliceRadioQueue {
	uint32_t m_aSamples[POLICE_RADIO_QUEUE_MAX_SAMPLES];
	uint8_t m_nSamplesInQueue;
	uint8_t m_nAddOffset;
	uint8_t m_nRemoveOffset;
} cPoliceRadioQueue;

static inline void
PoliceRadioQueue_Reset(cPoliceRadioQueue *queue)
{
	// Reset queue cursors and count while retaining the sample storage
	queue->m_nAddOffset = 0;
	queue->m_nRemoveOffset = 0;
	queue->m_nSamplesInQueue = 0;
}

static inline uint8_t
PoliceRadioQueue_Add(cPoliceRadioQueue *queue, uint32_t sample)
{
	// Append a sample only when the original fixed-capacity queue has space
	if (queue->m_nSamplesInQueue != POLICE_RADIO_QUEUE_MAX_SAMPLES) {
		// Advance the write cursor with the original wraparound arithmetic
		queue->m_aSamples[queue->m_nAddOffset] = sample;
		queue->m_nSamplesInQueue++;
		queue->m_nAddOffset = (queue->m_nAddOffset + 1) % POLICE_RADIO_QUEUE_MAX_SAMPLES;
		return 1;
	}
	return 0;
}

static inline uint32_t
PoliceRadioQueue_Remove(cPoliceRadioQueue *queue)
{
	// Return the oldest sample or the existing empty-queue sentinel
	if (queue->m_nSamplesInQueue != 0) {
		// Advance the read cursor with the original wraparound arithmetic
		uint32_t sample = queue->m_aSamples[queue->m_nRemoveOffset];
		queue->m_nSamplesInQueue--;
		queue->m_nRemoveOffset = (queue->m_nRemoveOffset + 1) % POLICE_RADIO_QUEUE_MAX_SAMPLES;
		return sample;
	}
	return NO_SAMPLE;
}
//- rouz edit (ChatGPT)
