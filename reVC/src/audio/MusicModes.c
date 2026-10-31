//+ rouz edit (ChatGPT)
#include "MusicManager.h"
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"

#define MUSIC_MODES_MIN(a,b) ((a) < (b) ? (a) : (b))

#if defined(_MSC_VER) && !defined(__clang__)
#define MUSIC_MODES_STORE_TRACK_VALUE(manager, field, expression) do { \
    /* Capture the provider result before selecting the live destination track */ \
    uint32_t result = (expression); \
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = result; \
} while (0)
#else
#define MUSIC_MODES_STORE_TRACK_VALUE(manager, field, expression) \
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = (expression)
#endif

void
MusicModes_ChangeMusicMode(cMusicManager *manager, uint8_t mode)
{
	// Preserve original mode gates, provider callbacks and partial state writes
	if (!MusicState_IsInitialised(manager)) return;

	switch (mode)
	{
	case MUSICMODE_FRONTEND:
		manager->m_nUpcomingMusicMode = MUSICMODE_FRONTEND;

#ifdef PAUSE_RADIO_IN_FRONTEND
		// rewind those streams we weren't listening right now
		for( uint32_t i = STREAMED_SOUND_RADIO_WILD; i < STREAMED_SOUND_CUTSCENE_ASS_1; i++ ) {
			manager->m_aTracks[i].m_nPosition = MusicPositions_GetTrackStartPos(manager, i);
			manager->m_aTracks[i].m_nLastPosCheckTimer = MusicInitHost_PauseTime();
		}
#endif

		break;
	case MUSICMODE_GAME: manager->m_nUpcomingMusicMode = MUSICMODE_GAME; break;
	case MUSICMODE_CUTSCENE:
		manager->m_nUpcomingMusicMode = MUSICMODE_CUTSCENE;
		if (SampleManager_IsStreamPlaying(&SampleManager, 0)) {
			if (manager->m_nPlayingTrack != NO_TRACK) {
				MusicPositions_RecordRadioStats(manager);
				MUSIC_MODES_STORE_TRACK_VALUE(manager, m_nPosition, SampleManager_GetStreamedFilePosition(&SampleManager, 0));
				MUSIC_MODES_STORE_TRACK_VALUE(manager, m_nLastPosCheckTimer, MusicInitHost_PauseTime());
			}
		}
		SampleManager_StopStreamedFile(&SampleManager, 0);
		while (SampleManager_IsStreamPlaying(&SampleManager, 0))
			SampleManager_StopStreamedFile(&SampleManager, 0);
		manager->m_nMusicMode = manager->m_nUpcomingMusicMode;
		manager->m_bMusicModeChangeStarted = 0;
		manager->m_bTrackChangeStarted = 0;
		manager->m_nNextTrack = NO_TRACK;
		manager->m_nNextLoopFlag = 0;
		manager->m_bVerifyNextTrackStartedToPlay = 0;
		manager->m_nPlayingTrack = NO_TRACK;
		manager->m_nFrontendTrack = NO_TRACK;
		manager->m_bAnnouncementInProgress = 0;
		manager->m_nAnnouncement = NO_TRACK;
		g_bAnnouncementReadPosAlready = 0;
		break;
	case MUSICMODE_DISABLE: manager->m_nUpcomingMusicMode = MUSICMODE_DISABLED; break;
	default: return;
	}
}

void
MusicModes_ServiceFrontEndMode(cMusicManager *manager)
{
	// Preserve original mode gates, provider callbacks and partial state writes
	static uint8_t bRadioStatsRecorded = 0;

#ifdef PAUSE_RADIO_IN_FRONTEND
	// pause radio
	for (uint32_t i = STREAMED_SOUND_RADIO_WILD; i < STREAMED_SOUND_CUTSCENE_ASS_1; i++)
		manager->m_aTracks[i].m_nLastPosCheckTimer = MusicInitHost_PauseTime();
#endif

	if (manager->m_bAnnouncementInProgress) {
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

	if (AudioManager.m_FrameCounter % 4 != 0) return;

	if (!manager->m_bTrackChangeStarted && !manager->m_bVerifyNextTrackStartedToPlay) {
		manager->m_nNextTrack = manager->m_nFrontendTrack;
		manager->m_nNextLoopFlag = manager->m_FrontendLoopFlag;
	}

	if (manager->m_nNextTrack == manager->m_nPlayingTrack) {
		if (SampleManager_IsStreamPlaying(&SampleManager, 0)) {
			if (manager->m_nVolumeLatency > 0) manager->m_nVolumeLatency--;
			else {
				if (manager->m_nCurrentVolume < manager->m_nMaxVolume)
					manager->m_nCurrentVolume = MUSIC_MODES_MIN(manager->m_nMaxVolume, manager->m_nCurrentVolume + 6);
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, manager->m_nCurrentVolume, 63, 0, 0);
			}
		} else {
			if (manager->m_nPlayingTrack == STREAMED_SOUND_RADIO_MP3_PLAYER)
				SampleManager_StartStreamedFile(&SampleManager, STREAMED_SOUND_RADIO_MP3_PLAYER, 0, 0);
			else if (manager->m_nPlayingTrack == STREAMED_SOUND_MISSION_COMPLETED && !AudioManager.m_bIsPaused)
				MusicModes_ChangeMusicMode(manager, MUSICMODE_GAME);
		}
	} else {
		manager->m_bTrackChangeStarted = 1;
		if (manager->m_bVerifyNextTrackStartedToPlay || !SampleManager_IsStreamPlaying(&SampleManager, 0)) {
			bRadioStatsRecorded = 0;
			if (SampleManager_IsStreamPlaying(&SampleManager, 0) || manager->m_nNextTrack == NO_TRACK) {
				manager->m_nPlayingTrack = manager->m_nNextTrack;
				manager->m_bVerifyNextTrackStartedToPlay = 0;
				manager->m_bTrackChangeStarted = 0;
			} else {
				uint32_t trackStartPos = (manager->m_nNextTrack > STREAMED_SOUND_RADIO_POLICE) ? 0 : MusicPositions_GetTrackStartPos(manager, manager->m_nNextTrack);
				if (manager->m_nNextTrack != NO_TRACK) {
					SampleManager_SetStreamedFileLoopFlag(&SampleManager, manager->m_nNextLoopFlag, 0);
					SampleManager_StartStreamedFile(&SampleManager, manager->m_nNextTrack, trackStartPos, 0);
					manager->m_nVolumeLatency = 3;
					manager->m_nCurrentVolume = 0;
					manager->m_nMaxVolume = 100;
					SampleManager_SetStreamedVolumeAndPan(&SampleManager, manager->m_nCurrentVolume, 63, 0, 0);
					if (manager->m_nNextTrack < STREAMED_SOUND_CITY_AMBIENT)
						manager->m_nLastTrackServiceTime = MusicInitHost_PauseTime();
					manager->m_bVerifyNextTrackStartedToPlay = 1;
				}
			}
		} else {
			if (manager->m_nPlayingTrack != NO_TRACK && !bRadioStatsRecorded) {
				MUSIC_MODES_STORE_TRACK_VALUE(manager, m_nPosition, SampleManager_GetStreamedFilePosition(&SampleManager, 0));
				MUSIC_MODES_STORE_TRACK_VALUE(manager, m_nLastPosCheckTimer, MusicInitHost_PauseTime());
				MusicPositions_RecordRadioStats(manager);
				bRadioStatsRecorded = 1;
			}
			SampleManager_SetStreamedVolumeAndPan(&SampleManager, 0, 63, 0, 0);
			SampleManager_StopStreamedFile(&SampleManager, 0);
		}
	}
}

void
DMAudio_ChangeMusicMode(uint8_t mode)
{
	// Preserve the original game audio operation through the C interface
	MusicModes_ChangeMusicMode(&MusicManager, mode);
}
//- rouz edit (ChatGPT)
