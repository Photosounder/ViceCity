//+ rouz edit (Claude)
#include "WorldLogic.h"
#include "WorldSectors.h"
#include "config.h"
#include <math.h>
#include "EntityLayout.h"
#include "EntityLogic.h"
#include "VehicleCreation.h"
#include "PedStates.h"
#include "PhysicalRecords.h"
#include "VehicleBombTypes.h"
#include "References.h"
#include "AutoPilot.h"
#include "ColModel.h"
#include "ModelRegistry.h"
#include "VectorMath.h"
#include "PedObjectives.h"
#include "PedType.h"
#include "VehicleLogic.h"
#include "EntityStatus.h"
#include "PedLogic.h"

void World_ClearScanCodesInList(CPtrList *list)
{
	// Reset the scan code of every entity in the list
	CPtrNode *node;
	for (node = list->first; node; node = node->next)
		((CEntityStruct *)node->item)->entity.m_scanCode = 0;
}

void World_SetAllCarsCanBeDamaged(const CPool *vehiclePool, bool toggle)
{
	// Set the damage flag of every occupied vehicle slot
	int poolSize = CPool_GetSize(vehiclePool);
	int poolIndex;
	for (poolIndex = 0; poolIndex < poolSize; poolIndex++) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, poolIndex);
		if (veh)
			veh->vehicle.bCanBeDamaged = toggle;
	}
}

void World_StopAllLawEnforcersInTheirTracks(const CPool *vehiclePool)
{
	// Zero the move speed of every law enforcement vehicle, walking the pool backwards
	int poolSize = CPool_GetSize(vehiclePool);
	int poolIndex;
	for (poolIndex = poolSize - 1; poolIndex >= 0; poolIndex--) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, poolIndex);
		if (veh) {
			if (veh->vehicle.bIsLawEnforcer) {
				veh->base.physical.m_vecMoveSpeed.x = 0.0f;
				veh->base.physical.m_vecMoveSpeed.y = 0.0f;
				veh->base.physical.m_vecMoveSpeed.z = 0.0f;
			}
		}
	}
}

void World_FindObjectsInRangeInList(CPtrList *list, uint16_t scanCode, float cx, float cy, float cz, float radius, bool ignoreZ, int16_t *numObjects, int16_t lastObject, void **objects)
{
	float radiusSqr = radius * radius;
	float objDistSqr;
	CPtrNode *node;

	for (node = list->first; node; node = node->next) {
		CEntityStruct *object = (CEntityStruct *)node->item;
		if (object->entity.m_scanCode != scanCode) {
			// Visit each entity once per query
			float dx, dy, dz;
			object->entity.m_scanCode = scanCode;

			dx = cx - object->placeable.m_matrix.px;
			dy = cy - object->placeable.m_matrix.py;
			dz = cz - object->placeable.m_matrix.pz;
			if (ignoreZ)
				objDistSqr = dx * dx + dy * dy;
			else
				objDistSqr = dx * dx + dy * dy + dz * dz;

			if (objDistSqr < radiusSqr && *numObjects < lastObject) {
				if (objects)
					objects[*numObjects] = object;
				(*numObjects)++;
			}
		}
	}
}

void World_FindObjectsOfTypeInRangeInList(uint32_t modelId, CPtrList *list, uint16_t scanCode, float px, float py, float pz, float radius, bool check2DOnly, int16_t *numFound, int16_t maxToFind, void **entities)
{
	CPtrNode *node;
	for (node = list->first; node; node = node->next) {
		CEntityStruct *entity = (CEntityStruct *)node->item;
		if (entity->entity.m_scanCode != scanCode) {
			entity->entity.m_scanCode = scanCode;
			if (modelId == (uint32_t)entity->entity.m_modelIndex) {
				// Only entities of the requested model are measured
				float fMagnitude;
				float dx = px - entity->placeable.m_matrix.px;
				float dy = py - entity->placeable.m_matrix.py;
				float dz = pz - entity->placeable.m_matrix.pz;
				if (check2DOnly)
					fMagnitude = dx * dx + dy * dy;
				else
					fMagnitude = dx * dx + dy * dy + dz * dz;
				if (fMagnitude < radius * radius && *numFound < maxToFind) {
					if (entities)
						entities[*numFound] = entity;
					++*numFound;
				}
			}
		}
	}
}

void World_FindObjectsIntersectingCubeInList(CPtrList *list, uint16_t scanCode, const float start[3], const float end[3], int16_t *numIntersecting, int16_t maxToFind, void **entities)
{
	CPtrNode *node;
	for (node = list->first; node; node = node->next) {
		CEntityStruct *entity = (CEntityStruct *)node->item;
		if (entity->entity.m_scanCode != scanCode) {
			// Test the bounding sphere extent against the box on each axis
			float fRadius;
			float x, y, z;
			entity->entity.m_scanCode = scanCode;
			fRadius = Entity_GetBoundRadius(&entity->entity);
			x = entity->placeable.m_matrix.px;
			y = entity->placeable.m_matrix.py;
			z = entity->placeable.m_matrix.pz;
			if (fRadius + x >= start[0] && x - fRadius <= end[0] &&
			    fRadius + y >= start[1] && y - fRadius <= end[1] &&
			    fRadius + z >= start[2] && z - fRadius <= end[2] &&
			    *numIntersecting < maxToFind) {
				if (entities)
					entities[*numIntersecting] = entity;
				++*numIntersecting;
			}
		}
	}
}

void World_FindObjectsKindaCollidingInList(CPtrList *list, uint16_t scanCode, float px, float py, float pz, float radius, bool check2DOnly, int16_t *numColliding, int16_t maxToFind, void **entities)
{
	CPtrNode *node;
	for (node = list->first; node; node = node->next) {
		CEntityStruct *entity = (CEntityStruct *)node->item;
		if (entity->entity.m_scanCode != scanCode) {
			// Compare the distance to the point with the bounding radius plus the query radius
			float fMagnitude;
			float dx = px - entity->placeable.m_matrix.px;
			float dy = py - entity->placeable.m_matrix.py;
			float dz = pz - entity->placeable.m_matrix.pz;
			entity->entity.m_scanCode = scanCode;
			if (check2DOnly)
				fMagnitude = sqrtf(dx * dx + dy * dy);
			else
				fMagnitude = sqrtf(dx * dx + dy * dy + dz * dz);
			if (Entity_GetBoundRadius(&entity->entity) + radius > fMagnitude && *numColliding < maxToFind) {
				if (entities)
					entities[*numColliding] = entity;
				++*numColliding;
			}
		}
	}
}

void World_FindMissionEntitiesIntersectingCubeInList(CPtrList *list, uint16_t scanCode, const float start[3], const float end[3], int16_t *numIntersecting, int16_t maxToFind, void **entities, bool isVehicleList, bool isPedList)
{
	CPtrNode *node;
	for (node = list->first; node; node = node->next) {
		CEntityStruct *entity = (CEntityStruct *)node->item;
		if (entity->entity.m_scanCode != scanCode) {
			bool isMissionEntity = false;
			float fRadius;
			float x, y, z;
			entity->entity.m_scanCode = scanCode;
			// Whether the entity was created by a mission depends on its kind
			if (isVehicleList)
				isMissionEntity = ((CVehicleStruct *)entity)->vehicle.VehicleCreatedBy == MISSION_VEHICLE;
			else if (isPedList)
				isMissionEntity = ((CPedStruct *)entity)->ped.CharCreatedBy == MISSION_CHAR;
			else
				isMissionEntity = ((CObjectStruct *)entity)->object.ObjectCreatedBy == MISSION_OBJECT;
			fRadius = Entity_GetBoundRadius(&entity->entity);
			x = entity->placeable.m_matrix.px;
			y = entity->placeable.m_matrix.py;
			z = entity->placeable.m_matrix.pz;
			if (isMissionEntity && fRadius + x >= start[0] &&
			    x - fRadius <= end[0] && fRadius + y >= start[1] &&
			    y - fRadius <= end[1] && fRadius + z >= start[2] &&
			    z - fRadius <= end[2] && *numIntersecting < maxToFind) {
				if (entities)
					entities[*numIntersecting] = entity;
				++*numIntersecting;
			}
		}
	}
}

int World_SelectPedsToClearFromArea(const CPool *pedPool, float x1, float y1, float z1, float x2, float y2, float z2, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = 0; i < CPool_GetSize(pedPool); i++) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, i);
		if (ped) {
			const CMatrixData *m = &ped->base.base.placeable.m_matrix;
			if (!PedLogic_IsPlayer(&ped->ped) && PedLogic_CanBeDeleted(&ped->ped) && m->px >= x1 && m->px <= x2 &&
			    m->py >= y1 && m->py <= y2 && m->pz >= z1 && m->pz <= z2) {
				if (n < maxOut)
					out[n] = ped;
				n++;
			}
		}
	}
	return n;
}

int World_SelectCarsToClearFromArea(const CPool *vehiclePool, float x1, float y1, float z1, float x2, float y2, float z2, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = 0; i < CPool_GetSize(vehiclePool); i++) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, i);
		if (veh) {
			const CMatrixData *m = &veh->base.base.placeable.m_matrix;
			if (m->px >= x1 && m->px <= x2 && m->py >= y1 && m->py <= y2 &&
			    m->pz >= z1 && m->pz <= z2 && !veh->vehicle.bIsLocked && VehicleLogic_CanBeDeleted(&veh->vehicle)) {
				if (n < maxOut)
					out[n] = veh;
				n++;
			}
		}
	}
	return n;
}

int World_SelectCarsToSetOnFire(const CPool *vehiclePool, float x, float y, float z, float radius, void **out, int maxOut)
{
	int n = 0;
	int poolIndex;
	for (poolIndex = CPool_GetSize(vehiclePool) - 1; poolIndex >= 0; poolIndex--) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, poolIndex);
		if (veh && veh->base.base.entity.m_status != STATUS_WRECKED && !veh->vehicle.m_pCarFire && !veh->base.base.entity.bFireProof) {
			const CMatrixData *m = &veh->base.base.placeable.m_matrix;
			if (fabsf(m->pz - z) < 5.0f && fabsf(m->px - x) < radius &&
			    fabsf(m->py - y) < radius) {
				if (n < maxOut)
					out[n] = veh;
				n++;
			}
		}
	}
	return n;
}

int World_SelectPedsToSetOnFire(const CPool *pedPool, float x, float y, float z, float radius, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = CPool_GetSize(pedPool) - 1; i >= 0; i--) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, i);
		if (ped && ped->ped.m_nPedState != PED_DEAD && !ped->ped.bInVehicle && !ped->ped.m_pFire && !ped->base.base.entity.bFireProof) {
			const CMatrixData *m = &ped->base.base.placeable.m_matrix;
			if (fabsf(m->pz - z) < 5.0f && fabsf(m->px - x) < radius &&
			    fabsf(m->py - y) < radius) {
				if (n < maxOut)
					out[n] = ped;
				n++;
			}
		}
	}
	return n;
}

int World_SelectPedsToChoke(const CPool *pedPool, float x, float y, float z, float radius, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = CPool_GetSize(pedPool) - 1; i >= 0; i--) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, i);
		if (ped && ped->ped.m_nPedState != PED_DEAD && !ped->ped.bInVehicle && !ped->ped.m_pFire && !ped->base.base.entity.bFireProof && ped->ped.CharCreatedBy != MISSION_CHAR) {
			const CMatrixData *m = &ped->base.base.placeable.m_matrix;
			if (fabsf(m->pz - z) < 5.0f && fabsf(m->px - x) < radius &&
			    fabsf(m->py - y) < radius) {
				if (n < maxOut)
					out[n] = ped;
				n++;
			}
		}
	}
	return n;
}

int World_SelectPedsToCallOffChase(CPtrList *list, uint16_t scanCode, float x1, float y1, float x2, float y2, const void *playerPed, const void *playerVehicle, void **out, int maxOut)
{
	int n = 0;
	CPtrNode *node;
	for (node = list->first; node; node = node->next) {
		CPedStruct *ped = (CPedStruct *)node->item;
		const CMatrixData *m = &ped->base.base.placeable.m_matrix;
		if (ped->base.base.entity.m_scanCode != scanCode) {
			ped->base.base.entity.m_scanCode = scanCode;
			if ((const void *)ped != playerPed && (const void *)ped->ped.m_leader != playerPed && m->px > x1 &&
			    m->px < x2 && m->py > y1 && m->py < y2 &&
			    ((const void *)ped->ped.m_pedInObjective == playerPed ||
			     (ped->ped.m_carInObjective && (const void *)ped->ped.m_carInObjective == playerVehicle)) &&
			    ped->ped.m_nPedState != PED_DEAD && ped->ped.m_nPedState != PED_DIE &&
			    (ped->ped.m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT ||
			     ped->ped.m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER ||
			     ped->ped.m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS)) {
				if (n < maxOut)
					out[n] = ped;
				n++;
			}
		}
	}
	return n;
}

void World_CallOffChaseForVehiclesInList(CPtrList *list, uint16_t scanCode, float x1, float y1, float x2, float y2, float startX, float startY, float endX, float endY, const void *playerVehicle, uint32_t timeMs)
{
	CPtrNode *node;
	for (node = list->first; node; node = node->next) {
		CVehicleStruct *vehicle = (CVehicleStruct *)node->item;
		CEntityStruct *ent = &vehicle->base.base;
		if (ent->entity.m_scanCode != scanCode) {
			const CMatrixData *vehiclePos = &ent->placeable.m_matrix;
			uint8_t carMission = vehicle->vehicle.AutoPilot.m_nCarMission;
			ent->entity.m_scanCode = scanCode;
			if ((const void *)vehicle != playerVehicle && vehiclePos->px > startX && vehiclePos->px < endX &&
			    vehiclePos->py > startY && vehiclePos->py < endY && vehicle->vehicle.bIsLawEnforcer &&
			    (carMission == MISSION_RAMPLAYER_FARAWAY || carMission == MISSION_RAMPLAYER_CLOSE ||
			     carMission == MISSION_BLOCKPLAYER_FARAWAY || carMission == MISSION_BLOCKPLAYER_CLOSE)) {
				CColModel *pColModel;
				bool bInsideSphere = false;
				int32_t i;
				vehicle->vehicle.AutoPilot.m_nTimeTempAction = timeMs + 2000;
				pColModel = (CColModel *)ModelRegistry_Collision(ModelRegistry_Get(ent->entity.m_modelIndex));
				for (i = 0; i < pColModel->numSpheres; i++) {
					VectorMath3 c = {pColModel->spheres[i].bounds.center.x, pColModel->spheres[i].bounds.center.y, pColModel->spheres[i].bounds.center.z};
					VectorMath3 pos = VectorMath_MatrixTimesVector(ent->placeable.m_matrix.f, c);
					float fRadius = pColModel->spheres[i].bounds.radius;
					if (pos.x + fRadius > x1 && pos.x - fRadius < x2 && pos.y + fRadius > y1 &&
					    pos.y - fRadius < y2)
						bInsideSphere = true;
				}
				if (bInsideSphere) {
					PhysicalVec *speed = &vehicle->base.physical.m_vecMoveSpeed;
					if (vehiclePos->px <= (x1 + x2) * 0.5f)
						speed->x = speed->x < 0.0f ? speed->x : 0.0f;
					else
						speed->x = speed->x > 0.0f ? speed->x : 0.0f;
					if (vehiclePos->py <= (y1 + y2) * 0.5f)
						speed->y = speed->y < 0.0f ? speed->y : 0.0f;
					else
						speed->y = speed->y > 0.0f ? speed->y : 0.0f;
				}
			}
		}
	}
}

int World_SelectFallenPeds(const CPool *pedPool, float lowLimit, void **out, int *action, int maxOut)
{
	int n = 0;
	int poolIndex;
	for (poolIndex = CPool_GetSize(pedPool) - 1; poolIndex >= 0; poolIndex--) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, poolIndex);
		if (ped) {
			if (ped->base.base.placeable.m_matrix.pz < lowLimit) {
				// Mission peds and players are brought back, random peds are removed
				int act = (ped->ped.CharCreatedBy != RANDOM_CHAR || PedLogic_IsPlayer(&ped->ped)) ? 1 : 2;
				if (n < maxOut) {
					out[n] = ped;
					action[n] = act;
				}
				n++;
			}
		}
	}
	return n;
}

int World_SelectFallenCars(const CPool *vehiclePool, float lowLimit, const void *playerVehicle, void **out, int *action, int maxOut)
{
	int n = 0;
	int poolIndex;
	for (poolIndex = CPool_GetSize(vehiclePool) - 1; poolIndex >= 0; poolIndex--) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, poolIndex);
		if (veh) {
			if (veh->base.base.placeable.m_matrix.pz < lowLimit) {
				int act = 0;
				if ((veh->vehicle.VehicleCreatedBy == MISSION_VEHICLE && !veh->base.base.entity.bRenderScorched) || (const void *)veh == playerVehicle ||
				    (veh->vehicle.pDriver && PedLogic_IsPlayer(&((CPedStruct *)veh->vehicle.pDriver)->ped)))
					act = 1;
				else if (veh->vehicle.VehicleCreatedBy == RANDOM_VEHICLE ||
				         veh->vehicle.VehicleCreatedBy == PARKED_VEHICLE)
					act = 2;
				if (act) {
					if (n < maxOut) {
						out[n] = veh;
						action[n] = act;
					}
					n++;
				}
			}
		}
	}
	return n;
}

void World_UseDetonator(const CPool *vehiclePool, void *entity)
{
	int32_t i = CPool_GetSize(vehiclePool);
	while (--i >= 0) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, i);
		if (veh && veh->vehicle.m_bombType == CARBOMB_REMOTE &&
		    (void *)veh->vehicle.m_pBombRigger == entity) {
			// Disarm the bomb and set it to explode soon, blamed on the rigger
			veh->vehicle.m_bombType = CARBOMB_NONE;
			veh->vehicle.m_nBombTimer = 500;
			veh->vehicle.m_pBlowUpEntity = veh->vehicle.m_pBombRigger;
			if (veh->vehicle.m_pBlowUpEntity)
				References_RegisterReference(veh->vehicle.m_pBlowUpEntity, &veh->vehicle.m_pBlowUpEntity);
		}
	}
}

void World_RemoveRefsToDeletedFromPeds(const CPool *pedPool, void *deleted)
{
	int32_t i = CPool_GetSize(pedPool);
	while (--i >= 0) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, i);
		if (ped && (void *)ped != deleted) {
			// Drop the collision records and the surface the ped stands on
			Physical_RemoveRefsToEntity(&ped->base.physical, deleted);
			if ((void *)ped->ped.m_pCurrentPhysSurface == deleted)
				ped->ped.m_pCurrentPhysSurface = NULL;
		}
	}
}

void World_RemoveRefsToDeletedFromObjects(const CPool *objectPool, void *deleted)
{
	int32_t i = CPool_GetSize(objectPool);
	while (--i >= 0) {
		CObjectStruct *object = (CObjectStruct *)CPool_GetSlot(objectPool, i);
		if (object && (void *)object != deleted)
			Physical_RemoveRefsToEntity(&object->base.physical, deleted);
	}
}

int World_SelectPedsInCircleToClear(const CPool *pedPool, float cx, float cy, float radius, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = 0; i < CPool_GetSize(pedPool); i++) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, i);
		if (ped && !PedLogic_IsPlayer(&ped->ped) && PedLogic_CanBeDeleted(&ped->ped)) {
			float dx = ped->base.base.placeable.m_matrix.px - cx;
			float dy = ped->base.base.placeable.m_matrix.py - cy;
			if (dx * dx + dy * dy < radius * radius) {
				if (n < maxOut)
					out[n] = ped;
				n++;
			}
		}
	}
	return n;
}

int World_SelectCarsInCircleToClear(const CPool *vehiclePool, float cx, float cy, float radius, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = 0; i < CPool_GetSize(vehiclePool); i++) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, i);
		if (veh) {
			float dx = veh->base.base.placeable.m_matrix.px - cx;
			float dy = veh->base.base.placeable.m_matrix.py - cy;
			if (dx * dx + dy * dy < radius * radius &&
			    !veh->vehicle.bIsLocked && VehicleLogic_CanBeDeleted(&veh->vehicle)) {
				if (n < maxOut)
					out[n] = veh;
				n++;
			}
		}
	}
	return n;
}

int World_SelectCarsInRangeOfPoint(const CPool *vehiclePool, float x, float y, float z, float range, void **out, int maxOut)
{
	int n = 0;
	int poolIndex;
	for (poolIndex = 0; poolIndex < CPool_GetSize(vehiclePool); poolIndex++) {
		CVehicleStruct *veh = (CVehicleStruct *)CPool_GetSlot(vehiclePool, poolIndex);
		if (veh) {
			float dx = x - veh->base.base.placeable.m_matrix.px;
			float dy = y - veh->base.base.placeable.m_matrix.py;
			float dz = z - veh->base.base.placeable.m_matrix.pz;
			if (dx * dx + dy * dy + dz * dz < range * range) {
				if (n < maxOut)
					out[n] = veh;
				n++;
			}
		}
	}
	return n;
}
void World_SectorRange(float startX, float startY, float endX, float endY, int32_t *sectorStartX, int32_t *sectorStartY, int32_t *sectorEndX, int32_t *sectorEndY)
{
	int32_t sx = (int32_t)((startX - WORLD_MIN_X) / SECTOR_SIZE_X);
	int32_t sy = (int32_t)((startY - WORLD_MIN_Y) / SECTOR_SIZE_Y);
	int32_t ex = (int32_t)((endX - WORLD_MIN_X) / SECTOR_SIZE_X);
	int32_t ey = (int32_t)((endY - WORLD_MIN_Y) / SECTOR_SIZE_Y);

	// Start at the first sector at the least, end at the last one or one past it without the bug fix
	*sectorStartX = sx > 0 ? sx : 0;
	*sectorStartY = sy > 0 ? sy : 0;
#ifdef FIX_BUGS
	*sectorEndX = ex < NUMSECTORS_X - 1 ? ex : NUMSECTORS_X - 1;
	*sectorEndY = ey < NUMSECTORS_Y - 1 ? ey : NUMSECTORS_Y - 1;
#else
	*sectorEndX = ex < NUMSECTORS_X ? ex : NUMSECTORS_X;
	*sectorEndY = ey < NUMSECTORS_Y ? ey : NUMSECTORS_Y;
#endif
}

void World_SectorRangeClamped(float startX, float startY, float endX, float endY, int32_t *sectorStartX, int32_t *sectorStartY, int32_t *sectorEndX, int32_t *sectorEndY)
{
	int32_t sx = (int32_t)((startX - WORLD_MIN_X) / SECTOR_SIZE_X);
	int32_t sy = (int32_t)((startY - WORLD_MIN_Y) / SECTOR_SIZE_Y);
	int32_t ex = (int32_t)((endX - WORLD_MIN_X) / SECTOR_SIZE_X);
	int32_t ey = (int32_t)((endY - WORLD_MIN_Y) / SECTOR_SIZE_Y);

	// Clamp to the sectors that exist
	*sectorStartX = sx < 0 ? 0 : sx;
	*sectorStartY = sy < 0 ? 0 : sy;
	*sectorEndX = ex >= NUMSECTORS_X ? NUMSECTORS_X - 1 : ex;
	*sectorEndY = ey >= NUMSECTORS_Y ? NUMSECTORS_Y - 1 : ey;
}

int World_SectorStreamRing(int32_t dx, int32_t dy)
{
	int32_t d = dx * dx + dy * dy;

	if (d <= 0)
		return 0;
	if (d <= 3 * 3)
		return 1;
	return -1;
}

float World_ExplosionDamageMultiplier(float radius, float magnitude)
{
	float v = (radius - magnitude) * 2.0f / radius;

	return v < 1.0f ? v : 1.0f;
}

void World_ExplosionForce(const float distance[3], float power, float mass, float damageMultiplier, float magnitude, bool isPlayer, float force[3])
{
	float m = magnitude > 0.01f ? magnitude : 0.01f;
	float k = power * mass / 1400.0f * damageMultiplier / m;

	force[0] = distance[0] * k;
	force[1] = distance[1] * k;
	force[2] = distance[2] * k;
	force[2] = force[2] > 0.0f ? force[2] : 0.0f;
	if (isPlayer)
		force[2] = force[2] < 1.0f ? force[2] : 1.0f;
}

float World_ExplosionTurnPointZ(float distanceZ, float boundRadius)
{
	float pointZ = boundRadius;
	float lower = distanceZ > -boundRadius ? distanceZ : -boundRadius;

	if (lower < boundRadius) {
		if (distanceZ <= -boundRadius)
			pointZ = -boundRadius;
		else
			pointZ = distanceZ;
	}
	return pointZ;
}
uint32_t World_WanderPathSteps(float length, int maxSteps)
{
	return (uint32_t)(length > maxSteps ? length : (float)maxSteps);
}
void World_ClampSectorRange(int lowIndex, int highIndex, int numSectors, int *start, int *end)
{
	*start = 0 > lowIndex ? 0 : lowIndex;
	*end = (numSectors - 1) < highIndex ? (numSectors - 1) : highIndex;
}
float World_FireDamage(float damageMultiplier)
{
	float m = damageMultiplier < 1.0f ? damageMultiplier : 1.0f;

	return 300.0f * m;
}
float World_HigherZ(float z1, float z2)
{
	return z1 > z2 ? z1 : z2;
}
//- rouz edit (Claude)
