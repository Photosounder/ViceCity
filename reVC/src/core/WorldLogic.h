//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "Lists.h"
#include "Pool.h"

#ifdef __cplusplus
extern "C" {
#endif
// World bookkeeping that walks entity lists and pools using the C views of the entity objects
void World_ClearScanCodesInList(CPtrList *list);
void World_SetAllCarsCanBeDamaged(const CPool *vehiclePool, bool toggle);
void World_StopAllLawEnforcersInTheirTracks(const CPool *vehiclePool);
// Collect entities of a sector list within a radius, the scan code stops an entity being visited twice per query
void World_FindObjectsInRangeInList(CPtrList *list, uint16_t scanCode, float cx, float cy, float cz, float radius, bool ignoreZ, int16_t *numObjects, int16_t lastObject, void **objects);
void World_FindObjectsOfTypeInRangeInList(uint32_t modelId, CPtrList *list, uint16_t scanCode, float px, float py, float pz, float radius, bool check2DOnly, int16_t *numFound, int16_t maxToFind, void **entities);

// Queries that use the bounding radius of the entity model
void World_FindObjectsIntersectingCubeInList(CPtrList *list, uint16_t scanCode, const float start[3], const float end[3], int16_t *numIntersecting, int16_t maxToFind, void **entities);
void World_FindObjectsKindaCollidingInList(CPtrList *list, uint16_t scanCode, float px, float py, float pz, float radius, bool check2DOnly, int16_t *numColliding, int16_t maxToFind, void **entities);

// Same as the cube query but only for mission entities of the list kind: vehicles, peds or objects
void World_FindMissionEntitiesIntersectingCubeInList(CPtrList *list, uint16_t scanCode, const float start[3], const float end[3], int16_t *numIntersecting, int16_t maxToFind, void **entities, bool isVehicleList, bool isPedList);

// Selectors for the area and fire loops: they only decide which entities are affected, the caller applies the effects in the returned order
int World_SelectPedsToClearFromArea(const CPool *pedPool, float x1, float y1, float z1, float x2, float y2, float z2, void **out, int maxOut);
int World_SelectCarsToClearFromArea(const CPool *vehiclePool, float x1, float y1, float z1, float x2, float y2, float z2, void **out, int maxOut);
// Peds of one sector list that must stop chasing the player inside the box, the scan code marks every visited ped
// Law enforcement vehicles of one sector list that chase the player stop pushing into the box, the whole effect is applied here
void World_CallOffChaseForVehiclesInList(CPtrList *list, uint16_t scanCode, float x1, float y1, float x2, float y2, float startX, float startY, float endX, float endY, const void *playerVehicle, uint32_t timeMs);
// Entities that fell below the map: the action is 1 to teleport the entity back on the paths and 2 to remove it
int World_SelectFallenPeds(const CPool *pedPool, float lowLimit, void **out, int *action, int maxOut);
int World_SelectFallenCars(const CPool *vehiclePool, float lowLimit, const void *playerVehicle, void **out, int *action, int maxOut);
// Blow up every vehicle whose remote bomb was planted by the entity, the detonator projectiles are removed by the caller
void World_UseDetonator(const CPool *vehiclePool, void *entity);
// Forget every reference peds and objects hold to a deleted entity, the vehicles are handled by the caller
void World_RemoveRefsToDeletedFromPeds(const CPool *pedPool, void *deleted);
void World_RemoveRefsToDeletedFromObjects(const CPool *objectPool, void *deleted);
// Selectors for the circular area loops, in pool order
int World_SelectPedsInCircleToClear(const CPool *pedPool, float cx, float cy, float radius, void **out, int maxOut);
int World_SelectCarsInCircleToClear(const CPool *vehiclePool, float cx, float cy, float radius, void **out, int maxOut);
int World_SelectCarsInRangeOfPoint(const CPool *vehiclePool, float x, float y, float z, float range, void **out, int maxOut);
int World_SelectPedsToCallOffChase(CPtrList *list, uint16_t scanCode, float x1, float y1, float x2, float y2, const void *playerPed, const void *playerVehicle, void **out, int maxOut);
int World_SelectPedsToSetOnFire(const CPool *pedPool, float x, float y, float z, float radius, void **out, int maxOut);
int World_SelectPedsToChoke(const CPool *pedPool, float x, float y, float z, float radius, void **out, int maxOut);
int World_SelectCarsToSetOnFire(const CPool *vehiclePool, float x, float y, float z, float radius, void **out, int maxOut);

// Range of sectors a box covers, clamped to the sectors that exist, the end indexes are inclusive
void World_SectorRange(float startX, float startY, float endX, float endY, int32_t *sectorStartX, int32_t *sectorStartY, int32_t *sectorEndX, int32_t *sectorEndY);

// Same as the sector range but the last sector is the end whether or not the bug fix is used
void World_SectorRangeClamped(float startX, float startY, float endX, float endY, int32_t *sectorStartX, int32_t *sectorStartY, int32_t *sectorEndX, int32_t *sectorEndY);
// Streaming ring of a sector around the one the player is in, from the sector offsets: 0 for the sector itself, 1 for the sectors up to 3 away and -1 for the rest
int World_SectorStreamRing(int32_t dx, int32_t dy);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
