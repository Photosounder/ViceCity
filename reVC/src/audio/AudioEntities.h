#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "AudioState.h"
#include "audio_enums.h"

typedef struct tAudioEntity {
	enum eAudioType m_nType;
	void *m_pEntity;
	uint8_t m_bIsUsed;
	uint8_t m_bStatus;
	int16_t m_awAudioEvent[NUM_AUDIOENTITY_EVENTS];
	float m_afVolume[NUM_AUDIOENTITY_EVENTS];
	uint8_t m_AudioEvents;
} tAudioEntity;


#ifdef __cplusplus
extern "C" {
#endif
void AudioEntity_Init(tAudioEntity *entity, enum eAudioType type, void *pointer);
void AudioEntities_Destroy(tAudioEntity *entities, uint32_t *order, uint32_t *count, uint8_t initialized, int32_t id);
uint8_t AudioEntities_GetStatus(tAudioEntity *entities, uint8_t initialized, int32_t id);
void AudioEntities_SetStatus(tAudioEntity *entities, uint8_t initialized, int32_t id, uint8_t status);
void *AudioEntities_GetPointer(tAudioEntity *entities, uint8_t initialized, int32_t id);
void AudioEntities_PlayOneShot(tAudioEntity *entities, cAudioScriptObjectManager *scripts, uint8_t initialized, int32_t index, uint16_t sound, float vol);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
