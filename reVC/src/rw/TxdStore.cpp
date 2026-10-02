#include "common.h"

#include "templates.h"
#include "General.h"
#include "Streaming.h"
#include "RwHelper.h"
#include "TxdStore.h"

//+ rouz edit (ChatGPT)
CPool *CTxdStore::ms_pTxdPool;
//- rouz edit (ChatGPT)
RwTexDictionary *CTxdStore::ms_pStoredTxd;

void
CTxdStore::Initialise(void)
{
	if(ms_pTxdPool == nil)
//+ rouz edit (ChatGPT)
		// Construct the TXD pool without invoking C++ new.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		ms_pTxdPool = CPool_Create(TXDSTORESIZE, sizeof(TxdDef), "TexDictionary");
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)
}

void
CTxdStore::Shutdown(void)
{
	if(ms_pTxdPool)
//+ rouz edit (ChatGPT)
		// Destroy and release the TXD pool without invoking C++ delete.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Destroy(ms_pTxdPool);
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)
}

void
CTxdStore::GameShutdown(void)
{
	int i;

	for(i = 0; i < TXDSTORESIZE; i++){
		TxdDef *def = GetSlot(i);
		if(def && GetNumRefs(i) == 0)
			RemoveTxdSlot(i);
	}
}

int
CTxdStore::AddTxdSlot(const char *name)
{
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	TxdDef *def = ((TxdDef*)CPool_New(ms_pTxdPool));
	//- rouz edit (ChatGPT)
	assert(def);
	def->texDict = nil;
	def->refCount = 0;
	strcpy(def->name, name);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	return CPool_GetJustIndex(ms_pTxdPool, def);
	//- rouz edit (ChatGPT)
}

void
CTxdStore::RemoveTxdSlot(int slot)
{
	TxdDef *def = GetSlot(slot);
	if(def->texDict)
		RwTexDictionaryDestroy(def->texDict);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPool_Delete(ms_pTxdPool, def);
	//- rouz edit (ChatGPT)
}

int
CTxdStore::FindTxdSlot(const char *name)
{
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int size = CPool_GetSize(ms_pTxdPool);
	//- rouz edit (ChatGPT)
	for(int i = 0; i < size; i++){
		TxdDef *def = GetSlot(i);
		if(def && !CGeneral::faststricmp(def->name, name))
			return i;
	}
	return -1;
}

char*
CTxdStore::GetTxdName(int slot)
{
	return GetSlot(slot)->name;
}

void
CTxdStore::PushCurrentTxd(void)
{
	ms_pStoredTxd = RwTexDictionaryGetCurrent();
}

void
CTxdStore::PopCurrentTxd(void)
{
	RwTexDictionarySetCurrent(ms_pStoredTxd);
	ms_pStoredTxd = nil;
}

void
CTxdStore::SetCurrentTxd(int slot)
{
	RwTexDictionarySetCurrent(GetSlot(slot)->texDict);
}

void
CTxdStore::Create(int slot)
{
	GetSlot(slot)->texDict = RwTexDictionaryCreate();
}

int
CTxdStore::GetNumRefs(int slot)
{
	return GetSlot(slot)->refCount;
}

void
CTxdStore::AddRef(int slot)
{
	GetSlot(slot)->refCount++;
}

void
CTxdStore::RemoveRef(int slot)
{
	if(--GetSlot(slot)->refCount <= 0)
		CStreaming::RemoveTxd(slot);
}

void
CTxdStore::RemoveRefWithoutDelete(int slot)
{
	GetSlot(slot)->refCount--;
}

bool
CTxdStore::LoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);

	if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil)){
		def->texDict = RwTexDictionaryGtaStreamRead(stream);
		return def->texDict != nil;
	}
	printf("Failed to load TXD\n");
	return false;
}

bool
CTxdStore::LoadTxd(int slot, const char *filename)
{
	RwStream *stream;
	bool ret;

	ret = false;
	_rwD3D8TexDictionaryEnableRasterFormatConversion(true);
	do
		stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	while(stream == nil);
	ret = LoadTxd(slot, stream);
	RwStreamClose(stream, nil);
	return ret;
}

bool
CTxdStore::StartLoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);
	if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil)){
		def->texDict = RwTexDictionaryGtaStreamRead1(stream);
		return def->texDict != nil;
	}else{
		printf("Failed to load TXD\n");
		return false;
	}
}

bool
CTxdStore::FinishLoadTxd(int slot, RwStream *stream)
{
	TxdDef *def = GetSlot(slot);
	def->texDict = RwTexDictionaryGtaStreamRead2(stream, def->texDict);
	return def->texDict != nil;
}

void
CTxdStore::RemoveTxd(int slot)
{
	TxdDef *def = GetSlot(slot);
	if(def->texDict)
		RwTexDictionaryDestroy(def->texDict);
	def->texDict = nil;
}
