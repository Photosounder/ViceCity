#pragma once
//+ rouz edit (ChatGPT)
#include <stddef.h>
#include <stdint.h>

typedef struct AudioScriptPosition {
	float x, y, z;
} AudioScriptPosition;

typedef struct cAudioScriptObject {
	int16_t AudioId;
	AudioScriptPosition Posn;
	int32_t AudioEntity;
} cAudioScriptObject;

#ifdef __cplusplus
static_assert(sizeof(cAudioScriptObject) == 20, "Audio script save record size");
static_assert(offsetof(cAudioScriptObject, Posn) == 4, "Audio script position offset");
static_assert(offsetof(cAudioScriptObject, AudioEntity) == 16, "Audio script entity offset");
extern "C" {
#else
_Static_assert(sizeof(cAudioScriptObject) == 20, "Audio script save record size");
_Static_assert(offsetof(cAudioScriptObject, Posn) == 4, "Audio script position offset");
_Static_assert(offsetof(cAudioScriptObject, AudioEntity) == 16, "Audio script entity offset");
#endif

void AudioScriptObject_Reset(cAudioScriptObject *object);
void AudioScriptObject_LoadAll(uint8_t *buf, uint32_t size);
void AudioScriptObject_SaveAll(uint8_t *buf, uint32_t *size);
void PlayOneShotScriptObject(uint8_t id, float x, float y, float z);

#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
