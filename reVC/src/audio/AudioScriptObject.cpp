#include "common.h"

#include "AudioScriptObject.h"
#include "Pools.h"
#include "DMAudio.h"
#include "SaveBuf.h"

cAudioScriptObject::cAudioScriptObject()
{
	Reset();
};

cAudioScriptObject::~cAudioScriptObject()
{
	Reset();
};

void
cAudioScriptObject::Reset()
{
	AudioId = SCRIPT_SOUND_INVALID;
	Posn = CVector(0.0f, 0.0f, 0.0f);
	AudioEntity = AEHANDLE_NONE;
}

void
cAudioScriptObject::LoadAllAudioScriptObjects(uint8 *buf, uint32 size)
{
	INITSAVEBUF

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	CheckSaveHeader(&buf, 'A', 'U', 'D', '\0', size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)

	int32 pool_size;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&pool_size, &buf, sizeof(pool_size));
	//- rouz edit (ChatGPT)
	for (int32 i = 0; i < pool_size; i++) {
		int32 handle;
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&handle, &buf, sizeof(handle));
		//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
		// Restore the audio script object into its saved pool slot.
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		cAudioScriptObject *p = ((cAudioScriptObject*)CPool_NewAt(CPools::GetAudioScriptObjectPool(), handle));
		//- rouz edit (ChatGPT)
		assert(p != nil);
		std::allocator<cAudioScriptObject>().construct(p);
//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		*p = *(cAudioScriptObject*)buf;
		SkipSaveBuf(&buf, sizeof(*p));
		//- rouz edit (ChatGPT)
		p->AudioEntity = DMAudio.CreateLoopingScriptObject(p);
	}

	VALIDATESAVEBUF(size);
}

void
cAudioScriptObject::SaveAllAudioScriptObjects(uint8 *buf, uint32 *size)
{
	INITSAVEBUF

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int32 pool_size = CPool_GetNoOfUsedSpaces(CPools::GetAudioScriptObjectPool());
	//- rouz edit (ChatGPT)
	*size = SAVE_HEADER_SIZE + sizeof(int32) + pool_size * (sizeof(cAudioScriptObject) + sizeof(int32));
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveHeader(&buf, 'A', 'U', 'D', '\0', *size - SAVE_HEADER_SIZE);
	WriteSaveBuf(&buf, &pool_size, sizeof(pool_size));
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int32 i = CPool_GetSize(CPools::GetAudioScriptObjectPool());
	//- rouz edit (ChatGPT)
	while (i--) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		cAudioScriptObject *p = ((cAudioScriptObject*)CPool_GetSlot(CPools::GetAudioScriptObjectPool(), i));
		//- rouz edit (ChatGPT)
		if (p != nil) {
			//+ rouz edit (ChatGPT)
			// Transfer save data through the C buffer API with explicit sizes
			{
				// Materialize the saved value with its original serialized type
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				int32 saveValue = CPool_GetIndex(CPools::GetAudioScriptObjectPool(), p);
				//- rouz edit (ChatGPT)
				WriteSaveBuf(&buf, &saveValue, sizeof(saveValue));
			}
			// Preserve record assignment semantics and compiler padding behavior
			*(cAudioScriptObject*)buf = *p;
			SkipSaveBuf(&buf, sizeof(*p));
			//- rouz edit (ChatGPT)
		}
	}

	VALIDATESAVEBUF(*size);
}

void
PlayOneShotScriptObject(uint8 id, CVector const &pos)
{
	if (!DMAudio.IsAudioInitialised()) return;

//+ rouz edit (ChatGPT)
	// Allocate a pooled audio script object without invoking C++ new.
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	cAudioScriptObject *audioScriptObject = ((cAudioScriptObject*)CPool_New(CPools::GetAudioScriptObjectPool()));
	//- rouz edit (ChatGPT)
	assert(audioScriptObject != nil);
	std::allocator<cAudioScriptObject>().construct(audioScriptObject);
//- rouz edit (ChatGPT)
	audioScriptObject->Posn = pos;
	audioScriptObject->AudioId = id;
	audioScriptObject->AudioEntity = AEHANDLE_NONE;
	DMAudio.CreateOneShotScriptObject(audioScriptObject);
}
