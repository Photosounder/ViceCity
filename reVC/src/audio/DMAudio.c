//+ rouz edit (ChatGPT)
#include <stdint.h>

#include "DMAudio.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "AudioScriptObject.h"
#include "AudioControls.h"
#include "AudioProvider.h"


















uint8_t
DMAudio_GetNum3DProvidersAvailable(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_GetNum3DProvidersAvailable(AudioManager.m_bIsInitialised);
}

char *
DMAudio_Get3DProviderName(uint8_t id)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_Get3DProviderName(AudioManager.m_bIsInitialised, id);
}

int8_t DMAudio_AutoDetect3DProviders(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_AutoDetect3DProviders(AudioManager.m_bIsInitialised);
}

int8_t
DMAudio_GetCurrent3DProviderIndex(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_GetCurrent3DProviderIndex(AudioManager.m_bIsInitialised);
}

int8_t
DMAudio_SetCurrent3DProvider(uint8_t which)
{
	// Preserve the original game audio operation through the C interface
	return AudioProvider_SetCurrent(AudioManager.m_bIsInitialised, which, &AudioManager.m_nActiveSamples, &AudioManager.m_nActiveQueue, AudioManager.m_aRequestedOrderList, AudioManager.m_nRequestedCount, AudioManager.m_asActiveSamples);
}

void
DMAudio_SetSpeakerConfig(int32_t config)
{
	// Preserve the original game audio operation through the C interface
	AudioControls_SetSpeakerConfig(config);
}

uint8_t
DMAudio_IsMP3RadioChannelAvailable(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_IsMP3RadioChannelAvailable(AudioManager.m_bIsInitialised);
}

void
DMAudio_ReleaseDigitalHandle(void)
{
	// Preserve the original game audio operation through the C interface
	AudioControls_ReleaseDigitalHandle(AudioManager.m_bIsInitialised);
}

void
DMAudio_ReacquireDigitalHandle(void)
{
	// Preserve the original game audio operation through the C interface
	AudioControls_ReacquireDigitalHandle(AudioManager.m_bIsInitialised);
}

void
DMAudio_SetDynamicAcousticModelingStatus(uint8_t status)
{
	// Preserve the original game audio operation through the C interface
#ifdef AUDIO_REFLECTIONS
	AudioManager.m_bDynamicAcousticModelingStatus = status;
#endif
}

uint8_t
DMAudio_CheckForAnAudioFileOnCD(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_CheckForAnAudioFileOnCD();
}

char
DMAudio_GetCDAudioDriveLetter(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_GetCDAudioDriveLetter(AudioManager.m_bIsInitialised);
}

uint8_t
DMAudio_IsAudioInitialised(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioManager.m_bIsInitialised;
}



int32_t
DMAudio_CreateLoopingScriptObject(cAudioScriptObject *scriptObject)
{
	// Preserve the original game audio operation through the C interface
	int32_t audioEntity = AudioManager_CreateEntity(&AudioManager, AUDIOTYPE_SCRIPTOBJECT, scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
		AudioEntities_SetStatus(AudioManager.m_asAudioEntities, AudioManager.m_bIsInitialised, audioEntity, 1);
	
	return audioEntity;
}

void
DMAudio_DestroyLoopingScriptObject(int32_t audioEntity)
{
	// Preserve the original game audio operation through the C interface
	AudioEntities_Destroy(AudioManager.m_asAudioEntities, AudioManager.m_aAudioEntityOrderList, &AudioManager.m_nAudioEntitiesCount, AudioManager.m_bIsInitialised, audioEntity);
}

void
DMAudio_CreateOneShotScriptObject(cAudioScriptObject *scriptObject)
{
	// Preserve the original game audio operation through the C interface
	int32_t audioEntity = AudioManager_CreateEntity(&AudioManager, AUDIOTYPE_SCRIPTOBJECT, scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
	{
		AudioEntities_SetStatus(AudioManager.m_asAudioEntities, AudioManager.m_bIsInitialised, audioEntity, 1);
		AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, audioEntity, scriptObject->AudioId, 0.0f);
	}
}



void
DMAudio_PlayFrontEndSound(uint16_t frontend, uint32_t volume)
{
	// Preserve the original game audio operation through the C interface
	AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, AudioManager.m_nFrontEndEntity, frontend, (float)volume);
}

void
DMAudio_PlayRadioAnnouncement(uint32_t announcement)
{
	// Preserve the original game audio operation through the C interface
	MusicState_PlayAnnouncement(&MusicManager, announcement);
}

void
DMAudio_PlayFrontEndTrack(uint32_t track, uint8_t frontendFlag)
{
	// Preserve the original game audio operation through the C interface
	MusicState_PlayFrontEndTrack(&MusicManager, track, frontendFlag);
}

void
DMAudio_StopFrontEndTrack(void)
{
	// Preserve the original game audio operation through the C interface
	MusicState_StopFrontEndTrack(&MusicManager);
}
















void
DMAudio_SetRadioChannel(uint32_t radio, int32_t pos)
{
	// Preserve the original game audio operation through the C interface
	MusicState_SetRadioChannelByScript(&MusicManager, radio, pos);
}


float *
DMAudio_GetListenTimeArray(void)
{
	// Preserve the original game audio operation through the C interface
	return MusicState_GetListenTimeArray(&MusicManager);
}

uint32_t
DMAudio_GetFavouriteRadioStation(void)
{
	// Preserve the original game audio operation through the C interface
	return MusicState_GetFavouriteRadioStation(&MusicManager);
}




void
DMAudio_ShutUpPlayerTalking(uint8_t state)
{
	// Preserve the original game audio operation through the C interface
	AudioManager.m_bIsPlayerShutUp = state;
}
//- rouz edit (ChatGPT)
