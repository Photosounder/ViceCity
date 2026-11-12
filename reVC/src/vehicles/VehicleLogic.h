//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "PlaceableData.h"
#include "PhysicalData.h"
#include "VehicleData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Vehicle state helpers that only need the entity data
bool VehicleLogic_CarHasRoof(const CVehicleData *vehicle);
bool VehicleLogic_IsUpsideDown(const CPlaceableData *placeable);
bool VehicleLogic_IsOnItsSide(const CPlaceableData *placeable);
bool VehicleLogic_CanPedOpenLocks(const CVehicleData *vehicle, bool pedIsPlayer);
bool VehicleLogic_CanDoorsBeDamaged(const CVehicleData *vehicle);
bool VehicleLogic_CanPedEnterCar(const CPlaceableData *placeable, const CPhysicalData *physical, const CVehicleData *vehicle);
bool VehicleLogic_CanPedExitCar(const CPlaceableData *placeable, const CPhysicalData *physical, const CVehicleData *vehicle, bool jumpExit);
// Occupant bookkeeping, peds are only compared by address
bool VehicleLogic_IsDriver(const CVehicleData *vehicle, const CPed *ped);
bool VehicleLogic_IsPassenger(const CVehicleData *vehicle, const CPed *ped);
void VehicleLogic_UpdatePassengerList(CVehicleData *vehicle);
// Count the car alarm down by the time step in milliseconds
void VehicleLogic_ProcessCarAlarm(CVehicleData *vehicle, uint32_t stepMs);

// Put a passenger in the first free seat, or in seat n, false when there is no room
bool VehicleLogic_AddPassengerToFirstFreeSeat(CVehicleData *vehicle, CPed *passenger);
bool VehicleLogic_AddPassengerToSeat(CVehicleData *vehicle, CPed *passenger, uint8_t n);
// Take a passenger out of the seats, trains have all eight seats usable
void VehicleLogic_RemovePassenger(CVehicleData *vehicle, const CPed *passenger, bool isTrain);

// Move a passenger to another free seat so that a new one can get in, isRoom tells whether a ped may leave through a door and is only asked when the move depends on it
typedef bool (*VehicleRoomFn)(void *context, uint32_t door);
bool VehicleLogic_ShufflePassengersToMakeSpace(CVehicleData *vehicle, VehicleRoomFn isRoom, void *context);

// Honk for a driver who is held up, with a random delay between honks, returns true when the driver has to complain out loud as well, random is only called when the delay ran out
typedef uint16_t (*VehicleRandomFn)(void);
bool VehicleLogic_PlayCarHorn(CVehicleData *vehicle, bool alarmOn, VehicleRandomFn random);

// Whether the vehicle can be deleted, this looks at the creation mode and at the state of its occupants
bool VehicleLogic_CanBeDeleted(const CVehicleData *vehicle);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
