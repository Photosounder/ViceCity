//+ rouz edit (ChatGPT)
#include "MusicManager.h"

void
MusicState_InitState(cMusicManager *manager)
{
	// Preserve the former constructor field writes and retained storage
	manager->m_bIsInitialised = 0;
	manager->m_bDisabled = 0;
	manager->m_nFrontendTrack = NO_TRACK;
	manager->m_nPlayingTrack = NO_TRACK;
	manager->m_nUpcomingMusicMode = MUSICMODE_DISABLED;
	manager->m_nMusicMode = MUSICMODE_DISABLED;
	manager->m_bSetNextStation = 0;

	for (int i = 0; i < NUM_RADIOS; i++)
		manager->aListenTimeArray[i] = 0.0f;

	manager->m_nLastTrackServiceTime = 0.0f;
	manager->m_nVolumeLatency = 0;
	manager->m_nCurrentVolume = 0;
	manager->m_nMaxVolume = 0;
	manager->m_nAnnouncement = NO_TRACK;
	manager->m_bAnnouncementInProgress = 0;
}

uint8_t
MusicState_IsInitialised(const cMusicManager *manager)
{
	// Read the original live music field without normalizing its value
	return manager->m_bIsInitialised;
}

uint8_t
MusicState_GetMusicMode(const cMusicManager *manager)
{
	// Read the original live music field without normalizing its value
	return manager->m_nMusicMode;
}

uint32_t
MusicState_GetCurrentTrack(const cMusicManager *manager)
{
	// Read the original live music field without normalizing its value
	return manager->m_nPlayingTrack;
}

void
MusicState_ResetTimers(cMusicManager *manager, uint32_t time)
{
	// Preserve the original music state gates and complete operation

	manager->m_bResetTimers = 1;
	manager->m_nResetTime = time;
}

void
MusicState_SetRadioChannelByScript(cMusicManager *manager, uint32_t station, int32_t pos)
{
	// Preserve the original music state gates and complete operation

	if (manager->m_bIsInitialised) {
		if (station == STREAMED_SOUND_RADIO_MP3_PLAYER)
			station = STREAMED_SOUND_CITY_AMBIENT;
		if (station <= STREAMED_SOUND_RADIO_POLICE) {
			manager->m_bRadioSetByScript = 1;
			manager->m_nRadioStationScript = station;
			manager->m_nRadioPosition = pos == -1 ? -1 : pos % manager->m_aTracks[station].m_nLength;
		}
	}
}

void
MusicState_PlayFrontEndTrack(cMusicManager *manager, uint32_t track, uint8_t loopFlag)
{
	// Preserve the original music state gates and complete operation

	if (MusicState_IsInitialised(manager) && !manager->m_bDisabled && track < TOTAL_STREAMED_SOUNDS && (manager->m_nUpcomingMusicMode == MUSICMODE_FRONTEND || manager->m_nMusicMode == MUSICMODE_FRONTEND))
	{
		manager->m_nFrontendTrack = track;
		manager->m_FrontendLoopFlag = loopFlag;
		if (manager->m_nMusicMode != MUSICMODE_FRONTEND)
			manager->m_bEarlyFrontendTrack = 1;
	}
}

void
MusicState_StopFrontEndTrack(cMusicManager *manager)
{
	// Preserve the original music state gates and complete operation

	if (manager->m_nUpcomingMusicMode == MUSICMODE_FRONTEND || manager->m_nMusicMode == MUSICMODE_FRONTEND)
		manager->m_nFrontendTrack = NO_TRACK;
}

void
MusicState_PlayAnnouncement(cMusicManager *manager, uint32_t announcement)
{
	// Preserve the original music state gates and complete operation

	if (MusicState_IsInitialised(manager) && !manager->m_bDisabled && !manager->m_bAnnouncementInProgress)
		manager->m_nAnnouncement = announcement;
}

float *
MusicState_GetListenTimeArray(cMusicManager *manager)
{
	// Preserve the original music state gates and complete operation

	return manager->aListenTimeArray;
}

uint32_t
MusicState_GetFavouriteRadioStation(const cMusicManager *manager)
{
	// Preserve the original music state gates and complete operation

	uint32_t favstation = 0;

	for (int i = 1; i < NUM_RADIOS; i++) {
		if (manager->aListenTimeArray[i] > manager->aListenTimeArray[favstation])
			favstation = i;
	}

	return favstation;
}

uint8_t
MusicState_CheckForMusicInterruptions(const cMusicManager *manager)
{
	// Preserve the original music state gates and complete operation

	return (manager->m_nPlayingTrack == STREAMED_SOUND_MISSION_COMPLETED) || (manager->m_nPlayingTrack == STREAMED_SOUND_CUTSCENE_FINALE);
}

//- rouz edit (ChatGPT)
