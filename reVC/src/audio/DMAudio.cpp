//+ rouz edit (ChatGPT)
#include "common.h"

#include "DMAudio.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "AudioScriptObject.h"
#include "sampman.h"
#include "AudioControls.h"
#include "AudioProvider.h"


















uint8
DMAudio_GetNum3DProvidersAvailable(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_GetNum3DProvidersAvailable(AudioManager.m_bIsInitialised);
}

char *
DMAudio_Get3DProviderName(uint8 id)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_Get3DProviderName(AudioManager.m_bIsInitialised, id);
}

int8 DMAudio_AutoDetect3DProviders(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_AutoDetect3DProviders(AudioManager.m_bIsInitialised);
}

int8
DMAudio_GetCurrent3DProviderIndex(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioControls_GetCurrent3DProviderIndex(AudioManager.m_bIsInitialised);
}

int8
DMAudio_SetCurrent3DProvider(uint8 which)
{
	// Preserve the original game audio operation through the C interface
	return AudioProvider_SetCurrent(AudioManager.m_bIsInitialised, which, &AudioManager.m_nActiveSamples, &AudioManager.m_nActiveQueue, AudioManager.m_aRequestedOrderList, AudioManager.m_nRequestedCount, AudioManager.m_asActiveSamples);
}

void
DMAudio_SetSpeakerConfig(int32 config)
{
	// Preserve the original game audio operation through the C interface
	AudioControls_SetSpeakerConfig(config);
}

bool8
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
DMAudio_SetDynamicAcousticModelingStatus(bool8 status)
{
	// Preserve the original game audio operation through the C interface
#ifdef AUDIO_REFLECTIONS
	AudioManager.m_bDynamicAcousticModelingStatus = status;
#endif
}

bool8
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

bool8
DMAudio_IsAudioInitialised(void)
{
	// Preserve the original game audio operation through the C interface
	return AudioManager.m_bIsInitialised;
}



int32
DMAudio_CreateLoopingScriptObject(cAudioScriptObject *scriptObject)
{
	// Preserve the original game audio operation through the C interface
	int32 audioEntity = AudioManager_CreateEntity(&AudioManager, AUDIOTYPE_SCRIPTOBJECT, scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
		AudioEntities_SetStatus(AudioManager.m_asAudioEntities, AudioManager.m_bIsInitialised, audioEntity, TRUE);
	
	return audioEntity;
}

void
DMAudio_DestroyLoopingScriptObject(int32 audioEntity)
{
	// Preserve the original game audio operation through the C interface
	AudioEntities_Destroy(AudioManager.m_asAudioEntities, AudioManager.m_aAudioEntityOrderList, &AudioManager.m_nAudioEntitiesCount, AudioManager.m_bIsInitialised, audioEntity);
}

void
DMAudio_CreateOneShotScriptObject(cAudioScriptObject *scriptObject)
{
	// Preserve the original game audio operation through the C interface
	int32 audioEntity = AudioManager_CreateEntity(&AudioManager, AUDIOTYPE_SCRIPTOBJECT, scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
	{
		AudioEntities_SetStatus(AudioManager.m_asAudioEntities, AudioManager.m_bIsInitialised, audioEntity, TRUE);
		AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, audioEntity, scriptObject->AudioId, 0.0f);
	}
}



void
DMAudio_PlayFrontEndSound(uint16 frontend, uint32 volume)
{
	// Preserve the original game audio operation through the C interface
	AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, AudioManager.m_nFrontEndEntity, frontend, (float)volume);
}

void
DMAudio_PlayRadioAnnouncement(uint32 announcement)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.PlayAnnouncement(announcement);
}

void
DMAudio_PlayFrontEndTrack(uint32 track, bool8 frontendFlag)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.PlayFrontEndTrack(track, frontendFlag);
}

void
DMAudio_StopFrontEndTrack(void)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.StopFrontEndTrack();
}


void
DMAudio_ChangeMusicMode(uint8 mode)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.ChangeMusicMode(mode);
}

void
DMAudio_PreloadCutSceneMusic(uint32 track)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.PreloadCutSceneMusic(track);
}

void
DMAudio_PlayPreloadedCutSceneMusic(void)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.PlayPreloadedCutSceneMusic();
}

void
DMAudio_StopCutSceneMusic(void)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.StopCutSceneMusic();
}









uint8
DMAudio_GetRadioInCar(void)
{
	// Preserve the original game audio operation through the C interface
	return MusicManager.GetRadioInCar();
}

void
DMAudio_SetRadioInCar(uint32 radio)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.SetRadioInCar(radio);
}

void
DMAudio_SetRadioChannel(uint32 radio, int32 pos)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.SetRadioChannelByScript(radio, pos);
}

void
DMAudio_SetStartingTrackPositions(bool8 isStartGame)
{
	// Preserve the original game audio operation through the C interface
	MusicManager.SetStartingTrackPositions(isStartGame);
}

float *
DMAudio_GetListenTimeArray()
{
	// Preserve the original game audio operation through the C interface
	return MusicManager.GetListenTimeArray();
}

uint32
DMAudio_GetFavouriteRadioStation()
{
	// Preserve the original game audio operation through the C interface
	return MusicManager.GetFavouriteRadioStation();
}

int32
DMAudio_GetRadioPosition(uint32 station)
{
	// Preserve the original game audio operation through the C interface
	return MusicManager.GetRadioPosition(station);
}

void
DMAudio_SetPedTalkingStatus(CPed *ped, bool8 status)
{
	// Preserve the original game audio operation through the C interface
	return AudioManager.SetPedTalkingStatus(ped, status);
}

void
DMAudio_SetPlayersMood(uint8 mood, uint32 time)
{
	// Preserve the original game audio operation through the C interface
	return AudioManager.SetPlayersMood(mood, time);
}

void
DMAudio_ShutUpPlayerTalking(bool8 state)
{
	// Preserve the original game audio operation through the C interface
	AudioManager.m_bIsPlayerShutUp = state;
}
//- rouz edit (ChatGPT)
