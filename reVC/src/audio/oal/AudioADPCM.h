#pragma once

//+ rouz edit (ChatGPT)
#include <stddef.h>
#include <stdint.h>

typedef struct CImaADPCMDecoder {
	int16_t Sample;
	int16_t StepIndex;
} CImaADPCMDecoder;

typedef struct CVagDecoder {
	double s_1;
	double s_2;
} CVagDecoder;

static const uint16_t AudioImaStepTable[89] = {
		7, 8, 9, 10, 11, 12, 13, 14,
		16, 17, 19, 21, 23, 25, 28, 31,
		34, 37, 41, 45, 50, 55, 60, 66,
		73, 80, 88, 97, 107, 118, 130, 143,
		157, 173, 190, 209, 230, 253, 279, 307,
		337, 371, 408, 449, 494, 544, 598, 658,
		724, 796, 876, 963, 1060, 1166, 1282, 1411,
		1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
		3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
		7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
		15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
		32767
	};

static const double AudioVagPredictors[5][2] = { { 0.0, 0.0 },
					{  60.0 / 64.0,  0.0 },
					{  115.0 / 64.0, -52.0 / 64.0 },
					{  98.0 / 64.0, -55.0 / 64.0 },
					{  122.0 / 64.0, -60.0 / 64.0 } };

static inline void
CImaADPCMDecoder_Init(CImaADPCMDecoder *state, int16_t _Sample, int16_t _StepIndex)
{
	// Initialize the IMA sample and step index explicitly
	state->Sample = _Sample;
	state->StepIndex = _StepIndex;
}

static inline int16_t
CImaADPCMDecoder_DecodeSample(CImaADPCMDecoder *state, uint8_t adpcm)
{
	// Decode one IMA nibble using the original step and saturation rules
	uint16_t step = AudioImaStepTable[state->StepIndex];

	if (adpcm & 4)
		state->StepIndex += ((adpcm & 3) + 1) * 2;
	else
		state->StepIndex--;

	state->StepIndex = (state->StepIndex < 0 ? 0 : state->StepIndex > 88 ? 88 : state->StepIndex);

	int delta = step >> 3;
	if (adpcm & 1) delta += step >> 2;
	if (adpcm & 2) delta += step >> 1;
	if (adpcm & 4) delta += step;
	if (adpcm & 8) delta = -delta;

	int newSample = state->Sample + delta;
	state->Sample = (newSample < -32768 ? -32768 : newSample > 32767 ? 32767 : newSample);
	return state->Sample;
}

static inline void
CImaADPCMDecoder_Decode(CImaADPCMDecoder *state, uint8_t *inbuf, int16_t *_outbuf, size_t size)
{
	// Decode both nibbles of each IMA input byte
	int16_t* outbuf = _outbuf;
	for (size_t i = 0; i < size; i++)
	{
		*(outbuf++) = CImaADPCMDecoder_DecodeSample(state, inbuf[i] & 0xF);
		*(outbuf++) = CImaADPCMDecoder_DecodeSample(state, inbuf[i] >> 4);
	}
}

static inline void
CVagDecoder_ResetState(CVagDecoder *state)
{
	// Reset the two VAG predictor samples
	state->s_1 = state->s_2 = 0.0;
}

static inline int16_t
CVagDecoder_quantize(double sample)
{
	// Preserve the VAG decoder rounding and saturation rules
	int a = (int)(sample + 0.5);
	return (int16_t)((a < -32768 ? -32768 : a > 32767 ? 32767 : a));
}

static inline void
CVagDecoder_Decode(CVagDecoder *state, void *_inbuf, int16_t *_outbuf, size_t size)
{
	// Decode VAG lines with the original predictor history and sample ordering
	uint8_t* inbuf = (uint8_t*)_inbuf;
	int16_t* outbuf = _outbuf;
	size &= ~(16 - 1);

	// Decode complete lines while retaining predictor history between lines
	while (size > 0) {
		double samples[28];

		int predict_nr, shift_factor, flags;
		predict_nr = *(inbuf++);
		shift_factor = predict_nr & 0xf;
		predict_nr >>= 4;
		flags = *(inbuf++);
		if (flags == 7) // TODO: ignore?
			break;
		// Expand low and high nibbles in the original sample order
		for (int i = 0; i < 28; i += 2) {
			int d = *(inbuf++);
			int16_t s = (int16_t)((d & 0xf) << 12);
			samples[i] = (double)(s >> shift_factor);
			s = (int16_t)((d & 0xf0) << 8);
			samples[i + 1] = (double)(s >> shift_factor);
		}

		// Apply prediction and preserve the original quantization sequence
		for (int i = 0; i < 28; i++) {
			samples[i] = samples[i] + state->s_1 * AudioVagPredictors[predict_nr][0] + state->s_2 * AudioVagPredictors[predict_nr][1];
			state->s_2 = state->s_1;
			state->s_1 = samples[i];
			*(outbuf++) = CVagDecoder_quantize(samples[i] + 0.5);
		}
		size -= 16;
	}
}

//- rouz edit (ChatGPT)
