#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/audio/oal/AudioFormats.h"

static void write16(FILE *file, uint16_t value)
{
    // Encode a sixteen-bit fixture field in little-endian order
    unsigned char bytes[2] = {(unsigned char)value, (unsigned char)(value >> 8)};
    assert(fwrite(bytes, 1, 2, file) == 2);
}
static void write32(FILE *file, uint32_t value)
{
    // Encode a thirty-two-bit fixture field in little-endian order
    write16(file, (uint16_t)value);
    write16(file, (uint16_t)(value >> 16));
}
static void write_wave(const char *path)
{
    // Generate stereo PCM with clearly distinct channel samples
    FILE *file = fopen(path, "wb");
    assert(file);
    fwrite("RIFF", 1, 4, file); write32(file, 36 + 128);
    fwrite("WAVEfmt ", 1, 8, file); write32(file, 16);
    write16(file, 1); write16(file, 2);
    write32(file, 32000); write32(file, 128000);
    write16(file, 4); write16(file, 16);
    fwrite("data", 1, 4, file); write32(file, 128);
    for(unsigned i = 0; i < 32; i++) {
        write16(file, (uint16_t)i);
        write16(file, (uint16_t)(i + 100));
    }
    fclose(file);
}
int main(int argc, char **argv)
{
    // Decode stereo WAV directly through the production C format implementation
    assert(argc == 2);
    char path[1024];
    snprintf(path, sizeof(path), "%s/pure-c.wav", argv[1]);
    write_wave(path);
    CWavFile wav;
    memset(&wav, 0xa5, sizeof(wav));
    CWavFile_Init(&wav, path);
    AudioDecoder_Init(&wav.decoder, &CWavFile_Ops, &wav);
    assert(AudioDecoder_IsOpened(&wav.decoder));
    assert(AudioDecoder_GetSampleRate(&wav.decoder) == 32000);
    assert(AudioDecoder_GetChannels(&wav.decoder) == 2);
    assert(AudioDecoder_GetSampleCount(&wav.decoder) == 32);
    assert(AudioDecoder_GetBufferSize(&wav.decoder) == 32000);
    assert(AudioDecoder_GetLength(&wav.decoder) == 1);
    int16_t *pcm = (int16_t*)malloc(AudioDecoder_GetBufferSize(&wav.decoder));
    assert(pcm);
    assert(AudioDecoder_Decode(&wav.decoder, pcm) == 128);
    for(unsigned i = 0; i < 32; i++) {
        assert(pcm[i] == (int16_t)i);
        assert(pcm[i + 32] == (int16_t)(i + 100));
    }
    assert(AudioDecoder_Decode(&wav.decoder, pcm) == 0);
    AudioDecoder_Seek(&wav.decoder, 0);
    assert(AudioDecoder_Tell(&wav.decoder) == 0);
    assert(AudioDecoder_Decode(&wav.decoder, pcm) == 128);
    AudioDecoder_Close(&wav.decoder);
    free(pcm);

    // Decode one silent VB block pair through the production C format implementation
    snprintf(path, sizeof(path), "%s/pure-c.vb", argv[1]);
    FILE *file = fopen(path, "wb");
    assert(file);
    for(unsigned i = 0; i < 0x4000; i++) fputc(0, file);
    fclose(file);
    CVbFile vb;
    memset(&vb, 0xa5, sizeof(vb));
    CVbFile_Init(&vb, path, 32000, 2);
    AudioDecoder_Init(&vb.decoder, &CVbFile_Ops, &vb);
    assert(AudioDecoder_IsOpened(&vb.decoder));
    assert(AudioDecoder_GetSampleCount(&vb.decoder) == 14336);
    assert(AudioDecoder_GetLength(&vb.decoder) == 448);
    pcm = (int16_t*)malloc(AudioDecoder_GetBufferSize(&vb.decoder));
    assert(pcm);
    uint32_t size = AudioDecoder_Decode(&vb.decoder, pcm);
    assert(size == 31920);
    for(unsigned i = 0; i < size / sizeof(*pcm); i++) assert(pcm[i] == 1);
    AudioDecoder_Close(&vb.decoder);
    free(pcm);

    // Verify the shared sorter can grow, shut down and restart without a C++ destructor
    uint16_t stereo[4096];
    for(unsigned frames = 1; frames <= 2048; frames *= 2) {
        for(unsigned i = 0; i < frames; i++) {
            stereo[i * 2] = (uint16_t)i;
            stereo[i * 2 + 1] = (uint16_t)(i + 10000);
        }
        CSortStereoBuffer_SortStereo(&SortStereoBuffer, stereo, frames * 4);
        for(unsigned i = 0; i < frames; i++) {
            assert(stereo[i] == i && stereo[i + frames] == i + 10000);
        }
    }
    CSortStereoBuffer_Close(&SortStereoBuffer);
    assert(SortStereoBuffer.PcmBuf == NULL && SortStereoBuffer.BufSize == 0);
    CSortStereoBuffer_Close(&SortStereoBuffer);
    stereo[0] = 1; stereo[1] = 2; stereo[2] = 3; stereo[3] = 4;
    CSortStereoBuffer_SortStereo(&SortStereoBuffer, stereo, 8);
    assert(stereo[0] == 1 && stereo[1] == 3 && stereo[2] == 2 && stereo[3] == 4);
    CSortStereoBuffer_Close(&SortStereoBuffer);

    // Verify failed opening initializes every cleanup-relevant field from dirty storage
    snprintf(path, sizeof(path), "%s/missing.wav", argv[1]);
    memset(&wav, 0xa5, sizeof(wav));
    CWavFile_Init(&wav, path);
    AudioDecoder_Init(&wav.decoder, &CWavFile_Ops, &wav);
    assert(!AudioDecoder_IsOpened(&wav.decoder));
    AudioDecoder_Close(&wav.decoder);
    memset(&vb, 0xa5, sizeof(vb));
    CVbFile_Init(&vb, path, 32000, 2);
    AudioDecoder_Init(&vb.decoder, &CVbFile_Ops, &vb);
    assert(!AudioDecoder_IsOpened(&vb.decoder));
    AudioDecoder_Close(&vb.decoder);
    puts("Production WAV/VB formats and stereo storage passed in C");
    return 0;
}
