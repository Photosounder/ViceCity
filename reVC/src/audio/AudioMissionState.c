//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"

uint8_t
AudioMission_GetLoadingStatus(cAudioManager *manager, uint8_t slot)
{
	// Preserve mission gates, state writes, and the original shared query history
	if (manager->m_bIsInitialised && slot < MISSION_AUDIO_SLOTS)
		return manager->m_nMissionAudioLoadingStatus[slot];

	return LOADING_STATUS_LOADED;
}

void
AudioMission_AllowPlay(cAudioManager *manager, uint8_t slot)
{
	// Preserve mission gates, state writes, and the original shared query history
	if (manager->m_bIsInitialised && slot < MISSION_AUDIO_SLOTS && manager->m_nMissionAudioSampleIndex[slot] != NO_SAMPLE && manager->m_nMissionAudioLoadingStatus[slot] == LOADING_STATUS_LOADED &&
	    manager->m_nMissionAudioPlayStatus[slot] == PLAY_STATUS_STOPPED)
		manager->m_bIsMissionAudioAllowedToPlay[slot] = 1;
}

uint8_t
AudioMission_ShouldDuck(cAudioManager *manager, uint8_t slot)
{
	// Preserve mission gates, state writes, and the original shared query history
	if (AudioMission_IsPlaying(manager, slot))
		return manager->m_nMissionAudioSampleIndex[slot] != STREAMED_SOUND_MISSION_ROK2_01;
	return 0;
}

uint8_t
AudioMission_IsPlaying(cAudioManager *manager, uint8_t slot)
{
	// Preserve mission gates, state writes, and the original shared query history
	if (manager->m_bIsInitialised) {
		if (slot < MISSION_AUDIO_SLOTS)
			return manager->m_nMissionAudioPlayStatus[slot] == PLAY_STATUS_PLAYING;
		else
			return 1;
	}

	static uint32_t cPretendFrame[MISSION_AUDIO_SLOTS] = { 1, 1 };

	return (cPretendFrame[slot]++ % 64) != 0;
}

uint8_t
AudioMission_IsFinished(cAudioManager *manager, uint8_t slot)
{
	// Preserve mission gates, state writes, and the original shared query history
	if (manager->m_bIsInitialised) {
		if (slot < MISSION_AUDIO_SLOTS)
			return manager->m_nMissionAudioPlayStatus[slot] == PLAY_STATUS_FINISHED;
		else
			return 1;
	}

	static uint32_t cPretendFrame[MISSION_AUDIO_SLOTS] = { 1, 1 };

	return (cPretendFrame[slot]++ % 64) == 0;
}

void
AudioMission_Clear(cAudioManager *manager, uint8_t slot)
{
	// Preserve mission gates, state writes, and the original shared query history
	if (manager->m_bIsInitialised && slot < MISSION_AUDIO_SLOTS) {
		manager->m_nMissionAudioSampleIndex[slot] = NO_SAMPLE;
		manager->m_nMissionAudioLoadingStatus[slot] = LOADING_STATUS_NOT_LOADED;
		manager->m_nMissionAudioPlayStatus[slot] = PLAY_STATUS_STOPPED;
		manager->m_bIsMissionAudioPlaying[slot] = 0;
		manager->m_bIsMissionAudioAllowedToPlay[slot] = 0;
		manager->m_bIsMissionAudio2D[slot] = 1;
		manager->m_nMissionAudioFramesToPlay[slot] = 0;
		manager->m_bIsMissionAudioPhoneCall[slot] = 0;
		SampleManager_StopStreamedFile(&SampleManager, slot + 1);
	}
}

uint8_t
AudioMission_UsesPoliceChannel(uint32_t soundMission)
{
	// Preserve mission gates, state writes, and the original shared query history
	return 0;
}

uint8_t
DMAudio_GetMissionAudioLoadingStatus(uint8_t slot)
{
	// Preserve the original game audio operation through the C interface
	return AudioMission_GetLoadingStatus(&AudioManager, slot);
}

void
DMAudio_SetMissionAudioLocation(uint8_t slot, float x, float y, float z)
{
	// Preserve the original game audio operation through the C interface
	AudioMissionPosition_Set(AudioManager.m_vecMissionAudioPosition, AudioManager.m_bIsMissionAudio2D, AudioManager.m_bIsInitialised, slot, x, y, z);
}

void
DMAudio_PlayLoadedMissionAudio(uint8_t slot)
{
	// Preserve the original game audio operation through the C interface
	AudioMission_AllowPlay(&AudioManager, slot);
}

uint8_t
DMAudio_IsMissionAudioSamplePlaying(uint8_t slot)
{
	// Preserve the original game audio operation through the C interface
	return AudioMission_IsPlaying(&AudioManager, slot);
}

uint8_t
DMAudio_IsMissionAudioSampleFinished(uint8_t slot)
{
	// Preserve the original game audio operation through the C interface
	return AudioMission_IsFinished(&AudioManager, slot);
}

void
DMAudio_ClearMissionAudio(uint8_t slot)
{
	// Preserve the original game audio operation through the C interface
	AudioMission_Clear(&AudioManager, slot);
}

//- rouz edit (ChatGPT)
