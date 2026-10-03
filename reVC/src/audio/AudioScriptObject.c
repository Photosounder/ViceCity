//+ rouz edit (ChatGPT)
#include "../core/config.h"
#include <assert.h>

extern void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0))
#else
#define assert(expression) (expression)
#endif

#include "AudioScriptObject.h"
#include "../core/Pools.h"
#include "DMAudio.h"
#include "../save/SaveBuf.h"

void
AudioScriptObject_Reset(cAudioScriptObject *object)
{
	// Reset the same fields as the former constructor and destructor
	object->AudioId = SCRIPT_SOUND_INVALID;
	object->Posn.x = 0.0f;
	object->Posn.y = 0.0f;
	object->Posn.z = 0.0f;
	object->AudioEntity = AEHANDLE_NONE;
}

void
AudioScriptObject_LoadAll(uint8_t *buf, uint32_t size)
{
	// Read the existing header and count before restoring each saved pool handle
	INITSAVEBUF
	CheckSaveHeader(&buf, 'A', 'U', 'D', '\0', size - SAVE_HEADER_SIZE);
	int32_t pool_size;
	ReadSaveBuf(&pool_size, &buf, sizeof(pool_size));
	for (int32_t i = 0; i < pool_size; i++) {
		// Restore the record and recreate its looping audio entity
		int32_t handle;
		ReadSaveBuf(&handle, &buf, sizeof(handle));
		cAudioScriptObject *object = (cAudioScriptObject *)CPool_NewAt(CPools_GetAudioScriptObjectPool(), handle);
		assert(object != NULL);
		AudioScriptObject_Reset(object);
		*object = *(cAudioScriptObject *)buf;
		SkipSaveBuf(&buf, sizeof(*object));
		object->AudioEntity = DMAudio_CreateLoopingScriptObject(object);
	}
	VALIDATESAVEBUF(size);
}

void
AudioScriptObject_SaveAll(uint8_t *buf, uint32_t *size)
{
	// Preserve the existing save header, record size, and descending pool order
	INITSAVEBUF
	CPool *pool = CPools_GetAudioScriptObjectPool();
	int32_t pool_size = CPool_GetNoOfUsedSpaces(pool);
	*size = SAVE_HEADER_SIZE + sizeof(int32_t) + pool_size * (sizeof(cAudioScriptObject) + sizeof(int32_t));
	WriteSaveHeader(&buf, 'A', 'U', 'D', '\0', *size - SAVE_HEADER_SIZE);
	WriteSaveBuf(&buf, &pool_size, sizeof(pool_size));
	int32_t i = CPool_GetSize(pool);
	while (i--) {
		// Write the original handle followed by the unchanged record layout
		cAudioScriptObject *object = (cAudioScriptObject *)CPool_GetSlot(pool, i);
		if (object != NULL) {
			// Preserve the pool generation in the saved handle
			int32_t handle = CPool_GetIndex(pool, object);
			WriteSaveBuf(&buf, &handle, sizeof(handle));
			*(cAudioScriptObject *)buf = *object;
			SkipSaveBuf(&buf, sizeof(*object));
		}
	}
	VALIDATESAVEBUF(*size);
}

void
PlayOneShotScriptObject(uint8_t id, float x, float y, float z)
{
	// Skip allocation while audio is inactive
	if (!DMAudio_IsAudioInitialised()) return;

	// Initialize a pooled record explicitly before creating its one-shot entity
	cAudioScriptObject *object = (cAudioScriptObject *)CPool_New(CPools_GetAudioScriptObjectPool());
	assert(object != NULL);
	AudioScriptObject_Reset(object);
	object->Posn.x = x;
	object->Posn.y = y;
	object->Posn.z = z;
	object->AudioId = id;
	object->AudioEntity = AEHANDLE_NONE;
	DMAudio_CreateOneShotScriptObject(object);
}
//- rouz edit (ChatGPT)
