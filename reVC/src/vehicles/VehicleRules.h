//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Vehicle models that belong to the police and military
bool Vehicle_IsLawEnforcementModel(int modelIndex);
// Vehicle models with a siren
bool Vehicle_UsesSirenModel(int modelIndex);
// Rotation of a wheel this frame, forwardSpeed is the dot product of the forward direction and the speed
float Vehicle_ProcessWheelRotation(int wheelState, float forwardSpeed, float radius, float timeStep);
// Index of the tyre closest to an offset from the vehicle, relative to the left front wheel: 0 left front, 1 right front, 2 left rear, 3 right rear
int Vehicle_FindTyreNearestPoint(bool isBike, float fwd, float right);
// Next alpha of a vehicle that fades out quickly or fades in
int Vehicle_UpdateClumpAlpha(int clumpAlpha, bool fadeOut);
// Kind of vehicle from the handling flags
int Vehicle_GetVehicleAppearance(uint32_t handlingFlags);
// Turn the law enforcer flag on or off, the count of such cars follows it
void Vehicle_ChangeLawEnforcerState(bool *isLawEnforcer, int32_t *numLawEnforcerCars, bool enable);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
