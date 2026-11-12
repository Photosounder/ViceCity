//+ rouz edit (Claude)
#include <stddef.h>
#include <math.h>
#include "HeliLogic.h"

void Heli_FindSwatPositionRelativeToHeli(int n, float *x, float *y, float *z)
{
	// Four rope positions below the cabin, left and right, back and front
	switch (n) {
	case 0: *x = -1.2f; *y = -1.0f; *z = -0.5f; break;
	case 1: *x =  1.2f; *y = -1.0f; *z = -0.5f; break;
	case 2: *x = -1.2f; *y =  1.0f; *z = -0.5f; break;
	case 3: *x =  1.2f; *y =  1.0f; *z = -0.5f; break;
	default: *x = 0.0f; *y = 0.0f; *z = 0.0f; break;
	}
}

void Heli_ClearNodes(CHeliData *self)
{
	// Forget all the frames
	int i;
	for (i = 0; i < NUM_HELI_NODES; i++)
		self->m_aHeliNodes[i] = NULL;
}
void Heli_ShootDown(CHeliData *self, bool spinPositive, uint32_t explosionTime)
{
	self->m_fAngularSpeed = spinPositive ? 0.05f : -0.05f;
	self->m_heliStatus = HELI_STATUS_SHOT_DOWN;
	self->m_nExplosionTimer = explosionTime;
}

bool Heli_AddBulletDamage(CHeliData *self, int32_t damage)
{
	// The Catalina helicopter is the weaker one
	self->m_nBulletDamage += damage;
	return (self->m_heliType == HELI_CATALINA && self->m_nBulletDamage > 400) ||
		(self->m_heliType != HELI_CATALINA && self->m_nBulletDamage > 700);
}

void Heli_BulletPosition(const float *line0, const float *line1, const float *heliPos, float *out)
{
	float dx = heliPos[0] - line0[0], dy = heliPos[1] - line0[1], dz = heliPos[2] - line0[2];
	float distToHeli = sqrtf(dx*dx + dy*dy + dz*dz);
	float lx = line1[0] - line0[0], ly = line1[1] - line0[1], lz = line1[2] - line0[2];
	float lineLength = sqrtf(lx*lx + ly*ly + lz*lz);
	float a = distToHeli-5.0f;
	float m = 1.0f > a ? 1.0f : a;

	// Five units before the helicopter, but at least one along the line
	out[0] = line0[0] + (lx*m)/lineLength;
	out[1] = line0[1] + (ly*m)/lineLength;
	out[2] = line0[2] + (lz*m)/lineLength;
}
//- rouz edit (Claude)
