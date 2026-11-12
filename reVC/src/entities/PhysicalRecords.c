//+ rouz edit (Claude)
#include "PhysicalRecords.h"

void Physical_SetDamagedPieceRecord(CPhysicalData *self, uint16_t piece, float impulse, CEntity *entity, float nx, float ny, float nz)
{
	// Store the damaged piece, its impulse, the damaging entity and the surface normal
	self->m_nDamagePieceType = piece;
	self->m_fDamageImpulse = impulse;
	self->m_pDamageEntity = entity;
	self->m_vecDamageNormal.x = nx;
	self->m_vecDamageNormal.y = ny;
	self->m_vecDamageNormal.z = nz;
}

void Physical_AddCollisionRecord(CEntityData *selfEntity, CPhysicalData *self, CEntityData *otherEntity, CEntity *other, uint32_t timeMs)
{
	// Flag both entities as having collided and remember when
	selfEntity->bHasCollided = true;
	otherEntity->bHasCollided = true;
	self->m_nLastTimeCollided = timeMs;

	// Keep a short list of distinct entities collided with
	if (selfEntity->bUseCollisionRecords) {
		int i;
		for (i = 0; i < self->m_nCollisionRecords; i++)
			if (self->m_aCollisionRecords[i] == other)
				return;
		if (self->m_nCollisionRecords < PHYSICAL_MAX_COLLISIONRECORDS)
			self->m_aCollisionRecords[self->m_nCollisionRecords++] = other;
	}
}

bool Physical_GetHasCollidedWith(const CEntityData *selfEntity, const CPhysicalData *self, const CEntity *other)
{
	// Search the collision records when they are being kept
	int i;
	if (selfEntity->bUseCollisionRecords)
		for (i = 0; i < self->m_nCollisionRecords; i++)
			if (self->m_aCollisionRecords[i] == other)
				return true;
	return false;
}

void Physical_RemoveRefsToEntity(CPhysicalData *self, const CEntity *other)
{
	// Remove every record of the entity, shifting the rest down
	int i = 0, j;

	while (i < self->m_nCollisionRecords) {
		if (self->m_aCollisionRecords[i] == other) {
			for (j = i; j < self->m_nCollisionRecords - 1; j++)
				self->m_aCollisionRecords[j] = self->m_aCollisionRecords[j + 1];
			self->m_nCollisionRecords--;
		} else
			i++;
	}
}
//- rouz edit (Claude)
