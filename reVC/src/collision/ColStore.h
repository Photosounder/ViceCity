#pragma once

#include "templates.h"

#include "ColDef.h"

//+ rouz edit (Claude)
// Formerly the static class CColStore, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern CPool * ColStore_ms_pColPool;
void ColStore_Initialise(void);
void ColStore_Shutdown(void);
int ColStore_AddColSlot(const char *name);
void ColStore_RemoveColSlot(int32 slot);
int ColStore_FindColSlot(const char *name);
char *ColStore_GetColName(int32 slot);
void ColStore_IncludeModelIndex(int32 slot, int32 modelIndex);
bool ColStore_LoadCol(int32 storeID, uint8 *buffer, int32 bufsize);
void ColStore_RemoveCol(int32 slot);
void ColStore_LoadAllCollision(void);
void ColStore_RemoveAllCollision(void);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
CRect &ColStore_GetBoundingBox(int32 slot);
void ColStore_AddCollisionNeededAtPosn(const CVector2D &pos);
void ColStore_LoadCollision(const CVector2D &pos);
void ColStore_RequestCollision(const CVector2D &pos);
void ColStore_EnsureCollisionIsInMemory(const CVector2D &pos);
bool ColStore_HasCollisionLoaded(const CVector2D &pos);
static inline ColDef *ColStore_GetSlot(int slot);
static inline ColDef *ColStore_GetSlot(int slot) {
		assert(slot >= 0);
		assert(ColStore_ms_pColPool);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		assert(slot < CPool_GetSize(ColStore_ms_pColPool));
		return ((ColDef*)CPool_GetSlot(ColStore_ms_pColPool, slot));
		//- rouz edit (ChatGPT)
	}
#endif
//- rouz edit (Claude)
