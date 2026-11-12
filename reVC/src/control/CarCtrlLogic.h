//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "EntityLayout.h"
#include "VehicleLogic.h"
#include "Pool.h"

#ifdef __cplusplus
extern "C" {
#endif
// Vehicle pool helpers of the traffic code, the tests that need other systems are supplied as callbacks
typedef bool (*CarCtrlVehicleTest)(void *vehicle);
// Closest vehicle to a point that may be removed to make room in the pool, or NULL
CVehicleStruct *CarCtrl_FindClosestRemovableCar(const CPool *vehiclePool, float cx, float cy, float cz, CarCtrlVehicleTest isInteresting, CarCtrlVehicleTest isTargetedByCrane);
// Number of vehicles of a model in the pool
int32_t CarCtrl_CountCarsOfType(const CPool *vehiclePool, int32_t modelIndex);
// Small deterministic lateral offset applied to a vehicle on rails, derived from its path node info and random seed
void CarCtrl_GetRandomOffsetForVehicle(const CVehicleStruct *vehicle, bool next, float out[3]);

// Speed factor from a heading change, 1 below minAngle falling to coef at maxAngle
float CarCtrl_FindSpeedMultiplier(float limitedAngleChange, float minAngle, float maxAngle, float coef);
// Speed factor of a road node type
float CarCtrl_FindSpeedMultiplierWithSpeedFromNodes(int type);
// Pick a car or boat class from a random number 0..1000 and the zone thresholds
int CarCtrl_ChooseRatingFromThresholds(int rnd, const int16_t *thresholds, int numClasses, int firstRating, bool returnIndex);

// The loaded vehicle models of one car class with their running frequency sums, each row ends with -1 after the last model
// Add a model with its frequency, the sum keeps adding the frequencies of the models before it
void CarCtrl_AddToLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi, int32_t freq);
// Remove a model and take its frequency out of the sums after it
void CarCtrl_RemoveFromLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi);
// Pick a loaded model in proportion to its frequency, -1 when none is loaded, the random number is asked for only then
int32_t CarCtrl_ChooseCarModel(const int32_t *loadedRow, const int32_t *freqRow, int32_t numLoaded, int32_t (*randomRange)(int32_t, int32_t));
// Whether a pointer is in a list of kept vehicles, and forget it there
bool CarCtrl_IsInPointerList(void *const *list, int count, const void *item);
void CarCtrl_RemoveFromPointerList(void **list, int count, const void *item);
// Move the traffic speed of a car on rails towards the speed it may drive, speeding up slowly and braking harder, returns true and the new speed when the speed has to be changed
bool CarCtrl_AdjustTrafficSpeed(float maxSpeed, float curSpeed, float timeStep, float *newSpeed);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
