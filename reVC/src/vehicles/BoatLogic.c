//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "BoatLogic.h"
#include "General.h"
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
void Boat_WaterResistanceFactors(float volumeUnderWater, float mass, float suspensionForceLevel, bool isSkimmer, float seaPlaneWaterResistance, float forwardSpeed,
	float moveResX, float moveResY, float moveResZ, float timeStep, float *fx, float *fy, float *fz)
{
	float resistance = 0.001f * suspensionForceLevel * (volumeUnderWater * volumeUnderWater) * mass;
	if (isSkimmer)
		resistance *= seaPlaneWaterResistance;
	float magic = ((forwardSpeed * forwardSpeed) + 0.05f) * resistance + 1.0f;
	magic = fabsf(magic);
	*fx = powf(moveResX / magic, 0.5f * timeStep);
	*fy = powf(moveResY / magic, 0.5f * timeStep);
	*fz = powf(moveResZ / magic, 0.5f * timeStep);
}

float Boat_WaterResistanceVertical(float speedZ, float fz)
{
	if (speedZ > 0.0f)
		speedZ *= fz;
	else
		speedZ *= (1.0f - fz) * 0.5f + fz;
	return speedZ;
}
bool Boat_WakeInViewRange(float camToBoatX, float camToBoatY, float camForwardX, float camForwardY, float *distSq)
{
	float distToCam = camForwardX * camToBoatX + camForwardY * camToBoatY;
	if (distToCam > 100.0f || distToCam < -15.0f)
		return false;
	*distSq = camToBoatX * camToBoatX + camToBoatY * camToBoatY;
	if (*distSq > 70.0f * 70.0f)
		return false;
	return true;
}

int Boat_PickWakeSlotToReplace(const float *slotDistSq, int numSlots, float distSq, bool isPlayerBoat)
{
	float nearest = 999999.88f;
	int frameId2 = -1;

	// Look for the boat of the list closest to the camera
	for (int j = 0; j < numSlots; j++) {
		if (slotDistSq[j] < nearest) {
			nearest = slotDistSq[j];
			frameId2 = j;
		}
	}

	if (frameId2 != -1 && (distSq < nearest || isPlayerBoat))
		return frameId2;
	return -1;
}
int Boat_SeaplaneRotorAlpha(float movingSpeed)
{
	const float pi = (float)3.14159265358979323846;
	float t = 2.0f * movingSpeed * 8.0f / pi;
	if (t > 1.0f)
		t = 1.0f;
	return (int)((1.0f - t) * 255.0f);
}

float Boat_RioPropellerSpeed(float flapHeading, float boatHeading, float wind, float movingSpeed, float timeStep)
{
	const float pi = (float)3.14159265358979323846;
	const float halfpi = pi / 2;
	float rot = -(45.0f * pi / 180.0f) - (flapHeading + boatHeading);

	// The angle is limited to half a turn then to the range of the propeller
	rot = General_LimitRadianAngle(rot);
	if (rot > halfpi) rot = pi;
	else if (rot < -halfpi) rot = -pi;
	if (rot < -(63.0f * pi / 180.0f)) rot = -(63.0f * pi / 180.0f);
	else if (rot > (63.0f * pi / 180.0f)) rot = (63.0f * pi / 180.0f);
	movingSpeed += (0.008f * wind + 0.002f) * rot;
	movingSpeed *= powf(0.9985f, timeStep) / (500.0f * (movingSpeed * movingSpeed) + 1.0f);
	return movingSpeed;
}
int Boat_SprayColorChannel(float scale, float ambient, float directionalFactor, float directional)
{
	int v = (int)(scale * (ambient + directionalFactor * directional));
	return v < 0 ? 0 : v > 255 ? 255 : v;
}

BoatCollisionResult Boat_CollisionDamage(float health, float collisionDamage, bool isPlayerVehicle, bool takeLessDamage, bool hasDriver)
{
	BoatCollisionResult r;
	r.health = health;
	r.driverAnnoyed = false;
	r.setOnFire = false;

	if (health >= 250.0f) {
		if (isPlayerVehicle) {
			if (takeLessDamage)
				r.health -= (collisionDamage - 25.0f) / 6.0f;
			else
				r.health -= (collisionDamage - 25.0f) / 2.0f;
		} else {
			if (collisionDamage > 60.0f && hasDriver)
				r.driverAnnoyed = true;
			if (takeLessDamage)
				r.health -= (collisionDamage - 25.0f) / 12.0f;
			else
				r.health -= (collisionDamage - 25.0f) / 4.0f;
		}

		if (r.health <= 0.0f && health > 0.0f) {
			r.health = 1.0f;
			r.setOnFire = true;
		}
	}
	return r;
}

float Boat_PropellerDepth(float waterLevel, float propellerZ)
{
	float propellerDepth = waterLevel - (propellerZ - 0.5f);
	if (propellerDepth > 1.0f)
		propellerDepth = 1.0f;
	else
		propellerDepth = propellerDepth * propellerDepth;
	return propellerDepth;
}

float Boat_SlowAheadForce(float tractionLoss, float speedForward, float turnMass, float gasPedal, bool isPlayer)
{
	float force = tractionLoss * speedForward;
	if (force > 0.01f * turnMass)
		force = 0.01f * turnMass;
	if (gasPedal > 0.01f) {
		if (isPlayer)
			force *= (0.55f - fabsf(gasPedal)) * 1.3f;
		else
			force *= (0.55f - fabsf(gasPedal)) * 2.5f;
	}
	if ((gasPedal < 0.0f && force > 0.0f) || (gasPedal > 0.0f && force < 0.0f))
		force *= -1.0f;
	return force;
}
float Boat_WorldEdgeSpeed(float speed, float position, float maxEdge, float minEdge)
{
	float limitMax = -(position - maxEdge) * 0.01f;
	speed = speed < limitMax ? speed : limitMax;
	float limitMin = -(position - minEdge) * 0.01f;
	speed = speed > limitMin ? speed : limitMin;
	return speed;
}

float Boat_TurnDrag(float turnResX, float turnResY, float turnResZ, float timeStep, float turnX, float *turnY, float *turnZ, float turnMass)
{
	float fx = powf(turnResX, timeStep);
	float fy = powf(turnResY, timeStep);
	float fz = powf(turnResZ, timeStep);
	float magic = 1.0f / (1000.0f * (turnX * turnX) + 1.0f) * fx;
	*turnY *= fy;
	*turnZ *= fz;
	return (magic - 1.0f) * turnX * turnMass;
}

void Boat_SplashImpulse(int deltaVolume, float speedSqr, float speedZ, float speedAlongForward, float brakeDeceleration, float brakeBias, float *speedUp, float *speedForward)
{
	float up = speedSqr * deltaVolume * 0.001f;
	if (up + speedZ > brakeDeceleration)
		up = brakeDeceleration - speedZ;
	if (up < 0.0f) up = 0.0f;
	float fwd = speedAlongForward;
	fwd *= -deltaVolume * 0.01f * brakeBias;
	*speedUp = up;
	*speedForward = fwd;
}

float Boat_SkimmerMovingSpeed(float movingSpeed, float timeStep, bool isPlayer)
{
	if (isPlayer) {
		if (movingSpeed < 0.22f)
			movingSpeed += 0.001f * timeStep;
	} else {
		if (movingSpeed > 0.0005f * timeStep)
			movingSpeed -= 0.0005f * timeStep;
		else
			movingSpeed = 0.0f;
	}
	return movingSpeed;
}
BoatSplashParams Boat_SplashParams(int kind)
{
	BoatSplashParams p;

	switch (kind) {
	case BOAT_SPLASH_RIO:
		p.sizeFactor = 1.0f; p.frontFactor = 0.9f; p.moveFactor = -0.5f; p.upFactor = 0.25f; p.rightFactor = 0.35f; p.rightOffset = 1.85f; p.upOffset = 0.5f; p.addBuoyancePoint = false;
		break;
	case BOAT_SPLASH_SQUALO:
		p.sizeFactor = 1.0f; p.frontFactor = 0.75f; p.moveFactor = -0.125f; p.upFactor = 0.15f; p.rightFactor = 0.25f; p.rightOffset = 0.5f; p.upOffset = 0.5f; p.addBuoyancePoint = true;
		break;
	case BOAT_SPLASH_REEFER:
		p.sizeFactor = 1.0f; p.frontFactor = 0.75f; p.moveFactor = -0.5f; p.upFactor = 0.15f; p.rightFactor = 0.5f; p.rightOffset = 1.3f; p.upOffset = 0.0f; p.addBuoyancePoint = true;
		break;
	case BOAT_SPLASH_COASTG:
		p.sizeFactor = 0.25f; p.frontFactor = 0.8f; p.moveFactor = 0.165f; p.upFactor = 0.25f; p.rightFactor = 0.15f; p.rightOffset = 0.65f; p.upOffset = 0.5f; p.addBuoyancePoint = false;
		break;
	case BOAT_SPLASH_DINGHY:
		p.sizeFactor = 0.25f; p.frontFactor = 0.9f; p.moveFactor = 0.35f; p.upFactor = 0.25f; p.rightFactor = 0.25f; p.rightOffset = 0.6f; p.upOffset = 0.5f; p.addBuoyancePoint = false;
		break;
	default:
		p.sizeFactor = 1.0f; p.frontFactor = 0.9f; p.moveFactor = -0.5f; p.upFactor = 0.25f; p.rightFactor = 0.35f; p.rightOffset = 0.5f; p.upOffset = 0.0f; p.addBuoyancePoint = true;
		break;
	}
	return p;
}
void Boat_SteerSinCos(float steerAngle, float steerFactor, float *steerSin, float *steerCos)
{
	*steerSin = sinf(-steerAngle * steerFactor);
	*steerCos = cosf(-steerAngle * steerFactor);
}
void Boat_PropellerThrust(float propellerDepth, float gasPedal, float engineAcceleration, float mass, const float dir[3], bool onLand, float force[3])
{
	float k = propellerDepth * gasPedal * 40.0f * engineAcceleration * mass;

	force[0] = k * dir[0];
	force[1] = k * dir[1];
	force[2] = k * dir[2];
	if (force[2] > 0.2f)
		force[2] = (1.2f - force[2]) * (1.2f - force[2]) + 0.2f;
	if (onLand) {
		if (gasPedal < 0.0f) {
			force[0] *= 5.0f;
			force[1] *= 5.0f;
		}
		if (force[2] < 0.0f)
			force[2] = 0.0f;
	}
}
float Boat_SmoothSteeringUD(float current, float target, float timeStep)
{
	float t = timeStep / 5.f;

	return current + (target - current) * (1.f < t ? 1.f : t);
}

int Boat_WindscreenAlpha(int alpha)
{
	int v = 150 - alpha;

	return 0 > v ? 0 : v;
}
//- rouz edit (Claude)
