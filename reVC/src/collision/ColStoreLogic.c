//+ rouz edit (Claude)
#include <stdint.h>
#include <string.h>
#include "ColStoreLogic.h"
#include "General.h"

void ColStore_InitSlot(ColDef *def, const char *name)
{
	// An empty area that any point will grow and an empty model range
	def->isLoaded = false;
	def->unused = 0;
	def->bounds.left = 1000000.0f;
	def->bounds.top = 1000000.0f;
	def->bounds.right = -1000000.0f;
	def->bounds.bottom = -1000000.0f;
	def->minIndex = INT16_MAX;
	def->maxIndex = INT16_MIN;
	strcpy(def->name, name);
}

void ColStore_IncludeModelIndex(ColDef *def, int32_t modelIndex)
{
	// Widen the range of models the file holds
	if (modelIndex < def->minIndex)
		def->minIndex = modelIndex;
	if (modelIndex > def->maxIndex)
		def->maxIndex = modelIndex;
}

int ColStore_FindColSlot(const CPool *pool, const char *name)
{
	int i;
	int size = CPool_GetSize(pool);

	// Names are compared without case
	for (i = 0; i < size; i++) {
		const ColDef *def = (const ColDef*)CPool_GetSlot(pool, i);
		if (def && !General_faststricmp(def->name, name))
			return i;
	}
	return -1;
}

bool ColStore_HasCollisionLoaded(const CPool *pool, int slotCount, float x, float y)
{
	int i;

	// The generic slot 0 is never streamed
	for (i = 1; i < slotCount; i++) {
		const ColDef *def = (const ColDef*)CPool_GetSlot(pool, i);
		if (def && Rect_IsPointInsideRadius(&def->bounds, &x, &y, -115.0f) && !def->isLoaded)
			return false;
	}
	return true;
}
//- rouz edit (Claude)
