//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "CraneData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Move the hook towards a target angle, distance and height, returns true when all three arrived
bool Crane_GoTowardsTarget(CCraneData *self, float angleToTarget, float distanceToTarget, float targetHeight, float speedMultiplier, float timeStep);
// Same for the height only
bool Crane_GoTowardsHeightTarget(CCraneData *self, float targetHeight, float speedMultiplier, float timeStep);
// Whether the crane takes cars of this model
bool Crane_DoesPickUpThisCarType(const CCraneData *self, uint32_t mi);
// The military crane collects one of seven models, collected is a bit mask
bool Cranes_DoesMilitaryCraneHaveThisOneAlready(uint32_t collected, uint32_t mi);
void Cranes_RegisterCarForMilitaryCrane(uint32_t *collected, uint32_t mi);
bool Cranes_HaveAllCarsBeenCollectedByMilitaryCrane(uint32_t collected);
// Offset on the ground of the hook of a crane from its position, from the angle of the hook and its offset
void Crane_HookOffset(float hookAngle, float hookOffset, float *dx, float *dy);
// Rotation of a car held by a crane towards its dropoff heading in a frame, from the angle left, the rotation speed and the time step: returns true when the rotation is done (nothing is applied), otherwise the angle to rotate by is stored
bool Crane_RotationStep(float angleDelta, float rotationSpeed, float timeStep, float *rotation);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
