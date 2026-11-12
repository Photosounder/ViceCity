//+ rouz edit (Claude)
#include <stddef.h>
#include "DummyLogic.h"
#include "EntityTypes.h"

void Dummy_Add(void *item, const CPlaceableData *placeable, const CEntityData *self, CSector *sectors, CEntryInfoList *entryInfoList)
{
	int x, y;
	EntitySectorRange r = Entity_ComputeSectorRange(Entity_GetBoundRect(placeable, self));

	// Insert in the middle sector as the main entry and in the others as overlaps
	for (y = r.ystart; y <= r.yend; y++)
		for (x = r.xstart; x <= r.xend; x++) {
			CSector *s = Entity_SectorAt(sectors, x, y);
			CPtrList *list = Entity_SectorListFor(s, ENTITY_TYPE_DUMMY, x == r.xmid && y == r.ymid);
			CPtrNode *node = PtrList_InsertItem(list, item);
			EntryInfoList_InsertItem(entryInfoList, list, node, s);
		}
}

void Dummy_Remove(CEntryInfoList *entryInfoList)
{
	CEntryInfoNode *node, *next;

	// Remove every list node and its entry
	for (node = entryInfoList->first; node; node = next) {
		next = node->next;
		PtrList_DeleteNode(node->list, node->listnode);
		EntryInfoList_DeleteNode(entryInfoList, node);
	}
}
//- rouz edit (Claude)
