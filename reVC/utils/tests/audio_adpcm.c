#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/audio/oal/AudioADPCM.h"

int main(void)
{
    // Verify representative IMA nibble values, step transitions and saturation
    CImaADPCMDecoder ima;
    CImaADPCMDecoder_Init(&ima, 0, 0);
    assert(CImaADPCMDecoder_DecodeSample(&ima, 0) == 0 && ima.StepIndex == 0);
    assert(CImaADPCMDecoder_DecodeSample(&ima, 7) == 11 && ima.StepIndex == 8);
    assert(CImaADPCMDecoder_DecodeSample(&ima, 15) == -19 && ima.StepIndex == 16);
    CImaADPCMDecoder_Init(&ima, 32760, 88);
    assert(CImaADPCMDecoder_DecodeSample(&ima, 7) == 32767 && ima.StepIndex == 88);
    CImaADPCMDecoder_Init(&ima, -32760, 88);
    assert(CImaADPCMDecoder_DecodeSample(&ima, 15) == -32768 && ima.StepIndex == 88);
    CImaADPCMDecoder_Init(&ima, 0, 0);
    uint8_t packed = 0x70;
    int16_t output[28];
    CImaADPCMDecoder_Decode(&ima, &packed, output, 1);
    assert(output[0] == 0 && output[1] == 11);

    // Verify VAG low-nibble ordering and preserve the original rounding behavior
    CVagDecoder vag;
    CVagDecoder_ResetState(&vag);
    uint8_t line[16] = {0x0c, 0};
    memset(line + 2, 0x21, 14);
    CVagDecoder_Decode(&vag, line, output, sizeof(line));
    for(unsigned i = 0; i < 28; i += 2) {
        assert(output[i] == 2 && output[i + 1] == 3);
    }
    assert(vag.s_1 == 2 && vag.s_2 == 1);
    assert(CVagDecoder_quantize(32770.0) == 32767);
    assert(CVagDecoder_quantize(-32770.0) == -32768);

    // Reject incomplete lines and terminal flags without touching output or predictor history
    CVagDecoder_ResetState(&vag);
    memset(output, 0x5a, sizeof(output));
    CVagDecoder_Decode(&vag, line, output, 15);
    assert(output[0] == 0x5a5a && vag.s_1 == 0 && vag.s_2 == 0);
    line[1] = 7;
    CVagDecoder_Decode(&vag, line, output, sizeof(line));
    assert(output[0] == 0x5a5a && vag.s_1 == 0 && vag.s_2 == 0);
    puts("C IMA and VAG decoder tests passed");
    return 0;
}
