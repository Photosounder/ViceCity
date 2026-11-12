//+ rouz edit (Claude)
#pragma once
#include "PhysicalLayout.h"
#include "EntityLogic.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Registration of a physical entity in the sector lists, each sector also gets an entry info node on the entity
CRect Physical_GetBoundRect(const CPlaceableData *placeable, const CEntityData *entity);
void Physical_Add(CPhysicalStruct *self, CSector *sectors);
void Physical_Remove(CPhysicalStruct *self);
void Physical_RemoveAndAdd(CPhysicalStruct *self, CSector *sectors);
// Moving entity list handling
void Physical_AddToMovingList(CPhysicalStruct *self, CPtrList *movingList);
void Physical_RemoveFromMovingList(CPhysicalStruct *self, CPtrList *movingList);
// First half of the per frame processing: clears the per frame state and puts resting entities to sleep, returns false when the rest of the processing must be skipped
bool Physical_ProcessControlStart(CPhysicalStruct *self, float timeStep);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
