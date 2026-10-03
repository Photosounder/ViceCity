//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "../core/Random.h"

void
AudioManager_InitState(cAudioManager *manager)
{
	// Initialize embedded C state before the existing audio setup runs
	AudioScriptObjectManager_Reset(&manager->m_sAudioScriptObjectManager);
	PoliceRadioQueue_Reset(&manager->m_sPoliceRadioQueue);
	AudioCollisionManager_Init(&manager->m_sCollisionManager);
	AudioCrimes_Init(manager->m_aCrimes);
	PedComments_Init(&manager->m_sPedComments);

	manager->m_bIsInitialised = 0;
	manager->m_bIsSurround = 1;
	manager->m_nChannelOffset = 0;
	manager->m_fSpeedOfSound = 343.f / 40;
	manager->m_nTimeSpent = 40;
	manager->m_nActiveSamples = NUM_CHANNELS_GENERIC;
	manager->m_nActiveQueue = 1;
	AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
	manager->m_nActiveQueue = 0;
	AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
	AudioSoundQueue_ClearActive(manager->m_asActiveSamples, manager->m_nActiveSamples);
	AudioManager_GenerateRandomTable(manager->m_anRandomTable);
	manager->m_bDoubleVolume = 0;
#ifdef AUDIO_REFLECTIONS
	manager->m_bDynamicAcousticModelingStatus = 1;
#endif

	for (uint32_t i = 0; i < NUM_AUDIOENTITIES; i++) {
		manager->m_asAudioEntities[i].m_bIsUsed = 0;
		manager->m_aAudioEntityOrderList[i] = NUM_AUDIOENTITIES;
	}
	manager->m_nAudioEntitiesCount = 0;
	manager->m_FrameCounter = 0;
	manager->m_bReduceReleasingPriority = 0;
	manager->m_bTimerJustReset = 0;
	manager->m_nTimer = 0;
}

void
AudioManager_GenerateRandomTable(int32_t randomTable[5])
{
	// Fill the five existing slots with the original ordered random draws
	for (uint32_t i = 0; i < 5; i++)
		randomTable[i] = myrand();
}
//- rouz edit (ChatGPT)
