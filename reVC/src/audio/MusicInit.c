//+ rouz edit (ChatGPT)
#include "MusicManager.h"
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"
#include <time.h>

uint32_t NewGameRadioTimers[10] =
{
	948160,
	452150,
	2438150,
	3538230,
	3513100,
	4246050,
	1418050,
	3178240,
	471210,
	0
};

uint8_t
MusicInit_Initialise(cMusicManager *manager)
{
	// Preserve original initialization gates, live callbacks and partial state writes

	if (!MusicState_IsInitialised(manager)) {
		manager->m_bIsInitialised = 1;
		MusicInit_SetStartingTrackPositions(manager, 0);
		manager->m_bResetTimers = 0;
		manager->m_nResetTime = 0;
		manager->m_bRadioSetByScript = 0;
		manager->m_nRadioStationScript = WILDSTYLE;
		manager->m_nRadioPosition = -1;
		manager->m_nRadioInCar = NO_TRACK;
		gRetuneCounter = 0;
		gNumRetunePresses = 0;
		manager->m_nFrontendTrack = NO_TRACK;
		manager->m_nPlayingTrack = NO_TRACK;
		manager->m_nUpcomingMusicMode = MUSICMODE_DISABLED;
		manager->m_nMusicMode = MUSICMODE_DISABLED;
		manager->m_FrontendLoopFlag = 0;
		manager->m_bTrackChangeStarted = 0;
		manager->m_nNextTrack = NO_TRACK;
		manager->m_nNextLoopFlag = 0;
		manager->m_bVerifyNextTrackStartedToPlay = 0;
		manager->m_bGameplayAllowsRadio = 0;
		manager->m_bRadioStreamReady = 0;
		manager->nFramesSinceCutsceneEnded = -1;
		manager->m_bUserResumedGame = 0;
		manager->m_bMusicModeChangeStarted = 0;
		manager->m_nMusicModeToBeSet = MUSICMODE_DISABLED;
		manager->m_bEarlyFrontendTrack = 0;
		manager->m_nVolumeLatency = 0;
		manager->m_nCurrentVolume = 0;
		manager->m_nMaxVolume = 0;
	}
	return manager->m_bIsInitialised;
}

void
MusicInit_SetStartingTrackPositions(cMusicManager *manager, uint8_t isNewGameTimer)
{
	// Preserve original initialization gates, live callbacks and partial state writes

	int pos;

	if (MusicState_IsInitialised(manager)) {
		time_t timevalue = time(0);
		if (timevalue == -1) {
			pos = AudioManager.m_anRandomTable[0];
		} else {
			struct tm* pTm = localtime(&timevalue);
			if (pTm->tm_sec == 0)
				pTm->tm_sec = AudioManager.m_anRandomTable[0];
			if (pTm->tm_min == 0)
				pTm->tm_min = AudioManager.m_anRandomTable[1];
			if (pTm->tm_hour == 0)
				pTm->tm_hour = AudioManager.m_anRandomTable[2];
			if (pTm->tm_mday == 0)
				pTm->tm_mday = AudioManager.m_anRandomTable[3];
			if (pTm->tm_mon == 0)
				pTm->tm_mon = AudioManager.m_anRandomTable[4];
			if (pTm->tm_year == 0)
				pTm->tm_year = AudioManager.m_anRandomTable[3];
			if (pTm->tm_wday == 0)
				pTm->tm_wday = AudioManager.m_anRandomTable[2];
			pos = pTm->tm_yday
				* pTm->tm_wday
				* pTm->tm_year
				* pTm->tm_mon
				* pTm->tm_mday
				* pTm->tm_hour * pTm->tm_hour
				* pTm->tm_min * pTm->tm_min
				* pTm->tm_sec * pTm->tm_sec * pTm->tm_sec * pTm->tm_sec;
		}

		for (int i = 0; i < TOTAL_STREAMED_SOUNDS; i++) {
			manager->m_aTracks[i].m_nLength = SampleManager_GetStreamedFileLength(&SampleManager, i);

			if (i < STREAMED_SOUND_CITY_AMBIENT && isNewGameTimer)
				manager->m_aTracks[i].m_nPosition = NewGameRadioTimers[i];
			else if (i < STREAMED_SOUND_ANNOUNCE_BRIDGE_CLOSED)
				manager->m_aTracks[i].m_nPosition = (pos * AudioManager.m_anRandomTable[i % 5]) % manager->m_aTracks[i].m_nLength;
			else
				manager->m_aTracks[i].m_nPosition = 0;
			
			manager->m_aTracks[i].m_nLastPosCheckTimer = MusicInitHost_PauseTime();
		}
	}
}

void
DMAudio_SetStartingTrackPositions(uint8_t isStartGame)
{
	// Preserve the original game audio operation through the C interface
	MusicInit_SetStartingTrackPositions(&MusicManager, isStartGame);
}

//- rouz edit (ChatGPT)
