//+ rouz edit (ChatGPT)
#include "Timer.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "sampman.h"
#include "AudioSampleConfig.h"
#include "soundlist.h"
#include <math.h>

void re3_debug(const char *format, ...);
#define MUSIC_STREAM_MIN(a,b) ((a) < (b) ? (a) : (b))
#define MUSIC_STREAM_SQUARE(x) ((x) * (x))

#if defined(_MSC_VER) && !defined(__clang__)
#define MUSIC_STREAM_STORE_TRACK_VALUE(manager, field, expression) do { \
    /* Capture the provider result before selecting the live destination track */ \
    uint32_t result = (expression); \
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = result; \
} while (0)
#else
#define MUSIC_STREAM_STORE_TRACK_VALUE(manager, field, expression) \
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = (expression)
#endif

static float
MusicStream_TargetDistanceSquared(void)
{
    // Preserve the original unweighted sum of coordinate squares
    AudioSoundPosition target, camera;
    float x, y, z;
    MusicStreamHost_TargetPosition(MusicStreamHost_Target(), &target);
    AudioGeometryHost_CameraPosition(&camera);
    x = target.x - camera.x;
    y = target.y - camera.y;
    z = target.z - camera.z;
    return x*x + y*y + z*z;
}

static void
MusicStream_TranslateTarget(AudioSoundPosition *translated)
{
    // Capture the live target coordinates before applying the existing C camera transform
    AudioSoundPosition target;
    MusicStreamHost_TargetPosition(MusicStreamHost_Target(), &target);
    AudioGeometry_Translate(&target, translated);
}

uint8_t
MusicStream_ServiceAnnouncement(cMusicManager *manager)
{
	// Preserve original streaming callbacks, shared state and live owner writes
	if (manager->m_bAnnouncementInProgress) {
		if (SampleManager_IsStreamPlaying(&SampleManager, 0))
			manager->m_nPlayingTrack = manager->m_nNextTrack;
		else if (manager->m_nPlayingTrack != NO_TRACK) {
			manager->m_nAnnouncement = NO_TRACK;
			manager->m_bAnnouncementInProgress = 0;
			manager->m_nPlayingTrack = NO_TRACK;
		}
		return 1;
	} else if (SampleManager_IsStreamPlaying(&SampleManager, 0)) {
		if (manager->m_nPlayingTrack != NO_TRACK && !g_bAnnouncementReadPosAlready) {
			MusicPositions_RecordRadioStats(manager);
			MUSIC_STREAM_STORE_TRACK_VALUE(manager, m_nPosition, SampleManager_GetStreamedFilePosition(&SampleManager, 0));
			g_bAnnouncementReadPosAlready = 1;
			MUSIC_STREAM_STORE_TRACK_VALUE(manager, m_nLastPosCheckTimer, Timer_GetTimeInMillisecondsPauseMode());
		}
		SampleManager_StopStreamedFile(&SampleManager, 0);
	} else {
		g_bAnnouncementReadPosAlready = 0;
		manager->m_nPlayingTrack = NO_TRACK;
		manager->m_nNextTrack = manager->m_nAnnouncement;
		SampleManager_SetStreamedFileLoopFlag(&SampleManager, 0, 0);
		SampleManager_StartStreamedFile(&SampleManager, manager->m_nNextTrack, 0, 0);
		SampleManager_SetStreamedVolumeAndPan(&SampleManager, MAX_VOLUME, 63, 0, 0);
		manager->m_bAnnouncementInProgress = 1;
	}

	return 1;
}

void
MusicStream_ServiceTrack(cMusicManager *manager, CVehicle *veh, CPed *ped)
{
	// Preserve original streaming callbacks, shared state and live owner writes
	static uint8_t bRadioStatsRecorded = 0;
	static uint8_t bRadioStatsRecorded2 = 0;
	uint8_t volume;
	if (!manager->m_bTrackChangeStarted)
		manager->m_nNextTrack = manager->m_nFrontendTrack;
	if (gRetuneCounter != 0 || manager->m_bSetNextStation) {
		if (SampleManager_IsStreamPlaying(&SampleManager, 0)) {
			if (manager->m_nPlayingTrack != NO_TRACK && !bRadioStatsRecorded) {
				MUSIC_STREAM_STORE_TRACK_VALUE(manager, m_nPosition, SampleManager_GetStreamedFilePosition(&SampleManager, 0));
				MUSIC_STREAM_STORE_TRACK_VALUE(manager, m_nLastPosCheckTimer, Timer_GetTimeInMillisecondsPauseMode());
				MusicPositions_RecordRadioStats(manager);
				bRadioStatsRecorded = 1;
			}
			SampleManager_SetStreamedVolumeAndPan(&SampleManager, 0, 63, 0, 0);
			SampleManager_StopStreamedFile(&SampleManager, 0);
		}
		return;
	}

	if (bRadioStatsRecorded) {
		bRadioStatsRecorded = 0;
		manager->m_nPlayingTrack = NO_TRACK;
	}

	if (manager->m_nNextTrack != manager->m_nPlayingTrack)
	{
		manager->m_bTrackChangeStarted = 1;
		SampleManager_SetStreamedVolumeAndPan(&SampleManager, 0, 63, 0, 0);
		if (!(AudioManager.m_FrameCounter & 1)) {
			if (manager->m_bVerifyNextTrackStartedToPlay || !SampleManager_IsStreamPlaying(&SampleManager, 0)) {
				bRadioStatsRecorded2 = 0;
				if (SampleManager_IsStreamPlaying(&SampleManager, 0)) {
					manager->m_nPlayingTrack = manager->m_nNextTrack;
					manager->m_bVerifyNextTrackStartedToPlay = 0;
					manager->m_bTrackChangeStarted = 0;
					if (veh) {
#ifdef FIX_BUGS
						if (manager->m_nPlayingTrack >= STREAMED_SOUND_CITY_AMBIENT && manager->m_nPlayingTrack <= STREAMED_SOUND_AMBSIL_AMBIENT)
							(*MusicRadioHost_Station(veh)) = RADIO_OFF;
						else if (manager->m_nPlayingTrack < STREAMED_SOUND_CITY_AMBIENT)
							(*MusicRadioHost_Station(veh)) = manager->m_nPlayingTrack;
#else
						if ((*MusicRadioHost_Station(veh)) >= STREAMED_SOUND_CITY_AMBIENT && (*MusicRadioHost_Station(veh)) <= STREAMED_SOUND_AMBSIL_AMBIENT)
							(*MusicRadioHost_Station(veh)) = RADIO_OFF;
						else
							(*MusicRadioHost_Station(veh)) = manager->m_nPlayingTrack;
#endif
					}
				} else {
					uint32_t pos = MusicPositions_GetTrackStartPos(manager, manager->m_nNextTrack);
					if (manager->m_nNextTrack != NO_TRACK) {
						SampleManager_SetStreamedFileLoopFlag(&SampleManager, 1, 0);
						SampleManager_StartStreamedFile(&SampleManager, manager->m_nNextTrack, pos, 0);
						if (manager->m_nFrontendTrack < STREAMED_SOUND_CITY_AMBIENT || manager->m_nFrontendTrack > STREAMED_SOUND_AMBSIL_AMBIENT)
						{
							manager->m_nVolumeLatency = 10;
							manager->m_nCurrentVolume = 0;
							manager->m_nMaxVolume = 100;
							SampleManager_SetStreamedVolumeAndPan(&SampleManager, manager->m_nCurrentVolume, 63, 0, 0);
						}
						else
						{
							MusicAmbience_ComputeAmbienceVol(manager, 1, &volume);
							SampleManager_SetStreamedVolumeAndPan(&SampleManager, volume, 63, 1, 0);
						}
						if (manager->m_nNextTrack < STREAMED_SOUND_CITY_AMBIENT)
							manager->m_nLastTrackServiceTime = Timer_GetTimeInMillisecondsPauseMode();
						manager->m_bVerifyNextTrackStartedToPlay = 1;
					}
				}
			} else {
				if (manager->m_nPlayingTrack == NO_TRACK)
					re3_debug("[DBG]: m_nPlayingTrack == NO_TRACK, yet track playing - tidying up\n");
				else if (!bRadioStatsRecorded2)
				{
					MUSIC_STREAM_STORE_TRACK_VALUE(manager, m_nPosition, SampleManager_GetStreamedFilePosition(&SampleManager, 0));
					MUSIC_STREAM_STORE_TRACK_VALUE(manager, m_nLastPosCheckTimer, Timer_GetTimeInMillisecondsPauseMode());
					bRadioStatsRecorded2 = 1;
					MusicPositions_RecordRadioStats(manager);
					if (manager->m_nPlayingTrack >= STREAMED_SOUND_HAVANA_CITY_AMBIENT && manager->m_nPlayingTrack <= STREAMED_SOUND_HAVANA_BEACH_AMBIENT)
					{
						if (manager->m_nNextTrack >= STREAMED_SOUND_HAVANA_CITY_AMBIENT && manager->m_nNextTrack <= STREAMED_SOUND_HAVANA_BEACH_AMBIENT)
							AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, AudioManager.m_nFrontEndEntity, SOUND_FRONTEND_HURRICANE, 0.0);
					}
				}
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, 0, 63, 0, 0);
				SampleManager_StopStreamedFile(&SampleManager, 0);
			}
		}
		return;
	}

	if (manager->m_nPlayingTrack >= STREAMED_SOUND_CITY_AMBIENT && manager->m_nPlayingTrack <= STREAMED_SOUND_AMBSIL_AMBIENT)
	{
		MusicAmbience_ComputeAmbienceVol(manager, 0, &volume);
		SampleManager_SetStreamedVolumeAndPan(&SampleManager, volume, 63, 1, 0);
		return;
	}
	if (Timer_GetIsSlowMotionActive())
	{
		if (MusicStreamHost_Target())
		{
			float DistToTargetSq = MusicStream_TargetDistanceSquared();
			if (DistToTargetSq >= MUSIC_STREAM_SQUARE(55.0f))
			{
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, 0, 63, 0, 0);
			}
			else if (DistToTargetSq >= MUSIC_STREAM_SQUARE(10.0f))
			{
				volume = (45.0f - (sqrtf(DistToTargetSq) - 10.0f)) / 45.0f * manager->m_nCurrentVolume;
				if (AudioMission_ShouldDuck(&AudioManager, 0) || AudioMission_ShouldDuck(&AudioManager, 1))
					volume /= 4;

				uint8_t pan = 0;
				if (volume > 0)
				{
					AudioSoundPosition panVec;
					MusicStream_TranslateTarget(&panVec);
					pan = AudioMath_ComputePan(55.0f, panVec.x);
				}
				if (gRetuneCounter != 0)
					volume = 0;
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, volume, pan, 0, 0);
			}
			else if (AudioMission_ShouldDuck(&AudioManager, 0) || AudioMission_ShouldDuck(&AudioManager, 1))
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, manager->m_nCurrentVolume, 63, 0, 0);
			else if (gRetuneCounter != 0)
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, 0, 63, 0, 0);
			else
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, manager->m_nCurrentVolume, 63, 0, 0);
		}
	} else if (AudioMission_ShouldDuck(&AudioManager, 0) || AudioMission_ShouldDuck(&AudioManager, 1)) {
		SampleManager_SetStreamedVolumeAndPan(&SampleManager, MUSIC_STREAM_MIN(manager->m_nCurrentVolume, 25), 63, 0, 0);
		manager->nFramesSinceCutsceneEnded = 0;
	} else {
		if (manager->nFramesSinceCutsceneEnded == -1)
			volume = manager->m_nCurrentVolume;
		else if (manager->nFramesSinceCutsceneEnded < 20)
		{
			volume = MUSIC_STREAM_MIN(manager->m_nCurrentVolume, 25);
			manager->nFramesSinceCutsceneEnded++;
		}
		else if (manager->nFramesSinceCutsceneEnded < 40)
		{
			volume = MUSIC_STREAM_MIN(manager->m_nCurrentVolume, 3 * (manager->nFramesSinceCutsceneEnded - 20) + 25);
			manager->nFramesSinceCutsceneEnded++;
		}
		else
		{
			volume = manager->m_nCurrentVolume;
			manager->nFramesSinceCutsceneEnded = -1;
		}
		if (gRetuneCounter != 0)
			volume = 0;
		SampleManager_SetStreamedVolumeAndPan(&SampleManager, volume, 63, 0, 0);
	}
	if (manager->m_nVolumeLatency > 0)
		manager->m_nVolumeLatency--;
	else if (manager->m_nCurrentVolume < manager->m_nMaxVolume)
		manager->m_nCurrentVolume = MUSIC_STREAM_MIN(manager->m_nMaxVolume, manager->m_nCurrentVolume + 6);
}

//- rouz edit (ChatGPT)
