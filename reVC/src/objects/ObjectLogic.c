//+ rouz edit (Claude)
#include <stddef.h>
#include "ObjectLogic.h"
#include "EntityTypes.h"
#include "GameAreas.h"

void Object_InitFields(CObjectStruct *self)
{
	CEntityData *e = &self->base.base.entity;
	CObjectState *o = &self->object;

	e->m_type = ENTITY_TYPE_OBJECT;
	o->m_fUprootLimit = 0.0f;
	o->m_nCollisionDamageEffect = 0;
	o->m_nSpecialCollisionResponseCases = COLLRESPONSE_NONE;
	o->m_bCameraToAvoidThisObject = false;
	o->ObjectCreatedBy = UNKNOWN_OBJECT;
	o->m_nEndOfLifeTime = 0;
	o->m_colour2 = 0;
	o->m_colour1 = o->m_colour2;
	o->m_nBonusValue = 0;
	o->m_nCostValue = 0;
	o->bIsPickup = false;
	o->bPickupObjWithMessage = false;
	o->bOutOfStock = false;
	o->bGlassCracked = false;
	o->bGlassBroken = false;
	o->bHasBeenDamaged = false;
	o->m_nRefModelIndex = -1;
	o->bUseVehicleColours = false;
	o->m_pCurSurface = NULL;
	o->m_pCollidingEntity = NULL;
	o->m_nBeachballBounces = 0;
	o->bIsStreetLight = false;
	e->m_area = AREA_EVERYWHERE;
}

bool Object_CanBeDeleted(const CObjectState *self)
{
	// Mission, cutscene and controlled sub objects are kept
	switch (self->ObjectCreatedBy) {
		case GAME_OBJECT:
			return true;
		case MISSION_OBJECT:
			return false;
		case TEMP_OBJECT:
			return true;
		case CUTSCENE_OBJECT:
			return false;
		case CONTROLLED_SUB_OBJECT:
			return false;
		default:
			return true;
	}
}

int Object_SelectByCreation(const CPool *objectPool, int creationMode, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = 0; i < CPool_GetSize(objectPool); i++) {
		CObjectStruct *object = (CObjectStruct *)CPool_GetSlot(objectPool, i);
		if (object && object->object.ObjectCreatedBy == creationMode) {
			if (n < maxOut)
				out[n] = object;
			n++;
		}
	}
	return n;
}

int Object_SelectTempInArea(const CPool *objectPool, float x, float y, float z, float radius, void **out, int maxOut)
{
	int n = 0;
	int i;
	for (i = 0; i < CPool_GetSize(objectPool); i++) {
		CObjectStruct *object = (CObjectStruct *)CPool_GetSlot(objectPool, i);
		if (object && object->object.ObjectCreatedBy == TEMP_OBJECT) {
			const CMatrixData *m = &object->base.base.placeable.m_matrix;
			float dx = x - m->px;
			float dy = y - m->py;
			float dz = z - m->pz;
			if (dx * dx + dy * dy + dz * dz < radius * radius) {
				if (n < maxOut)
					out[n] = object;
				n++;
			}
		}
	}
	return n;
}

int Object_SelectDeletable(const CPool *objectPool, void **out, int maxOut)
{
	int n = 0;
	uint32_t i = (uint32_t)CPool_GetSize(objectPool);
	while (i--) {
		CObjectStruct *object = (CObjectStruct *)CPool_GetSlot(objectPool, (int32_t)i);
		if (object && Object_CanBeDeleted(&object->object)) {
			if (n < maxOut)
				out[n] = object;
			n++;
		}
	}
	return n;
}
//- rouz edit (Claude)
