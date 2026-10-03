//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioScriptObject.h"
#include "../core/Pools.h"
#include "DMAudio.h"
#include "sampman.h"

void
AudioReload_ResetTimers(cAudioManager *manager, uint32_t time)
{
	// Preserve the original reload gates, partial writes and callback order
	if (manager->m_bIsInitialised) {
		manager->m_bTimerJustReset = 1;
		manager->m_nTimer = time;
		AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
		if (manager->m_nActiveQueue) {
			manager->m_nActiveQueue = 0;
			AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
			manager->m_nActiveQueue = 1;
		} else {
			manager->m_nActiveQueue = 1;
			AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
			manager->m_nActiveQueue = 0;
		}
		AudioSoundQueue_ClearActive(manager->m_asActiveSamples, manager->m_nActiveSamples);
		AudioMission_Clear(manager, 0);
		AudioMission_Clear(manager, 1);
		SampleManager_StopChannel(&SampleManager, CHANNEL_POLICE_RADIO);
		SampleManager_SetEffectsFadeVolume(&SampleManager, 0);
		SampleManager_SetMusicFadeVolume(&SampleManager, 0);
		AudioReloadHost_ResetMusic();
		manager->m_bIsPlayerShutUp = 0;
#ifdef AUDIO_OAL
		SampleManager_Service(&SampleManager);
#endif
	}
}

void
AudioReload_DestroyEntities(cAudioManager *manager)
{
	// Preserve the original reload gates, partial writes and callback order
	cAudioScriptObject *entity;

	if (manager->m_bIsInitialised) {
		for (uint32_t i = 0; i < NUM_AUDIOENTITIES; i++) {
			if (manager->m_asAudioEntities[i].m_bIsUsed) {
				switch (manager->m_asAudioEntities[i].m_nType) {
				case AUDIOTYPE_PHYSICAL:
				case AUDIOTYPE_EXPLOSION:
				case AUDIOTYPE_WEATHER:
				//case AUDIOTYPE_CRANE:
				case AUDIOTYPE_GARAGE:
				case AUDIOTYPE_FIREHYDRANT:
					AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, i);
					break;
				case AUDIOTYPE_SCRIPTOBJECT:
					entity = (cAudioScriptObject *)manager->m_asAudioEntities[i].m_pEntity;
					if (entity) {
						// Reset and release the pooled audio script object
						AudioScriptObject_Reset(entity);
						// Access raw storage through the C store or pool API
						CPool_Delete(CPools_GetAudioScriptObjectPool(), entity);
						manager->m_asAudioEntities[i].m_pEntity = NULL;
					}
					AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, i);
					break;
				default:
					break;
				}
			}
		}
		// Reset the active script-object count through the C state API
		AudioScriptObjectManager_Reset(&manager->m_sAudioScriptObjectManager);
	}
}

void
DMAudio_ResetTimers(uint32_t time)
{
    // Reset the live owner through the actual C reload operation
    AudioReload_ResetTimers(&AudioManager, time);
}

void
DMAudio_DestroyAllGameCreatedEntities(void)
{
    // Clean up the live owner's game-created entities through C
    AudioReload_DestroyEntities(&AudioManager);
}

void
DMAudio_SetOutputMode(uint8_t surround)
{
    // Retain the original no-op output mode operation
    (void)surround;
}
//- rouz edit (ChatGPT)
