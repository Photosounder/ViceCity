//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "ColDef.h"
#include "Pool.h"

#ifdef __cplusplus
extern "C" {
#endif
// Reset a new slot to an empty area and model range and give it its name
void ColStore_InitSlot(ColDef *def, const char *name);
// Grow the model index range of a slot to include a model
void ColStoreLogic_IncludeModelIndex(ColDef *def, int32_t modelIndex);
// Index of the slot with a name or -1
int ColStoreLogic_FindColSlot(const CPool *pool, const char *name);
// Whether every slot that covers a position is loaded
bool ColStoreLogic_HasCollisionLoaded(const CPool *pool, int slotCount, float x, float y);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
