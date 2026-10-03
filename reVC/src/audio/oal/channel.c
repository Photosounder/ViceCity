//+ rouz edit (ChatGPT)
#include "../../core/config.h"
#include "audio_enums.h"
#include "AudioSampleConfig.h"
#include "AudioStreamHost.h"
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <math.h>
#define nil NULL
#define Sqrt sqrtf
#define SQR(value) ((value) * (value))
#ifndef MASTER
#define CHANNEL_ASSERT(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define CHANNEL_ASSERT(expression) ((void)(expression))
#endif

#ifdef AUDIO_OAL
#include "channel.h"



ALuint alSources[NUM_CHANNELS];
ALuint alFilters[NUM_CHANNELS];
ALuint alBuffers[NUM_CHANNELS];
bool bChannelsCreated = false;

int32_t CChannel_channelsThatNeedService = 0;

uint8_t tempStereoBuffer[PED_BLOCKSIZE * 2];

void
CChannel_InitChannels(void)
{
    // Manage shared OpenAL channel resources through the C API

	alGenSources(NUM_CHANNELS, alSources);
	alGenBuffers(NUM_CHANNELS, alBuffers);
	if (IsFXSupported())
		AudioEFX_alGenFilters(NUM_CHANNELS, alFilters);
	bChannelsCreated = true;
}

void
CChannel_DestroyChannels(void)
{
    // Manage shared OpenAL channel resources through the C API

	if (bChannelsCreated) 
	{
		alDeleteSources(NUM_CHANNELS, alSources);
		memset(alSources, 0, sizeof(alSources));
		alDeleteBuffers(NUM_CHANNELS, alBuffers);
		memset(alBuffers, 0, sizeof(alBuffers));
		if (IsFXSupported())
		{
			AudioEFX_alDeleteFilters(NUM_CHANNELS, alFilters);
			memset(alFilters, 0, sizeof(alFilters));
		}
		bChannelsCreated = false;
	}
}


void CChannel_InitializeState(CChannel *channel)
{
    // Initialize the former static channel defaults before any source access
    channel->id = 0;

	channel->Data = nil;
	channel->DataSize = 0;
	channel->bIs2D = false;
	CChannel_SetDefault(channel);
}

void CChannel_SetDefault(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	channel->Pitch = 1.0f;
	channel->Gain = 1.0f;
	channel->Mix = 0.0f;
		
	channel->Position[0] = 0.0f; channel->Position[1] = 0.0f; channel->Position[2] = 0.0f;
	channel->Distances[0] = 0.0f; channel->Distances[1] = FLT_MAX;

	channel->LoopCount = 1;
	channel->LastProcessedOffset = UINT32_MAX;
	channel->LoopPoints[0] = 0; channel->LoopPoints[1] = -1;
	
	channel->Frequency = MAX_FREQ;
}

void CChannel_Reset(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	// Here is safe because ctor don't call this
	if (channel->LoopCount > 1)
		CChannel_channelsThatNeedService--;

	CChannel_ClearBuffer(channel);
	CChannel_SetDefault(channel);
}

void CChannel_Init(CChannel *channel, uint32_t _id, bool Is2D)
{
    // Apply the channel operation to explicitly owned C state

	channel->id = _id;
	if ( CChannel_HasSource(channel) )
	{
		alSourcei(alSources[channel->id], AL_SOURCE_RELATIVE, AL_TRUE);
		if ( IsFXSupported() )
			alSource3i(alSources[channel->id], AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
		
		if ( Is2D )
		{
			channel->bIs2D = true;
			alSource3f(alSources[channel->id], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef(alSources[channel->id], AL_GAIN, 1.0f);
		}
	}
}

void CChannel_Term(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	CChannel_Stop(channel);
	if ( CChannel_HasSource(channel) )
	{
		if ( IsFXSupported() )
		{
			alSource3i(alSources[channel->id], AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
		}
	}
}

void CChannel_Start(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	if ( !CChannel_HasSource(channel) ) return;
	if ( !channel->Data ) return;

	if ( channel->bIs2D )
	{
		// convert mono data to stereo
		int16_t *monoData = (int16_t*)channel->Data;
		int16_t *stereoData = (int16_t*)tempStereoBuffer;
		for (size_t i = 0; i < channel->DataSize / 2; i++)
		{
			*(stereoData++) = *monoData;
			*(stereoData++) = *(monoData++);
		}
		alBufferData(alBuffers[channel->id], AL_FORMAT_STEREO16, tempStereoBuffer, channel->DataSize * 2, channel->Frequency);
	}
	else
		alBufferData(alBuffers[channel->id], AL_FORMAT_MONO16, channel->Data, channel->DataSize, channel->Frequency);
	if ( channel->LoopPoints[0] != 0 && channel->LoopPoints[0] != -1 )
		alBufferiv(alBuffers[channel->id], AL_LOOP_POINTS_SOFT, channel->LoopPoints);
	alSourcei(alSources[channel->id], AL_BUFFER, alBuffers[channel->id]);
	alSourcePlay(alSources[channel->id]);
}

void CChannel_Stop(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	if ( CChannel_HasSource(channel) )
		alSourceStop(alSources[channel->id]);
	
	CChannel_Reset(channel);
}

bool CChannel_HasSource(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	return alSources[channel->id] != AL_NONE;
}
	
bool CChannel_IsUsed(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	if ( CChannel_HasSource(channel) )
	{
		ALint sourceState;
		alGetSourcei(alSources[channel->id], AL_SOURCE_STATE, &sourceState);
		return sourceState == AL_PLAYING;
	}
	return false;
}

void CChannel_SetPitch(CChannel *channel, float pitch)
{
    // Apply the channel operation to explicitly owned C state

	if ( !CChannel_HasSource(channel) ) return;
	alSourcef(alSources[channel->id], AL_PITCH, pitch);
}

void CChannel_SetGain(CChannel *channel, float gain)
{
    // Apply the channel operation to explicitly owned C state

	if ( !CChannel_HasSource(channel) ) return;
	alSourcef(alSources[channel->id], AL_GAIN, gain);
}
	
void CChannel_SetVolume(CChannel *channel, int32_t vol)
{
    // Apply the channel operation to explicitly owned C state

	CChannel_SetGain(channel, (ALfloat)vol / MAX_VOLUME);
}

void CChannel_SetSampleData(CChannel *channel, void *_data, size_t _DataSize, int32_t freq)
{
    // Apply the channel operation to explicitly owned C state

	channel->Data = _data;
	channel->DataSize = _DataSize;
	channel->Frequency = freq;
}
	
void CChannel_SetCurrentFreq(CChannel *channel, uint32_t freq)
{
    // Apply the channel operation to explicitly owned C state

	CChannel_SetPitch(channel, (ALfloat)freq / channel->Frequency);
}

void CChannel_SetLoopCount(CChannel *channel, int32_t count)
{
    // Apply the channel operation to explicitly owned C state

	if ( !CChannel_HasSource(channel) ) return;

	// 0: loop indefinitely, 1: play one time, 2: play two times etc...
	// only > 1 needs manual processing

	if (channel->LoopCount > 1 && count < 2)
		CChannel_channelsThatNeedService--;
	else if (channel->LoopCount < 2 && count > 1)
		CChannel_channelsThatNeedService++;

	alSourcei(alSources[channel->id], AL_LOOPING, count == 1 ? AL_FALSE : AL_TRUE);
	channel->LoopCount = count;
}

bool CChannel_Update(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	if (!CChannel_HasSource(channel)) return false;
	if (channel->LoopCount < 2) return false;

	ALint state;
	alGetSourcei(alSources[channel->id], AL_SOURCE_STATE, &state);
	if (state == AL_STOPPED) {
		re3_debug("[DBG]: " "Looping channels(%d in this case) shouldn't report AL_STOPPED, but nvm\n", channel->id);
		CChannel_SetLoopCount(channel, 1);
		return true;
	}

	CHANNEL_ASSERT(CChannel_channelsThatNeedService > 0 && "Ref counting is broken");

	ALint offset;
	alGetSourcei(alSources[channel->id], AL_SAMPLE_OFFSET, &offset);

	// Rewound
	if (offset < channel->LastProcessedOffset) {
		channel->LoopCount--;
		if (channel->LoopCount == 1) {
			// Playing last tune...
			CChannel_channelsThatNeedService--;
			alSourcei(alSources[channel->id], AL_LOOPING, AL_FALSE);
		}
	}
	channel->LastProcessedOffset = offset;
	return true;
}

void CChannel_SetLoopPoints(CChannel *channel, ALint start, ALint end)
{
    // Apply the channel operation to explicitly owned C state

	channel->LoopPoints[0] = start;
	channel->LoopPoints[1] = end;
}
	
void CChannel_SetPosition(CChannel *channel, float x, float y, float z)
{
    // Apply the channel operation to explicitly owned C state

	if ( !CChannel_HasSource(channel) ) return;
	alSource3f(alSources[channel->id], AL_POSITION, x, y, z);
}
	
void CChannel_SetDistances(CChannel *channel, float max, float min)
{
    // Apply the channel operation to explicitly owned C state

	if ( !CChannel_HasSource(channel) ) return;
	alSourcef   (alSources[channel->id], AL_MAX_DISTANCE,       max);
	alSourcef   (alSources[channel->id], AL_REFERENCE_DISTANCE, min);
	alSourcef   (alSources[channel->id], AL_MAX_GAIN, 1.0f);
	alSourcef   (alSources[channel->id], AL_ROLLOFF_FACTOR, 1.0f);
}
	
void CChannel_SetPan(CChannel *channel, int32_t pan)
{
    // Apply the channel operation to explicitly owned C state

	CChannel_SetPosition(channel, (pan-63)/64.0f, 0.0f, Sqrt(1.0f-SQR((pan-63)/64.0f)));
}

void CChannel_ClearBuffer(CChannel *channel)
{
    // Apply the channel operation to explicitly owned C state

	if ( !CChannel_HasSource(channel) ) return;
	alSourcei(alSources[channel->id], AL_LOOPING, AL_FALSE);
	alSourcei(alSources[channel->id], AL_BUFFER, AL_NONE);
	channel->Data = nil;
	channel->DataSize = 0;
}

void CChannel_SetReverbMix(CChannel *channel, ALuint slot, float mix)
{
    // Apply the channel operation to explicitly owned C state

	if ( !IsFXSupported() ) return;
	if ( !CChannel_HasSource(channel) ) return;
	if ( alFilters[channel->id] == AL_FILTER_NULL ) return;
	
	channel->Mix = mix;
	EAX3_SetReverbMix(alFilters[channel->id], mix);
	alSource3i(alSources[channel->id], AL_AUXILIARY_SEND_FILTER, slot, 0, alFilters[channel->id]);
}

void CChannel_UpdateReverb(CChannel *channel, ALuint slot)
{
    // Apply the channel operation to explicitly owned C state

	if ( !IsFXSupported() ) return;
	if ( !CChannel_HasSource(channel) ) return;
	if ( alFilters[channel->id] == AL_FILTER_NULL ) return;
	EAX3_SetReverbMix(alFilters[channel->id], channel->Mix);
	alSource3i(alSources[channel->id], AL_AUXILIARY_SEND_FILTER, slot, 0, alFilters[channel->id]);
}

#endif
//- rouz edit (ChatGPT)
