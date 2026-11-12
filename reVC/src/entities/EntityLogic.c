//+ rouz edit (Claude)
#include <stddef.h>
#include "EntityLogic.h"
#include "EntityTypes.h"
#include "EntityStatus.h"
#include "GameAreas.h"
#include "config.h"
#include "ModelRegistry.h"
#include "ColModel.h"
#include "VectorMath.h"
#include "CameraMath.h"

void re3_assert(const char *expression, const char *file, unsigned int line, const char *function);
#ifndef MASTER
#define Entity_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define Entity_Assert(expression) (expression)
#endif

void Entity_Init(CEntityData *self)
{
	// A new entity is of no type and abandoned
	self->m_type = ENTITY_TYPE_NOTHING;
	self->m_status = STATUS_ABANDONED;

	// All the flags start cleared except visibility
	self->bUsesCollision = false;
	self->bCollisionProcessed = false;
	self->bIsStatic = false;
	self->bHasContacted = false;
	self->bPedPhysics = false;
	self->bIsStuck = false;
	self->bIsInSafePosition = false;
	self->bUseCollisionRecords = false;
	self->bWasPostponed = false;
	self->bExplosionProof = false;
	self->bHasCollided = false;
	self->bRenderScorched = false;
	self->bHasBlip = false;
	self->bIsBIGBuilding = false;
	self->bStreamBIGBuilding = false;
	self->bRenderDamaged = false;
	self->bBulletProof = false;
	self->bFireProof = false;
	self->bCollisionProof = false;
	self->bMeleeProof = false;
	self->bOnlyDamagedByPlayer = false;
	self->bStreamingDontDelete = false;
	self->bRemoveFromWorld = false;
	self->bHasHitWall = false;
	self->bImBeingRendered = false;
	self->bTouchingWater = false;
	self->bIsSubway = false;
	self->bDrawLast = false;
	self->bNoBrightHeadLights = false;
	self->bDoNotRender = false;
	self->bDistanceFade = false;
	self->m_flagE1 = false;
	self->bDontCastShadowsOn = false;
	self->bOffscreen = false;
	self->bIsStaticWaitingForCollision = false;
	self->bDontStream = false;
	self->bUnderwater = false;
	self->bHasPreRenderEffects = false;
	self->bDrawFarAway = false;
	self->bIsVisible = true;

	// No model, no render object, main map area and no references
	self->m_scanCode = 0;
	self->m_modelIndex = -1;
	self->m_rwObject = NULL;
	self->m_area = AREA_MAIN_MAP;
	self->m_pFirstReference = NULL;
}

bool Entity_IsBuilding(const CEntityData *self)
{
	// Type check
	return self->m_type == ENTITY_TYPE_BUILDING;
}

bool Entity_IsVehicle(const CEntityData *self)
{
	// Type check
	return self->m_type == ENTITY_TYPE_VEHICLE;
}

bool Entity_IsPed(const CEntityData *self)
{
	// Type check
	return self->m_type == ENTITY_TYPE_PED;
}

bool Entity_IsObject(const CEntityData *self)
{
	// Type check
	return self->m_type == ENTITY_TYPE_OBJECT;
}

bool Entity_IsDummy(const CEntityData *self)
{
	// Type check
	return self->m_type == ENTITY_TYPE_DUMMY;
}

bool Entity_GetIsStatic(const CEntityData *self)
{
	// Static either permanently or until the collision below it is loaded
	return self->bIsStatic || self->bIsStaticWaitingForCollision;
}

float Entity_GetBoundRadius(const CEntityData *self)
{
	// The model registry gives the collision model whose bounding sphere is the entity bound
	return ((CColModel *)ModelRegistry_Collision(ModelRegistry_Get(self->m_modelIndex)))->boundingSphere.radius;
}

static CColModel *EntityColModel(const CEntityData *self)
{
	// The collision model of the entity model
	return (CColModel *)ModelRegistry_Collision(ModelRegistry_Get(self->m_modelIndex));
}

void Entity_GetBoundCentre(const CPlaceableData *placeable, const CEntityData *self, float out[3])
{
	// Transform the sphere centre by the entity matrix
	CollisionVector3 centre = EntityColModel(self)->boundingSphere.center;
	VectorMath3 v = {centre.x, centre.y, centre.z};
	v = VectorMath_MatrixTimesVector(placeable->m_matrix.f, v);
	out[0] = v.x;
	out[1] = v.y;
	out[2] = v.z;
}

bool Entity_GetIsTouching(const CPlaceableData *placeable, const CEntityData *self, float cx, float cy, float cz, float radius)
{
	// Compare the squared distance between the centres with the squared sum of the radii
	float bc[3];
	float dx, dy, dz, r;
	Entity_GetBoundCentre(placeable, self, bc);
	dx = bc[0] - cx;
	dy = bc[1] - cy;
	dz = bc[2] - cz;
	r = Entity_GetBoundRadius(self) + radius;
	return r * r > dx * dx + dy * dy + dz * dz;
}

float Entity_GetDistanceFromCentreOfMassToBaseOfModel(const CEntityData *self)
{
	// Negated minimum Z of the bounding box
	return -EntityColModel(self)->boundingBox.min.z;
}

CRect Entity_GetBoundRect(const CPlaceableData *placeable, const CEntityData *self)
{
	// Grow a rectangle over the four transformed bounding box corners that matter in 2D
	CRect rect;
	CBox box = EntityColModel(self)->boundingBox;
	VectorMath3 p;
	Rect_Init(&rect);

	p.x = box.min.x; p.y = box.min.y; p.z = box.min.z;
	p = VectorMath_MatrixTimesVector(placeable->m_matrix.f, p);
	Rect_ContainPoint(&rect, &p.x, &p.y);

	p.x = box.max.x; p.y = box.max.y; p.z = box.max.z;
	p = VectorMath_MatrixTimesVector(placeable->m_matrix.f, p);
	Rect_ContainPoint(&rect, &p.x, &p.y);

	p.x = box.max.x; p.y = box.min.y; p.z = box.min.z;
	p = VectorMath_MatrixTimesVector(placeable->m_matrix.f, p);
	Rect_ContainPoint(&rect, &p.x, &p.y);

	p.x = box.min.x; p.y = box.max.y; p.z = box.max.z;
	p = VectorMath_MatrixTimesVector(placeable->m_matrix.f, p);
	Rect_ContainPoint(&rect, &p.x, &p.y);

	return rect;
}

CPtrList *Entity_SectorListFor(CSector *s, int type, bool middle)
{
	// The main list of the middle sector, the overlap list of the other sectors the entity touches
	switch (type) {
	case ENTITY_TYPE_BUILDING:
		return &s->m_lists[middle ? ENTITYLIST_BUILDINGS : ENTITYLIST_BUILDINGS_OVERLAP];
	case ENTITY_TYPE_VEHICLE:
		return &s->m_lists[middle ? ENTITYLIST_VEHICLES : ENTITYLIST_VEHICLES_OVERLAP];
	case ENTITY_TYPE_PED:
		return &s->m_lists[middle ? ENTITYLIST_PEDS : ENTITYLIST_PEDS_OVERLAP];
	case ENTITY_TYPE_OBJECT:
		return &s->m_lists[middle ? ENTITYLIST_OBJECTS : ENTITYLIST_OBJECTS_OVERLAP];
	case ENTITY_TYPE_DUMMY:
		return &s->m_lists[middle ? ENTITYLIST_DUMMIES : ENTITYLIST_DUMMIES_OVERLAP];
	}
	return NULL;
}

EntitySectorRange Entity_ComputeSectorRange(CRect bounds)
{
	// Sector index range covered by the bounding rectangle and its middle
	EntitySectorRange r;
	r.xstart = (int)((bounds.left - WORLD_MIN_X) / SECTOR_SIZE_X);
	r.xend   = (int)((bounds.right - WORLD_MIN_X) / SECTOR_SIZE_X);
	r.xmid   = (int)(((bounds.left + bounds.right) / 2.0f - WORLD_MIN_X) / SECTOR_SIZE_X);
	r.ystart = (int)((bounds.top - WORLD_MIN_Y) / SECTOR_SIZE_Y);
	r.yend   = (int)((bounds.bottom - WORLD_MIN_Y) / SECTOR_SIZE_Y);
	r.ymid   = (int)(((bounds.top + bounds.bottom) / 2.0f - WORLD_MIN_Y) / SECTOR_SIZE_Y);
	Entity_Assert(r.xstart >= 0);
	Entity_Assert(r.xend < NUMSECTORS_X);
	Entity_Assert(r.ystart >= 0);
	Entity_Assert(r.yend < NUMSECTORS_Y);
	return r;
}

CSector *Entity_SectorAt(CSector *sectors, int x, int y)
{
	// Out of range indices fall back to the first sector like the world accessor does
	return (x > NUMSECTORS_X - 1 || y > NUMSECTORS_Y - 1) ? &sectors[0] : &sectors[y * NUMSECTORS_X + x];
}

static void EntitySectorsApply(void *item, const CPlaceableData *placeable, const CEntityData *self, CSector *sectors, bool add)
{
	int x, y;
	EntitySectorRange r = Entity_ComputeSectorRange(Entity_GetBoundRect(placeable, self));

	for (y = r.ystart; y <= r.yend; y++)
		for (x = r.xstart; x <= r.xend; x++) {
			CSector *s = Entity_SectorAt(sectors, x, y);
			CPtrList *list = Entity_SectorListFor(s, self->m_type, x == r.xmid && y == r.ymid);
			if (add)
				PtrList_InsertItem(list, item);
			else
				PtrList_RemoveItem(list, item);
		}
}

void Entity_AddToSectors(void *item, const CPlaceableData *placeable, const CEntityData *self, CSector *sectors)
{
	// Register the entity in the sector lists
	EntitySectorsApply(item, placeable, self, sectors, true);
}

void Entity_RemoveFromSectors(void *item, const CPlaceableData *placeable, const CEntityData *self, CSector *sectors)
{
	// Unregister the entity from the sector lists
	EntitySectorsApply(item, placeable, self, sectors, false);
}

bool Entity_GetIsOnScreen(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4])
{
	// The bounding sphere against the view frustum
	float bc[3];
	VectorMath3 centre;
	Entity_GetBoundCentre(placeable, self, bc);
	centre.x = bc[0];
	centre.y = bc[1];
	centre.z = bc[2];
	return CameraMath_IsSphereVisible(normals, centre, Entity_GetBoundRadius(self), cameraMatrix);
}

bool Entity_GetIsOnScreenComplex(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4])
{
	VectorMath3 boundBox[8];
	float bc[3];
	VectorMath3 centre;
	CRect rect;
	CColModel *colmodel;
	float z, minz, maxz;

	// A visible bounding centre is enough
	Entity_GetBoundCentre(placeable, self, bc);
	centre.x = bc[0];
	centre.y = bc[1];
	centre.z = bc[2];
	if (CameraMath_IsPointVisible(normals, centre, cameraMatrix))
		return true;

	// Otherwise test the corners of the world rectangle at the bottom and top of the model
	rect = Entity_GetBoundRect(placeable, self);
	colmodel = EntityColModel(self);
	z = placeable->m_matrix.pz;
	minz = z + colmodel->boundingBox.min.z;
	maxz = z + colmodel->boundingBox.max.z;
	boundBox[0].x = rect.left;
	boundBox[0].y = rect.bottom;
	boundBox[0].z = minz;
	boundBox[1].x = rect.left;
	boundBox[1].y = rect.top;
	boundBox[1].z = minz;
	boundBox[2].x = rect.right;
	boundBox[2].y = rect.bottom;
	boundBox[2].z = minz;
	boundBox[3].x = rect.right;
	boundBox[3].y = rect.top;
	boundBox[3].z = minz;
	boundBox[4].x = rect.left;
	boundBox[4].y = rect.bottom;
	boundBox[4].z = maxz;
	boundBox[5].x = rect.left;
	boundBox[5].y = rect.top;
	boundBox[5].z = maxz;
	boundBox[6].x = rect.right;
	boundBox[6].y = rect.bottom;
	boundBox[6].z = maxz;
	boundBox[7].x = rect.right;
	boundBox[7].y = rect.top;
	boundBox[7].z = maxz;

	return CameraMath_IsBoxVisible(normals, boundBox, cameraMatrix);
}

bool Entity_IsVisible(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4])
{
	// Needs a render object that is visible and on screen
	return self->m_rwObject && self->bIsVisible && Entity_GetIsOnScreen(placeable, self, normals, cameraMatrix);
}

bool Entity_IsVisibleComplex(const CPlaceableData *placeable, const CEntityData *self, const VectorMath3 normals[4], const float cameraMatrix[4][4])
{
	// Same with the complex screen test
	return self->m_rwObject && self->bIsVisible && Entity_GetIsOnScreenComplex(placeable, self, normals, cameraMatrix);
}
//- rouz edit (Claude)
