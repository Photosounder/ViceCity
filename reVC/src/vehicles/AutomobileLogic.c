//+ rouz edit (Claude)
#include <stddef.h>
#include "AutomobileLogic.h"
#include "Vector.h" // rouz edit (Claude)
#include "VectorMath.h"
#include "ModelIds.h"
#include "DoorTypes.h"
#include "WheelState.h"
#include "WheelIndices.h"
#include "AutoPilot.h"
#include "EntityStatus.h"
#include "General.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"
#include "DamageManager.h"
#include "Door.h"
#include "SurfaceTypes.h"
#include "CarNodes.h"

bool Automobile_IsComponentPresent(const CAutomobileData *self, int comp)
{
	// The component exists when its frame does
	return self->m_aCarNodes[comp] != NULL;
}

bool Automobile_IsDoorMissing(CAutomobileData *self, int door)
{
	// Taken off by damage
	return Damage_GetDoorStatus(&self->Damage, door) == DOOR_STATUS_MISSING;
}

bool Automobile_IsDoorFullyOpen(CAutomobileData *self, int door)
{
	// A missing door counts as fully open
	return Door_IsFullyOpen(&self->Doors[door]) || Automobile_IsDoorMissing(self, door);
}

bool Automobile_IsDoorClosed(CAutomobileData *self, int door)
{
	// The door state is closed
	return !!Door_IsClosed(&self->Doors[door]);
}

bool Automobile_IsDoorReady(CAutomobileData *self, int door, int gettingInFlags)
{
	// Closed and missing doors are ready, otherwise nobody may be getting in through it
	int doorflag = 0;
	if (Door_IsClosed(&self->Doors[door]) || Automobile_IsDoorMissing(self, door))
		return true;
	switch (door) {
	case DOOR_FRONT_LEFT: doorflag = CAR_DOOR_FLAG_LF; break;
	case DOOR_FRONT_RIGHT: doorflag = CAR_DOOR_FLAG_RF; break;
	case DOOR_REAR_LEFT: doorflag = CAR_DOOR_FLAG_LR; break;
	case DOOR_REAR_RIGHT: doorflag = CAR_DOOR_FLAG_RR; break;
	default: break;
	}
	return (doorflag & gettingInFlags) == 0;
}

bool Automobile_IsDoorReadyByNode(CAutomobileData *self, unsigned node, int gettingInFlags)
{
	// Doors given as car nodes
	switch (node) {
	case CAR_DOOR_RF: return Automobile_IsDoorReady(self, DOOR_FRONT_RIGHT, gettingInFlags);
	case CAR_DOOR_RR: return Automobile_IsDoorReady(self, DOOR_REAR_RIGHT, gettingInFlags);
	case CAR_DOOR_LF: return Automobile_IsDoorReady(self, DOOR_FRONT_LEFT, gettingInFlags);
	case CAR_DOOR_LR: return Automobile_IsDoorReady(self, DOOR_REAR_LEFT, gettingInFlags);
	default:
		return false;
	}
}

bool Automobile_IsDoorMissingByNode(CAutomobileData *self, unsigned node)
{
	// Doors given as car nodes
	switch (node) {
	case CAR_DOOR_RF: return Automobile_IsDoorMissing(self, DOOR_FRONT_RIGHT);
	case CAR_DOOR_RR: return Automobile_IsDoorMissing(self, DOOR_REAR_RIGHT);
	case CAR_DOOR_LF: return Automobile_IsDoorMissing(self, DOOR_FRONT_LEFT);
	case CAR_DOOR_LR: return Automobile_IsDoorMissing(self, DOOR_REAR_LEFT);
	default:
		return false;
	}
}

bool Automobile_IsOpenTopCar(const CAutomobileStruct *self)
{
	int16_t model = self->base.base.base.entity.m_modelIndex;
	const CVehicleData *v = &self->base.vehicle;
	return model == MI_STINGER ||
		// component 0 is assumed to be a roof
		(model == MI_COMET && v->m_aExtras[0] != 0 && v->m_aExtras[1] != 0) ||
		(model == MI_STALLION && v->m_aExtras[0] != 0 && v->m_aExtras[1] != 0);
}

void Automobile_ResetSuspension(CAutomobileData *self)
{
	// Fully extended springs and stopped wheels
	int i;
	for (i = 0; i < 4; i++) {
		self->m_aSuspensionSpringRatio[i] = 1.0f;
		self->m_aWheelTimer[i] = 0.0f;
		self->m_aWheelRotation[i] = 0.0f;
		self->m_aWheelState[i] = WHEEL_STATE_NORMAL;
	}
}

bool Automobile_GetAllWheelsOffGround(const CAutomobileData *self)
{
	// No drive wheel touches the ground
	return self->m_nDriveWheelsOnGround == 0;
}

void Automobile_ReduceHornCounter(CVehicleData *vehicle)
{
	// Count the horn timer down to zero
	if (vehicle->m_nCarHornTimer != 0)
		vehicle->m_nCarHornTimer--;
}

static void SetFlyToCoors(CAutomobileStruct *self, int mission, float x, float y, float z, uint8_t speed)
{
	// Mission, destination and speed, then let the physics run
	CAutoPilot *ap = &self->base.vehicle.AutoPilot;
	ap->m_nCarMission = mission;
	ap->m_vecDestinationCoors.x = x;
	ap->m_vecDestinationCoors.y = y;
	ap->m_vecDestinationCoors.z = z;
	ap->m_nCruiseSpeed = speed;
	self->base.base.base.entity.m_status = STATUS_PHYSICS;
}

void Automobile_TellHeliToGoToCoors(CAutomobileStruct *self, float x, float y, float z, uint8_t speed)
{
	const CMatrixData *m = &self->base.base.base.placeable.m_matrix;
	SetFlyToCoors(self, MISSION_HELI_FLYTOCOORS, x, y, z, speed);

	// Start facing the way the helicopter points
	if (self->automobile.m_fOrientation == 0.0f) {
		self->automobile.m_fOrientation = General_GetATanOfXY(m->fx, m->fy) + PI;
		while (self->automobile.m_fOrientation > TWOPI) self->automobile.m_fOrientation -= TWOPI;
	}
}

void Automobile_TellPlaneToGoToCoors(CAutomobileStruct *self, float x, float y, float z, uint8_t speed)
{
	const CMatrixData *m = &self->base.base.base.placeable.m_matrix;
	SetFlyToCoors(self, MISSION_PLANE_FLYTOCOORS, x, y, z, speed);

	// Start facing the way the plane points
	if (self->automobile.m_fOrientation == 0.0f)
		self->automobile.m_fOrientation = General_GetATanOfXY(m->fx, m->fy);
}

void Automobile_PopBootUsingPhysics(CAutomobileData *self)
{
	// An intact or smashed boot swings open
	switch (Damage_GetDoorStatus(&self->Damage, DOOR_BOOT)) {
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		Damage_SetDoorStatus(&self->Damage, DOOR_BOOT, DOOR_STATUS_SWINGING);
	}
	self->Doors[DOOR_BOOT].m_fAngVel = -2.0f;
}

void Automobile_ClearCarNodes(CAutomobileData *self)
{
	// Forget all the frames
	int i;
	for (i = 0; i < NUM_CAR_NODES; i++)
		self->m_aCarNodes[i] = NULL;
}
void Automobile_SetBusDoorTimer(CAutomobileData *self, uint32_t timer, uint8_t type, uint32_t now)
{
	// At least a second
	if (timer < 1000)
		timer = 1000;
	if (type == 0)
		// Open and close
		self->m_nBusDoorTimerStart = now;
	else
		// Only close
		self->m_nBusDoorTimerStart = now - 500;
	self->m_nBusDoorTimerEnd = self->m_nBusDoorTimerStart + timer;
}

bool Automobile_HoverWheelInWater(CAutomobileData *self, int i, const float *upper, const float *lower, float waterZ)
{
	float z;

	// Water counts when it is above the contact point of the wheel, if it has one
	if (self->m_aSuspensionSpringRatio[i] < 1.0f)
		z = self->m_aWheelColPoints[i].point.z;
	else
		z = -100.0f;
	if (!(waterZ > z && lower[2]-1.0f < waterZ))
		return false;

	// Compress the spring
	if (lower[2] < waterZ) {
		if (upper[2] < waterZ)
			self->m_aSuspensionSpringRatio[i] = 0.0f;
		else
			self->m_aSuspensionSpringRatio[i] = (upper[2] - waterZ)/(upper[2] - lower[2]);
	} else
		self->m_aSuspensionSpringRatio[i] = 0.99999f;

	// The contact point is at the water surface with the normal up
	self->m_aWheelColPoints[i].point.x = (lower[0] - upper[0])*self->m_aSuspensionSpringRatio[i] + upper[0];
	self->m_aWheelColPoints[i].point.y = (lower[1] - upper[1])*self->m_aSuspensionSpringRatio[i] + upper[1];
	self->m_aWheelColPoints[i].point.z = waterZ;
	self->m_aWheelColPoints[i].normal.x = 0.0f;
	self->m_aWheelColPoints[i].normal.y = 0.0f;
	self->m_aWheelColPoints[i].normal.z = 1.0f;
	self->m_aWheelColPoints[i].surfaceB = SURFACE_WATER;
	return true;
}
int Automobile_ProcessAutoBusDoors(CAutomobileStruct *self, uint32_t now, const bool doorMissing[2], const bool doorClosed[2], AutomobileDoorAction actions[2])
{
	CAutomobileData *a = &self->automobile;
	int gettingIn = self->base.vehicle.m_nGettingInFlags;
	int n = 0;

	if (now < a->m_nBusDoorTimerEnd) {
		if (a->m_nBusDoorTimerEnd != 0 && now > a->m_nBusDoorTimerEnd-500) {
			// Close the door
			if (!doorMissing[0] && (gettingIn & CAR_DOOR_FLAG_LF) == 0) {
				if (doorClosed[0]) {
					a->m_nBusDoorTimerEnd = now;
					actions[n].component = CAR_DOOR_LF; actions[n].door = DOOR_FRONT_LEFT; actions[n].ratio = 0.0f; n++;
				} else {
					actions[n].component = CAR_DOOR_LF; actions[n].door = DOOR_FRONT_LEFT;
					actions[n].ratio = 1.0f - (now - (a->m_nBusDoorTimerEnd-500))/500.0f; n++;
				}
			}

			if (!doorMissing[1] && (gettingIn & CAR_DOOR_FLAG_RF) == 0) {
				if (doorClosed[1]) {
					a->m_nBusDoorTimerEnd = now;
					actions[n].component = CAR_DOOR_RF; actions[n].door = DOOR_FRONT_RIGHT; actions[n].ratio = 0.0f; n++;
				} else {
					actions[n].component = CAR_DOOR_RF; actions[n].door = DOOR_FRONT_RIGHT;
					actions[n].ratio = 1.0f - (now - (a->m_nBusDoorTimerEnd-500))/500.0f; n++;
				}
			}
		}
	} else {
		// Ended, shut both doors for good
		if (a->m_nBusDoorTimerStart) {
			if (!doorMissing[0] && (gettingIn & CAR_DOOR_FLAG_LF) == 0) {
				actions[n].component = CAR_DOOR_LF; actions[n].door = DOOR_FRONT_LEFT; actions[n].ratio = 0.0f; n++;
			}
			if (!doorMissing[1] && (gettingIn & CAR_DOOR_FLAG_RF) == 0) {
				actions[n].component = CAR_DOOR_RF; actions[n].door = DOOR_FRONT_RIGHT; actions[n].ratio = 0.0f; n++;
			}
			a->m_nBusDoorTimerStart = 0;
			a->m_nBusDoorTimerEnd = 0;
		}
	}
	return n;
}
void AutomobileLogic_Pedals(float speed, int16_t accelerate, int16_t brake, bool isDodo, float *gasPedal, float *brakePedal, bool *burnout)
{
	*burnout = false;

	float acceleration = (accelerate - brake)/255.0f;
	if(isDodo && acceleration < 0.0f)
		acceleration *= 0.3f;
	if(fabsf(speed) < 0.01f){
		// standing still, go into direction we want
		if(accelerate > 150.0f && brake > 150.0f){
			*gasPedal = accelerate/255.0f;
			*brakePedal = brake/255.0f;
			*burnout = true;
		}else{
			*gasPedal = acceleration;
			*brakePedal = 0.0f;
		}
	}else{
		if(speed * acceleration < 0.0f){
			// if opposite directions, have to brake first
			*gasPedal = acceleration;
			*brakePedal = fabsf(acceleration);
		}else{
			// accelerating in same direction we were already going
			*gasPedal = acceleration;
			*brakePedal = 0.0f;
		}
	}
}

float AutomobileLogic_SteerPad(float steerInput, int16_t steeringLeftRight, float timeStep)
{
	steerInput += (-steeringLeftRight/128.0f - steerInput)*
		0.2f*timeStep;
	return steerInput;
}

float AutomobileLogic_SteerMouse(float steerInput, float mouseX, float sensitivity, float centreRange, float centreMult, float timeStep)
{
	steerInput += sensitivity*mouseX;
	if(fabsf(steerInput) < centreRange)
		steerInput *= powf(centreMult, timeStep);
	return steerInput;
}

float AutomobileLogic_WheelAngle(float steerInput, float steeringLockDegrees)
{
	float fValue;
	if(steerInput < 0.0f)
		fValue = -(steerInput*steerInput);
	else
		fValue = steerInput*steerInput;
	return (steeringLockDegrees * (float)M_PI / 180.0f) * fValue;
}

int AutomobileLogic_ComedyControls(int state, int rnd, float *steerAngle)
{
	switch(state){
	case 0:
		if(rnd < 2)
			state = 1;
		else if(rnd < 4)
			state = 2;
		break;
	case 1:
		*steerAngle += 0.05f;
		if(rnd < 2)
			state = 0;
		break;
	case 2:
		*steerAngle -= 0.05f;
		if(rnd < 2)
			state = 0;
		break;
	}
	return state;
}

VectorMath3 AutomobileLogic_ApproxWheelOffset(int wheel, float boxMaxX, float boxMaxY, float boxMinY)
{
	VectorMath3 offset = { 0.0f, 0.0f, 0.0f };

	switch (wheel) {
	case 0:
		offset.x = -boxMaxX; offset.y = boxMaxY;
		break;
	case 1:
		offset.x = -boxMaxX; offset.y = boxMinY;
		break;
	case 2:
		offset.x = boxMaxX; offset.y = boxMaxY;
		break;
	case 3:
		offset.x = boxMaxX; offset.y = boxMinY;
		break;
	default:
		break;
	}
	return offset;
}

VectorMath3 AutomobileLogic_ApproxBikeWheelOffset(int wheel, float boxMaxY, float boxMinY)
{
	VectorMath3 offset = { 0.0f, 0.0f, 0.0f };

	switch (wheel) {
	case 0:
		offset.y = 0.8f * boxMaxY;
		break;
	case 1:
		offset.y = 0.8f * boxMinY;
		break;
	default:
		break;
	}
	return offset;
}

float Automobile_HydraulicSpringRatio(float springRatio, float lineTopZ, float wheelZ, float lineLength)
{
	if (springRatio < 1.0f) {
		springRatio = (lineTopZ - wheelZ) / lineLength;
		if (springRatio > 1.0f)
			springRatio = 1.0f;
	}
	return springRatio;
}

void Automobile_HydraulicWheelLift(float upDownInput, float leftRightInput, float lift[4])
{
	float rear = upDownInput / 128.0f;
	float front = -rear;
	float right = leftRightInput / 128.0f;
	float left = -right;
	float v;

	v = front + left;
	lift[CARWHEEL_FRONT_LEFT] = v > 0.0f ? v : 0.0f;
	v = rear + left;
	lift[CARWHEEL_REAR_LEFT] = v > 0.0f ? v : 0.0f;
	v = front + right;
	lift[CARWHEEL_FRONT_RIGHT] = v > 0.0f ? v : 0.0f;
	v = rear + right;
	lift[CARWHEEL_REAR_RIGHT] = v > 0.0f ? v : 0.0f;
}

bool Automobile_HydraulicSoundLevel(float maxDelta, float limitDiff, float *level)
{
	if (limitDiff != 0.0f && fabsf(maxDelta / limitDiff) > 0.01f) {
		float f = (maxDelta + limitDiff) / 2.0f / limitDiff;
		*level = f < 0.0f ? 0.0f : f > 1.0f ? 1.0f : f;
		return true;
	}
	return false;
}

float Automobile_PrevSpringRatio(float springRatio, float springLength, float lineLength)
{
	// Wheel radius in relation to the suspension line
	float wheelRadius = 1.0f - springLength / lineLength;
	return (springRatio - wheelRadius) / (1.0f - wheelRadius);
}
float Automobile_PedImpulseReduction(float impulse, float pedMoveDeltaY, float dotForwardNormal, float pedMass)
{
	float speed = pedMoveDeltaY * dotForwardNormal;

	if (speed < 0.0f) {
		float v = impulse + pedMass * speed;
		impulse = v > 0.0f ? v : 0.0f;
	}
	return impulse;
}

unsigned char Automobile_ShakeFrequency(float impulse, float mass)
{
	float v = 0.4f * impulse * 2000.0f / mass + 100.0f;
	return (unsigned char)(v < 250.0f ? v : 250.0f);
}

float Automobile_CollisionDamage(float impulse, float minImpulse, float collisionDamageMultiplier, float damageMultiplier, bool securicaHitByPlayer, bool isRC)
{
	float damage = (impulse - minImpulse) * collisionDamageMultiplier * 0.6f * damageMultiplier;

	if (securicaHitByPlayer)
		damage *= 7.0f;

	if (isRC)
		damage *= 30.0f;
	return damage;
}

float Automobile_HealthAfterDamage(float health, float damage, bool isPlayerVehicle, bool takeLessDamage, bool hitByPlayerVehicle)
{
	int oldHealth = (int)health;

	if (isPlayerVehicle)
		health -= takeLessDamage ? damage / 6.0f : damage / 2.0f;
	else if (takeLessDamage)
		health -= damage / 12.0f;
	else if (hitByPlayerVehicle)
		health -= damage / 1.5f;
	else
		health -= damage / 4.0f;
	if (health <= 0.0f && oldHealth > 0)
		health = 1.0f;
	return health;
}
float Automobile_WaterImpulseRatio(float impulseZ, float mass, float timeStep)
{
	float ts = timeStep > 0.01f ? timeStep : 0.01f;
	return impulseZ / (0.008f * mass * ts);
}

float Automobile_WaterResistance(float impulseRatio, float timeStep)
{
	return powf(1.0f - 0.05f * impulseRatio, timeStep);
}

bool Automobile_IsDrowning(bool isSeaSparrow, float impulseRatio, float upZ, bool hoversOnWater, bool heliHitWaterHard, bool anySuspensionFullyExtended)
{
	if ((isSeaSparrow && impulseRatio < 3.0f && (upZ > -0.5f || impulseRatio < 0.6f)) || hoversOnWater)
		return false;
	return heliHitWaterHard || impulseRatio > 1.0f || (impulseRatio > 0.6f && anySuspensionFullyExtended);
}

unsigned char Automobile_WaterSplashChannel(float directional, float ambient)
{
	return (unsigned char)((0.5f * directional + ambient) * 0.45f * 255);
}

float Automobile_WheelSplashSize(float speed)
{
	float v = (speed < 0.15f ? 0.25f : 0.75f) * speed;
	return v < 0.6f ? v : 0.6f;
}
void Automobile_FlyingComponentSpeed(const CVector *moveSpeed, float upZ, bool isBonnetBootOrWindscreen, CVector *out)
{
	out->x = moveSpeed->x;
	out->y = moveSpeed->y;
	out->z = moveSpeed->z;
	if (out->z > 0.0f) {
		out->z *= 1.5f;
	} else if (upZ > 0.0f && isBonnetBootOrWindscreen) {
		out->z *= -1.5f;
		out->z += 0.04f;
	} else {
		out->z *= 0.25f;
	}
	out->x *= 0.75f;
	out->y *= 0.75f;
}
int Automobile_BurstWheelSparkCount(float speedSqr, int surface)
{
	int count;

	if (!(speedSqr > (0.1f * 0.1f)) || surface == SURFACE_GRASS || surface == SURFACE_MUD_DRY || surface == SURFACE_SAND ||
		surface == SURFACE_SAND_BEACH || surface == SURFACE_WATER)
		return 0;

	count = 1;
	if (speedSqr > 0.04f)
		count++;
	if (speedSqr > 0.16f)
		count += 2;
	return count;
}

AutomobileSirenSetup Automobile_SirenSetup(int kind)
{
	AutomobileSirenSetup s = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0, 0, 0 }, { 0, 0, 0 } };

	switch (kind) {
	case AUTOMOBILE_SIREN_FIRETRUCK:
		s.pos1[0] = 1.1f;  s.pos1[1] = 1.7f; s.pos1[2] = 2.0f;
		s.pos2[0] = -1.1f; s.pos2[1] = 1.7f; s.pos2[2] = 2.0f;
		s.color1[0] = 255; s.color1[1] = 0; s.color1[2] = 0;
		s.color2[0] = 255; s.color2[1] = 255; s.color2[2] = 0;
		break;
	case AUTOMOBILE_SIREN_AMBULANCE:
		s.pos1[0] = 1.1f;  s.pos1[1] = 0.9f; s.pos1[2] = 1.6f;
		s.pos2[0] = -1.1f; s.pos2[1] = 0.9f; s.pos2[2] = 1.6f;
		s.color1[0] = 255; s.color1[1] = 0; s.color1[2] = 0;
		s.color2[0] = 255; s.color2[1] = 255; s.color2[2] = 255;
		break;
	case AUTOMOBILE_SIREN_POLICE:
		s.pos1[0] = 0.7f;  s.pos1[1] = -0.4f; s.pos1[2] = 1.0f;
		s.pos2[0] = -0.7f; s.pos2[1] = -0.4f; s.pos2[2] = 1.0f;
		s.color1[0] = 255; s.color1[1] = 0; s.color1[2] = 0;
		s.color2[0] = 0; s.color2[1] = 0; s.color2[2] = 255;
		break;
	case AUTOMOBILE_SIREN_ENFORCER:
		s.pos1[0] = 1.1f;  s.pos1[1] = 0.8f; s.pos1[2] = 1.2f;
		s.pos2[0] = -1.1f; s.pos2[1] = 0.8f; s.pos2[2] = 1.2f;
		s.color1[0] = 255; s.color1[1] = 0; s.color1[2] = 0;
		s.color2[0] = 0; s.color2[1] = 0; s.color2[2] = 255;
		break;
	}
	return s;
}

void Automobile_SirenLightColor(uint32_t timeMs, const unsigned char color1[3], const unsigned char color2[3], unsigned char out[3])
{
	uint32_t t = timeMs & 0x3FF;
	int i;

	for (i = 0; i < 3; i++)
		out[i] = (t < 512 ? color1[i] : color2[i]) / 6;

	t = timeMs & 0x1FF;
	if (t < 100) {
		float f = t / 100.0f;
		for (i = 0; i < 3; i++)
			out[i] = (unsigned char)(out[i] * f);
	} else if (t > (512 - 100)) {
		float f = (512 - t) / 100.0f;
		for (i = 0; i < 3; i++)
			out[i] = (unsigned char)(out[i] * f);
	}
}
float Automobile_WheelTilt(float wheelZ, float heightAboveRoad, float wheelScale, bool isVoodoo, bool rightSide)
{
	float groundOffset = wheelZ + heightAboveRoad - 0.5f * wheelScale;
	float v;

	if (isVoodoo)
		groundOffset *= 0.6f;
	v = rightSide ? -groundOffset : groundOffset;
	v = v < -1.0f ? -1.0f : v > 1.0f ? 1.0f : v;
	return asinf(v);
}

float Automobile_HeliDustAmount(float rotorSpeed, float timeStep)
{
	float steps = 16.0f - 4.0f * timeStep;

	return (rotorSpeed - 0.1125f) * ((int)(steps > 2.0f ? steps : 2.0f)) * 400.0f / 43.0f;
}
void Automobile_HeliRotorAlpha(float rotorSpeed, int *rotorAlpha, int *blurAlpha)
{
	float a = 1.7f * (rotorSpeed > 0.0f ? rotorSpeed : 0.0f) / 0.22f;
	float b = 1.5f * rotorSpeed / 0.22f - 0.4f;

	*rotorAlpha = (int)((1.5f - (a < 1.5f ? a : 1.5f)) * 255.0f);
	*rotorAlpha = *rotorAlpha < 255 ? *rotorAlpha : 255;
	*blurAlpha = (int)((b > 0.0f ? b : 0.0f) * 150.0f);
	*blurAlpha = *blurAlpha < 150 ? *blurAlpha : 150;
}

void Automobile_CannonDirection(float angleLR, float angleUD, CVector *dir)
{
	dir->x = sinf(angleLR) * cosf(angleUD);
	dir->y = cosf(angleLR) * cosf(angleUD);
	dir->z = sinf(angleUD);
}

float Automobile_TankTurretAngle(float angle)
{
	const float twopi = (float)3.14159265358979323846 * 2;

	if (angle < 0.0f)
		angle += twopi;
	if (angle > twopi)
		angle -= twopi;
	return angle;
}

void Automobile_TankGunEnd(float angle, const CVector *baseToEnd, const CVector *turretBase, CVector *out)
{
	float c = cosf(angle);
	float s = sinf(angle);

	out->x = c * baseToEnd->x - s * baseToEnd->y;
	out->y = s * baseToEnd->x + c * baseToEnd->y;
	out->z = baseToEnd->z - 1.0f;	// Correct the offset here
	out->x += turretBase->x;
	out->y += turretBase->y;
	out->z += turretBase->z;
}

void Automobile_StrongGrip(bool playerDrivesOtherVehicle, int wantedLevel, int carMission, float playerSpeed, float ownSpeed, float distToPlayer, bool isRCBandit, bool notRemoteControlled, bool *strongGrip1, bool *strongGrip2)
{
	*strongGrip1 = false;
	*strongGrip2 = false;
	if (playerDrivesOtherVehicle && wantedLevel > 3 &&
	    (carMission == MISSION_RAMPLAYER_FARAWAY || carMission == MISSION_RAMPLAYER_CLOSE ||
	     carMission == MISSION_BLOCKPLAYER_FARAWAY || carMission == MISSION_BLOCKPLAYER_CLOSE) &&
	    playerSpeed > 0.3f) {
		*strongGrip1 = true;
		if (playerSpeed > 0.4f && ownSpeed < 0.3f)
			*strongGrip2 = true;
		else if (distToPlayer > 50.0f)
			*strongGrip2 = true;
	} else if (isRCBandit && notRemoteControlled)
		*strongGrip1 = true;
}

bool Automobile_TwoWheelCentreOfMassZ(float handlingCentreZ, int twoWheelTime, float steeringLeftRight, float balanceMult, float colMaxZ, float upZ, float rightZ, bool isHeliOrPlane, float *centreZ)
{
	if (twoWheelTime > 500 && !isHeliOrPlane) {
		int t = twoWheelTime - 500;
		float tweak = (t < 1000 ? t : 1000) / 500.0f;
		if (upZ > 0.0f) {
			// Positive when on the left wheels, negative on the right wheels
			if (rightZ <= 0.0f)
				tweak *= -1.0f;
			*centreZ = handlingCentreZ + steeringLeftRight / 128.0f * balanceMult * tweak * colMaxZ;
			return true;
		}
		return false;
	}
	*centreZ = handlingCentreZ;
	return true;
}

float Automobile_SandSlowdown(float parallelSpeedSqr, bool goodInSand, float wetRoads, float sandSlowdown, float *parallelScale, bool *stuck)
{
	float slowdown;

	*parallelScale = 1.0f;
	*stuck = false;
	if (parallelSpeedSqr > (0.3f * 0.3f)) {
		float fSpeed = sqrtf(parallelSpeedSqr);
		*parallelScale = 0.3f / fSpeed;
		float v = 1.0f - 2.0f * fSpeed;
		slowdown = sandSlowdown * (v > 0.2f ? v : 0.2f);
	} else {
		*stuck = true;
		slowdown = sandSlowdown;
	}
	if (goodInSand)
		slowdown *= 0.5f;
	if (wetRoads > 0.2f)
		slowdown *= (1.2f - wetRoads);
	return slowdown;
}
bool Automobile_IsBarelyMoving(bool isWrecked, float moveSpeedAvgSqr, float turnSpeedAvgSqr, float timeStep, float distanceTravelled, bool damagedByPed, bool makeStatic)
{
	float moveSpeedLimit, turnSpeedLimit, distanceLimit;

	if (isWrecked) {
		moveSpeedLimit = 0.006f;
		turnSpeedLimit = 0.0015f;
		distanceLimit = 0.015f;
	} else {
		moveSpeedLimit = 0.003f;
		turnSpeedLimit = 0.0009f;
		distanceLimit = 0.005f;
	}

	return (moveSpeedAvgSqr <= (moveSpeedLimit * timeStep) * (moveSpeedLimit * timeStep) &&
		turnSpeedAvgSqr <= (turnSpeedLimit * timeStep) * (turnSpeedLimit * timeStep) &&
		distanceTravelled < distanceLimit &&
		!damagedByPed) ||
		makeStatic;
}

float Automobile_SandSpringRatio(float springRatio, float fwdSpeed, float wetRoads, float lineLength, float springLength)
{
	float f = 1.0f - fwdSpeed / 0.3f - 0.7f * wetRoads;
	f = f > 0.4f ? f : 0.4f;
	springRatio += 0.35f * f * (lineLength - springLength) / springLength;
	if (springRatio > 1.0f)
		springRatio = 1.0f;
	return springRatio;
}
float Automobile_FreeWheelSpeed(float wheelSpeed, float acceleration, bool isDriven, bool handbrakeOn)
{
	if (handbrakeOn)
		return 0.0f;
	if (isDriven && acceleration != 0.0f) {
		if (acceleration > 0.0f) {
			if (wheelSpeed < 2.0f)
				wheelSpeed -= 0.2f;
		} else {
			if (wheelSpeed > -2.0f)
				wheelSpeed += 0.1f;
		}
	} else {
		wheelSpeed *= 0.95f;
	}
	return wheelSpeed;
}

void Automobile_WheelBiases(bool neutralHandling, float brakeBias, float tractionBias, float biases[4])
{
	float tractionBiasFront = neutralHandling ? 1.0f : 2.0f * tractionBias;

	biases[0] = neutralHandling ? 1.0f : 2.0f * brakeBias;
	biases[1] = neutralHandling ? 1.0f : 2.0f - brakeBias;
	biases[2] = tractionBiasFront;
	biases[3] = neutralHandling ? 1.0f : 2.0f - tractionBiasFront;
}

float Automobile_TireTemperature(float temperature, bool burnoutWithSpinningWheel, float timeStep)
{
	if (burnoutWithSpinningWheel) {
		temperature += 0.001f * timeStep;
		if (temperature > 3.0f)
			temperature = 3.0f;
	} else if (temperature > 1.0f) {
		temperature = (temperature - 1.0f) * powf(0.995f, timeStep) + 1.0f;
	}
	return temperature;
}
float Automobile_SurfaceShake(int surface, float currentShake)
{
	if (surface == SURFACE_GRAVEL || surface == SURFACE_WATER || surface == SURFACE_HEDGE) {
		if (currentShake < 0.2f)
			currentShake = 0.3f;
	} else if (surface == SURFACE_MUD_DRY) {
		if (currentShake < 0.1f)
			currentShake = 0.2f;
	} else if (surface == SURFACE_GRASS) {
		if (currentShake < 0.05f)
			currentShake = 0.1f;
	}
	return currentShake;
}

unsigned char Automobile_PadShakeFrequency(float shake, float speed, float mass, float base, float cap)
{
	float v = 200.0f * shake * speed * 2000.0f / mass + base;
	return (unsigned char)(v < cap ? v : cap);
}

float Automobile_WheelPosition(float lineTopZ, float springRatio, float springLength, bool followsDirectly, float currentPosition)
{
	float wheelPos = lineTopZ;

	if (springRatio > 0.0f)
		wheelPos -= springRatio * springLength;
	if (followsDirectly)
		return wheelPos;
	return currentPosition + (wheelPos - currentPosition) * 0.75f;
}

float Automobile_WheelWobble(float wheelRotation)
{
	return 0.3f * sinf(wheelRotation);
}
float Automobile_AIFireTruckGunUD(uint32_t timeMs)
{
	const float twoPi = (float)3.14159265358979323846 * 2;
	float upDown = sinf((float)(timeMs & 0xFFF) / 0x1000 * twoPi);

	return 0.2f + 0.2f * upDown;
}
float Automobile_WingPropeller(bool isBFInjection, bool gasPressed, float *propellerRotation, float timeStep, uint32_t timeMs)
{
	const float twoPi = (float)3.14159265358979323846 * 2;
	float rotation = 0.0f;

	if (isBFInjection)
		if (*propellerRotation > twoPi)
			*propellerRotation -= twoPi;

	if (gasPressed) {
		if (isBFInjection) {
			*propellerRotation += 0.2f * timeStep;
			rotation = *propellerRotation;
		} else {
			if (*propellerRotation < 1.3f) {
				float v = *propellerRotation + 0.1f * timeStep;
				*propellerRotation = v < 1.3f ? v : 1.3f;
				rotation = *propellerRotation;
			} else {
				float wave = sinf((timeMs % 10000) / 70.0f);
				rotation = *propellerRotation + 0.13 * wave;
			}
		}
	} else {
		if (isBFInjection) {
			*propellerRotation += 0.1f * timeStep;
			rotation = *propellerRotation;
		} else {
			if (*propellerRotation > 0.0f) {
				float v = *propellerRotation - 0.05f * timeStep;
				*propellerRotation = v > 0.0f ? v : 0.0f;
				rotation = *propellerRotation;
			}
		}
	}
	return rotation;
}
void Automobile_SteerForward(float steerAngle, float forward[2])
{
	forward[0] = -sinf(steerAngle);
	forward[1] = cosf(steerAngle);
}
float Automobile_WheelAdhesion(float adhesiveLimit, float traction, bool isPlayer, float wetMultiplier)
{
	float adhesion = adhesiveLimit * traction;

	if (isPlayer)
		adhesion *= wetMultiplier;
	return adhesion;
}
void Automobile_SteeredWheelAxes(const CVector *forward, const CVector *normal, float s, float c, CVector *wheelFwd, CVector *wheelRight)
{
	VectorMath3 fwd = { forward->x, forward->y, forward->z };
	VectorMath3 n = { normal->x, normal->y, normal->z };
	VectorMath3 right, tmp;
	float dot = fwd.x * n.x + fwd.y * n.y + fwd.z * n.z;

	fwd.x -= dot * n.x;
	fwd.y -= dot * n.y;
	fwd.z -= dot * n.z;
	VectorMath_Normalise(&fwd);
	right = VectorMath_Cross(fwd, n);
	VectorMath_Normalise(&right);
	tmp.x = c * fwd.x - s * right.x;
	tmp.y = c * fwd.y - s * right.y;
	tmp.z = c * fwd.z - s * right.z;
	right.x = s * fwd.x + c * right.x;
	right.y = s * fwd.y + c * right.y;
	right.z = s * fwd.z + c * right.z;
	wheelFwd->x = tmp.x;
	wheelFwd->y = tmp.y;
	wheelFwd->z = tmp.z;
	wheelRight->x = right.x;
	wheelRight->y = right.y;
	wheelRight->z = right.z;
}
void Automobile_SurfaceWheelAxes(const CVector *forward, const CVector *normal, CVector *wheelFwd, CVector *wheelRight)
{
	VectorMath3 fwd = { forward->x, forward->y, forward->z };
	VectorMath3 n = { normal->x, normal->y, normal->z };
	VectorMath3 right;
	float dot = fwd.x * n.x + fwd.y * n.y + fwd.z * n.z;

	fwd.x -= dot * n.x;
	fwd.y -= dot * n.y;
	fwd.z -= dot * n.z;
	VectorMath_Normalise(&fwd);
	right = VectorMath_Cross(fwd, n);
	VectorMath_Normalise(&right);
	wheelFwd->x = fwd.x;
	wheelFwd->y = fwd.y;
	wheelFwd->z = fwd.z;
	wheelRight->x = right.x;
	wheelRight->y = right.y;
	wheelRight->z = right.z;
}
bool Automobile_RotorSoundCheck(float distSq, float propellerRotation, float bladeRotation)
{
	return distSq < 20.0f * 20.0f && fabsf(propellerRotation - bladeRotation) > 30.0f * (float)M_PI / 180.0f;
}

bool Automobile_RotorBladeInLine(float dot, float limit)
{
	return fabsf(dot) > limit;
}
float Automobile_CannonTrackStep(float gunAngle, float targetAngle, float cannonDelta, float angleStep)
{
	if (fabsf(cannonDelta) < angleStep)
		return targetAngle;
	else if (cannonDelta > 0.0f)
		return gunAngle + angleStep;
	return gunAngle - angleStep;
}
void Automobile_SirenPosition(const float pos1[3], const float pos2[3], int i, float out[3])
{
	int k;

	for (k = 0; k < 3; k++)
		out[k] = (pos1[k] * i + pos2[k] * (3.0f - i)) / 3.0f;
}
float Automobile_Behindness(float dot, float *angle)
{
	float behindness = dot < -1.0f ? -1.0f : dot > 1.0f ? 1.0f : dot;

	*angle = fabsf(acosf(behindness));
	return behindness;
}
void Automobile_RoadAlignedMatrix(const float front[2], const float rear[2], float frontZ, float rearZ, float len, float heightAboveRoad, CVector *right, CVector *forward, CVector *up, CVector *pos)
{
	float angle = atanf((frontZ - rearZ) / len);
	float cs = cosf(angle);
	float sn = sinf(angle);
	VectorMath3 r, f, u;

	r.x = (front[1] - rear[1]) / len;
	r.y = -(front[0] - rear[0]) / len;
	r.z = 0.0f;
	f.x = -cs * r.y;
	f.y = cs * r.x;
	f.z = sn;
	u = VectorMath_Cross(r, f);
	right->x = r.x; right->y = r.y; right->z = r.z;
	forward->x = f.x; forward->y = f.y; forward->z = f.z;
	up->x = u.x; up->y = u.y; up->z = u.z;
	pos->x = (front[0] + rear[0]) / 2.0f;
	pos->y = (front[1] + rear[1]) / 2.0f;
	pos->z = (frontZ + rearZ) / 2.0f + heightAboveRoad;
}
bool Automobile_SpeedAbove(float speed, float limit)
{
	return fabsf(speed) > limit;
}

bool Automobile_SpeedBelow(float speed, float limit)
{
	return fabsf(speed) < limit;
}

bool Automobile_LineDeltaExceeds(float posZ, float lineZ, float maxDelta)
{
	return fabsf(posZ - lineZ) > fabsf(maxDelta);
}

float Automobile_WheelNudgeForce(float mass, float factor)
{
	float v = mass * factor;

	return 50.0f * (1.0f < v ? 1.0f : v);
}
void Automobile_RocketLead(float forwardSpeed, const CVector *forward, float timeStep, CVector *lead)
{
	float s = forwardSpeed > 0.0f ? forwardSpeed : 0.0f;

	lead->x = s * forward->x * timeStep;
	lead->y = s * forward->y * timeStep;
	lead->z = s * forward->z * timeStep;
}

float Automobile_SafeDistance(float distSq)
{
	float d = sqrtf(distSq);

	return d > 0.01f ? d : 0.01f;
}

float Automobile_SmokeLift(float speed)
{
	float v = 1.0f - 1.6f * speed;

	return 0.05f * (v > 0.0f ? v : 0.0f);
}

float Automobile_WaterImpactDivisor(float impulseRatio)
{
	float v = 8.0f * impulseRatio;

	return v > 1.0f ? v : 1.0f;
}
void Automobile_TurretDirection(float turretAngle, float dir[2])
{
	dir[0] = sinf(-turretAngle);
	dir[1] = cosf(-turretAngle);
}

bool Automobile_CameraIsFarAway(float dx, float dy, float limit)
{
	return fabsf(dx) + fabsf(dy) > limit;
}
bool Automobile_WheelsOnBothSides(const float springRatio[4])
{
	return (springRatio[0] < 1.0f || springRatio[2] < 1.0f) &&
		(springRatio[1] < 1.0f || springRatio[3] < 1.0f);
}

float Automobile_AntiRollStrength(float gravity, float turnMass)
{
	return -gravity * (turnMass < 2500.0f ? turnMass : 2500.0f);
}
//- rouz edit (Claude)
