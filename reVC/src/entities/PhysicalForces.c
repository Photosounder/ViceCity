//+ rouz edit (Claude)
#include <math.h>
#include "config.h"
#include "PhysicalForces.h"

#define GRAVITY (0.008f)

static VectorMath3 Vec(PhysicalVec v)
{
	// Copy the components into the plain math record
	VectorMath3 r = {v.x, v.y, v.z};
	return r;
}

static VectorMath3 Add(VectorMath3 a, VectorMath3 b)
{
	// Component-wise sum
	VectorMath3 r = {a.x + b.x, a.y + b.y, a.z + b.z};
	return r;
}

static VectorMath3 Sub(VectorMath3 a, VectorMath3 b)
{
	// Component-wise difference
	VectorMath3 r = {a.x - b.x, a.y - b.y, a.z - b.z};
	return r;
}

static VectorMath3 Scale(VectorMath3 a, float s)
{
	// Component-wise product with a scalar
	VectorMath3 r = {a.x * s, a.y * s, a.z * s};
	return r;
}

VectorMath3 Physical_GetSpeed(const CPhysicalData *self, VectorMath3 r)
{
	// Linear speed plus the speed of the point due to rotation, friction included
	return Add(Add(Vec(self->m_vecMoveSpeed), Vec(self->m_vecMoveFriction)),
		VectorMath_Cross(Add(Vec(self->m_vecTurnFriction), Vec(self->m_vecTurnSpeed)), r));
}

void Physical_ApplyMoveSpeed(CPlaceableData *placeable, CPhysicalData *self, float timeStep)
{
	if (self->bIsFrozen) {
		self->m_vecMoveSpeed.x = 0.0f;
		self->m_vecMoveSpeed.y = 0.0f;
		self->m_vecMoveSpeed.z = 0.0f;
	} else {
		// Translate the matrix by the speed scaled to the time step
		VectorMath3 d = Scale(Vec(self->m_vecMoveSpeed), timeStep);
		placeable->m_matrix.px += d.x;
		placeable->m_matrix.py += d.y;
		placeable->m_matrix.pz += d.z;
	}
}

void Physical_ApplyTurnSpeed(CPlaceableData *placeable, CPhysicalData *self, float timeStep)
{
	if (self->bIsFrozen) {
		self->m_vecTurnSpeed.x = 0.0f;
		self->m_vecTurnSpeed.y = 0.0f;
		self->m_vecTurnSpeed.z = 0.0f;
	} else {
		// Move the coordinate axes by their speed, note that this denormalises the matrix
		VectorMath3 turnvec = Scale(Vec(self->m_vecTurnSpeed), timeStep);
		float (*m)[4] = placeable->m_matrix.f;
		VectorMath3 axis, d;
		int i;

		for (i = 0; i < 3; i++) {
			axis.x = m[i][0]; axis.y = m[i][1]; axis.z = m[i][2];
			d = VectorMath_Cross(turnvec, axis);
			m[i][0] += d.x; m[i][1] += d.y; m[i][2] += d.z;
		}
	}
}

void Physical_ApplyMoveForce(CPhysicalData *self, float jx, float jy, float jz)
{
	// Divide the impulse by the mass and add it to the speed
	VectorMath3 d = {jx, jy, jz};
	d = Scale(d, 1.0f / self->m_fMass);
	self->m_vecMoveSpeed.x += d.x;
	self->m_vecMoveSpeed.y += d.y;
	self->m_vecMoveSpeed.z += d.z;
}

void Physical_ApplyFrictionMoveForce(CPhysicalData *self, float jx, float jy, float jz)
{
	// Same as the move force but added to the friction speed
	VectorMath3 d = {jx, jy, jz};
	d = Scale(d, 1.0f / self->m_fMass);
	self->m_vecMoveFriction.x += d.x;
	self->m_vecMoveFriction.y += d.y;
	self->m_vecMoveFriction.z += d.z;
}

static VectorMath3 TurnImpulse(const CPlaceableData *placeable, const CPhysicalData *self, float jx, float jy, float jz, float px, float py, float pz)
{
	// Torque of the impulse about the centre of mass divided by the turn mass
	VectorMath3 com = VectorMath_MatrixTimesVector3x3(placeable->m_matrix.f, Vec(self->m_vecCentreOfMass));
	VectorMath3 p = {px, py, pz};
	VectorMath3 j = {jx, jy, jz};
	return Scale(VectorMath_Cross(Sub(p, com), j), 1.0f / self->m_fTurnMass);
}

void Physical_ApplyTurnForce(const CPlaceableData *placeable, CPhysicalData *self, float jx, float jy, float jz, float px, float py, float pz)
{
	// Add the turn impulse to the turn speed
	VectorMath3 d = TurnImpulse(placeable, self, jx, jy, jz, px, py, pz);
	self->m_vecTurnSpeed.x += d.x;
	self->m_vecTurnSpeed.y += d.y;
	self->m_vecTurnSpeed.z += d.z;
}

void Physical_ApplyFrictionTurnForce(const CPlaceableData *placeable, CPhysicalData *self, float jx, float jy, float jz, float px, float py, float pz)
{
	// Add the turn impulse to the friction turn speed
	VectorMath3 d = TurnImpulse(placeable, self, jx, jy, jz, px, py, pz);
	self->m_vecTurnFriction.x += d.x;
	self->m_vecTurnFriction.y += d.y;
	self->m_vecTurnFriction.z += d.z;
}

void Physical_ApplyFriction(CPhysicalData *self)
{
	// Fold the friction speeds into the real speeds then clear them
	self->m_vecMoveSpeed.x += self->m_vecMoveFriction.x;
	self->m_vecMoveSpeed.y += self->m_vecMoveFriction.y;
	self->m_vecMoveSpeed.z += self->m_vecMoveFriction.z;
	self->m_vecTurnSpeed.x += self->m_vecTurnFriction.x;
	self->m_vecTurnSpeed.y += self->m_vecTurnFriction.y;
	self->m_vecTurnSpeed.z += self->m_vecTurnFriction.z;
	self->m_vecMoveFriction.x = 0.0f;
	self->m_vecMoveFriction.y = 0.0f;
	self->m_vecMoveFriction.z = 0.0f;
	self->m_vecTurnFriction.x = 0.0f;
	self->m_vecTurnFriction.y = 0.0f;
	self->m_vecTurnFriction.z = 0.0f;
}

static float Dot(VectorMath3 a, VectorMath3 b)
{
	// Dot product with the same operation order as the engine vector
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void Physical_ApplySpringCollision(const CPlaceableData *placeable, CPhysicalData *self, float timeStep, float springConst, VectorMath3 springDir, VectorMath3 point, float springRatio, float bias)
{
	// Push back along the spring direction in proportion to the compression
	float compression = 1.0f - springRatio;
	if (compression > 0.0f) {
		float step = timeStep < 3.0f ? timeStep : 3.0f;
		float impulse = -GRAVITY * self->m_fMass * step * springConst * compression * bias * 2.0f;
		VectorMath3 j = Scale(springDir, impulse);
		Physical_ApplyMoveForce(self, j.x, j.y, j.z);
		Physical_ApplyTurnForce(placeable, self, j.x, j.y, j.z, point.x, point.y, point.z);
	}
}

void Physical_ApplySpringDampening(const CPlaceableData *placeable, CPhysicalData *self, float timeStep, float damping, VectorMath3 springDir, VectorMath3 point, VectorMath3 speed)
{
	float speedA = Dot(speed, springDir);
	float speedB = Dot(Physical_GetSpeed(self, point), springDir);
#ifdef FIX_BUGS
	if (speedB == 0.0f)
		return;
#endif
	float step = timeStep < 3.0f ? timeStep : 3.0f;
	float impulse = -damping * (speedA + speedB) / 2.0f * self->m_fMass * step * 0.53f;
	if (self->bIsHeavy)
		impulse *= 2.0f;

	// Limit the impulse so that it cannot reverse the point speed by too much
	float a = self->m_fTurnMass / ((Dot(point, point) + 1.0f) * 2.0f * self->m_fMass);
	a = a < 1.0f ? a : 1.0f;
	float b = fabsf(impulse / (speedB * self->m_fMass));
	if (a < b)
		impulse *= a / b;

	{
		VectorMath3 j = Scale(springDir, impulse);
		Physical_ApplyMoveForce(self, j.x, j.y, j.z);
		Physical_ApplyTurnForce(placeable, self, j.x, j.y, j.z, point.x, point.y, point.z);
	}
}
void Physical_ApplySpringCollisionAlt(const CPlaceableData *placeable, CPhysicalData *self, float timeStep, float springConst, VectorMath3 springDir, VectorMath3 point, float springRatio, float bias, CColPoint *contact)
{
	float compression = 1.0f - springRatio;
	if (compression > 0.0f) {
		float step, impulse;
		VectorMath3 normal, j;

		// A normal that points along the spring is turned around
		if (springDir.x*contact->normal.x + springDir.y*contact->normal.y + springDir.z*contact->normal.z > 0.0f)
			ColPoint_ReverseNormal(contact);

		step = timeStep < 3.0f ? timeStep : 3.0f;
		impulse = GRAVITY*self->m_fMass*step * springConst * compression * bias*2.0f;
		if (self->bIsHeavy)
			impulse *= 0.75f;

		// Push and turn along the normal
		normal.x = contact->normal.x; normal.y = contact->normal.y; normal.z = contact->normal.z;
		j = Scale(normal, impulse);
		Physical_ApplyMoveForce(self, j.x, j.y, j.z);
		Physical_ApplyTurnForce(placeable, self, j.x, j.y, j.z, point.x, point.y, point.z);
	}
}

void Physical_ApplyAirResistance(CPhysicalData *self, float timeStep, bool isGhost)
{
	if (self->m_fAirResistance > 0.1f) {
		// Constant factor per time step
		float f = powf(self->m_fAirResistance, timeStep);
		self->m_vecMoveSpeed.x *= f; self->m_vecMoveSpeed.y *= f; self->m_vecMoveSpeed.z *= f;
		self->m_vecTurnSpeed.x *= f; self->m_vecTurnSpeed.y *= f; self->m_vecTurnSpeed.z *= f;
	} else if (!isGhost) {
		// Resistance that grows with the squared speed, and spin that decays at a constant rate
		float sq = self->m_vecMoveSpeed.x*self->m_vecMoveSpeed.x + self->m_vecMoveSpeed.y*self->m_vecMoveSpeed.y + self->m_vecMoveSpeed.z*self->m_vecMoveSpeed.z;
		float f = powf(1.0f/fabsf(1.0f + self->m_fAirResistance*0.5f*sq), timeStep);
		float t = powf(0.99f, timeStep);
		self->m_vecMoveSpeed.x *= f; self->m_vecMoveSpeed.y *= f; self->m_vecMoveSpeed.z *= f;
		self->m_vecTurnSpeed.x *= t; self->m_vecTurnSpeed.y *= t; self->m_vecTurnSpeed.z *= t;
	}
}
//- rouz edit (Claude)
