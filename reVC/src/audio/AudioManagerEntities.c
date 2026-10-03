//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "DMAudio.h"

int32_t
AudioManager_CreateEntity(cAudioManager *manager, enum eAudioType type, void *entity)
{
	// Preserve validation, queued-ID exclusion, and first-free-slot selection
	if (!manager->m_bIsInitialised)
		return AEHANDLE_ERROR_NOAUDIOSYS;
	if (!entity)
		return AEHANDLE_ERROR_NOENTITY;
	if (type >= TOTAL_AUDIO_TYPES)
		return AEHANDLE_ERROR_BADAUDIOTYPE;

#ifdef FIX_BUGS
	// since sound could still play after entity deletion let's make sure we don't override one that is in use
	// find all the free entity IDs that are being used by queued samples
	int32_t stillUsedEntities[NUM_CHANNELS_GENERIC * NUM_SOUND_QUEUES];
	uint32_t stillUsedEntitiesCount = 0;

	for (uint8_t i = 0; i < NUM_SOUND_QUEUES; i++)
		for (uint8_t j = 0; j < manager->m_nRequestedCount[i]; j++) {
			tSound *sound = & manager->m_aRequestedQueue[i][manager->m_aRequestedOrderList[i][j]];
			if (sound->m_nEntityIndex < 0) continue;
			if (!manager->m_asAudioEntities[sound->m_nEntityIndex].m_bIsUsed) {
				uint8_t found = 0;
				for (uint8_t k = 0; k < stillUsedEntitiesCount; k++) {
					if (stillUsedEntities[k] == sound->m_nEntityIndex) {
						found = 1;
						break;
					}
				}
				if (!found)
					stillUsedEntities[stillUsedEntitiesCount++] = sound->m_nEntityIndex;
			}
		}
#endif

	for (uint32_t i = 0; i < NUM_AUDIOENTITIES; i++) {
		if (!manager->m_asAudioEntities[i].m_bIsUsed) {
#ifdef FIX_BUGS
			// skip if ID is still used by queued sample
			uint8_t skip = 0;
			for (uint8_t j = 0; j < stillUsedEntitiesCount; j++) {
				if (stillUsedEntities[j] == i) {
					//debug("audio entity %i still used, skipping\n", i);
					skip = 1;
					break;
				}
			}
			if (skip)
				continue;
#endif

			// Initialize the selected entity record through its C interface
			AudioEntity_Init(&manager->m_asAudioEntities[i], type, entity);
			manager->m_aAudioEntityOrderList[manager->m_nAudioEntitiesCount++] = i;
			return i;
		}
	}
	return AEHANDLE_ERROR_NOFREESLOT;
}

int32_t
DMAudio_CreateEntity(enum eAudioType type, void *UID)
{
	// Preserve the original game audio operation through the C interface
	return AudioManager_CreateEntity(&AudioManager, type, UID);
}

void
DMAudio_DestroyEntity(int32_t audioEntity)
{
	// Preserve the original game audio operation through the C interface
	AudioEntities_Destroy(AudioManager.m_asAudioEntities, AudioManager.m_aAudioEntityOrderList, &AudioManager.m_nAudioEntitiesCount, AudioManager.m_bIsInitialised, audioEntity);
}

uint8_t
DMAudio_GetEntityStatus(int32_t audioEntity)
{
	// Preserve the original game audio operation through the C interface
	return AudioEntities_GetStatus(AudioManager.m_asAudioEntities, AudioManager.m_bIsInitialised, audioEntity);
}

void
DMAudio_SetEntityStatus(int32_t audioEntity, uint8_t status)
{
	// Preserve the original game audio operation through the C interface
	AudioEntities_SetStatus(AudioManager.m_asAudioEntities, AudioManager.m_bIsInitialised, audioEntity, status);
}

void
DMAudio_PlayOneShot(int32_t audioEntity, uint16_t oneShot, float volume)
{
	// Preserve the original game audio operation through the C interface
	AudioEntities_PlayOneShot(AudioManager.m_asAudioEntities, &AudioManager.m_sAudioScriptObjectManager, AudioManager.m_bIsInitialised, audioEntity, oneShot, volume);
}

//- rouz edit (ChatGPT)
