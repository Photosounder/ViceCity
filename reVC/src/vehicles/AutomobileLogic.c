//+ rouz edit (Claude)
#include <stddef.h>
#include "AutomobileLogic.h"
#include "ModelIds.h"
#include "DoorTypes.h"
#include "WheelState.h"
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
//- rouz edit (Claude)
