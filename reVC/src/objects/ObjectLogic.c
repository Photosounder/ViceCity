//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "ObjectLogic.h"
#include "EntityTypes.h"
#include "GameAreas.h"
#include "General.h"

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
static void SetColor(unsigned char *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;
}

void ObjectLogic_SmashDebrisColors(int damageEffect, unsigned char color1[4], unsigned char color2[4])
{
	switch (damageEffect) {
	case 91:	// DAMAGE_EFFECT_SMASH_NEWSTANDNEW1
		SetColor(color1, 0xC0, 0x3E, 0xC, 0xFF);
		SetColor(color2, 0xC0, 0x3E, 0xC, 0xFF);
		break;
	case 92:	// DAMAGE_EFFECT_SMASH_NEWSTANDNEW2
		SetColor(color1, 0xA3, 0x36, 0x21, 0xFF);
		SetColor(color2, 0xA3, 0x36, 0x21, 0xFF);
		break;
	case 93:	// DAMAGE_EFFECT_SMASH_NEWSTANDNEW3
		SetColor(color1, 0x12, 0x31, 0x24, 0xFF);
		SetColor(color2, 0x12, 0x31, 0x24, 0xFF);
		break;
	case 94:	// DAMAGE_EFFECT_SMASH_NEWSTANDNEW4
		SetColor(color1, 0xC0, 0xC8, 0xBE, 0xFF);
		SetColor(color2, 0x10, 0x57, 0x85, 0xFF);
		break;
	case 95:	// DAMAGE_EFFECT_SMASH_NEWSTANDNEW5
		SetColor(color1, 0xD0, 0x94, 0x1B, 0xFF);
		SetColor(color2, 0xD0, 0x94, 0x1B, 0xFF);
		break;
	case 100:	// DAMAGE_EFFECT_SMASH_BLACKBAG
		SetColor(color1, 0, 0, 0, 0xFF);
		SetColor(color2, 0, 0, 0, 0xFF);
		break;
	case 131:	// DAMAGE_EFFECT_SMASH_BEACHLOUNGE_WOOD
		SetColor(color1, 0x8F, 0x8A, 0x8C, 0xFF);
		SetColor(color2, 0x73, 0x75, 0x7B, 0xFF);
		break;
	case 132:	// DAMAGE_EFFECT_SMASH_BEACHLOUNGE_TOWEL
		SetColor(color1, 0x52, 0x92, 0x4A, 0xFF);
		SetColor(color2, 0xCE, 0xCF, 0xCE, 0xFF);
		break;
	}
}
unsigned char ObjectLogic_RenderColorChannel(float directional, float ambient)
{
	float v = (0.8f * directional + ambient) * 165.75f;
	v = v < 0.0f ? 0.0f : v > 255.0f ? 255.0f : v;
	return (unsigned char)v;
}

float ObjectLogic_RCBombSpeedFactor(float speedSqr, float timeStep)
{
	float fScalar = 1.0f - speedSqr / 5.0f;
	return powf(fScalar, timeStep);
}

bool ObjectLogic_BeachballKickedByPlayer(float offsetZ, float distance2D, float ballSpeed2D, float playerSpeed)
{
	return offsetZ > 0.0 && offsetZ < 1.5f && distance2D < 1.0f && ballSpeed2D < 0.05f && playerSpeed > 0.1f;
}

bool ObjectLogic_BeachballHitsHead(float offsetZ, float ballSpeedZ, float distance2D)
{
	return offsetZ > -1.05 && offsetZ < -0.6 && ballSpeedZ < 0.0f && distance2D < 0.9f;
}
void Stinger_SpikeAngles(float maxZ, uint32_t elapsed, bool deploying, int numPositions, float angles[2], int *positionIndex)
{
	const float pi = (float)3.14159265358979323846;
	float progress = elapsed / 2500.0f;
	float degangle, angle1, angle2;

	if (!deploying)
		progress = 1.0f - progress;
	degangle = progress * numPositions;
	angle1 = maxZ + degangle * pi / 180.0f;
	angle2 = maxZ - degangle * pi / 180.0f;
	*positionIndex = degangle < 0 ? 0 : degangle > numPositions - 1 ? numPositions - 1 : degangle;
	angles[0] = General_LimitRadianAngle(angle1);
	angles[1] = General_LimitRadianAngle(angle2);
}
float ObjectLogic_SpeedDamping(float damping, float timeStep)
{
	return powf(damping, timeStep);
}

float ObjectLogic_BeachballKickSpeed(float speedZ)
{
	float v = speedZ + 0.3f;

	return v > 0.2f ? v : 0.2f;
}
//- rouz edit (Claude)
