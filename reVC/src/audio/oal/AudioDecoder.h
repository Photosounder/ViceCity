#pragma once

//+ rouz edit (ChatGPT)
#include <stdbool.h>
#include <stdint.h>

typedef struct AudioDecoderOps {
    void (*close)(void *state);
    bool (*isOpened)(void *state);
    void (*fileOpen)(void *state);
    uint32_t (*getSampleSize)(void *state);
    uint32_t (*getSampleCount)(void *state);
    uint32_t (*getSampleRate)(void *state);
    uint32_t (*getChannels)(void *state);
    void (*seek)(void *state, uint32_t milliseconds);
    uint32_t (*tell)(void *state);
    uint32_t (*decode)(void *state, void *buffer);
} AudioDecoderOps;

typedef struct AudioDecoder {
    const AudioDecoderOps *ops;
    void *state;
} AudioDecoder;

static inline void AudioDecoder_Init(AudioDecoder *decoder, const AudioDecoderOps *ops, void *state)
{
    // Bind externally owned state without allocating storage
    decoder->ops = ops;
    decoder->state = state;
}

static inline void AudioDecoder_Close(AudioDecoder *decoder)
{
    // Release format resources while leaving state storage to the owning caller
    decoder->ops->close(decoder->state);
}

static inline bool AudioDecoder_IsOpened(const AudioDecoder *decoder)
{
    // Query the concrete decoder through its C function table
    return decoder->ops->isOpened(decoder->state);
}

static inline void AudioDecoder_FileOpen(const AudioDecoder *decoder)
{
    // Complete deferred format initialization through its concrete opening path
    decoder->ops->fileOpen(decoder->state);
}

static inline uint32_t AudioDecoder_GetSampleSize(const AudioDecoder *decoder)
{
    // Obtain the concrete sample width
    return decoder->ops->getSampleSize(decoder->state);
}

static inline uint32_t AudioDecoder_GetSampleCount(const AudioDecoder *decoder)
{
    // Obtain the concrete sample count
    return decoder->ops->getSampleCount(decoder->state);
}

static inline uint32_t AudioDecoder_GetSampleRate(const AudioDecoder *decoder)
{
    // Obtain the concrete sample rate
    return decoder->ops->getSampleRate(decoder->state);
}

static inline uint32_t AudioDecoder_GetChannels(const AudioDecoder *decoder)
{
    // Obtain the concrete channel count
    return decoder->ops->getChannels(decoder->state);
}

static inline void AudioDecoder_Seek(const AudioDecoder *decoder, uint32_t milliseconds)
{
    // Position the concrete decoder using the existing millisecond convention
    decoder->ops->seek(decoder->state, milliseconds);
}

static inline uint32_t AudioDecoder_Tell(const AudioDecoder *decoder)
{
    // Read the concrete decoder position
    return decoder->ops->tell(decoder->state);
}

static inline uint32_t AudioDecoder_Decode(const AudioDecoder *decoder, void *buffer)
{
    // Decode into storage supplied by the owning stream
    return decoder->ops->decode(decoder->state, buffer);
}

static inline uint32_t AudioDecoder_GetAvgSamplesPerSec(const AudioDecoder *decoder)
{
    // Preserve the existing channel-weighted sample rate
    return AudioDecoder_GetChannels(decoder) * AudioDecoder_GetSampleRate(decoder);
}

static inline uint32_t AudioDecoder_MillisecondsToSamples(const AudioDecoder *decoder, uint32_t milliseconds)
{
    // Preserve the original floating-point conversion and truncation
    return (uint32_t)((float)milliseconds / 1000.0f * (float)AudioDecoder_GetSampleRate(decoder));
}

static inline uint32_t AudioDecoder_SamplesToMilliseconds(const AudioDecoder *decoder, uint32_t samples)
{
    // Preserve the original floating-point conversion and truncation
    return (uint32_t)((float)samples * 1000.0f / (float)AudioDecoder_GetSampleRate(decoder));
}

static inline uint32_t AudioDecoder_GetBufferSamples(const AudioDecoder *decoder)
{
    // Keep the quarter-second PCM buffer sizing rule
    return AudioDecoder_GetAvgSamplesPerSec(decoder) / 4;
}

static inline uint32_t AudioDecoder_GetBufferSize(const AudioDecoder *decoder)
{
    // Convert the PCM buffer sample count to bytes
    return AudioDecoder_GetBufferSamples(decoder) * AudioDecoder_GetSampleSize(decoder);
}

static inline uint32_t AudioDecoder_GetLength(const AudioDecoder *decoder)
{
    // Open deferred files before evaluating the original duration conversion
    AudioDecoder_FileOpen(decoder);
    return (uint32_t)((float)AudioDecoder_GetSampleCount(decoder) * 1000.0f / (float)AudioDecoder_GetSampleRate(decoder));
}
//- rouz edit (ChatGPT)
