//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "AutomobileLayout.h"
#include "VehicleDoors.h"

#ifdef __cplusplus
extern "C" {
#endif
// Small automobile state helpers, doors are the eDoors values
bool Automobile_IsComponentPresent(const CAutomobileData *self, int comp);
bool Automobile_IsDoorMissing(CAutomobileData *self, int door);
bool Automobile_IsDoorFullyOpen(CAutomobileData *self, int door);
bool Automobile_IsDoorClosed(CAutomobileData *self, int door);
bool Automobile_IsDoorReady(CAutomobileData *self, int door, int gettingInFlags);
bool Automobile_IsDoorReadyByNode(CAutomobileData *self, unsigned node, int gettingInFlags);
bool Automobile_IsDoorMissingByNode(CAutomobileData *self, unsigned node);
bool Automobile_IsOpenTopCar(const CAutomobileStruct *self);
void Automobile_ResetSuspension(CAutomobileData *self);
bool Automobile_GetAllWheelsOffGround(const CAutomobileData *self);
void Automobile_ReduceHornCounter(CVehicleData *vehicle);
// Fly-to-coordinates missions of helicopters and planes and other small state changes
void Automobile_TellHeliToGoToCoors(CAutomobileStruct *self, float x, float y, float z, uint8_t speed);
void Automobile_TellPlaneToGoToCoors(CAutomobileStruct *self, float x, float y, float z, uint8_t speed);
void Automobile_PopBootUsingPhysics(CAutomobileData *self);
void Automobile_ClearCarNodes(CAutomobileData *self);
// Start the doors timer of a bus, type 0 opens and closes and anything else only closes
void Automobile_SetBusDoorTimer(CAutomobileData *self, uint32_t timer, uint8_t type, uint32_t now);
// Compress the suspension of a wheel that is in water, upper and lower are the ends of the suspension line in the world and waterZ the water level at the lower one, returns true when the wheel was changed
bool Automobile_HoverWheelInWater(CAutomobileData *self, int i, const float *upper, const float *lower, float waterZ);
// Opening a door of a bus as asked by the door timer, the component is the car node and the door the eDoors value
typedef struct AutomobileDoorAction {
	int component;
	int door;
	float ratio;
} AutomobileDoorAction;
// Run the doors of a bus by its timer: they close in the last half second before the end and open again afterwards, the front doors that are present and not in use are the ones that move
// doorMissing and doorClosed hold the state of the front left and front right door, the doors to move are written to actions in order and their number is returned
int Automobile_ProcessAutoBusDoors(CAutomobileStruct *self, uint32_t now, const bool doorMissing[2], const bool doorClosed[2], AutomobileDoorAction actions[2]);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
