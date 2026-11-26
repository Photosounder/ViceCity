//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "VehicleLogic.h"
#include "Vector.h" // rouz edit (Claude)
#include "WeaponType.h"
#include "SurfaceTypes.h"
#include "General.h"
#include "WheelState.h"
#include "CarLock.h"
#include "HandlingFlags.h"
#include "VehicleTypes.h"
#include "VehicleCreation.h"
#include "PedStates.h"
#include "PedLogic.h"
#include "EntityLayout.h"
#include "CarNodes.h"
#include "VehicleDoors.h"

static float MagSqr(PhysicalVec v)
{
	// Squared length with the same operation order as the engine vector
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

bool VehicleLogic_CarHasRoof(const CVehicleData *vehicle)
{
	// Cars without the no-roof flag always have one
	if ((vehicle->pHandling->Flags & HANDLING_HAS_NO_ROOF) == 0)
		return true;
	// component 0 is assumed to be a roof
	return vehicle->m_aExtras[0] == 0 || vehicle->m_aExtras[1] == 0;
}

bool VehicleLogic_IsUpsideDown(const CPlaceableData *placeable)
{
	// The up axis points mostly downwards
	if (placeable->m_matrix.uz > -0.9f)
		return false;
	return true;
}

bool VehicleLogic_IsOnItsSide(const CPlaceableData *placeable)
{
	// The right axis points mostly up or down
	if (placeable->m_matrix.rz < 0.8f && placeable->m_matrix.rz > -0.8f)
		return false;
	return true;
}

bool VehicleLogic_CanPedOpenLocks(const CVehicleData *vehicle, bool pedIsPlayer)
{
	// Locked doors cannot be opened, the player is also kept out by the player lockout
	if (vehicle->m_nDoorLock == CARLOCK_LOCKED ||
	    vehicle->m_nDoorLock == CARLOCK_LOCKED_INITIALLY ||
	    vehicle->m_nDoorLock == CARLOCK_LOCKED_PLAYER_INSIDE ||
	    vehicle->m_nDoorLock == CARLOCK_LOCKED_BUT_CAN_BE_DAMAGED)
		return false;
	if (pedIsPlayer && vehicle->m_nDoorLock == CARLOCK_LOCKOUT_PLAYER_ONLY)
		return false;
	return true;
}

bool VehicleLogic_CanDoorsBeDamaged(const CVehicleData *vehicle)
{
	// Only some lock states allow door damage
	return vehicle->m_nDoorLock == CARLOCK_NOT_USED ||
		vehicle->m_nDoorLock == CARLOCK_UNLOCKED ||
		vehicle->m_nDoorLock == CARLOCK_LOCKED_BUT_CAN_BE_DAMAGED;
}

bool VehicleLogic_CanPedEnterCar(const CPlaceableData *placeable, const CPhysicalData *physical, const CVehicleData *vehicle)
{
	// can't enter when car is on side
	if (vehicle->m_vehType == VEHICLE_TYPE_BIKE || placeable->m_matrix.uz > 0.1f || placeable->m_matrix.uz < -0.1f) {
		// also when car is moving too fast
		if (MagSqr(physical->m_vecMoveSpeed) > 0.2f * 0.2f)
			return false;
		if (MagSqr(physical->m_vecTurnSpeed) > 0.2f * 0.2f)
			return false;
		return true;
	}
	return false;
}

bool VehicleLogic_CanPedExitCar(const CPlaceableData *placeable, const CPhysicalData *physical, const CVehicleData *vehicle, bool jumpExit)
{
	// speed threshold is 80 km/h
	if (MagSqr(physical->m_vecMoveSpeed) < 0.2f)
		return true;
	if (placeable->m_matrix.uz > 0.1f || placeable->m_matrix.uz < -0.1f) {
		if (vehicle->m_vehType == VEHICLE_TYPE_BOAT)
			return true;
		// can't exit when car is moving too fast
		if (MagSqr(physical->m_vecMoveSpeed) > 0.005f && !jumpExit)
			return false;
		// if car is slow enough, check turn speed
		if (fabsf(physical->m_vecTurnSpeed.x) > 0.01f ||
		    fabsf(physical->m_vecTurnSpeed.y) > 0.01f ||
		    fabsf(physical->m_vecTurnSpeed.z) > 0.01f)
			return false;
		return true;
	} else {
		// can't exit when car is moving too fast
		if (MagSqr(physical->m_vecMoveSpeed) >= 0.005f)
			return false;
		// if car is slow enough, check turn speed
		if (fabsf(physical->m_vecTurnSpeed.x) >= 0.01f ||
		    fabsf(physical->m_vecTurnSpeed.y) >= 0.01f ||
		    fabsf(physical->m_vecTurnSpeed.z) >= 0.01f)
			return false;
		return true;
	}
}

bool VehicleLogic_IsDriver(const CVehicleData *vehicle, const CPed *ped)
{
	// A non-null ped that is the driver
	return ped && ped == vehicle->pDriver;
}

bool VehicleLogic_IsPassenger(const CVehicleData *vehicle, const CPed *ped)
{
	// A non-null ped found in one of the eight passenger slots
	int i;
	if (ped == NULL)
		return false;
	for (i = 0; i < 8; i++)
		if (vehicle->pPassengers[i] == ped)
			return true;
	return false;
}

void VehicleLogic_UpdatePassengerList(CVehicleData *vehicle)
{
	// Reset the passenger count when every slot is actually empty
	int i;
	bool hasPassenger = false;
	if (vehicle->m_nNumPassengers)
		for (i = 0; i < 8; i++)
			if (vehicle->pPassengers[i]) {
				hasPassenger = true;
				break;
			}
	if (!hasPassenger)
		vehicle->m_nNumPassengers = 0;
}

void VehicleLogic_ProcessCarAlarm(CVehicleData *vehicle, uint32_t stepMs)
{
	// Alarm states 0 (off) and -1 (not armed yet) do not count down
	if (vehicle->m_nAlarmState == 0 || vehicle->m_nAlarmState == -1)
		return;

	if ((uint16_t)vehicle->m_nAlarmState < stepMs) {
		vehicle->m_nAlarmState = 0;
		vehicle->m_nCarHornTimer = 0;
	} else
		vehicle->m_nAlarmState -= stepMs;
}

bool VehicleLogic_CanBeDeleted(const CVehicleData *vehicle)
{
	int i;

	if (vehicle->m_nNumGettingIn || vehicle->m_nGettingOutFlags)
		return false;

	if (vehicle->pDriver) {
		// This looks like it was inlined
		const CPedData *driver = &((const CPedStruct *)vehicle->pDriver)->ped;
		if (driver->CharCreatedBy == MISSION_CHAR)
			return false;
		if (driver->m_nPedState != PED_DRIVING &&
		    driver->m_nPedState != PED_DEAD)
			return false;
	}

	for (i = 0; i < 8; i++) {
		// Same check as above
		if (vehicle->pPassengers[i]) {
			const CPedData *passenger = &((const CPedStruct *)vehicle->pPassengers[i])->ped;
			if (passenger->CharCreatedBy == MISSION_CHAR)
				return false;
			if (passenger->m_nPedState != PED_DRIVING &&
			    passenger->m_nPedState != PED_DEAD)
				return false;
		}
		// and then again... probably because something was inlined
		if (vehicle->pPassengers[i]) {
			const CPedData *passenger = &((const CPedStruct *)vehicle->pPassengers[i])->ped;
			if (passenger->m_nPedState != PED_DRIVING &&
			    passenger->m_nPedState != PED_DEAD)
				return false;
		}
	}

	switch (vehicle->VehicleCreatedBy) {
	case RANDOM_VEHICLE: return true;
	case MISSION_VEHICLE: return false;
	case PARKED_VEHICLE: return true;
	case PERMANENT_VEHICLE: return false;
	}
	return true;
}
bool VehicleLogic_AddPassengerToFirstFreeSeat(CVehicleData *vehicle, CPed *passenger)
{
	int i;
	for (i = 0; i < vehicle->m_nNumMaxPassengers; i++)
		if (vehicle->pPassengers[i] == NULL) {
			vehicle->pPassengers[i] = passenger;
			vehicle->m_nNumPassengers++;
			return true;
		}
	return false;
}

bool VehicleLogic_AddPassengerToSeat(CVehicleData *vehicle, CPed *passenger, uint8_t n)
{
	if (n < vehicle->m_nNumMaxPassengers && vehicle->pPassengers[n] == NULL) {
		vehicle->pPassengers[n] = passenger;
		vehicle->m_nNumPassengers++;
		return true;
	}
	return false;
}

void VehicleLogic_RemovePassenger(CVehicleData *vehicle, const CPed *passenger, bool isTrain)
{
	int i;

	// Trains use every seat, other vehicles only the ones they have
	int seats = isTrain ? 8 : vehicle->m_nNumMaxPassengers;
	for (i = 0; i < seats; i++) {
		if (vehicle->pPassengers[i] == passenger) {
			vehicle->pPassengers[i] = NULL;
			vehicle->m_nNumPassengers--;
			return;
		}
	}
}
// Move the passenger in one seat to another, and tell the ped which door it uses now
static void MovePassenger(CVehicleData *v, int from, int to, uint16_t door)
{
	v->pPassengers[to] = v->pPassengers[from];
	v->pPassengers[from] = NULL;
	((CPedStruct*)v->pPassengers[to])->ped.m_vehDoor = door;
}

bool VehicleLogic_ShufflePassengersToMakeSpace(CVehicleData *v, VehicleRoomFn isRoom, void *context)
{
	// Nothing to do when the vehicle is full
	if (v->m_nNumPassengers >= v->m_nNumMaxPassengers)
		return false;

	// Passenger in the left rear seat
	if (v->pPassengers[1] &&
		!(v->m_nGettingInFlags & CAR_DOOR_FLAG_LR) &&
		isRoom(context, CAR_DOOR_LR)) {
		if (!v->pPassengers[2] && !(v->m_nGettingInFlags & CAR_DOOR_FLAG_RR)) {
			MovePassenger(v, 1, 2, CAR_DOOR_RR);
			return true;
		}
		if (!v->pPassengers[0] && !(v->m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
			MovePassenger(v, 1, 0, CAR_DOOR_RF);
			return true;
		}
		return false;
	}

	// Passenger in the right rear seat
	if (v->pPassengers[2] &&
		!(v->m_nGettingInFlags & CAR_DOOR_FLAG_RR) &&
		isRoom(context, CAR_DOOR_RR)) {
		if (!v->pPassengers[1] && !(v->m_nGettingInFlags & CAR_DOOR_FLAG_LR)) {
			MovePassenger(v, 2, 1, CAR_DOOR_LR);
			return true;
		}
		if (!v->pPassengers[0] && !(v->m_nGettingInFlags & CAR_DOOR_FLAG_RF)) {
			MovePassenger(v, 2, 0, CAR_DOOR_RF);
			return true;
		}
		return false;
	}

	// Passenger in the front seat
	if (v->pPassengers[0] &&
		!(v->m_nGettingInFlags & CAR_DOOR_FLAG_RF) &&
		isRoom(context, CAR_DOOR_RF)) {
		if (!v->pPassengers[1] && !(v->m_nGettingInFlags & CAR_DOOR_FLAG_LR)) {
			MovePassenger(v, 0, 1, CAR_DOOR_LR);
			return true;
		}
		if (!v->pPassengers[2] && !(v->m_nGettingInFlags & CAR_DOOR_FLAG_RR)) {
			MovePassenger(v, 0, 2, CAR_DOOR_RR);
			return true;
		}
		return false;
	}
	return false;
}
bool VehicleLogic_PlayCarHorn(CVehicleData *vehicle, bool alarmOn, VehicleRandomFn random)
{
	uint32_t r;

	// Silent while the alarm or a honk is going on
	if (alarmOn || vehicle->m_nCarHornTimer != 0)
		return false;

	// Wait out the delay since the last honk
	if (vehicle->m_nCarHornDelay) {
		vehicle->m_nCarHornDelay--;
		return false;
	}

	// Pick the next delay, its low bits choose between a honk, a honk with a shout and a shout
	vehicle->m_nCarHornDelay = (random() & 0x7F) + 150;
	r = vehicle->m_nCarHornDelay & 7;
	if (r < 2) {
		vehicle->m_nCarHornTimer = 45;
		return false;
	} else if (r < 4) {
		vehicle->m_nCarHornTimer = 45;
		return true;
	}
	return true;
}
float VehicleLogic_PlaneSteerMultiplier(float steerUpDown, float steerLeftRight)
{
	const float pi = (float)3.14159265358979323846;
	const float halfpi = pi / 2;
	float fSteerAngle = atan2f(steerUpDown, steerLeftRight);
	float fSteerMult = 1.0f;

	if (fSteerAngle > -pi / 4.0f && fSteerAngle <= pi / 4.0f)
		fSteerMult = 1.0f / cosf(fSteerAngle);
	else if (fSteerAngle > pi / 4.0f && fSteerAngle <= pi * 3.0f / 4.0f)
		fSteerMult = 1.0f / cosf(fSteerAngle - halfpi);
	else if (fSteerAngle > pi * 3.0f / 4.0f)
		fSteerMult = 1.0f / cosf(fSteerAngle - pi);
	else if (fSteerAngle <= -pi * 3.0f / 4.0f)
		fSteerMult = 1.0f / cosf(fSteerAngle + pi);
	else if (fSteerAngle > -pi * 3.0f / 4.0f && fSteerAngle < -pi / 4.0f)
		fSteerMult = 1.0f / cosf(fSteerAngle + halfpi);
	return fSteerMult;
}

float VehicleLogic_PlaneThrustAccel(float pedalState, float forwardSpeed, float thrust, float thrustFallOff, float thrustFallOffBack, bool isUnusedPlane, bool isPlane)
{
	float fThrustAccel;

	if (forwardSpeed > 0.0f || pedalState > 0.0f)
		fThrustAccel = (pedalState - thrustFallOff * forwardSpeed) * thrust;
	else {
		float v = pedalState - thrustFallOffBack * forwardSpeed;
		fThrustAccel = (v < 0.0f ? v : 0.0f) * thrust;
	}
	if (isUnusedPlane)
		fThrustAccel *= 0.3f;
	else if (isPlane)
		fThrustAccel *= 0.1f;
	return fThrustAccel;
}

float VehicleLogic_HeliThrust(float pedalState, bool isRCHeli, float flyingThrust, float thrustFallOff, float upSpeed, float height)
{
	float fThrust = pedalState;

	if (fThrust < 0.0f)
		fThrust *= 2.0f;
	if (isRCHeli)
		fThrust = flyingThrust * fThrust + 0.45f;
	else
		fThrust = flyingThrust * fThrust + 0.95f;
	fThrust -= thrustFallOff * upSpeed;
	if (isRCHeli && height > 40.0f)
		fThrust *= 10.0f / (height - 30.0f);
	else if (height > 80.0f)
		fThrust *= 10.0f / (height - 70.0f);
	return fThrust;
}

void VehicleLogic_HeliStabilise(float upZ, float rightZ, float forwardZ, float formLift, float *upRight, float *upFwd)
{
	if (upZ > 0.0f) {
		float r = rightZ, f = forwardZ;
		*upRight = r < -formLift ? -formLift : r > formLift ? formLift : r;
		*upFwd = f < -formLift ? -formLift : f > formLift ? formLift : f;
	} else {
		*upRight = rightZ < 0.0f ? -formLift : formLift;
		*upFwd = forwardZ < 0.0f ? -formLift : formLift;
	}
}
void VehicleLogic_ProcessWheel(const VehicleWheelInput *in, bool *alreadySkidding, VehicleWheelOutput *out)
{
	float thrust = in->thrust;
	float brake = in->brake;
	float adhesion = in->adhesion;
	int wheelState = in->wheelState;
	float contactSpeedFwd = in->contactSpeedFwd;
	float contactSpeedRight = in->contactSpeedRight;
	int wheelsOnGround = in->wheelsOnGround;

	// How much force we want to apply in these axes
	float fwd = 0.0f;
	float right = 0.0f;

	bool bBraking = brake != 0.0f;
	if (bBraking)
		thrust = 0.0f;
	bool bDriving = thrust != 0.0f;

	if (wheelState != WHEEL_STATE_NORMAL)
		*alreadySkidding = true;
	wheelState = WHEEL_STATE_NORMAL;

	adhesion *= in->timeStep;
	if (*alreadySkidding)
		adhesion *= in->tractionLoss;

	if (in->slippery) {
		if (in->isFrontWheel)
			adhesion *= 0.2f;
		else
			adhesion *= 0.19f;
	}

	// Moving sideways
	if (contactSpeedRight != 0.0f) {
		// Exert an opposing force
		right = -contactSpeedRight / wheelsOnGround;

		if (in->isBurst) {
			float fwdspeed = contactSpeedFwd < in->burstSpeedMax ? contactSpeedFwd : in->burstSpeedMax;
			right += fwdspeed * General_GetRandomNumberInRangeFloat(-in->burstTyreMod, in->burstTyreMod);
		}
	}

	if (bDriving) {
		fwd = thrust;

		// Limit the sideways force
		if (right > 0.0f) {
			if (right > adhesion)
				right = adhesion;
		} else {
			if (right < -adhesion)
				right = -adhesion;
		}
	} else if (contactSpeedFwd != 0.0f) {
		fwd = -contactSpeedFwd / wheelsOnGround;

		if (!bBraking) {
			if (in->gasPedal < 0.01f) {
				if (in->isBike)
					brake = 0.6f * in->wheelFriction / (in->mass + 200.0f);
				else if (in->mass < 500.0f)
					brake = 0.2f * in->wheelFriction / in->mass;
				else if (in->isRCBandit)
					brake = 0.2f * in->wheelFriction / in->mass;
				else
					brake = in->wheelFriction / in->mass;
				if (in->useTimeStepFix)
					brake *= in->timeStepFix;
			}
		}

		if (brake > adhesion) {
			if (fabsf(contactSpeedFwd) > 0.005f)
				wheelState = WHEEL_STATE_FIXED;
		} else {
			if (fwd > 0.0f) {
				if (fwd > brake)
					fwd = brake;
			} else {
				if (fwd < -brake)
					fwd = -brake;
			}
		}
	}

	float speedSq = (right * right) + (fwd * fwd);
	if ((adhesion * adhesion) < speedSq) {
		if (wheelState != WHEEL_STATE_FIXED) {
			if (bDriving && contactSpeedFwd < 0.2f)
				wheelState = WHEEL_STATE_SPINNING;
			else
				wheelState = WHEEL_STATE_SKIDDING;
		}

		float l = sqrtf(speedSq);
		float tractionLoss = *alreadySkidding ? 1.0f : in->tractionLoss;
		right *= adhesion * tractionLoss / l;
		fwd *= adhesion * tractionLoss / l;
	}

	out->fwd = fwd;
	out->right = right;
	out->wheelState = wheelState;
	out->braking = bBraking;
	out->driving = bDriving;
}
void VehicleLogic_ProcessBikeWheel(const VehicleBikeWheelInput *in, bool *alreadySkidding, VehicleBikeWheelOutput *out)
{
	float thrust = in->thrust;
	float brake = in->brake;
	float adhesion = in->adhesion;
	float destabTraction = in->destabTraction;
	int wheelState = in->wheelState;
	float contactSpeedFwd = in->contactSpeedFwd;
	float contactSpeedRight = in->contactSpeedRight;
	int wheelsOnGround = in->wheelsOnGround;

	// How much force we want to apply in these axes
	float fwd = 0.0f;
	float right = 0.0f;

	bool bBraking = brake != 0.0f;
	if (bBraking)
		thrust = 0.0f;
	bool bDriving = thrust != 0.0f;
	bool bReversing = thrust < 0.0f;

	if (wheelState != WHEEL_STATE_NORMAL)
		*alreadySkidding = true;
	wheelState = WHEEL_STATE_NORMAL;

	adhesion *= in->timeStep;
	if (*alreadySkidding)
		adhesion *= in->tractionLoss;

	// Moving sideways
	if (contactSpeedRight != 0.0f) {
		// Exert an opposing force
		right = -contactSpeedRight / wheelsOnGround;

		if (in->isBurst) {
			float fwdspeed = contactSpeedFwd < in->burstSpeedMax ? contactSpeedFwd : in->burstSpeedMax;
			right += fwdspeed * General_GetRandomNumberInRangeFloat(-in->burstTyreMod, in->burstTyreMod);
		}
	}

	if (bDriving) {
		fwd = thrust;

		// Limit the sideways force
		if (right > 0.0f) {
			if (right > adhesion)
				right = adhesion;
		} else {
			if (right < -adhesion)
				right = -adhesion;
		}
	} else if (contactSpeedFwd != 0.0f) {
		fwd = -contactSpeedFwd / wheelsOnGround;

		if (!bBraking) {
			if (in->gasPedal < 0.01f) {
				if (in->isBike)
					brake = 0.6f * in->wheelFriction / (in->handlingMass + 200.0f);
				else if (in->handlingMass < 500.0f)
					brake = in->wheelFriction / in->mass;
				else if (in->isRCBandit)
					brake = 0.2f * in->wheelFriction / in->mass;
				else
					brake = in->wheelFriction / in->mass;
				if (in->useTimeStepFix)
					brake *= in->timeStepFix;
			}
		}

		if (brake > adhesion) {
			if (fabsf(contactSpeedFwd) > 0.005f)
				wheelState = WHEEL_STATE_FIXED;
		} else {
			if (fwd > 0.0f) {
				if (fwd > brake)
					fwd = brake;
			} else {
				if (fwd < -brake)
					fwd = -brake;
			}
		}
	}

	float speedSq = (right * right) + (fwd * fwd);
	if ((adhesion * adhesion) < speedSq) {
		if (wheelState != WHEEL_STATE_FIXED) {
			if (bDriving && contactSpeedFwd < 0.2f)
				wheelState = WHEEL_STATE_SPINNING;
			else
				wheelState = WHEEL_STATE_SKIDDING;
		}

		float l = sqrtf(speedSq);
		float tractionLoss = *alreadySkidding ? 1.0f : in->tractionLoss;
		right *= adhesion * tractionLoss / l;
		fwd *= adhesion * tractionLoss / l;

		if (destabTraction < 1.0f)
			right *= destabTraction;
	} else if (destabTraction < 1.0f) {
		if (!*alreadySkidding)
			destabTraction *= in->tractionLoss;
		if ((adhesion * destabTraction) * (adhesion * destabTraction) < speedSq) {
			float l = sqrtf(speedSq);
			right *= adhesion * destabTraction / l;
		}
	}

	out->fwd = fwd;
	out->right = right;
	out->wheelState = wheelState;
	out->braking = bBraking;
	out->driving = bDriving;
	out->reversing = bReversing;
}
bool VehicleLogic_SunGlareStrength(float fwdness, float dist, float *strength)
{
	// Glare needs a car facing the camera close enough
	if (fwdness > 0.995f)
		*strength = 1.0f;
	else if (fwdness > 0.99f)
		*strength = (fwdness - 0.99f) / (0.995f - 0.99f);
	else
		return false;

	// There is no maximum distance
	if (dist > 30.0f) {
	} else if (dist > 13.0f)
		*strength *= (dist - 13.0f) / (30.0f - 13.0f);
	else
		return false;
	return true;
}

int VehicleLogic_SunGlareChannel(float intensity, float sunCore)
{
	return (int)(intensity * (sunCore + 2 * 255) / 3.0f);
}

int VehicleLogic_FindQuadVertex(int a1, int b1, int c1, int a2, int b2, int c2, int *vertex)
{
	int numTri2Verts = 0;

	if (a2 != a1 && a2 != b1 && a2 != c1) {
		numTri2Verts++;
		*vertex = a2;
	}
	if (b2 != a1 && b2 != b1 && b2 != c1) {
		numTri2Verts++;
		*vertex = b2;
	}
	if (c2 != a1 && c2 != b1 && c2 != c1) {
		numTri2Verts++;
		*vertex = c2;
	}
	return numTri2Verts;
}
void VehicleLogic_HeliDustColor(int surface, unsigned char *r, unsigned char *g, unsigned char *b)
{
	switch (surface) {
	default:
	case SURFACE_TARMAC:
		*r = 10;
		*g = 10;
		*b = 10;
		break;
	case SURFACE_GRASS:
		*r = 10;
		*g = 10;
		*b = 3;
		break;
	case SURFACE_GRAVEL:
		*r = 10;
		*g = 8;
		*b = 7;
		break;
	case SURFACE_MUD_DRY:
		*r = 10;
		*g = 6;
		*b = 3;
		break;
	case SURFACE_SAND:
	case SURFACE_SAND_BEACH:
		*r = 10;
		*g = 10;
		*b = 7;
		break;
	}
}

unsigned char VehicleLogic_HeliWaterChannel(float directional, float ambient)
{
	float v = (0.3 * directional + ambient) * 255.0f / 4.0f;
	v = v < 0.0f ? 0.0f : v > 255.0f ? 255.0f : v;
	return (unsigned char)v;
}
int VehicleLogic_DamageProofKind(int weaponType)
{
	switch (weaponType) {
	case WEAPONTYPE_UNARMED:
	case WEAPONTYPE_BRASSKNUCKLE:
	case WEAPONTYPE_SCREWDRIVER:
	case WEAPONTYPE_GOLFCLUB:
	case WEAPONTYPE_NIGHTSTICK:
	case WEAPONTYPE_KNIFE:
	case WEAPONTYPE_BASEBALLBAT:
	case WEAPONTYPE_HAMMER:
	case WEAPONTYPE_CLEAVER:
	case WEAPONTYPE_MACHETE:
	case WEAPONTYPE_KATANA:
	case WEAPONTYPE_CHAINSAW:
		return VEHICLE_DAMAGE_MELEE;
	case WEAPONTYPE_COLT45:
	case WEAPONTYPE_PYTHON:
	case WEAPONTYPE_SHOTGUN:
	case WEAPONTYPE_SPAS12_SHOTGUN:
	case WEAPONTYPE_STUBBY_SHOTGUN:
	case WEAPONTYPE_TEC9:
	case WEAPONTYPE_UZI:
	case WEAPONTYPE_SILENCED_INGRAM:
	case WEAPONTYPE_MP5:
	case WEAPONTYPE_M4:
	case WEAPONTYPE_RUGER:
	case WEAPONTYPE_SNIPERRIFLE:
	case WEAPONTYPE_LASERSCOPE:
	case WEAPONTYPE_M60:
	case WEAPONTYPE_MINIGUN:
	case WEAPONTYPE_HELICANNON:
	case WEAPONTYPE_UZI_DRIVEBY:
		return VEHICLE_DAMAGE_BULLET;
	case WEAPONTYPE_GRENADE:
	case WEAPONTYPE_MOLOTOV:
	case WEAPONTYPE_ROCKET:
	case WEAPONTYPE_EXPLOSION:
		return VEHICLE_DAMAGE_EXPLOSION;
	case WEAPONTYPE_FLAMETHROWER:
		return VEHICLE_DAMAGE_FIRE;
	case WEAPONTYPE_RAMMEDBYCAR:
		return VEHICLE_DAMAGE_COLLISION;
	default:
		return VEHICLE_DAMAGE_OTHER;
	}
}

int VehicleLogic_TyreBurstAccuracy(int weaponType, bool shooterIsPlayer)
{
	int accuracy = 0;

	switch (weaponType) {
	case WEAPONTYPE_COLT45:
		accuracy = 10;
		break;
	case WEAPONTYPE_PYTHON:
		if (!shooterIsPlayer)
			accuracy = 64;
		break;
	case WEAPONTYPE_SHOTGUN:
	case WEAPONTYPE_STUBBY_SHOTGUN:
	case WEAPONTYPE_M60:
	case WEAPONTYPE_HELICANNON:
		accuracy = 25;
		break;
	case WEAPONTYPE_TEC9:
	case WEAPONTYPE_UZI:
	case WEAPONTYPE_SILENCED_INGRAM:
	case WEAPONTYPE_MP5:
	case WEAPONTYPE_UZI_DRIVEBY:
		accuracy = 15;
		break;
	case WEAPONTYPE_M4:
	case WEAPONTYPE_RUGER:
		if (!shooterIsPlayer)
			accuracy = 15;
		break;
	}
	return accuracy;
}
bool VehicleLogic_ShouldLightsBeOn(int hours, int minutes, uint16_t randomSeed, float foggyness, float wetRoads)
{
	return hours > 20 ||
		(hours > 19 && minutes > (randomSeed & 0x3F)) ||
		hours < 7 ||
		(hours < 8 && minutes < (randomSeed & 0x3F)) ||
		randomSeed / 50000.0f < foggyness ||
		randomSeed / 50000.0f < wetRoads;
}

bool VehicleLogic_NextLightsState(bool shouldBeOn, bool lightsOn, bool isWrecked, bool isAbandoned, bool farFromCamera)
{
	if (shouldBeOn != lightsOn && !isWrecked) {
		if (isAbandoned) {
			// Turn off the lights on abandoned vehicles only when they are far away
			if (lightsOn && farFromCamera)
				return false;
		} else
			return shouldBeOn;
	}
	return lightsOn;
}
void VehicleLogic_AlarmFlash(bool alarmIsOn, uint32_t timeMs, bool *lightsOn, bool *lightsOff)
{
	*lightsOn = false;
	*lightsOff = false;
	if (alarmIsOn) {
		if (timeMs & 0x100)
			*lightsOn = true;
		else
			*lightsOff = true;
	}
}

void VehicleLogic_HeadlightGlow(float behindness, float *intensity, float *size)
{
	*intensity = -0.5f * behindness + 0.3f;
	*size = 1.0f - behindness;
}

float VehicleLogic_ExitCheckScale(float length)
{
	return (length + 0.6f) / length;
}

bool VehicleLogic_ExitBlockedAbove(float ceilingZ, float doorZ)
{
	return ceilingZ > doorZ && ceilingZ < doorZ + 0.6f;
}

bool VehicleLogic_ExitBlockedBelow(float ceilingZ, float groundZ)
{
	return ceilingZ != 0.0f && ceilingZ < groundZ;
}
float VehicleLogic_SpeedLimitScale(float speedSqr, float limit)
{
	if (speedSqr > limit * limit)
		return limit / sqrtf(speedSqr);
	return 1.0f;
}

float VehicleLogic_PlaneRollResistance(float turnSpeedY, float turnResY, float speedResY, float timeStep)
{
	float rY = powf(turnResY, timeStep);

	return turnSpeedY * (1.0f / (speedResY * (turnSpeedY * turnSpeedY) + 1.0f)) * rY - turnSpeedY;
}

float VehicleLogic_HeliYawResistance(float turnSpeedZ, float turnResZ, float speedResZ, float timeStep, float *resistance)
{
	float rZ = powf(turnResZ, timeStep);
	float multiplier = powf(1.0f / (speedResZ * (turnSpeedZ * turnSpeedZ) + 1.0f) * rZ, timeStep);

	*resistance = turnSpeedZ * multiplier - turnSpeedZ;
	return multiplier;
}
float VehicleLogic_PlaneAxisAccel(float stab, float tailSpeed, float gain, float steer, float fwdSpeed)
{
	return stab * tailSpeed * fabsf(tailSpeed) + gain * steer * fwdSpeed;
}

float VehicleLogic_PlaneRollAccel(bool isRCPlane, float lookDirection, float steerLR, float roll)
{
	if (isRCPlane)
		return (0.5f * lookDirection + steerLR) * roll;
	return steerLR * roll;
}

float VehicleLogic_PlaneRollStabilise(float rollStab, float rightZ, float rightDotStabilise, float forwardZ)
{
	float direction = (rightZ > 0.0f) ? -1.0f : 1.0f;

	return rollStab * direction * (1.0f - rightDotStabilise) * (1.0f - fabsf(forwardZ));
}

float VehicleLogic_PlaneLiftAccel(float formLift, float attackLift, float liftRatio, float fwdSpeed)
{
	return (formLift - attackLift * liftRatio) * (fwdSpeed * fwdSpeed);
}
float VehicleLogic_DodoImpulse(float axisSpeed, float moveSpeed)
{
	return -1.0f * axisSpeed / moveSpeed;
}

float VehicleLogic_DodoMagic(float moveSpeedSqr, float fwdSpeed)
{
	return moveSpeedSqr * (fwdSpeed * fwdSpeed);
}

float VehicleLogic_DodoTurnForce(float impulse, float impulseWeight, float steer, float steerWeight, float magic, float turnMass, float timeStep)
{
	return (impulse * impulseWeight + steer * steerWeight) * magic * turnMass * timeStep;
}

float VehicleLogic_DodoSideForce(float impulse, float magic, float mass, float timeStep)
{
	return impulse * 0.2f * magic * mass * timeStep;
}

float VehicleLogic_DodoLiftForce(float upImpulse, float magic, float mass, float timeStep, float gravity, float height)
{
	float impulse = (upImpulse * 3.5f + 0.5f) * 0.05f * magic * mass * timeStep;

	if (gravity * mass * timeStep < impulse && height > 100.0f)
		impulse = 0.9f * gravity * mass * timeStep;
	return impulse;
}
void VehicleLogic_FixedGunDirection(float forwardX, float forwardY, float *dirX, float *dirY)
{
	float len = sqrtf(forwardX * forwardX + forwardY * forwardY);

	if (len < 0.1f)
		len = 0.1f;
	*dirX = forwardX / len;
	*dirY = forwardY / len;
}
bool VehicleLogic_AllSpeedsBelow(float x, float y, float z, float limit)
{
	return fabsf(x) < limit && fabsf(y) < limit && fabsf(z) < limit;
}

bool VehicleLogic_AnySpeedAbove(float x, float y, float z, float limit)
{
	return fabsf(x) > limit || fabsf(y) > limit || fabsf(z) > limit;
}
void VehicleLogic_MirrorLight(const float lightR[3], const float right[3], float offsetX, float lightL[3])
{
	int k;

	for (k = 0; k < 3; k++)
		lightL[k] = lightR[k] - right[k] * 2.0f * offsetX;
}
bool VehicleLogic_WithinSquare(float ax, float ay, float bx, float by, float limit)
{
	return fabsf(ax - bx) < limit && fabsf(ay - by) < limit;
}
float VehicleLogic_ResistancePerStep(float resistance, float timeStep)
{
	return powf(resistance, timeStep);
}
float VehicleLogic_DecayToZero(float value, float amount)
{
	float v = value - amount;

	return v > 0.0f ? v : 0.0f;
}
float VehicleLogic_SideSlipAccel(float sideSlip, float sideSpeed)
{
	return sideSlip * sideSpeed * fabsf(sideSpeed);
}

float VehicleLogic_StabilityClamp(float stability, float speedAlongAxis)
{
	float v = stability * speedAlongAxis;

	return v < -200.0f ? -200.0f : v > 1.3f ? 1.3f : v;
}
bool VehicleLogic_RotorMissesCollision(float axisDir, float colDir, float semiThickness, float disregardMult)
{
	return 2.0f * semiThickness < fabsf(axisDir) &&
		disregardMult * fabsf(colDir) < fabsf(axisDir);
}

float VehicleLogic_GlareDistance(float dx, float dy)
{
	float ax = fabsf(dx);
	float ay = fabsf(dy);

	return 1.4f * (ax < ay ? ax : ay);
}
float VehicleLogic_AmbulanceFreebieHealth(float health, float maxHealth)
{
	float up = health + 20.0f;
	float m = up < maxHealth ? up : maxHealth;

	return health > m ? health : m;
}

float VehicleLogic_EnforcerFreebieArmour(float armour, float maxArmour)
{
	return armour > maxArmour ? armour : maxArmour;
}
float VehicleLogic_BoundingRadius(const CVector *boxMin, const CVector *boxMax)
{
	float a = sqrtf(boxMin->x * boxMin->x + boxMin->y * boxMin->y + boxMin->z * boxMin->z);
	float b = sqrtf(boxMax->x * boxMax->x + boxMax->y * boxMax->y + boxMax->z * boxMax->z);

	return a > b ? a : b;
}

bool VehicleLogic_BikeTippedOver(float rightZ, float forwardZ)
{
	return fabsf(rightZ) > 0.35f || fabsf(forwardZ) > 0.5f;
}
float VehicleLogic_ClampValue(float value, float low, float high)
{
	return value < low ? low : value > high ? high : value;
}
bool VehicleLogic_AnySpeedAtLeast(float x, float y, float z, float limit)
{
	return fabsf(x) >= limit || fabsf(y) >= limit || fabsf(z) >= limit;
}
bool VehicleLogic_HeightDifferenceBelow(float zA, float zB, float limit)
{
	return fabsf(zA - zB) < limit;
}
bool VehicleLogic_IsBeyond(float distance, float limit)
{
	return !(distance < limit);
}
//- rouz edit (Claude)
