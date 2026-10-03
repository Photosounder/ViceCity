#pragma once

//+ rouz edit (ChatGPT)
#ifdef AUDIO_OAL
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "oal_utils.h"
#include <AL/al.h>
#include <AL/alext.h>
#include <AL/efx.h>
typedef struct CChannel {
	uint32_t id;
	float  Pitch, Gain;
	float  Mix;
	void  *Data;
	size_t DataSize;
	int32_t  Frequency;
	float  Position[3];
	float  Distances[2];
	int32_t  LoopCount;
	ALint  LoopPoints[2];
	ALint  LastProcessedOffset;
	bool   bIs2D;
} CChannel;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t CChannel_channelsThatNeedService;
void CChannel_InitializeState(CChannel *channel);
void CChannel_InitChannels(void);
void CChannel_DestroyChannels(void);
void CChannel_SetDefault(CChannel *channel);
void CChannel_Reset(CChannel *channel);
void CChannel_Init(CChannel *channel, uint32_t _id, bool Is2D);
void CChannel_Term(CChannel *channel);
void CChannel_Start(CChannel *channel);
void CChannel_Stop(CChannel *channel);
bool CChannel_HasSource(CChannel *channel);
bool CChannel_IsUsed(CChannel *channel);
void CChannel_SetPitch(CChannel *channel, float pitch);
void CChannel_SetGain(CChannel *channel, float gain);
void CChannel_SetVolume(CChannel *channel, int32_t vol);
void CChannel_SetSampleData(CChannel *channel, void *_data, size_t _DataSize, int32_t freq);
void CChannel_SetCurrentFreq(CChannel *channel, uint32_t freq);
void CChannel_SetLoopCount(CChannel *channel, int32_t count);
void CChannel_SetLoopPoints(CChannel *channel, ALint start, ALint end);
void CChannel_SetPosition(CChannel *channel, float x, float y, float z);
void CChannel_SetDistances(CChannel *channel, float max, float min);
void CChannel_SetPan(CChannel *channel, int32_t pan);
void CChannel_ClearBuffer(CChannel *channel);
void CChannel_SetReverbMix(CChannel *channel, ALuint slot, float mix);
void CChannel_UpdateReverb(CChannel *channel, ALuint slot);
bool CChannel_Update(CChannel *channel);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
