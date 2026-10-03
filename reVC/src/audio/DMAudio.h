#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "../core/config.h"

#include "audio_enums.h"
#include "soundlist.h"
#include "../core/CrimeTypes.h"

#define AEHANDLE_IS_FAILED(h) ((h)<0)
#define AEHANDLE_IS_OK(h)     ((h)>=0)

#define NO_AUDIO_PROVIDER -3
#define AUDIO_PROVIDER_NOT_DETERMINED -99

#ifdef __cplusplus
typedef struct cAudioScriptObject cAudioScriptObject;
class CEntity;
class CPed;
class CVector;
extern "C" {
#else
typedef struct cAudioScriptObject cAudioScriptObject;
typedef struct CEntity CEntity;
typedef struct CPed CPed;
typedef struct CVector CVector;
#endif


	void DMAudio_Initialise(void);
	void DMAudio_Terminate(void);
	void DMAudio_Service(void);
	
	int32_t DMAudio_CreateEntity(enum eAudioType type, void *UID);
	void DMAudio_DestroyEntity(int32_t audioEntity);
	uint8_t DMAudio_GetEntityStatus(int32_t audioEntity);
	void DMAudio_SetEntityStatus(int32_t audioEntity, uint8_t status);
	void DMAudio_PlayOneShot(int32_t audioEntity, uint16_t oneShot, float volume);
	void DMAudio_DestroyAllGameCreatedEntities(void);
	
	void DMAudio_SetOutputMode(uint8_t surround);
	void DMAudio_SetMP3BoostVolume(uint8_t volume);
	void DMAudio_SetEffectsMasterVolume(uint8_t volume);
	void DMAudio_SetMusicMasterVolume(uint8_t volume);
	void DMAudio_SetEffectsFadeVol(uint8_t volume);
	void DMAudio_SetMusicFadeVol(uint8_t volume);
	
	uint8_t DMAudio_GetNum3DProvidersAvailable(void);
	char *DMAudio_Get3DProviderName(uint8_t id);
	
	int8_t DMAudio_AutoDetect3DProviders(void);
	
	int8_t DMAudio_GetCurrent3DProviderIndex(void);
	int8_t DMAudio_SetCurrent3DProvider(uint8_t which);
	
	void DMAudio_SetSpeakerConfig(int32_t config);
	
	uint8_t DMAudio_IsMP3RadioChannelAvailable(void);
	
	void DMAudio_ReleaseDigitalHandle(void);
	void DMAudio_ReacquireDigitalHandle(void);
	
	void DMAudio_SetDynamicAcousticModelingStatus(uint8_t status);
	
	uint8_t DMAudio_CheckForAnAudioFileOnCD(void);
	
	char DMAudio_GetCDAudioDriveLetter(void);
	uint8_t DMAudio_IsAudioInitialised(void);

	void DMAudio_ResetPoliceRadio(void);
	void DMAudio_ReportCrime(enum eCrimeType crime, const CVector *pos);
	
	int32_t DMAudio_CreateLoopingScriptObject(cAudioScriptObject *scriptObject);
	void DMAudio_DestroyLoopingScriptObject(int32_t audioEntity);
	void DMAudio_CreateOneShotScriptObject(cAudioScriptObject *scriptObject);
	
	void DMAudio_PlaySuspectLastSeen(float x, float y, float z);
	
	void DMAudio_ReportCollision(CEntity *entityA, CEntity *entityB, uint8_t surfaceTypeA, uint8_t surfaceTypeB, float collisionPower, float velocity);
	
	void DMAudio_PlayFrontEndSound(uint16_t frontend, uint32_t volume);
	void DMAudio_PlayRadioAnnouncement(uint32_t announcement);
	void DMAudio_PlayFrontEndTrack(uint32_t track, uint8_t frontendFlag);
	void DMAudio_StopFrontEndTrack(void);
	
	void DMAudio_ResetTimers(uint32_t time);
	
	void DMAudio_ChangeMusicMode(uint8_t mode);
	
	void DMAudio_PreloadCutSceneMusic(uint32_t track);
	void DMAudio_PlayPreloadedCutSceneMusic(void);
	void DMAudio_StopCutSceneMusic(void);
	
	void DMAudio_PreloadMissionAudio(uint8_t slot, const char *missionAudio);
	uint8_t DMAudio_GetMissionAudioLoadingStatus(uint8_t slot);
	void DMAudio_SetMissionAudioLocation(uint8_t slot, float x, float y, float z);
	void DMAudio_PlayLoadedMissionAudio(uint8_t slot);
	uint8_t DMAudio_IsMissionAudioSamplePlaying(uint8_t slot);
	uint8_t DMAudio_IsMissionAudioSampleFinished(uint8_t slot);
	void DMAudio_ClearMissionAudio(uint8_t slot);
	const char *DMAudio_GetMissionAudioLoadedLabel(uint8_t slot);

	uint8_t DMAudio_GetRadioInCar(void);
	void DMAudio_SetRadioInCar(uint32_t radio);
	void DMAudio_SetRadioChannel(uint32_t radio, int32_t pos);

	void DMAudio_SetStartingTrackPositions(uint8_t isStartGame);
	float *DMAudio_GetListenTimeArray(void);
	uint32_t DMAudio_GetFavouriteRadioStation(void);
	int32_t DMAudio_GetRadioPosition(uint32_t station);
	void DMAudio_SetPedTalkingStatus(CPed *ped, uint8_t status);
	void DMAudio_SetPlayersMood(uint8_t mood, uint32_t time);
	void DMAudio_ShutUpPlayerTalking(uint8_t state);

#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
