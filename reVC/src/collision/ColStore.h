#pragma once

#include "templates.h"

struct ColDef {	// made up name
	int32 unused;
	bool isLoaded;
	CRect bounds;
	char name[20];
	int16 minIndex;
	int16 maxIndex;
};

class CColStore
{
	//+ rouz edit (ChatGPT)
	static CPool *ms_pColPool;
	//- rouz edit (ChatGPT)

public:
	static void Initialise(void);
	static void Shutdown(void);
	static int AddColSlot(const char *name);
	static void RemoveColSlot(int32 slot);
	static int FindColSlot(const char *name);
	static char *GetColName(int32 slot);
	static CRect &GetBoundingBox(int32 slot);
	static void IncludeModelIndex(int32 slot, int32 modelIndex);
	static bool LoadCol(int32 storeID, uint8 *buffer, int32 bufsize);
	static void RemoveCol(int32 slot);
	static void AddCollisionNeededAtPosn(const CVector2D &pos);
	static void LoadAllCollision(void);
	static void RemoveAllCollision(void);
	static void LoadCollision(const CVector2D &pos);
	static void RequestCollision(const CVector2D &pos);
	static void EnsureCollisionIsInMemory(const CVector2D &pos);
	static bool HasCollisionLoaded(const CVector2D &pos);

	static ColDef *GetSlot(int slot) {
		assert(slot >= 0);
		assert(ms_pColPool);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		assert(slot < CPool_GetSize(ms_pColPool));
		return ((ColDef*)CPool_GetSlot(ms_pColPool, slot));
		//- rouz edit (ChatGPT)
	}
};
