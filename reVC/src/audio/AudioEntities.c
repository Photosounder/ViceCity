//+ rouz edit (ChatGPT)
#include "AudioEntities.h"
#include "soundlist.h"
#include <string.h>

void
AudioEntity_Init(tAudioEntity *entity, enum eAudioType type, void *pointer)
{
	// Initialize the selected record without clearing its retained volumes
	entity->m_bIsUsed = 1;
	entity->m_bStatus = 0;
	entity->m_nType = type;
	entity->m_pEntity = pointer;
	entity->m_awAudioEvent[0] = SOUND_NO_SOUND;
	entity->m_awAudioEvent[1] = SOUND_NO_SOUND;
	entity->m_awAudioEvent[2] = SOUND_NO_SOUND;
	entity->m_awAudioEvent[3] = SOUND_NO_SOUND;
	entity->m_AudioEvents = 0;
}

void
AudioEntities_Destroy(tAudioEntity *entities, uint32_t *order, uint32_t *count, uint8_t initialized, int32_t id)
{
	// Preserve the original validation and state updates
	if (initialized && id >= 0 && id < NUM_AUDIOENTITIES && entities[id].m_bIsUsed) {
		entities[id].m_bIsUsed = 0;
		for (uint32_t i = 0; i < (*count); ++i) {
			if (id == order[i]) {
				if (i < NUM_AUDIOENTITIES - 1)
					memmove(&order[i], &order[i + 1], sizeof(uint32_t) * ((*count) - (i + 1)));
				order[--(*count)] = NUM_AUDIOENTITIES;
				return;
			}
		}
	}
}

uint8_t
AudioEntities_GetStatus(tAudioEntity *entities, uint8_t initialized, int32_t id)
{
	// Preserve the original validation and state updates
	if (initialized && id >= 0 && id < NUM_AUDIOENTITIES && entities[id].m_bIsUsed)
		return entities[id].m_bStatus;
	return 0;
}

void
AudioEntities_SetStatus(tAudioEntity *entities, uint8_t initialized, int32_t id, uint8_t status)
{
	// Preserve the original validation and state updates
	if (initialized && id >= 0 && id < NUM_AUDIOENTITIES && entities[id].m_bIsUsed)
		entities[id].m_bStatus = status;
}

void *
AudioEntities_GetPointer(tAudioEntity *entities, uint8_t initialized, int32_t id)
{
	// Preserve the original validation and state updates
	if (initialized && id >= 0 && id < NUM_AUDIOENTITIES && entities[id].m_bIsUsed)
		return entities[id].m_pEntity;
	return NULL;
}

static const uint8_t OneShotPriority[] = {
										3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 4, 2, 5, 5, 3, 5, 2, 2, 1, 1, 3, 1, 3, 3, 1, 1, 1, 1, 4, 4, 4, 3, 1, 1, 1, 1, 1,
										1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 1, 1, 1, 1, 1, 3, 4, 2, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0,
										0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 3, 1, 1, 1, 9, 0, 0, 0, 1, 2, 2, 0, 0, 2, 3, 3, 3, 5, 1, 1,
										1, 1, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 7, 1, 4, 3, 4, 2, 2, 2, 3, 1, 2, 1, 3, 5, 3, 4, 6, 4, 6, 3, 0, 0, 0, 0, 0,
										0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 3, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0, 3, 3, 1, 0
										};

void
AudioEntities_PlayOneShot(tAudioEntity *entities, cAudioScriptObjectManager *scripts, uint8_t initialized, int32_t index, uint16_t sound, float vol)
{
	// Preserve the original validation and state updates

	if (initialized) {
		if (index >= 0 && index < NUM_AUDIOENTITIES) {
			tAudioEntity *entity = &entities[index];
			if (entity->m_bIsUsed) {
				if (sound < SOUND_TOTAL_SOUNDS) {
					if (entity->m_nType == AUDIOTYPE_SCRIPTOBJECT) {
						if (scripts->m_nScriptObjectEntityTotal < NUM_SCRIPT_MAX_ENTITIES) {
							entity->m_awAudioEvent[0] = sound;
							entity->m_AudioEvents = 1;
							scripts->m_anScriptObjectEntityIndices[scripts->m_nScriptObjectEntityTotal++] = index;
						}
					} else {
						int32_t i = 0;
						while (1) {
							if (i >= entity->m_AudioEvents) {
								if (entity->m_AudioEvents < NUM_AUDIOENTITY_EVENTS) {
									entity->m_awAudioEvent[i] = sound;
									entity->m_afVolume[i] = vol;
									entity->m_AudioEvents++;
								}
								return;
							}
							if (OneShotPriority[entity->m_awAudioEvent[i]] > OneShotPriority[sound])
								break;
							i++;
						}
						if (i < NUM_AUDIOENTITY_EVENTS - 1) {
							memmove(&entity->m_awAudioEvent[i + 1], &entity->m_awAudioEvent[i], (NUM_AUDIOENTITY_EVENTS - 1 - i) * sizeof(int16_t));
							memmove(&entity->m_afVolume[i + 1], &entity->m_afVolume[i], (NUM_AUDIOENTITY_EVENTS - 1 - i) * sizeof(float));
						}
						entity->m_awAudioEvent[i] = sound;
						entity->m_afVolume[i] = vol;
						if (entity->m_AudioEvents < NUM_AUDIOENTITY_EVENTS)
							entity->m_AudioEvents++;
					}
				}
			}
		}
	}
}

//- rouz edit (ChatGPT)
