#pragma once

//+ rouz edit (ChatGPT)
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { AUDIO_BUFFER_PAIR_CAPACITY = 4 };

typedef struct AudioBufferPair {
	uint32_t left;
	uint32_t right;
} AudioBufferPair;

typedef struct AudioBufferQueue {
	AudioBufferPair entries[AUDIO_BUFFER_PAIR_CAPACITY];
	uint32_t head;
	uint32_t count;
	uint64_t revision;
} AudioBufferQueue;

static inline void
AudioBufferQueue_Init(AudioBufferQueue *queue)
{
	// Initialize an empty embedded queue without allocating or clearing buffer IDs
	queue->head = 0;
	queue->count = 0;
	queue->revision = 0;
}

static inline void
AudioBufferQueue_Clear(AudioBufferQueue *queue)
{
	// Discard pending pairs and invalidate work decoded against the old front
	queue->head = 0;
	queue->count = 0;
	queue->revision++;
}

static inline bool
AudioBufferQueue_IsEmpty(const AudioBufferQueue *queue)
{
	// Inspect queue occupancy without touching unused entries
	return queue->count == 0;
}

static inline bool
AudioBufferQueue_Push(AudioBufferQueue *queue, AudioBufferPair pair)
{
	// Reject an exhausted queue without overwriting any live pair
	if(queue->count == AUDIO_BUFFER_PAIR_CAPACITY)
		return false;
	// Append after the current tail while preserving the front revision
	queue->entries[(queue->head + queue->count) % AUDIO_BUFFER_PAIR_CAPACITY] = pair;
	queue->count++;
	return true;
}

static inline bool
AudioBufferQueue_Peek(const AudioBufferQueue *queue, AudioBufferPair *pair)
{
	// Copy the front pair only when one is available
	if(AudioBufferQueue_IsEmpty(queue))
		return false;
	*pair = queue->entries[queue->head];
	return true;
}

static inline bool
AudioBufferQueue_Pop(AudioBufferQueue *queue, AudioBufferPair *pair)
{
	// Leave the output unchanged if the queue has no pair to consume
	if(!AudioBufferQueue_Peek(queue, pair))
		return false;
	// Advance the front and invalidate tokens for the consumed pair
	queue->head = (queue->head + 1) % AUDIO_BUFFER_PAIR_CAPACITY;
	queue->count--;
	queue->revision++;
	return true;
}

static inline bool
AudioBufferQueue_IsCurrent(const AudioBufferQueue *queue, uint64_t revision)
{
	// Reject stale decoding after front removal, clearing or ring-slot reuse
	return !AudioBufferQueue_IsEmpty(queue) && queue->revision == revision;
}
//- rouz edit (ChatGPT)
