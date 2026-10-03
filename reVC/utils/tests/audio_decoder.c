#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/audio/oal/AudioDecoder.h"

typedef struct State {
    uint32_t rate, channels, samples, position;
    unsigned opened, closed, openCalls, decodeCalls;
} State;

static void close_state(void *opaque)
{
    // Record resource cleanup without freeing the caller-owned state
    ((State*)opaque)->closed++;
}
static bool is_opened(void *opaque)
{
    // Query the selected state rather than a global decoder
    return ((State*)opaque)->opened != 0;
}
static void open_state(void *opaque)
{
    // Complete a deferred opening operation
    State *state = (State*)opaque;
    state->opened = 1;
    state->openCalls++;
}
static uint32_t sample_size(void *opaque)
{
    // Describe signed sixteen-bit PCM
    (void)opaque;
    return 2;
}
static uint32_t sample_count(void *opaque)
{
    // Return the selected state's sample count
    return ((State*)opaque)->samples;
}
static uint32_t sample_rate(void *opaque)
{
    // Return the selected state's sample rate
    return ((State*)opaque)->rate;
}
static uint32_t channels(void *opaque)
{
    // Return the selected state's channel count
    return ((State*)opaque)->channels;
}
static void seek_state(void *opaque, uint32_t position)
{
    // Store the exact seek argument in the selected state
    ((State*)opaque)->position = position;
}
static uint32_t tell_state(void *opaque)
{
    // Read back the selected state's seek position
    return ((State*)opaque)->position;
}
static uint32_t decode_state(void *opaque, void *buffer)
{
    // Write a sample through the selected state and supplied buffer
    State *state = (State*)opaque;
    uint16_t sample = (uint16_t)state->position;
    memcpy(buffer, &sample, sizeof(sample));
    state->decodeCalls++;
    return sizeof(sample);
}
static const AudioDecoderOps ops = {
    close_state, is_opened, open_state, sample_size, sample_count,
    sample_rate, channels, seek_state, tell_state, decode_state
};

int main(void)
{
    // Bind two independent caller-owned decoders and verify every dispatch slot
    State first = {32000, 2, 96000, 0, 0, 0, 0, 0};
    State second = {48000, 1, 240000, 0, 1, 0, 0, 0};
    AudioDecoder a, b;
    AudioDecoder_Init(&a, &ops, &first);
    AudioDecoder_Init(&b, &ops, &second);
    assert(a.state == &first && b.state == &second);
    assert(!AudioDecoder_IsOpened(&a) && AudioDecoder_IsOpened(&b));
    assert(AudioDecoder_GetLength(&a) == 3000 && first.openCalls == 1);
    assert(AudioDecoder_GetLength(&b) == 5000 && second.openCalls == 1);
    assert(AudioDecoder_GetAvgSamplesPerSec(&a) == 64000);
    assert(AudioDecoder_GetBufferSamples(&a) == 16000);
    assert(AudioDecoder_GetBufferSize(&a) == 32000);
    assert(AudioDecoder_GetChannels(&b) == 1);
    assert(AudioDecoder_GetSampleCount(&b) == 240000);
    assert(AudioDecoder_GetSampleSize(&b) == 2);
    AudioDecoder_Seek(&a, 12345);
    assert(AudioDecoder_Tell(&a) == 12345 && AudioDecoder_Tell(&b) == 0);
    uint16_t output = 0;
    assert(AudioDecoder_Decode(&a, &output) == 2);
    assert(output == 12345 && first.decodeCalls == 1 && second.decodeCalls == 0);

    // Compare C helpers with the original floating-point expressions over varied valid inputs
    for(uint32_t i = 1; i < 1000000; i++) {
        first.rate = 8000 + i % 184001;
        first.channels = 1 + i % 8;
        first.samples = i;
        assert(AudioDecoder_MillisecondsToSamples(&a, i) == (uint32_t)((float)i / 1000.0f * (float)first.rate));
        assert(AudioDecoder_SamplesToMilliseconds(&a, i) == (uint32_t)((float)i * 1000.0f / (float)first.rate));
        assert(AudioDecoder_GetBufferSamples(&a) == first.channels * first.rate / 4);
        assert(AudioDecoder_GetLength(&a) == (uint32_t)((float)i * 1000.0f / (float)first.rate));
    }

    // Verify cleanup reaches only the selected state and leaves storage owned by the caller
    AudioDecoder_Close(&a);
    assert(first.closed == 1 && second.closed == 0);
    assert(a.state == &first);
    AudioDecoder_Close(&b);
    assert(second.closed == 1);
    puts("C decoder dispatch and conversion tests passed");
    return 0;
}
