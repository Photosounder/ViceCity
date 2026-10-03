//+ rouz edit (ChatGPT)
#include "../core/config.h"
#include <stdint.h>
#include <stddef.h>
#include "oal/AudioStreamHost.h"
typedef uint8_t bool8;
#define FALSE 0
#define TRUE 1
#ifndef MASTER
#define ASSERT(value) ((void)((!!(value)) || (re3_assert(#value, __FILE__, __LINE__, __func__), 0)))
#else
#define ASSERT(value) ((void)(value))
#endif
#if !defined(AUDIO_OAL) &&  !defined(AUDIO_MSS)
#include "sampman.h"
#include "AudioReflectionTypes.h"

cSampleManager SampleManager = {0}; // rouz edit (ChatGPT)
bool8 _bSampmanInitialised = FALSE;

uint32_t BankStartOffset[MAX_SFX_BANKS];
uint32_t     nNumMP3s;





#ifdef EXTERNAL_3D_SOUND
void SampleManager_SetSpeakerConfig(cSampleManager *manager, int32_t nConfig)
{
    // Operate on the explicitly supplied sample-manager state


}

uint32_t SampleManager_GetMaximumSupportedChannels(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state
	
	return MAXCHANNELS;
}

uint32_t SampleManager_GetNum3DProvidersAvailable(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return 1;
}

void SampleManager_SetNum3DProvidersAvailable(cSampleManager *manager, uint32_t num)
{
    // Operate on the explicitly supplied sample-manager state

	
}

char *SampleManager_Get3DProviderName(cSampleManager *manager, uint8_t id)
{
    // Operate on the explicitly supplied sample-manager state

	static char name[64] = "NULL";
	return name;
}

void SampleManager_Set3DProviderName(cSampleManager *manager, uint8_t id, char *name)
{
    // Operate on the explicitly supplied sample-manager state

	
}

int8_t SampleManager_GetCurrent3DProviderIndex(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return 0;
}

int8_t SampleManager_SetCurrent3DProvider(cSampleManager *manager, uint8_t nProvider)
{
    // Operate on the explicitly supplied sample-manager state

	return 0;
}
#endif

bool8
SampleManager_IsMP3RadioChannelAvailable(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return nNumMP3s != 0;
}


void SampleManager_ReleaseDigitalHandle(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

}

void SampleManager_ReacquireDigitalHandle(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

}

bool8
SampleManager_Initialise(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return TRUE;
}

void
SampleManager_Terminate(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state


}

bool8 SampleManager_CheckForAnAudioFileOnCD(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return TRUE;
}

char SampleManager_GetCDAudioDriveLetter(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return '\0';
}

void
SampleManager_UpdateEffectsVolume(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	
}

void
SampleManager_SetEffectsMasterVolume(cSampleManager *manager, uint8_t nVolume)
{
    // Operate on the explicitly supplied sample-manager state

}

void
SampleManager_SetMusicMasterVolume(cSampleManager *manager, uint8_t nVolume)
{
    // Operate on the explicitly supplied sample-manager state

}

void
SampleManager_SetMP3BoostVolume(cSampleManager *manager, uint8_t nVolume)
{
    // Operate on the explicitly supplied sample-manager state

}

void
SampleManager_SetEffectsFadeVolume(cSampleManager *manager, uint8_t nVolume)
{
    // Operate on the explicitly supplied sample-manager state

}

void
SampleManager_SetMusicFadeVolume(cSampleManager *manager, uint8_t nVolume)
{
    // Operate on the explicitly supplied sample-manager state

}

void
SampleManager_SetMonoMode(cSampleManager *manager, bool8 nMode)
{
    // Operate on the explicitly supplied sample-manager state

}

bool8
SampleManager_LoadSampleBank(cSampleManager *manager, uint8_t nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nBank < MAX_SFX_BANKS );
	return FALSE;
}

void
SampleManager_UnloadSampleBank(cSampleManager *manager, uint8_t nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nBank < MAX_SFX_BANKS );
}

int8_t
SampleManager_IsSampleBankLoaded(cSampleManager *manager, uint8_t nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nBank < MAX_SFX_BANKS );
	
	return LOADING_STATUS_NOT_LOADED;
}

uint8_t
SampleManager_IsMissionAudioLoaded(cSampleManager *manager, uint8_t nSlot, uint32_t nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT(nSlot < MISSION_AUDIO_COUNT);
	
	return LOADING_STATUS_NOT_LOADED;
}

bool8
SampleManager_LoadMissionAudio(cSampleManager *manager, uint8_t nSlot, uint32_t nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT(nSlot < MISSION_AUDIO_COUNT);
	
	return FALSE;
}

uint8_t
SampleManager_IsPedCommentLoaded(cSampleManager *manager, uint32_t nComment)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );

	return LOADING_STATUS_NOT_LOADED;
}


int32_t
SampleManager__GetPedCommentSlot(cSampleManager *manager, uint32_t nComment)
{
    // Operate on the explicitly supplied sample-manager state

	return -1;
}

bool8
SampleManager_LoadPedComment(cSampleManager *manager, uint32_t nComment)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );
	return FALSE;
}

int32_t
SampleManager_GetBankContainingSound(cSampleManager *manager, uint32_t offset)
{
    // Operate on the explicitly supplied sample-manager state

	return INVALID_SFX_BANK;
}

uint32_t
SampleManager_GetSampleBaseFrequency(cSampleManager *manager, uint32_t nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

uint32_t
SampleManager_GetSampleLoopStartOffset(cSampleManager *manager, uint32_t nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

int32_t
SampleManager_GetSampleLoopEndOffset(cSampleManager *manager, uint32_t nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

uint32_t
SampleManager_GetSampleLength(cSampleManager *manager, uint32_t nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return 0;
}

bool8 SampleManager_UpdateReverb(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return FALSE;
}

void
SampleManager_SetChannelReverbFlag(cSampleManager *manager, uint32_t nChannel, bool8 nReverbFlag)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

bool8
SampleManager_InitialiseChannel(cSampleManager *manager, uint32_t nChannel, uint32_t nSfx, uint8_t nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
	return FALSE;
}

#ifdef EXTERNAL_3D_SOUND
void
SampleManager_SetChannelEmittingVolume(cSampleManager *manager, uint32_t nChannel, uint32_t nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_SetChannel3DPosition(cSampleManager *manager, uint32_t nChannel, float fX, float fY, float fZ)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_SetChannel3DDistances(cSampleManager *manager, uint32_t nChannel, float fMax, float fMin)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}
#endif

void
SampleManager_SetChannelVolume(cSampleManager *manager, uint32_t nChannel, uint32_t nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel >= MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_SetChannelPan(cSampleManager *manager, uint32_t nChannel, uint32_t nPan)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel >= MAXCHANNELS );
	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_SetChannelFrequency(cSampleManager *manager, uint32_t nChannel, uint32_t nFreq)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_SetChannelLoopPoints(cSampleManager *manager, uint32_t nChannel, uint32_t nLoopStart, int32_t nLoopEnd)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_SetChannelLoopCount(cSampleManager *manager, uint32_t nChannel, uint32_t nLoopCount)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

bool8
SampleManager_GetChannelUsedFlag(cSampleManager *manager, uint32_t nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );

	return FALSE;
}

void
SampleManager_StartChannel(cSampleManager *manager, uint32_t nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_StopChannel(cSampleManager *manager, uint32_t nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS+MAX2DCHANNELS );
}

void
SampleManager_PreloadStreamedFile(cSampleManager *manager, uint32_t nFile, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
}

void
SampleManager_PauseStream(cSampleManager *manager, bool8 nPauseFlag, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
}

void
SampleManager_StartPreloadedStreamedFile(cSampleManager *manager, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
}

bool8
SampleManager_StartStreamedFile(cSampleManager *manager, uint32_t nFile, uint32_t nPos, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state
	
	ASSERT( nStream < MAX_STREAMS );
	
	return FALSE;
}

void
SampleManager_StopStreamedFile(cSampleManager *manager, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
}

int32_t
SampleManager_GetStreamedFilePosition(cSampleManager *manager, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
	
	return 0;
}

void
SampleManager_SetStreamedVolumeAndPan(cSampleManager *manager, uint8_t nVolume, uint8_t nPan, bool8 nEffectFlag, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
}

int32_t
SampleManager_GetStreamedFileLength(cSampleManager *manager, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < TOTAL_STREAMED_SOUNDS );

	return 1;
}

bool8
SampleManager_IsStreamPlaying(cSampleManager *manager, uint8_t nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );

	return FALSE;
}

bool8
SampleManager_InitialiseSampleBanks(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	
	return TRUE;
}

void
SampleManager_SetStreamedFileLoopFlag(cSampleManager *manager, bool8 nLoopFlag, uint8_t nChannel)
{
    // Operate on the explicitly supplied sample-manager state

}

int8_t SampleManager_AutoDetect3DProviders(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return -1;
}

#endif
//- rouz edit (ChatGPT)
