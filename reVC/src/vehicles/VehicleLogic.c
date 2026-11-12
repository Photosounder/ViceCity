//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "VehicleLogic.h"
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
//- rouz edit (Claude)
