//+ rouz edit (Claude)
#pragma once
#include "PlaceableData.h"
#include "PhysicalData.h"
#include "VectorMath.h"
#include "ColPoint.h"

#ifdef __cplusplus
extern "C" {
#endif
// Speed of a point relative to the entity centre
VectorMath3 Physical_GetSpeed(const CPhysicalData *self, VectorMath3 r);
// Advance position by the move speed or stop when frozen
void Physical_ApplyMoveSpeed(CPlaceableData *placeable, CPhysicalData *self, float timeStep);
// Rotate the axes by the turn speed (denormalises the matrix) or stop when frozen
void Physical_ApplyTurnSpeed(CPlaceableData *placeable, CPhysicalData *self, float timeStep);
// Add a move impulse to the speed or the friction speed
void Physical_ApplyMoveForce(CPhysicalData *self, float jx, float jy, float jz);
void Physical_ApplyFrictionMoveForce(CPhysicalData *self, float jx, float jy, float jz);
// Add a turn impulse applied at a point to the turn speed or the friction turn speed
void Physical_ApplyTurnForce(const CPlaceableData *placeable, CPhysicalData *self, float jx, float jy, float jz, float px, float py, float pz);
void Physical_ApplyFrictionTurnForce(const CPlaceableData *placeable, CPhysicalData *self, float jx, float jy, float jz, float px, float py, float pz);
// Fold the friction speeds into the speeds and clear them
void Physical_ApplyFriction(CPhysicalData *self);
// Spring force along a direction applied at a point when the spring is compressed
void Physical_ApplySpringCollision(const CPlaceableData *placeable, CPhysicalData *self, float timeStep, float springConst, VectorMath3 springDir, VectorMath3 point, float springRatio, float bias);
// Damping force along a spring direction applied at a point
void Physical_ApplySpringDampening(const CPlaceableData *placeable, CPhysicalData *self, float timeStep, float damping, VectorMath3 springDir, VectorMath3 point, VectorMath3 speed);

// Spring force that pushes along the contact normal, the normal is turned around first when it points the same way as the spring
void Physical_ApplySpringCollisionAlt(const CPlaceableData *placeable, CPhysicalData *self, float timeStep, float springConst, VectorMath3 springDir, VectorMath3 point, float springRatio, float bias, CColPoint *contact);
// Slow the movement down by the air resistance, a body with strong resistance loses speed and spin by a constant factor, a weak one depending on its speed
void Physical_ApplyAirResistance(CPhysicalData *self, float timeStep, bool isGhost);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
