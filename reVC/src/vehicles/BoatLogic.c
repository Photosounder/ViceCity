//+ rouz edit (Claude)
#include <stddef.h>
#include "BoatLogic.h"
#include "EntityStatus.h"
#include "VehicleCreation.h"

#define WAKE_POINTS 32

void Boat_PruneWakeTrail(CBoatData *self, float timeStep)
{
	int i;

	// Age the wake points until the first dead one
	for (i = 0; i < WAKE_POINTS; i++) {
		if (self->m_afWakePointLifeTime[i] <= 0.0f)
			break;
		if (self->m_afWakePointLifeTime[i] <= timeStep) {
			self->m_afWakePointLifeTime[i] = 0.0f;
			break;
		}
		self->m_afWakePointLifeTime[i] -= timeStep;
	}
	self->m_nNumWakePoints = i;
}

void Boat_AddWakePoint(CBoatStruct *self, float x, float y)
{
	CBoatData *b = &self->boat;
	const CMatrixData *m = &self->base.base.base.placeable.m_matrix;
	int i;

	// Start a new trail when the old one is dead
	if (b->m_afWakePointLifeTime[0] <= 0.0f) {
		b->m_avec2dWakePoints[0].x = x;
		b->m_avec2dWakePoints[0].y = y;
		b->m_afWakePointLifeTime[0] = 150.0f;
		b->m_nNumWakePoints = 1;
		return;
	}

	// Only add points once the boat moved far enough from the last one
	{
		float dx = m->px - b->m_avec2dWakePoints[0].x;
		float dy = m->py - b->m_avec2dWakePoints[0].y;
		if (!(dx * dx + dy * dy < 2.0f * 2.0f))
			return;
	}

	// Limit the trail length by the kind of driver
	if (self->base.base.base.entity.m_status == STATUS_PLAYER) {
		if (b->m_nNumWakePoints >= 31)
			b->m_nNumWakePoints = 31;
	} else if (self->base.vehicle.VehicleCreatedBy == MISSION_VEHICLE) {
		if (b->m_nNumWakePoints >= 20)
			b->m_nNumWakePoints = 20;
	} else {
		if (b->m_nNumWakePoints >= 15)
			b->m_nNumWakePoints = 15;
	}

	// Shift the trail and put the new point in front
	for (i = b->m_nNumWakePoints; i != 0; i--) {
		b->m_avec2dWakePoints[i] = b->m_avec2dWakePoints[i-1];
		b->m_afWakePointLifeTime[i] = b->m_afWakePointLifeTime[i-1];
	}
	b->m_avec2dWakePoints[0].x = x;
	b->m_avec2dWakePoints[0].y = y;
	b->m_afWakePointLifeTime[0] = 150.0f;
	if (b->m_nNumWakePoints < WAKE_POINTS)
		b->m_nNumWakePoints++;
}

void Boat_ClearNodes(CBoatData *self)
{
	// Forget all the frames
	int i;
	for (i = 0; i < NUM_BOAT_NODES; i++)
		self->m_aBoatNodes[i] = NULL;
}
//- rouz edit (Claude)
