//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "sampman.h"

void
AudioPolice_Crackle(cAudioManager *manager)
{
	// Preserve the original sound fields, shared history, and backend call order
	manager->m_sQueueSample.m_nEntityIndex = manager->m_nPoliceChannelEntity;
	manager->m_sQueueSample.m_nCounter = 0;
	manager->m_sQueueSample.m_nSampleIndex = SFX_POLICE_RADIO_CRACKLE;
	manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
	manager->m_sQueueSample.m_bIs2D = 1;
	manager->m_sQueueSample.m_nPriority = 10;
	manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_POLICE_RADIO_CRACKLE);
	manager->m_sQueueSample.m_nVolume = manager->m_anRandomTable[2] % 20 + 15;
	manager->m_sQueueSample.m_nLoopCount = 0;
	// Retain the original external-volume field update
#ifdef EXTERNAL_3D_SOUND
	manager->m_sQueueSample.m_nEmittingVolume = manager->m_sQueueSample.m_nVolume;
#endif
	// Read manual loop endpoints only for the original non-PS2 sound layout
#ifndef GTA_PS2
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, SFX_POLICE_RADIO_CRACKLE);
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, SFX_POLICE_RADIO_CRACKLE);
#endif
	manager->m_sQueueSample.m_bStatic = 0;
	// Clear the sound reverb flag only in the original enabled configuration
#ifdef AUDIO_REVERB
	manager->m_sQueueSample.m_bReverb = 0;
#endif
	manager->m_sQueueSample.m_nPan = 63;
	manager->m_sQueueSample.m_nFramesToPlay = 3;
	// Clear the sound reflection flag only when its field exists
#ifdef AUDIO_REFLECTIONS
	manager->m_sQueueSample.m_bReflections = 0;
#endif
	AudioRequests_Submit(manager);
}

void
AudioPolice_ServiceChannel(cAudioManager *manager, uint8_t wantedLevel)
{
	// Preserve the original sound fields, shared history, and backend call order
	uint8_t processed = 0;
	uint32_t sample;
	uint32_t freq;

	static int cWait = 0;
	static uint8_t bChannelOpen = 0;
	static uint8_t bMissionAudioPhysicalPlayingStatus = PLAY_STATUS_STOPPED;
	static uint32_t PoliceChannelFreq = 22050;

	if (!manager->m_bIsInitialised) return;

	if (manager->m_bIsPaused) {
		if (SampleManager_GetChannelUsedFlag(&SampleManager, CHANNEL_POLICE_RADIO)) SampleManager_StopChannel(&SampleManager, CHANNEL_POLICE_RADIO);
		if (g_nMissionAudioSfx != NO_SAMPLE && bMissionAudioPhysicalPlayingStatus == PLAY_STATUS_PLAYING &&
			SampleManager_IsStreamPlaying(&SampleManager, 1)) {
			SampleManager_PauseStream(&SampleManager, 1, 1);
		}
	} else {
		if (manager->m_bWasPaused && g_nMissionAudioSfx != NO_SAMPLE &&
			bMissionAudioPhysicalPlayingStatus == PLAY_STATUS_PLAYING) {
			SampleManager_PauseStream(&SampleManager, 0, 1);
		}
		if (manager->m_sPoliceRadioQueue.m_nSamplesInQueue == 0) bChannelOpen = 0;
		if (cWait) {
#ifdef FIX_BUGS
			cWait -= AudioPoliceHost_LogicalFramesPassed();
#else
			--cWait;
#endif
			return;
		}
		if (g_nMissionAudioSfx != NO_SAMPLE && !bChannelOpen) {
			if (g_nMissionAudioPlayingStatus != PLAY_STATUS_STOPPED) {
				if (g_nMissionAudioPlayingStatus == PLAY_STATUS_PLAYING && bMissionAudioPhysicalPlayingStatus == PLAY_STATUS_STOPPED &&
					SampleManager_IsStreamPlaying(&SampleManager, 1)) {
					bMissionAudioPhysicalPlayingStatus = PLAY_STATUS_PLAYING;
				}
				if (bMissionAudioPhysicalPlayingStatus == PLAY_STATUS_PLAYING) {
					if (SampleManager_IsStreamPlaying(&SampleManager, 1)) {
						AudioPolice_Crackle(manager);
					} else {
						bMissionAudioPhysicalPlayingStatus = PLAY_STATUS_FINISHED;
						g_nMissionAudioPlayingStatus = PLAY_STATUS_FINISHED;
						g_nMissionAudioSfx = NO_SAMPLE;
						cWait = 30;
					}
					return;
				}
			} else if (!SampleManager_GetChannelUsedFlag(&SampleManager, CHANNEL_POLICE_RADIO)) {
				SampleManager_PreloadStreamedFile(&SampleManager, g_nMissionAudioSfx, 1);
				SampleManager_SetStreamedVolumeAndPan(&SampleManager, MAX_VOLUME, 63, 1, 1);
				SampleManager_StartPreloadedStreamedFile(&SampleManager, 1);
				g_nMissionAudioPlayingStatus = PLAY_STATUS_PLAYING;
				bMissionAudioPhysicalPlayingStatus = PLAY_STATUS_STOPPED;
				return;
			}
		}
		if (bChannelOpen) AudioPolice_Crackle(manager);
		if ((g_nMissionAudioSfx == NO_SAMPLE || g_nMissionAudioPlayingStatus != PLAY_STATUS_PLAYING) &&
			!SampleManager_GetChannelUsedFlag(&SampleManager, CHANNEL_POLICE_RADIO) && manager->m_sPoliceRadioQueue.m_nSamplesInQueue != 0) {
			sample = PoliceRadioQueue_Remove(&manager->m_sPoliceRadioQueue);
			if (wantedLevel == 0) {
				if (gSpecialSuspectLastSeenReport) {
					gSpecialSuspectLastSeenReport = 0;
				} else if (sample == SFX_POLICE_RADIO_MESSAGE_NOISE_1) {
					bChannelOpen = 0;
					processed = 1;
				}
			}
			if (sample == NO_SAMPLE) {
				if (!processed) cWait = 30;
			} else {
				SampleManager_InitialiseChannel(&SampleManager, CHANNEL_POLICE_RADIO, sample, SFX_BANK_0);
				switch (sample) {
				case SFX_POLICE_RADIO_MESSAGE_NOISE_1:
					freq = manager->m_anRandomTable[4] % 2000 + 10025;
					bChannelOpen = bChannelOpen == 0;
					break;
				default: freq = SampleManager_GetSampleBaseFrequency(&SampleManager, sample); break;
				}
				PoliceChannelFreq = freq;
#ifdef USE_TIME_SCALE_FOR_AUDIO
				SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_POLICE_RADIO, freq * AudioPoliceHost_TimeScale());
#else
				SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_POLICE_RADIO, freq);
#endif
				SampleManager_SetChannelVolume(&SampleManager, CHANNEL_POLICE_RADIO, 100);
				SampleManager_SetChannelPan(&SampleManager, CHANNEL_POLICE_RADIO, 63);
				SampleManager_SetChannelLoopCount(&SampleManager, CHANNEL_POLICE_RADIO, 1);
#ifndef GTA_PS2
				SampleManager_SetChannelLoopPoints(&SampleManager, CHANNEL_POLICE_RADIO, 0, -1);
#endif
				SampleManager_StartChannel(&SampleManager, CHANNEL_POLICE_RADIO);
			}
			if (processed) AudioPolice_Reset(manager);
		}
	}
}

//- rouz edit (ChatGPT)
