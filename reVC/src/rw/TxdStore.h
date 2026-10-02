#pragma once

#include "templates.h"

struct TxdDef {
	RwTexDictionary *texDict;
	int refCount;
	char name[20];
};

class CTxdStore
{
	//+ rouz edit (ChatGPT)
	static CPool *ms_pTxdPool;
	//- rouz edit (ChatGPT)
	static RwTexDictionary *ms_pStoredTxd;
public:
	static void Initialise(void);
	static void Shutdown(void);
	static void GameShutdown(void);
	static int AddTxdSlot(const char *name);
	static void RemoveTxdSlot(int slot);
	static int FindTxdSlot(const char *name);
	static char *GetTxdName(int slot);
	static void PushCurrentTxd(void);
	static void PopCurrentTxd(void);
	static void SetCurrentTxd(int slot);
	static void Create(int slot);
	static int GetNumRefs(int slot);
	static void AddRef(int slot);
	static void RemoveRef(int slot);
	static void RemoveRefWithoutDelete(int slot);
	static bool LoadTxd(int slot, RwStream *stream);
	static bool LoadTxd(int slot, const char *filename);
	static bool StartLoadTxd(int slot, RwStream *stream);
	static bool FinishLoadTxd(int slot, RwStream *stream);
	static void RemoveTxd(int slot);

	static TxdDef *GetSlot(int slot) {
		assert(slot >= 0);
		assert(ms_pTxdPool);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		assert(slot < CPool_GetSize(ms_pTxdPool));
		return ((TxdDef*)CPool_GetSlot(ms_pTxdPool, slot));
		//- rouz edit (ChatGPT)
	}
	static bool isTxdLoaded(int slot);
};
