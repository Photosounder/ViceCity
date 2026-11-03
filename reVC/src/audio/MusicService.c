//+ rouz edit (ChatGPT)
#include "Timer.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"
#include "AudioSampleConfig.h"
#include "soundlist.h"
#include "../core/GameAreas.h"
#include "../peds/PedStates.h"
#include "../skel/RsKeys.h"
#include <stddef.h>

int32_t gNumRetunePresses;
int32_t gRetuneCounter;
uint8_t g_bAnnouncementReadPosAlready;
uint8_t RadioStaticCounter = 5;

#if defined(_MSC_VER) && !defined(__clang__)
#define MUSIC_SERVICE_STORE_TRACK_VALUE(manager, index, field, expression) do { \
    /* Capture the provider result before selecting the live destination track */ \
    uint32_t result = (expression); \
    (manager)->m_aTracks[(manager)->index].field = result; \
} while (0)
#else
#define MUSIC_SERVICE_STORE_TRACK_VALUE(manager, index, field, expression) \
    (manager)->m_aTracks[(manager)->index].field = (expression)
#endif

static uint8_t
MusicService_PedDyingOrDead(const CPed *ped)
{
    // Preserve the original short circuit between the two live pedestrian-state reads
    return AudioPedOneShotsHost_State(ped) == PED_DIE || AudioPedOneShotsHost_State(ped) == PED_DEAD;
}

void
MusicService_Run(cMusicManager *manager)
{
	// Preserve original music service gates, live control reads and owner writes
	if (manager->m_bResetTimers) {
		manager->m_bResetTimers = 0;
		manager->m_nLastTrackServiceTime = manager->m_nResetTime;
	}

	static uint8_t bRadioStatsRecorded = 0;

	if (!manager->m_bIsInitialised || manager->m_bDisabled) return;

	if (!manager->m_bMusicModeChangeStarted)
		manager->m_nMusicModeToBeSet = manager->m_nUpcomingMusicMode;
	if (manager->m_nMusicModeToBeSet == manager->m_nMusicMode) {
		if (!AudioManager.m_bIsPaused || AudioManager.m_bWasPaused || manager->m_nMusicMode != MUSICMODE_FRONTEND)
		{
			switch (manager->m_nMusicMode)
			{
			case MUSICMODE_FRONTEND: MusicModes_ServiceFrontEndMode(manager); break;
			case MUSICMODE_GAME: MusicService_GameMode(manager); break;
			case MUSICMODE_CUTSCENE: SampleManager_SetStreamedVolumeAndPan(&SampleManager, MAX_VOLUME, 63, 1, 0); break;
			}
		}
		else
			manager->m_nMusicMode = MUSICMODE_DISABLED;
	} else {
		manager->m_bMusicModeChangeStarted = 1;
		if (!manager->m_bUserResumedGame && !AudioManager.m_bIsPaused && AudioManager.m_bWasPaused)
			manager->m_bUserResumedGame = 1;
		if (AudioManager.m_FrameCounter % 4 == 0) {
			gNumRetunePresses = 0;
			gRetuneCounter = 0;
			manager->m_bSetNextStation = 0;
			if (SampleManager_IsStreamPlaying(&SampleManager, 0)) {
				if (manager->m_nPlayingTrack != NO_TRACK && !bRadioStatsRecorded)
				{
					MusicPositions_RecordRadioStats(manager);
					MUSIC_SERVICE_STORE_TRACK_VALUE(manager, m_nPlayingTrack, m_nPosition, SampleManager_GetStreamedFilePosition(&SampleManager, 0));
					MUSIC_SERVICE_STORE_TRACK_VALUE(manager, m_nPlayingTrack, m_nLastPosCheckTimer, Timer_GetTimeInMillisecondsPauseMode());
					bRadioStatsRecorded = 1;
				}
				SampleManager_StopStreamedFile(&SampleManager, 0);
			} else {
				bRadioStatsRecorded = 0;
				manager->m_nMusicMode = manager->m_nMusicModeToBeSet;
				manager->m_bMusicModeChangeStarted = 0;
				manager->m_bTrackChangeStarted = 0;
				manager->m_nNextTrack = NO_TRACK;
				manager->m_nNextLoopFlag = 0;
				manager->m_bVerifyNextTrackStartedToPlay = 0;
				manager->m_nPlayingTrack = NO_TRACK;
				if (manager->m_bEarlyFrontendTrack)
					manager->m_bEarlyFrontendTrack = 0;
				else
					manager->m_nFrontendTrack = NO_TRACK;
			}
		}
	}
}

void
MusicService_GameMode(cMusicManager *manager)
{
	// Preserve original music service gates, live control reads and owner writes
	CPed *ped = AudioPedOneShotsHost_Player();
	CVehicle *vehicle = AudioGame_FindVehicle();
	manager->m_bRadioStreamReady = manager->m_bGameplayAllowsRadio;
	manager->m_bGameplayAllowsRadio = 0;

	switch (MusicAmbienceHost_Area())
	{
	case AREA_HOTEL:
	case AREA_MALL:
	case AREA_STRIP_CLUB:
	case AREA_DIRT:
	case AREA_BLOOD:
	case AREA_OVALRING:
	case AREA_MALIBU_CLUB:
		manager->m_bGameplayAllowsRadio = 0;
		break;
	default:
		if (SampleManager_GetMusicVolume(&SampleManager)) {
			if (MusicRadio_PlayerInCar(manager))
				manager->m_bGameplayAllowsRadio = 1;
		} else
			manager->m_bGameplayAllowsRadio = 0;
		break;
	}

	if (!manager->m_bGameplayAllowsRadio) {
		manager->nFramesSinceCutsceneEnded = -1;
		gNumRetunePresses = 0;
		gRetuneCounter = 0;
		manager->m_bSetNextStation = 0;
	} else if (ped) {
		if(!MusicService_PedDyingOrDead(ped) && vehicle) {
#ifdef GTA_PC
			if (SampleManager_IsMP3RadioChannelAvailable(&SampleManager)
				&& (*MusicRadioHost_Station(vehicle)) < USERTRACK
				&& MusicServiceHost_F9())
			{
				if (!MusicRadio_UsesPoliceRadio(vehicle) && !MusicRadio_UsesTaxiRadio(vehicle)) {
					gNumRetunePresses = 0;
					gRetuneCounter = 20;
					RadioStaticCounter = 0;
					if ((*MusicRadioHost_Station(vehicle)) < USERTRACK)
					{
						do
							++gNumRetunePresses;
						while (gNumRetunePresses + (*MusicRadioHost_Station(vehicle)) < USERTRACK);
					}
				}
			}
#endif
			if (MusicServiceHost_ChangeStation())
			{
				if (!MusicRadio_UsesPoliceRadio(vehicle) && !MusicRadio_UsesTaxiRadio(vehicle)) {
					gNumRetunePresses++;
					gRetuneCounter = 20;
					RadioStaticCounter = 0;
				}
			}
#ifdef RADIO_SCROLL_TO_PREV_STATION
			else if(!MusicServiceHost_ControlsDisabled() && (MusicServiceHost_WheelDown() || MusicServiceHost_WheelUp())) {
				if(!MusicRadio_UsesPoliceRadio(vehicle) && !MusicRadio_UsesTaxiRadio(vehicle)) {
					int scrollNext = MusicServiceHost_NextMouseKey();
					int scrollPrev = scrollNext == rsMOUSEWHEELUPBUTTON ? rsMOUSEWHEELDOWNBUTTON
																		: scrollNext == rsMOUSEWHEELDOWNBUTTON ? rsMOUSEWHEELUPBUTTON : -1;

					if(scrollPrev != -1 && !MusicServiceHost_MouseAssigned(scrollPrev)) {
						gNumRetunePresses--;
						gRetuneCounter = 20;
						RadioStaticCounter = 0;
						int track = gNumRetunePresses + (*MusicRadioHost_Station(vehicle));
						while(track < 0) track += NUM_RADIOS + 1;
						while(track >= NUM_RADIOS + 1) track -= NUM_RADIOS + 1;
						if(!DMAudio_IsMP3RadioChannelAvailable() && track == USERTRACK) gNumRetunePresses--;
					}
				}
			}
#endif
		}
	}

	if (manager->m_bUserResumedGame)
	{
		manager->m_bRadioStreamReady = 0;
		manager->m_bUserResumedGame = 0;
	}
	if (manager->m_nPlayingTrack == NO_TRACK && manager->m_nFrontendTrack == NO_TRACK)
		manager->m_bRadioStreamReady = 0;

	if (manager->m_bGameplayAllowsRadio)
	{
		if (!manager->m_bRadioStreamReady)
		{
			if(vehicle == NULL) {
				manager->m_nFrontendTrack = STREAMED_SOUND_RADIO_WAVE; // huh?
				return;
			}
			if(manager->m_bRadioSetByScript) {
				if(MusicRadio_UsesPoliceRadio(vehicle))
					manager->m_nFrontendTrack = STREAMED_SOUND_RADIO_POLICE;
				else if(MusicRadio_UsesTaxiRadio(vehicle))
					manager->m_nFrontendTrack = STREAMED_SOUND_RADIO_TAXI;
				else {
					manager->m_nFrontendTrack = manager->m_nRadioStationScript;
					(*MusicRadioHost_Station(vehicle)) = manager->m_nRadioStationScript;
				}
				if(manager->m_nRadioPosition != -1) {
					manager->m_aTracks[manager->m_nFrontendTrack].m_nPosition = manager->m_nRadioPosition;
					MUSIC_SERVICE_STORE_TRACK_VALUE(manager, m_nFrontendTrack, m_nLastPosCheckTimer, Timer_GetTimeInMillisecondsPauseMode());
				}
				manager->m_bRadioSetByScript = 0;
				return;
			}

			// This starts the radio when you enter the car.
			manager->m_nFrontendTrack = MusicRadio_GetCarTuning(manager);
			return;
		}
		if (manager->m_nAnnouncement < NO_TRACK) {
			if ((manager->m_bAnnouncementInProgress || manager->m_nFrontendTrack == manager->m_nPlayingTrack) && MusicStream_ServiceAnnouncement(manager)) {
				if (manager->m_bAnnouncementInProgress) {
					manager->m_bSetNextStation = 0;
					gNumRetunePresses = 0;
					gRetuneCounter = 0;
					return;
				}
				if(manager->m_nAnnouncement == NO_TRACK) {
					manager->m_nNextTrack = NO_TRACK;
					manager->m_nFrontendTrack = MusicRadio_GetCarTuning(manager);
					manager->m_bSetNextStation = 0;
					gRetuneCounter = 0;
					gNumRetunePresses = 0;
				}
			}
		}
		if (!manager->m_bAnnouncementInProgress
			&& manager->m_nAnnouncement == NO_TRACK
			&& manager->m_nPlayingTrack == STREAMED_SOUND_RADIO_MP3_PLAYER
			&& !SampleManager_IsStreamPlaying(&SampleManager, 0))
		{
			SampleManager_StartStreamedFile(&SampleManager, STREAMED_SOUND_RADIO_MP3_PLAYER, 0, 0);
		}

		if (!manager->m_bRadioSetByScript)
		{
			// Because when you switch radio back and forth, gNumRetunePresses will be 0 but gRetuneCounter won't.
#ifdef RADIO_SCROLL_TO_PREV_STATION
			if(gRetuneCounter != 0) {
				if(gRetuneCounter > 1)
					gRetuneCounter--;
				else if(gRetuneCounter == 1) {
					manager->m_bSetNextStation = 1;
					gRetuneCounter = 0;
				}
			}
#else
			if (gNumRetunePresses != 0)
			{
				if (--gRetuneCounter == 0)
				{
					manager->m_bSetNextStation = 1;
					gRetuneCounter = 0;
				}
			}
#endif
			if (gRetuneCounter)
			{
				int32_t station = gNumRetunePresses + (*MusicRadioHost_Station(vehicle));
#ifdef RADIO_SCROLL_TO_PREV_STATION
				while (station < 0) station += NUM_RADIOS + 1;
#endif
				while (station >= NUM_RADIOS + 1) station -= NUM_RADIOS + 1;

				// Scrolling back won't hit here, so increasing isn't problem
				if (!DMAudio_IsMP3RadioChannelAvailable() && station == USERTRACK)
				{
					++gNumRetunePresses;
					station = RADIO_OFF;
				}
				if (station == RADIO_OFF)
				{
					if (gRetuneCounter == 19) // One less then what switching radio sets, so runs right after turning off radio
					{
						AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_RADIO_TURN_OFF, 0.0f);
						RadioStaticCounter = 5;
					}
				}
				else
				{
#ifdef RADIO_SCROLL_TO_PREV_STATION
					if ((*MusicRadioHost_Station(vehicle)) == RADIO_OFF && gRetuneCounter == 19) // Right after turning on the radio
#else
					if (station == 0 && gRetuneCounter == 19) // Right after turning on the radio
#endif
						AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_RADIO_TURN_ON, 0.0f);
					AudioPolice_Crackle(&AudioManager);
				}
			}
			if (RadioStaticCounter < 2 && AudioPlayerEngineHost_Time() > RadioStaticTimer + 800)
			{
				AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, AudioManager.m_nFrontEndEntity, SOUND_RADIO_CHANGE, 0.0f);
				RadioStaticCounter++;
				RadioStaticTimer = AudioPlayerEngineHost_Time();
			}
			if (manager->m_bSetNextStation)
				manager->m_nFrontendTrack = MusicRadio_GetNextCarTuning(manager);
			if (manager->m_nFrontendTrack >= STREAMED_SOUND_CITY_AMBIENT && manager->m_nFrontendTrack <= STREAMED_SOUND_AMBSIL_AMBIENT)
				MusicAmbience_SetUpCorrectAmbienceTrack(manager);
			MusicStream_ServiceTrack(manager, vehicle, ped);
			if (manager->m_bSetNextStation)
				manager->m_bSetNextStation = 0;
			return;
		}
		if (MusicRadio_UsesPoliceRadio(vehicle))
			manager->m_nFrontendTrack = STREAMED_SOUND_RADIO_POLICE;
		else if (MusicRadio_UsesTaxiRadio(vehicle))
			manager->m_nFrontendTrack = STREAMED_SOUND_RADIO_TAXI;
		else {
			manager->m_nFrontendTrack = manager->m_nRadioStationScript;
			(*MusicRadioHost_Station(vehicle)) = manager->m_nRadioStationScript;
		}

		if (manager->m_nRadioPosition != -1) {
			manager->m_aTracks[manager->m_nFrontendTrack].m_nPosition = manager->m_nRadioPosition;
			MUSIC_SERVICE_STORE_TRACK_VALUE(manager, m_nFrontendTrack, m_nLastPosCheckTimer, Timer_GetTimeInMillisecondsPauseMode());
		}

		gRetuneCounter = 0;
		gNumRetunePresses = 0;
		manager->m_bSetNextStation = 0;
		manager->m_bRadioSetByScript = 0;
		if (manager->m_nFrontendTrack >= STREAMED_SOUND_CITY_AMBIENT && manager->m_nFrontendTrack <= STREAMED_SOUND_AMBSIL_AMBIENT)
			MusicAmbience_SetUpCorrectAmbienceTrack(manager);
		MusicStream_ServiceTrack(manager, vehicle, ped);
		if (manager->m_bSetNextStation)
			manager->m_bSetNextStation = 0;
		return;
	}

	if (manager->m_bAnnouncementInProgress)
	{
		SampleManager_StopStreamedFile(&SampleManager, 0);
		if (SampleManager_IsStreamPlaying(&SampleManager, 0))
			return;
		g_bAnnouncementReadPosAlready = 0;
		manager->m_nAnnouncement = NO_TRACK;
		manager->m_bAnnouncementInProgress = 0;
		manager->m_nNextTrack = NO_TRACK;
		manager->m_nFrontendTrack = NO_TRACK;
		manager->m_nPlayingTrack = NO_TRACK;
	}
	MusicAmbience_SetUpCorrectAmbienceTrack(manager);
	MusicStream_ServiceTrack(manager, NULL, ped);
}

//- rouz edit (ChatGPT)
