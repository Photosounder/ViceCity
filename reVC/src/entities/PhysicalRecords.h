//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "EntityData.h"
#include "PhysicalData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Collision record bookkeeping of a physical entity, entities are only compared by address here
void Physical_SetDamagedPieceRecord(CPhysicalData *self, uint16_t piece, float impulse, CEntity *entity, float nx, float ny, float nz);
void Physical_AddCollisionRecord(CEntityData *selfEntity, CPhysicalData *self, CEntityData *otherEntity, CEntity *other, uint32_t timeMs);
bool Physical_GetHasCollidedWith(const CEntityData *selfEntity, const CPhysicalData *self, const CEntity *other);
void Physical_RemoveRefsToEntity(CPhysicalData *self, const CEntity *other);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
