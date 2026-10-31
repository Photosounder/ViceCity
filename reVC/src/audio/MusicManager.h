//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "AudioSound.h"
#include "../core/config.h"

#include "audio_enums.h"

typedef struct tStreamedSample {
	uint32_t m_nLength;
	uint32_t m_nPosition;
	uint32_t m_nLastPosCheckTimer;
} tStreamedSample;

#ifdef __cplusplus
class CVehicle;
class CPed;
class CEntity;
#else
typedef struct CVehicle CVehicle;
typedef struct CPed CPed;
typedef struct CEntity CEntity;
#endif

typedef struct cMusicManager {
	uint8_t m_bIsInitialised;
	uint8_t m_bDisabled;
	uint8_t m_bSetNextStation;
	uint8_t m_nVolumeLatency;
	uint8_t m_nCurrentVolume;
	uint8_t m_nMaxVolume;
	uint32_t m_nAnnouncement;
	uint8_t m_bAnnouncementInProgress;
	tStreamedSample m_aTracks[TOTAL_STREAMED_SOUNDS];
	uint8_t m_bResetTimers;
	uint32_t m_nResetTime;
	uint8_t m_bRadioSetByScript;
	uint8_t m_nRadioStationScript;
	int32_t m_nRadioPosition;
	uint32_t m_nRadioInCar;
	uint32_t m_nFrontendTrack;
	uint32_t m_nPlayingTrack;
	uint8_t m_nUpcomingMusicMode;
	uint8_t m_nMusicMode;
	uint8_t m_FrontendLoopFlag;
	uint8_t m_bTrackChangeStarted;
	uint32_t m_nNextTrack;
	uint8_t m_nNextLoopFlag;
	uint8_t m_bVerifyNextTrackStartedToPlay;
	uint8_t m_bGameplayAllowsRadio;
	uint8_t m_bRadioStreamReady;
	int8_t nFramesSinceCutsceneEnded;
	uint8_t m_bUserResumedGame;
	uint8_t m_bMusicModeChangeStarted;
	uint8_t m_nMusicModeToBeSet;
	uint8_t m_bEarlyFrontendTrack;
	float aListenTimeArray[NUM_RADIOS];
	float m_nLastTrackServiceTime;

#ifdef __cplusplus
	cMusicManager();

#endif
} cMusicManager;

#ifdef __cplusplus
VALIDATE_SIZE(cMusicManager, 0x95C);
#elif defined(CHECK_STRUCT_SIZES)
_Static_assert(sizeof(cMusicManager) == 0x95C, "cMusicManager layout");
#endif

#ifdef __cplusplus
extern "C" {
#endif
void MusicPlayback_Terminate(cMusicManager *manager);
void MusicPlayback_PreloadCutSceneMusic(cMusicManager *manager, uint32_t track);
void MusicPlayback_PlayPreloadedCutSceneMusic(cMusicManager *manager);
void MusicPlayback_StopCutSceneMusic(cMusicManager *manager);
uint8_t MusicInit_Initialise(cMusicManager *manager);
void MusicInit_SetStartingTrackPositions(cMusicManager *manager, uint8_t isNewGameTimer);
uint32_t MusicInitHost_PauseTime(void);
extern int32_t gNumRetunePresses;
extern int32_t gRetuneCounter;
extern uint32_t NewGameRadioTimers[10];
void MusicPositions_RecordRadioStats(cMusicManager *manager);
uint32_t MusicPositions_GetTrackStartPos(cMusicManager *manager, uint32_t track);
uint32_t MusicPositions_GetRadioPosition(cMusicManager *manager, uint32_t station);
void MusicReload_ResetMusicAfterReload(cMusicManager *manager);
extern uint32_t RadioStaticTimer;
void MusicAmbience_SetUpCorrectAmbienceTrack(cMusicManager *manager);
void MusicAmbience_ComputeAmbienceVol(cMusicManager *manager, uint8_t reset, uint8_t *outVolume);
float MusicAmbience_GetHeightScale(void);
int32_t MusicAmbienceHost_Area(void);
uint8_t MusicAmbienceHost_RiotIntensity(void);
uint8_t MusicAmbienceHost_AtBeach(void);
int16_t MusicAmbienceHost_OldWeather(void);
int16_t MusicAmbienceHost_NewWeather(void);
float MusicAmbienceHost_DistanceToWater(void);
void MusicClubs_SetMalibuClubTrackPos(cMusicManager *manager, uint8_t scriptObject);
void MusicClubs_SetStripClubTrackPos(cMusicManager *manager, uint8_t scriptObject);
void MusicModes_ChangeMusicMode(cMusicManager *manager, uint8_t mode);
void MusicModes_ServiceFrontEndMode(cMusicManager *manager);
uint8_t MusicRadio_PlayerInCar(cMusicManager *manager);
uint32_t MusicRadio_GetRadioInCar(cMusicManager *manager);
void MusicRadio_SetRadioInCar(cMusicManager *manager, uint32_t station);
uint32_t MusicRadio_GetNextCarTuning(cMusicManager *manager);
uint32_t MusicRadio_GetCarTuning(cMusicManager *manager);
uint8_t MusicRadio_UsesPoliceRadio(CVehicle *vehicle);
uint8_t MusicRadio_UsesTaxiRadio(CVehicle *vehicle);
uint8_t *MusicRadioHost_Station(CVehicle *vehicle);
uint8_t MusicRadioHost_UsesSiren(CVehicle *vehicle);
uint8_t MusicRadioHost_HasMetDebbie(void);
uint8_t MusicStream_ServiceAnnouncement(cMusicManager *manager);
void MusicStream_ServiceTrack(cMusicManager *manager, CVehicle *vehicle, CPed *ped);
uint8_t MusicStreamHost_SlowMotion(void);
CEntity *MusicStreamHost_Target(void);
void MusicStreamHost_TargetPosition(CEntity *target, AudioSoundPosition *position);
void MusicService_Run(cMusicManager *manager);
void MusicService_GameMode(cMusicManager *manager);
extern uint8_t RadioStaticCounter;
#ifdef GTA_PC
uint8_t MusicServiceHost_F9(void);
#endif
uint8_t MusicServiceHost_ChangeStation(void);
#ifdef RADIO_SCROLL_TO_PREV_STATION
uint8_t MusicServiceHost_ControlsDisabled(void);
uint8_t MusicServiceHost_WheelDown(void);
uint8_t MusicServiceHost_WheelUp(void);
int32_t MusicServiceHost_NextMouseKey(void);
uint8_t MusicServiceHost_MouseAssigned(int32_t key);
#endif
void MusicDisplay_StationName(cMusicManager *manager);
uint8_t MusicDisplayHost_Paused(void);
uint8_t MusicDisplayHost_Widescreen(void);
uint16_t * MusicDisplayHost_Text(const char *key);
uint16_t * MusicDisplayHost_ErrorText(void);
float MusicDisplayHost_ScaleX(float value);
float MusicDisplayHost_ScaleY(float value);
float MusicDisplayHost_Width(void);
float MusicDisplayHost_CentreSize(void);
void MusicDisplayFont_JustifyOff(void);
void MusicDisplayFont_BackgroundOff(void);
void MusicDisplayFont_Scale(float x, float y);
void MusicDisplayFont_PropOn(void);
void MusicDisplayFont_Standard(void);
void MusicDisplayFont_CentreOn(void);
void MusicDisplayFont_CentreSize(float size);
void MusicDisplayFont_Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void MusicDisplayFont_Print(float x, float y, uint16_t *text);
void MusicDisplayFont_Draw(void);
void MusicState_InitState(cMusicManager *manager);
uint8_t MusicState_IsInitialised(const cMusicManager *manager);
uint8_t MusicState_GetMusicMode(const cMusicManager *manager);
uint32_t MusicState_GetCurrentTrack(const cMusicManager *manager);
void MusicState_ResetTimers(cMusicManager *manager, uint32_t time);
void MusicState_SetRadioChannelByScript(cMusicManager *manager, uint32_t station, int32_t pos);
void MusicState_PlayFrontEndTrack(cMusicManager *manager, uint32_t track, uint8_t loopFlag);
void MusicState_StopFrontEndTrack(cMusicManager *manager);
void MusicState_PlayAnnouncement(cMusicManager *manager, uint32_t announcement);
float *MusicState_GetListenTimeArray(cMusicManager *manager);
uint32_t MusicState_GetFavouriteRadioStation(const cMusicManager *manager);
uint8_t MusicState_CheckForMusicInterruptions(const cMusicManager *manager);
extern cMusicManager MusicManager;
extern uint8_t g_bAnnouncementReadPosAlready; // we have a symbol of this so it was declared in .h
#ifdef __cplusplus
}
#endif

//- rouz edit (ChatGPT)
