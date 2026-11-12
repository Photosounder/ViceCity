//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "PlaceableData.h"
#include "EntityData.h"
#include "Rect.h"
#include "WorldSectors.h"
#include "VectorMath.h"

#ifdef __cplusplus
extern "C" {
#endif
// Set every entity flag and field to its freshly created value, the random seed is assigned by the caller
void Entity_Init(CEntityData *self);
// Entity type helpers
bool Entity_IsBuilding(const CEntityData *self);
bool Entity_IsVehicle(const CEntityData *self);
bool Entity_IsPed(const CEntityData *self);
bool Entity_IsObject(const CEntityData *self);
bool Entity_IsDummy(const CEntityData *self);
bool Entity_GetIsStatic(const CEntityData *self);
// Radius of the bounding sphere of the entity collision model
float Entity_GetBoundRadius(const CEntityData *self);
// Centre of the bounding sphere in world space
void Entity_GetBoundCentre(const CPlaceableData *placeable, const CEntityData *self, float out[3]);
// Whether a sphere touches the bounding sphere of the entity
bool Entity_GetIsTouching(const CPlaceableData *placeable, const CEntityData *self, float cx, float cy, float cz, float radius);
// Height of the lowest point of the model below the origin
float Entity_GetDistanceFromCentreOfMassToBaseOfModel(const CEntityData *self);
// Axis aligned world rectangle containing the transformed bounding box
CRect Entity_GetBoundRect(const CPlaceableData *placeable, const CEntityData *self);
// Screen visibility against the camera: its four frustum normals and its matrix
bool Entity_GetIsOnScreen(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4]);
bool Entity_GetIsOnScreenComplex(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4]);
bool Entity_IsVisible(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4]);
bool Entity_IsVisibleComplex(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4]);
// Insert or remove an entity in every sector its bounding rectangle covers, the sector array is row major with NUMSECTORS_X columns
void Entity_AddToSectors(void *item, const CPlaceableData *placeable, const CEntityData *self, CSector *sectors);
void Entity_RemoveFromSectors(void *item, const CPlaceableData *placeable, const CEntityData *self, CSector *sectors);

// Helpers shared by the entity classes that register in the sectors: the covered sector range and the list for a sector
typedef struct EntitySectorRange { int xstart, xend, xmid, ystart, yend, ymid; } EntitySectorRange;
EntitySectorRange Entity_ComputeSectorRange(CRect bounds);
CPtrList *Entity_SectorListFor(CSector *sector, int type, bool middle);
CSector *Entity_SectorAt(CSector *sectors, int x, int y);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
