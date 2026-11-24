//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Whether a vehicle may be the one the player enters: not wrecked or a moving train, upright or a bike, and at a reachable height
bool PlayerInfoLogic_IsCarCandidate(int status, float upZ, bool isBike, bool isCar, float pedZ, float carCentreZ);
// Score of a car for being the closest one to enter, taking the angle the ped has to turn into account, updates lastCloseness and returns true when this car is the best so far
bool PlayerInfoLogic_EvaluateCarPosition(float carX, float carY, float pedX, float pedY, float pedForwardX, float pedForwardY, float carBoundCentrePedDist, float *lastCloseness);
// Timer that cancels a wheel stunt after a frame: it counts down by a rate per millisecond of the time step but never below 0 (truncated to a whole number)
int32_t PlayerInfo_CancelStuntTimer(int32_t timer, float timeStepMs, float rate);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
