//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"

void
AudioService_Run(cAudioManager *manager)
{
	// Preserve the original service gates, field accesses and callback order
	AudioManager_GenerateRandomTable(manager->m_anRandomTable);
	if (manager->m_bTimerJustReset) {
		AudioService_ResetLogicTimers(manager, manager->m_nTimer);
		AudioServiceHost_ResetMusicTimers(manager->m_nTimer);
		manager->m_bTimerJustReset = 0;
	}
	if (manager->m_bIsInitialised) {
		manager->m_bWasPaused = manager->m_bIsPaused;
		manager->m_bIsPaused = AudioServiceHost_UserPaused();
#ifdef AUDIO_REFLECTIONS
		AudioReflections_Update(manager);
#endif
		AudioEffects_Service(manager);
		AudioServiceHost_Music();
	}
}

void
AudioService_ResetLogicTimers(cAudioManager *manager, uint32_t timer)
{
	// Preserve the original service gates, field accesses and callback order
	for (uint32_t i = 0; i < manager->m_nAudioEntitiesCount; i++) {
		if (manager->m_asAudioEntities[manager->m_aAudioEntityOrderList[i]].m_nType == AUDIOTYPE_PHYSICAL) {
			CPed *ped = (CPed *)manager->m_asAudioEntities[manager->m_aAudioEntityOrderList[i]].m_pEntity;
			if (AudioServiceHost_IsPed(ped)) {
				AudioServiceHost_PedLastStart(ped, timer);
				AudioServiceHost_PedSoundStart(ped, timer + manager->m_anRandomTable[0] % 3000);
			}
		}
	}
	AudioMission_Clear(manager, 0);
	AudioMission_Clear(manager, 1);
	SampleManager_StopChannel(&SampleManager, CHANNEL_POLICE_RADIO);
}

void
DMAudio_Service(void)
{
    // Service the live global owner through the actual C operation
    AudioService_Run(&AudioManager);
}
//- rouz edit (ChatGPT)
