//+ rouz edit (Claude)
#include <math.h>
#include "config.h"
#include "PhysicalForces.h"
#include "Vector.h" // rouz edit (Claude)
#include "VectorMath.h"

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
float Physical_CollisionMassFactorA(bool bIsPedPhysics, bool aIsHeavy)
{
	if (bIsPedPhysics)
		return 10.0f;
	return aIsHeavy ? 2.0f : 1.0f;
}

float Physical_CollisionMassFactorB(bool aIsPedPhysics, bool aIsPlayerPed, bool bIsVehicle, bool bIsAbandonedOrWrecked, bool aHasHitWall, float bMass, bool bIsHeavy)
{
	if (aIsPedPhysics) {
		if (aIsPlayerPed && bIsVehicle && (bIsAbandonedOrWrecked || aHasHitWall)) {
			float excess = bMass - 2000.0f;
			return 1.0f / ((excess > 0.0f ? excess : 0.0f) / 5000.0f + 1.0f);
		}
		return 10.0f;
	}
	return bIsHeavy ? 2.0f : 1.0f;
}
float Physical_VehicleAdhesion(float adhesion, bool isRCBandit, bool isBoat, float normalZ, bool surfaceIsLooseOrSand, bool isWrecked, float upZ, float impulse)
{
	if (isRCBandit)
		adhesion *= 0.2f;
	else if (isBoat) {
		if (normalZ > 0.6f) {
			if (surfaceIsLooseOrSand)
				adhesion *= 3.0f;
		} else
			adhesion = 0.0f;
	} else if (isWrecked)
		adhesion *= 3.0f;
	else if (upZ > 0.3f)
		adhesion = 0.0f;
	else {
		float factor = 0.03f * impulse + 1.0f;
		adhesion *= (5.0f < factor ? 5.0f : factor);
	}
	return adhesion;
}

float Physical_ReportedImpulse(float upZ, float moveSpeedX, float moveSpeedY, float impulse)
{
	if (upZ < -0.6f && fabsf(moveSpeedX) < 0.05f && fabsf(moveSpeedY) < 0.05f)
		return 0.1f * impulse;
	return impulse;
}
static float NumSteps(float distSq, float x)
{
	// Number of units of x we would like to step
	return ceilf(sqrtf(distSq) * (1.0f / x));
}

int Physical_PedCollisionSteps(float distSq, bool isPlayer, bool hasPhysSurface)
{
	float n;

	if (isPlayer) {
		if (hasPhysSurface) {
			n = NumSteps(distSq, 0.15f);
			n = n > 4.0f ? n : 4.0f;
		} else {
			n = NumSteps(distSq, 0.3f);
			n = n > 2.0f ? n : 2.0f;
		}
	} else
		n = NumSteps(distSq, 0.45f);
	return (int)n;
}

int Physical_VehicleCollisionSteps(float distSq, bool isPlayerStatus)
{
	if (isPlayerStatus)
		return (int)NumSteps(distSq, 0.2f);
	return (int)(distSq > 0.32f ? NumSteps(distSq, 0.3f) : NumSteps(distSq, 0.4f));
}

bool Physical_VehicleMovesLessThanSize(float speedRight, float speedForward, float speedUp, float timeStep, float sizeX, float sizeY, float sizeZ)
{
	float relDistX = fabsf(speedRight) * timeStep / sizeX;
	float relDistY = fabsf(speedForward) * timeStep / sizeY;
	float relDistZ = fabsf(speedUp) * timeStep / sizeZ;
	float m = relDistY > relDistZ ? relDistY : relDistZ;
	m = relDistX > m ? relDistX : m;
	return m < 1.0f;
}

int Physical_ObjectCollisionSteps(float distSq, bool isLampost, bool isUnknown5, bool isSmallBoxOrFencePart)
{
	if (isLampost) {
		if (distSq >= (0.3f * 0.3f))
			return (int)NumSteps(distSq, 0.3f);
	} else if (isUnknown5) {
		if (distSq >= 0.009f)
			return (int)NumSteps(distSq, 0.09f);
	} else if (isSmallBoxOrFencePart) {
		if (distSq >= (0.15f * 0.15f))
			return (int)NumSteps(distSq, 0.15f);
	} else {
		if (distSq >= (0.3f * 0.3f))
			return (int)NumSteps(distSq, 0.3f);
	}
	return 1;
}
bool Physical_ShiftIsDamped(bool hasHitWall, bool isPed, bool hasSurface, bool surfaceInfiniteMass, bool surfaceFlagA08, bool secondShift)
{
	return hasHitWall && ((isPed && (!hasSurface || !surfaceInfiniteMass || surfaceFlagA08)) || secondShift);
}
float Physical_CollisionImpulse(int elasticityType, bool hasContacted, float relX, float relY, float relZ, float minSpeed, float normalSpeed, float mass, bool isBoatOnWood, float elasticity)
{
	float impulse = 0.0f;
	bool small = fabsf(relX) < minSpeed && fabsf(relY) < minSpeed && fabsf(relZ) < minSpeed * 2.0f;

	if (elasticityType == 1 && !hasContacted && small)
		impulse = -0.98f * normalSpeed * mass;
	if (elasticityType == 3 && small)
		impulse = -0.8f * normalSpeed * mass;
	else if (elasticityType == 2 && small)
		impulse = -0.92f * normalSpeed * mass;
	else if (elasticityType == 4 && small)
		impulse = -0.8f * normalSpeed * mass;
	else if (isBoatOnWood)
		impulse = -(2.0f * elasticity + 1.0f) * normalSpeed * mass;
	else
		impulse = -(elasticity + 1.0f) * normalSpeed * mass;
	return impulse;
}
float Physical_ShiftFactor(bool secondShift, bool deepestOnly)
{
	if (deepestOnly)
		return secondShift ? 0.4f : 0.2f;
	return secondShift ? 1.5f : 1.2f;
}
float Physical_AudioSpeedDiff(float turnSpeedDiff, float moveSpeedDiff)
{
	return turnSpeedDiff > moveSpeedDiff ? turnSpeedDiff : moveSpeedDiff;
}

float Physical_ShiftDamping(float damp, float timeStep)
{
	return powf(damp, timeStep);
}
void Physical_BlendGravityUp(const CVector *gravityUp, const CVector *surfaceUp, float timeStep, CVector *out)
{
	float t = timeStep * 0.5f;
	float x, y, z;

	// Blend factor limited between 0.05 and 0.8
	t = t < 0.05f ? 0.05f : t > 0.8f ? 0.8f : t;

	// Blend the previous gravity direction with the surface direction
	x = gravityUp->x * (1.0f - t) + surfaceUp->x * t;
	y = gravityUp->y * (1.0f - t) + surfaceUp->y * t;
	z = gravityUp->z * (1.0f - t) + surfaceUp->z * t;

	// Normalise it unless it is too short, then use straight up
	if (x * x + y * y + z * z < 0.1f) {
		out->x = 0.0f; out->y = 0.0f; out->z = 1.0f;
	} else {
		VectorMath3 v = { x, y, z };

		VectorMath_Normalise(&v);
		out->x = v.x; out->y = v.y; out->z = v.z;
	}
}
//- rouz edit (Claude)
