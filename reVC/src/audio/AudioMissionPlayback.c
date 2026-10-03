//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "sampman.h"
#include <math.h>
#define SQR(x) ((x) * (x))
enum { MISSION_AUDIO_MAX_DIST = 80, MISSION_AUDIO_VOLUME = 80 };

void
AudioMission_ProcessSlot(cAudioManager *manager, uint8_t slot)
{
	// Preserve playback transitions, persistent counters, and backend call ordering
	float dist;
	uint8_t Vol;
	uint8_t pan;
	float distSquared;
	AudioSoundPosition vec;

	static uint8_t nCheckPlayingDelay[MISSION_AUDIO_SLOTS] = { 0, 0 };
	static uint8_t nFramesUntilFailedLoad[MISSION_AUDIO_SLOTS] = { 0, 0 };
	static uint8_t nFramesForPretendPlaying[MISSION_AUDIO_SLOTS] = { 0, 0 };

	if (manager->m_nMissionAudioSampleIndex[slot] != NO_SAMPLE) {
		switch (manager->m_nMissionAudioLoadingStatus[slot]) {
		case LOADING_STATUS_NOT_LOADED:
			SampleManager_PreloadStreamedFile(&SampleManager, manager->m_nMissionAudioSampleIndex[slot], slot + 1);
			manager->m_nMissionAudioLoadingStatus[slot] = LOADING_STATUS_LOADED;
			nFramesUntilFailedLoad[slot] = 0;
			break;
		case LOADING_STATUS_LOADING:
			if (++nFramesUntilFailedLoad[slot] >= 120) {
				nFramesForPretendPlaying[slot] = 0;
				g_bMissionAudioLoadFailed[slot] = 1;
				nFramesUntilFailedLoad[slot] = 0;
				manager->m_nMissionAudioLoadingStatus[slot] = LOADING_STATUS_LOADED;
			}
			return;
		default:
			return;
		case LOADING_STATUS_LOADED:
			if (!manager->m_bIsMissionAudioAllowedToPlay[slot])
				break;
			if (g_bMissionAudioLoadFailed[slot]) {
				if (manager->m_bTimerJustReset) {
					AudioMission_Clear(manager, slot);
					SampleManager_StopStreamedFile(&SampleManager, slot + 1);
					nFramesForPretendPlaying[slot] = 0;
					nCheckPlayingDelay[slot] = 0;
					nFramesUntilFailedLoad[slot] = 0;
				} else if (!manager->m_bIsPaused) {
					if (++nFramesForPretendPlaying[slot] >= 90) {
						manager->m_nMissionAudioPlayStatus[slot] = PLAY_STATUS_FINISHED;
						manager->m_nMissionAudioSampleIndex[slot] = NO_SAMPLE;
					} else
						manager->m_nMissionAudioPlayStatus[slot] = PLAY_STATUS_PLAYING;
				}
				break;
			}
			switch (manager->m_nMissionAudioPlayStatus[slot]) {
			case PLAY_STATUS_STOPPED:
				if (AudioMission_UsesPoliceChannel(manager->m_nMissionAudioSampleIndex[slot]))
					AudioPolice_SetMission(manager->m_bIsInitialised, manager->m_nMissionAudioSampleIndex[slot]);
				else {
					if (manager->m_bIsPaused)
						SampleManager_PauseStream(&SampleManager, 1, slot + 1);
					if (manager->m_bIsMissionAudio2D[slot]) {
						if (manager->m_nMissionAudioSampleIndex[slot] == STREAMED_SOUND_MISSION_CAMERAL)
							SampleManager_SetStreamedVolumeAndPan(&SampleManager, MISSION_AUDIO_VOLUME, 0, 1, slot + 1);
						else if (manager->m_nMissionAudioSampleIndex[slot] == STREAMED_SOUND_MISSION_CAMERAR)
							SampleManager_SetStreamedVolumeAndPan(&SampleManager, MISSION_AUDIO_VOLUME, 127, 1, slot + 1);
						else
							SampleManager_SetStreamedVolumeAndPan(&SampleManager, MISSION_AUDIO_VOLUME, 63, 1, slot + 1);
					} else {
						distSquared = AudioGeometry_DistanceSquared(&manager->m_vecMissionAudioPosition[slot]);
						if (distSquared < SQR(MISSION_AUDIO_MAX_DIST)) {
							if (distSquared > 0.0f) {
								dist = sqrtf(distSquared);
								Vol = AudioMath_ComputeVolume(MISSION_AUDIO_VOLUME, MISSION_AUDIO_MAX_DIST, dist);
							} else
								Vol = MISSION_AUDIO_VOLUME;
							// Translate a copied game vector through the existing camera matrix
							const AudioSoundPosition missionPosition = manager->m_vecMissionAudioPosition[slot];
							AudioGeometry_Translate(&missionPosition, &vec);
							pan = AudioMath_ComputePan(MISSION_AUDIO_MAX_DIST, vec.x);
						} else {
							Vol = 0;
							pan = 63;
						} 
						SampleManager_SetStreamedVolumeAndPan(&SampleManager, Vol, pan, 1, slot + 1);
					}
					SampleManager_StartPreloadedStreamedFile(&SampleManager, slot + 1);
				}
				manager->m_nMissionAudioPlayStatus[slot] = PLAY_STATUS_PLAYING;
				nCheckPlayingDelay[slot] = 30;
				if (manager->m_nMissionAudioSampleIndex[slot] >= STREAMED_SOUND_MISSION_MOB_01A && manager->m_nMissionAudioSampleIndex[slot] <= STREAMED_SOUND_MISSION_MOB_99A)
					manager->m_bIsMissionAudioPhoneCall[slot] = 1;
				break;
			case PLAY_STATUS_PLAYING:
				if (manager->m_bTimerJustReset) {
					AudioMission_Clear(manager, slot);
					SampleManager_StopStreamedFile(&SampleManager, slot + 1);
					return;
				}
				if (AudioMission_UsesPoliceChannel(manager->m_nMissionAudioSampleIndex[slot])) {
					if (!manager->m_bIsPaused) {
						if (nCheckPlayingDelay[slot] > 0) {
							nCheckPlayingDelay[slot]--;
						} else if ((g_bMissionAudioLoadFailed[slot] && manager->m_nMissionAudioFramesToPlay[slot]-- == 0) || AudioPolice_GetMissionStatus() == PLAY_STATUS_FINISHED) {
							manager->m_nMissionAudioPlayStatus[slot] = PLAY_STATUS_FINISHED;
							if (manager->m_nMissionAudioSampleIndex[slot] >= STREAMED_SOUND_MISSION_MOB_01A && manager->m_nMissionAudioSampleIndex[slot] <= STREAMED_SOUND_MISSION_MOB_99A)
								manager->m_bIsMissionAudioPhoneCall[slot] = 0;
							manager->m_nMissionAudioSampleIndex[slot] = NO_SAMPLE;
							SampleManager_StopStreamedFile(&SampleManager, slot + 1);
							manager->m_nMissionAudioFramesToPlay[slot] = 0;
						}
					}
				} else if (manager->m_bIsMissionAudioPlaying[slot]) {
					if (!SampleManager_IsStreamPlaying(&SampleManager, slot + 1) && !manager->m_bIsPaused && !manager->m_bWasPaused) {
						if (manager->m_nMissionAudioSampleIndex[slot] == STREAMED_SOUND_MISSION_ROK2_01)
							manager->m_nMissionAudioPlayStatus[slot] = PLAY_STATUS_STOPPED;
						else {
							manager->m_nMissionAudioPlayStatus[slot] = PLAY_STATUS_FINISHED;
							if (manager->m_nMissionAudioSampleIndex[slot] >= STREAMED_SOUND_MISSION_MOB_01A && manager->m_nMissionAudioSampleIndex[slot] <= STREAMED_SOUND_MISSION_MOB_99A)
								manager->m_bIsMissionAudioPhoneCall[slot] = 0;
							manager->m_nMissionAudioSampleIndex[slot] = NO_SAMPLE;
							SampleManager_StopStreamedFile(&SampleManager, slot + 1);
							manager->m_nMissionAudioFramesToPlay[slot] = 0;
						}
					} else {
						if (manager->m_bIsPaused)
							SampleManager_PauseStream(&SampleManager, 1, slot + 1);
						else {
							SampleManager_PauseStream(&SampleManager, 0, slot + 1);
							if (!manager->m_bIsMissionAudio2D[slot]) {
								distSquared = AudioGeometry_DistanceSquared(&manager->m_vecMissionAudioPosition[slot]);
								if (distSquared < SQR(MISSION_AUDIO_MAX_DIST)) {
									// BUG? Why MAX_VOLUME instead of MISSION_AUDIO_VOLUME?
									if (distSquared > 0.0f) {
										dist = sqrtf(distSquared);
										Vol = AudioMath_ComputeVolume(MAX_VOLUME, MISSION_AUDIO_MAX_DIST, dist);
									} else
										Vol = MAX_VOLUME;
									// Translate a copied game vector through the existing camera matrix
									const AudioSoundPosition missionPosition = manager->m_vecMissionAudioPosition[slot];
									AudioGeometry_Translate(&missionPosition, &vec);
									pan = AudioMath_ComputePan(MISSION_AUDIO_MAX_DIST, vec.x);
								} else {
									Vol = 0;
									pan = 63;
								} 
								SampleManager_SetStreamedVolumeAndPan(&SampleManager, Vol, pan, 1, slot + 1);
							}
						}
					} 
				} else {
					if (manager->m_bIsPaused)
						break;
					if (nCheckPlayingDelay[slot]-- > 0) {
						if (!SampleManager_IsStreamPlaying(&SampleManager, slot + 1))
							break;
						nCheckPlayingDelay[slot] = 0;
					}
					manager->m_bIsMissionAudioPlaying[slot] = 1;
				}
				break;
			default:
				break;
			}
			break;
		}
	}
}

void
AudioMission_Process(cAudioManager *manager)
{
	// Preserve playback transitions, persistent counters, and backend call ordering
	if (!manager->m_bIsInitialised) return;
	
	for (int i = 0; i < MISSION_AUDIO_SLOTS; i++)
		AudioMission_ProcessSlot(manager, i);

	if (manager->m_bIsMissionAudioPhoneCall[0] || manager->m_bIsMissionAudioPhoneCall[1])
		manager->m_nGlobalSfxVolumeMultiplier = 64;
	else if (manager->m_nGlobalSfxVolumeMultiplier < 127) {
		manager->m_nGlobalSfxVolumeMultiplier += 5;
		if (manager->m_nGlobalSfxVolumeMultiplier > 127)
			manager->m_nGlobalSfxVolumeMultiplier = 127;
	}
}

//- rouz edit (ChatGPT)
