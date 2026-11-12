//+ rouz edit (Claude)
#include <stddef.h>
#include "config.h"
#include "PhysicalSectors.h"
#include "EntityTypes.h"
#include "EntityStatus.h"

void re3_assert(const char *expression, const char *file, unsigned int line, const char *function);
#ifndef MASTER
#define Physical_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define Physical_Assert(expression) (expression)
#endif

CRect Physical_GetBoundRect(const CPlaceableData *placeable, const CEntityData *entity)
{
	// Square around the bounding sphere
	float c[3];
	float radius;
	Entity_GetBoundCentre(placeable, entity, c);
	radius = Entity_GetBoundRadius(entity);
	return Rect_Make(c[0] - radius, c[1] - radius, c[0] + radius, c[1] + radius);
}

void Physical_Add(CPhysicalStruct *self, CSector *sectors)
{
	int x, y;
	EntitySectorRange r = Entity_ComputeSectorRange(Physical_GetBoundRect(&self->base.placeable, &self->base.entity));

	for (y = r.ystart; y <= r.yend; y++)
		for (x = r.xstart; x <= r.xend; x++) {
			CSector *s = Entity_SectorAt(sectors, x, y);
			CPtrList *list;
			CPtrNode *node;
			// Only vehicles, peds and objects are physical
			if (self->base.entity.m_type != ENTITY_TYPE_VEHICLE && self->base.entity.m_type != ENTITY_TYPE_PED && self->base.entity.m_type != ENTITY_TYPE_OBJECT)
				Physical_Assert(0);
			list = Entity_SectorListFor(s, self->base.entity.m_type, x == r.xmid && y == r.ymid);
			node = PtrList_InsertItem(list, self);
			Physical_Assert(node);
			EntryInfoList_InsertItem(&self->physical.m_entryInfoList, list, node, s);
		}
}

void Physical_Remove(CPhysicalStruct *self)
{
	// Delete every sector node and its entry info
	CEntryInfoNode *node, *next;
	for (node = self->physical.m_entryInfoList.first; node; node = next) {
		next = node->next;
		PtrList_DeleteNode(node->list, node->listnode);
		EntryInfoList_DeleteNode(&self->physical.m_entryInfoList, node);
	}
}

void Physical_RemoveAndAdd(CPhysicalStruct *self, CSector *sectors)
{
	int x, y;
	EntitySectorRange r = Entity_ComputeSectorRange(Physical_GetBoundRect(&self->base.placeable, &self->base.entity));
	CEntryInfoNode *node;

	// we'll try to recycle nodes from here
	CEntryInfoNode *next = self->physical.m_entryInfoList.first;

	for (y = r.ystart; y <= r.yend; y++)
		for (x = r.xstart; x <= r.xend; x++) {
			CSector *s = Entity_SectorAt(sectors, x, y);
			CPtrList *list = Entity_SectorListFor(s, self->base.entity.m_type, x == r.xmid && y == r.ymid);
			if (next) {
				// If we still have old nodes, use them
				PtrList_RemoveNode(next->list, next->listnode);
				PtrList_InsertNode(list, next->listnode);
				next->list = list;
				next->sector = s;
				next = next->next;
			} else {
				CPtrNode *newNode = PtrList_InsertItem(list, self);
				EntryInfoList_InsertItem(&self->physical.m_entryInfoList, list, newNode, s);
			}
		}

	// Remove old nodes we no longer need
	for (node = next; node; node = next) {
		next = node->next;
		PtrList_DeleteNode(node->list, node->listnode);
		EntryInfoList_DeleteNode(&self->physical.m_entryInfoList, node);
	}
}

void Physical_AddToMovingList(CPhysicalStruct *self, CPtrList *movingList)
{
	// Entities waiting for collision to load are not moved yet
	if (!self->base.entity.bIsStaticWaitingForCollision)
		self->physical.m_movingListNode = PtrList_InsertItem(movingList, self);
}

void Physical_RemoveFromMovingList(CPhysicalStruct *self, CPtrList *movingList)
{
	// Forget the moving list node when there is one
	if (self->physical.m_movingListNode) {
		PtrList_DeleteNode(movingList, self->physical.m_movingListNode);
		self->physical.m_movingListNode = NULL;
	}
}

static float VecMagSqr(PhysicalVec v)
{
	// Squared length with the same operation order as the engine vector
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

bool Physical_ProcessControlStart(CPhysicalStruct *self, float timeStep)
{
	CEntityData *e = &self->base.entity;
	CPhysicalData *p = &self->physical;

	// Start the frame with the contact and collision state cleared
	e->bHasContacted = false;
	e->bIsInSafePosition = false;
	e->bWasPostponed = false;
	e->bHasHitWall = false;

	if (e->m_status == STATUS_SIMPLE)
		return false;

	p->m_nCollisionRecords = 0;
	e->bHasCollided = false;
	p->m_nDamagePieceType = 0;
	p->m_fDamageImpulse = 0.0f;
	p->m_pDamageEntity = NULL;

	if (!e->bIsStuck) {
		if (e->m_type == ENTITY_TYPE_OBJECT ||
		    (e->m_type == ENTITY_TYPE_PED && !e->bPedPhysics)) {
			// Track a running average of the speeds to detect an entity at rest
			float step;
			p->m_vecMoveSpeedAvg.x = (p->m_vecMoveSpeedAvg.x + p->m_vecMoveSpeed.x) / 2.0f;
			p->m_vecMoveSpeedAvg.y = (p->m_vecMoveSpeedAvg.y + p->m_vecMoveSpeed.y) / 2.0f;
			p->m_vecMoveSpeedAvg.z = (p->m_vecMoveSpeedAvg.z + p->m_vecMoveSpeed.z) / 2.0f;
			p->m_vecTurnSpeedAvg.x = (p->m_vecTurnSpeedAvg.x + p->m_vecTurnSpeed.x) / 2.0f;
			p->m_vecTurnSpeedAvg.y = (p->m_vecTurnSpeedAvg.y + p->m_vecTurnSpeed.y) / 2.0f;
			p->m_vecTurnSpeedAvg.z = (p->m_vecTurnSpeedAvg.z + p->m_vecTurnSpeed.z) / 2.0f;
			step = timeStep * 0.003f;
			if (VecMagSqr(p->m_vecMoveSpeedAvg) < step * step &&
			    VecMagSqr(p->m_vecTurnSpeedAvg) < step * step) {
				p->m_nStaticFrames++;
				if (p->m_nStaticFrames > 10) {
					// Resting for long enough, make it static and stop it
					p->m_nStaticFrames = 10;
					e->bIsStatic = true;
					p->m_vecMoveSpeed.x = p->m_vecMoveSpeed.y = p->m_vecMoveSpeed.z = 0.0f;
					p->m_vecTurnSpeed.x = p->m_vecTurnSpeed.y = p->m_vecTurnSpeed.z = 0.0f;
					p->m_vecMoveFriction = p->m_vecMoveSpeed;
					p->m_vecTurnFriction = p->m_vecTurnSpeed;
					return false;
				}
			} else
				p->m_nStaticFrames = 0;
		}
	}
	return true;
}
//- rouz edit (Claude)
