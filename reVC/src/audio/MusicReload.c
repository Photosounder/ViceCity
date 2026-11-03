//+ rouz edit (ChatGPT)
#include "Timer.h"
#include "MusicManager.h"
#include "MusicSavedData.h"

void re3_debug(const char *format, ...);
#define MUSIC_RELOAD_DEBUG(format, ...) re3_debug("[DBG]: " format, __VA_ARGS__)

uint32_t RadioStaticTimer;

void
MusicReload_ResetMusicAfterReload(cMusicManager *manager)
{
	// Reset playback flags before capturing saved listening times and restoring valid positions
	float afRadioTime[NUM_RADIOS];

	manager->m_bRadioSetByScript = 0;
	manager->m_nRadioStationScript = WILDSTYLE;
	manager->m_nRadioPosition = -1;
	manager->m_nAnnouncement = NO_TRACK;
	manager->m_bAnnouncementInProgress = 0;
	manager->m_bSetNextStation = 0;
	RadioStaticTimer = 0;
	gNumRetunePresses = 0;
	gRetuneCounter = 0;
	manager->m_nFrontendTrack = NO_TRACK;
	manager->m_nPlayingTrack = NO_TRACK;
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
	manager->m_bEarlyFrontendTrack = 0;
	manager->m_nVolumeLatency = 0;
	manager->m_nCurrentVolume = 0;
	manager->m_nMaxVolume = 0;

	uint8_t bRadioWasEverListened = 0;

	for (int i = 0; i < NUM_RADIOS; i++) {
		afRadioTime[i] = MusicSavedData_GetFavoriteRadioStationList(i);
		if (!bRadioWasEverListened && afRadioTime[i] != 0.0f)
			bRadioWasEverListened = 1;
	}

	if (!bRadioWasEverListened) return;

	for (int i = 0; i < NUM_RADIOS; i++) {
		manager->aListenTimeArray[i] = afRadioTime[i];
		int32_t trackPos = GetSavedRadioStationPosition(i);
		if (trackPos != -1) {
			if (trackPos > manager->m_aTracks[i].m_nLength) {
				MUSIC_RELOAD_DEBUG("Radio Track %d saved position is %d, Length is only %d\n", i, trackPos, manager->m_aTracks[i].m_nLength);
				trackPos %= manager->m_aTracks[i].m_nLength;
			}
			manager->m_aTracks[i].m_nPosition = trackPos;
			manager->m_aTracks[i].m_nLastPosCheckTimer = Timer_GetTimeInMillisecondsPauseMode();
		}
	}
}
//- rouz edit (ChatGPT)
