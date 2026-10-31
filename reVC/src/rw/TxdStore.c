//+ rouz edit (ChatGPT)
#include "config.h"
#include "TxdStore.h"
#include "TxdStore_backend.h"
#include "General.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#include "Pool.h"
static CPool *TxdStore_pool;
static void *TxdStore_stored;
TxdDef *TxdStore_GetSlot(int slot)
{
    // Preserve original slot assertions and live pool lookup

		assert(slot >= 0);
		assert(TxdStore_pool);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		assert(slot < CPool_GetSize(TxdStore_pool));
		return ((TxdDef*)CPool_GetSlot(TxdStore_pool, slot));
		//- rouz edit (ChatGPT)
	
}
void TxdStore_Initialise(void)
{
    // Preserve the complete original Initialise operation and live mutation order

	if(TxdStore_pool == NULL)
//+ rouz edit (ChatGPT)
		// Construct the TXD pool without invoking C++ new.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		TxdStore_pool = CPool_Create(TXDSTORESIZE, sizeof(TxdDef), "TexDictionary");
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)

}
void TxdStore_Shutdown(void)
{
    // Preserve the complete original Shutdown operation and live mutation order

	if(TxdStore_pool)
//+ rouz edit (ChatGPT)
		// Destroy and release the TXD pool without invoking C++ delete.
	{
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Destroy(TxdStore_pool);
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)

}
void TxdStore_GameShutdown(void)
{
    // Preserve the complete original GameShutdown operation and live mutation order

	int i;

	for(i = 0; i < TXDSTORESIZE; i++){
		TxdDef *def = TxdStore_GetSlot(i);
		if(def && TxdStore_GetNumRefs(i) == 0)
			TxdStore_RemoveTxdSlot(i);
	}

}
int TxdStore_AddTxdSlot(const char *name)
{
    // Preserve the complete original AddTxdSlot operation and live mutation order

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	TxdDef *def = ((TxdDef*)CPool_New(TxdStore_pool));
	//- rouz edit (ChatGPT)
	assert(def);
	def->texDict = NULL;
	def->refCount = 0;
	strcpy(def->name, name);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	return CPool_GetJustIndex(TxdStore_pool, def);
	//- rouz edit (ChatGPT)

}
void TxdStore_RemoveTxdSlot(int slot)
{
    // Preserve the complete original RemoveTxdSlot operation and live mutation order

	TxdDef *def = TxdStore_GetSlot(slot);
	if(def->texDict)
		TxdStore_HostDestroyDictionary(def->texDict);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPool_Delete(TxdStore_pool, def);
	//- rouz edit (ChatGPT)

}
int TxdStore_FindTxdSlot(const char *name)
{
    // Preserve the complete original FindTxdSlot operation and live mutation order

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int size = CPool_GetSize(TxdStore_pool);
	//- rouz edit (ChatGPT)
	for(int i = 0; i < size; i++){
		TxdDef *def = TxdStore_GetSlot(i);
		if(def && !General_faststricmp(def->name, name))
			return i;
	}
	return -1;

}
char * TxdStore_GetTxdName(int slot)
{
    // Preserve the complete original GetTxdName operation and live mutation order

	return TxdStore_GetSlot(slot)->name;

}
void TxdStore_PushCurrentTxd(void)
{
    // Preserve the complete original PushCurrentTxd operation and live mutation order

	TxdStore_stored = TxdStore_HostGetCurrent();

}
void TxdStore_PopCurrentTxd(void)
{
    // Preserve the complete original PopCurrentTxd operation and live mutation order

	TxdStore_HostSetCurrent(TxdStore_stored);
	TxdStore_stored = NULL;

}
void TxdStore_SetCurrentTxd(int slot)
{
    // Preserve the complete original SetCurrentTxd operation and live mutation order

	TxdStore_HostSetCurrent(TxdStore_GetSlot(slot)->texDict);

}
void TxdStore_Create(int slot)
{
    // Preserve the complete original Create operation and live mutation order

	TxdStore_GetSlot(slot)->texDict = TxdStore_HostCreateDictionary();

}
int TxdStore_GetNumRefs(int slot)
{
    // Preserve the complete original GetNumRefs operation and live mutation order

	return TxdStore_GetSlot(slot)->refCount;

}
void TxdStore_AddRef(int slot)
{
    // Preserve the complete original AddRef operation and live mutation order

	TxdStore_GetSlot(slot)->refCount++;

}
void TxdStore_RemoveRef(int slot)
{
    // Preserve the complete original RemoveRef operation and live mutation order

	if(--TxdStore_GetSlot(slot)->refCount <= 0)
		TxdStore_HostRemoveStreamingTxd(slot);

}
void TxdStore_RemoveRefWithoutDelete(int slot)
{
    // Preserve the complete original RemoveRefWithoutDelete operation and live mutation order

	TxdStore_GetSlot(slot)->refCount--;

}
bool TxdStore_LoadStream(int slot,void *stream)
{
    // Preserve the complete original LoadStream operation and live mutation order

	TxdDef *def = TxdStore_GetSlot(slot);

	if(TxdStore_HostFindDictionaryChunk(stream)){
		def->texDict = TxdStore_HostReadDictionary(stream);
		return def->texDict != NULL;
	}
	printf("Failed to load TXD\n");
	return false;

}
bool TxdStore_LoadFile(int slot,const char *filename)
{
    // Preserve the complete original LoadFile operation and live mutation order

	void *stream;
	bool ret;

	ret = false;
	TxdStore_HostEnableRasterConversion(true);
	do
		stream = TxdStore_HostOpenFileStream(filename);
	while(stream == NULL);
	ret = TxdStore_LoadStream(slot, stream);
	TxdStore_HostCloseStream(stream);
	return ret;

}
bool TxdStore_StartLoadTxd(int slot,void *stream)
{
    // Preserve the complete original StartLoadTxd operation and live mutation order

	TxdDef *def = TxdStore_GetSlot(slot);
	if(TxdStore_HostFindDictionaryChunk(stream)){
		def->texDict = TxdStore_HostReadDictionary1(stream);
		return def->texDict != NULL;
	}else{
		printf("Failed to load TXD\n");
		return false;
	}

}
bool TxdStore_FinishLoadTxd(int slot,void *stream)
{
    // Preserve the complete original FinishLoadTxd operation and live mutation order

	TxdDef *def = TxdStore_GetSlot(slot);
	def->texDict = TxdStore_HostReadDictionary2(stream, def->texDict);
	return def->texDict != NULL;

}
void TxdStore_RemoveTxd(int slot)
{
    // Preserve the complete original RemoveTxd operation and live mutation order

	TxdDef *def = TxdStore_GetSlot(slot);
	if(def->texDict)
		TxdStore_HostDestroyDictionary(def->texDict);
	def->texDict = NULL;

}
//- rouz edit (ChatGPT)
