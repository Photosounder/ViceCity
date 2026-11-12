//+ rouz edit (Claude)
#pragma once
#include "EntityLogic.h"
#include "Lists.h"

#ifdef __cplusplus
extern "C" {
#endif
// Register a dummy in the sectors its bounding rectangle covers and remember each list node in its entry info list
void Dummy_Add(void *item, const CPlaceableData *placeable, const CEntityData *self, CSector *sectors, CEntryInfoList *entryInfoList);
// Take a dummy out of all the sector lists it is in
void Dummy_Remove(CEntryInfoList *entryInfoList);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
