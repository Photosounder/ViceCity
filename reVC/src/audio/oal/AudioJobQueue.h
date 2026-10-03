#pragma once

//+ rouz edit (ChatGPT)
#include <stdbool.h>
#include <stddef.h>
#include "AudioDecoder.h"

typedef struct AudioProcessJob {
    struct AudioProcessJob *next;
    void *stream;
} AudioProcessJob;

typedef struct AudioProcessQueue {
    AudioProcessJob *head;
    AudioProcessJob *tail;
} AudioProcessQueue;

typedef struct AudioCloseJob {
    struct AudioCloseJob *next;
    AudioDecoder *decoder;
    void *buffer;
} AudioCloseJob;

typedef struct AudioCloseQueue {
    AudioCloseJob *head;
    AudioCloseJob *tail;
} AudioCloseQueue;

static inline bool AudioProcessQueue_IsEmpty(const AudioProcessQueue *queue)
{
    // Inspect pending work under the caller's scheduler lock
    return queue->head == NULL;
}

static inline void AudioProcessQueue_Push(AudioProcessQueue *queue, AudioProcessJob *job)
{
    // Append caller-owned storage without dropping duplicate stream requests
    job->next = NULL;
    if(queue->tail) queue->tail->next = job;
    else queue->head = job;
    queue->tail = job;
}

static inline AudioProcessJob *AudioProcessQueue_Pop(AudioProcessQueue *queue)
{
    // Return ownership of the oldest job to the caller
    AudioProcessJob *job = queue->head;
    if(job) {
        queue->head = job->next;
        if(!queue->head) queue->tail = NULL;
        job->next = NULL;
    }
    return job;
}

static inline bool AudioCloseQueue_IsEmpty(const AudioCloseQueue *queue)
{
    // Inspect pending cleanup under the caller's scheduler lock
    return queue->head == NULL;
}

static inline void AudioCloseQueue_Push(AudioCloseQueue *queue, AudioCloseJob *job)
{
    // Append a captured decoder and PCM buffer without allocating queue storage
    job->next = NULL;
    if(queue->tail) queue->tail->next = job;
    else queue->head = job;
    queue->tail = job;
}

static inline AudioCloseJob *AudioCloseQueue_Pop(AudioCloseQueue *queue)
{
    // Return ownership of the oldest captured cleanup request to the caller
    AudioCloseJob *job = queue->head;
    if(job) {
        queue->head = job->next;
        if(!queue->head) queue->tail = NULL;
        job->next = NULL;
    }
    return job;
}
//- rouz edit (ChatGPT)
