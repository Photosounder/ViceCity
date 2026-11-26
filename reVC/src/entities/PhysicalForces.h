//+ rouz edit (Claude)
#pragma once
#include "VectorFwd.h" // rouz edit (Claude)
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
// Factor the mass of the entity A of a collision counts with, a ped that bumps into it is heavier, from whether the other entity B is a ped and whether A is heavy
float Physical_CollisionMassFactorA(bool bIsPedPhysics, bool aIsHeavy);
// Factor the mass of the entity B of a collision counts with, a player ped that walks into an abandoned, wrecked or hit vehicle is slowed by its mass
float Physical_CollisionMassFactorB(bool aIsPedPhysics, bool aIsPlayerPed, bool bIsVehicle, bool bIsAbandonedOrWrecked, bool aHasHitWall, float bMass, bool bIsHeavy);
// Adhesion a vehicle gets on a surface it collides with, from the adhesion of the surface, the kind of vehicle, the height of the normal of the contact, whether the surface is loose or sand, the state of the vehicle and the impulse of the collision
float Physical_VehicleAdhesion(float adhesion, bool isRCBandit, bool isBoat, float normalZ, bool surfaceIsLooseOrSand, bool isWrecked, float upZ, float impulse);
// Impulse a collision reports to the audio, a vehicle that lies on its roof and does not move reports a tenth of it
float Physical_ReportedImpulse(float upZ, float moveSpeedX, float moveSpeedY, float impulse);
// Number of steps the time step is divided into for the collision of a ped that moves fast or that is the player
int Physical_PedCollisionSteps(float distSq, bool isPlayer, bool hasPhysSurface);
// Number of steps the time step is divided into for the collision of a fast vehicle
int Physical_VehicleCollisionSteps(float distSq, bool isPlayerStatus);
// Whether a vehicle moves less than its size along each axis, so that it can get away with a simpler collision check, from the speed along each of its axes, the time step and its size
bool Physical_VehicleMovesLessThanSize(float speedRight, float speedForward, float speedUp, float timeStep, float sizeX, float sizeY, float sizeZ);
// Number of steps for the collision of an object from the squared distance it moves, 1 when it is slow, depending on how it responds to collisions
int Physical_ObjectCollisionSteps(float distSq, bool isLampost, bool isUnknown5, bool isSmallBoxOrFencePart);
// Whether the speed of an entity that hit a wall is damped while it gets shifted, true for a ped that is not carried by a surface of infinite mass or in the second shift
bool Physical_ShiftIsDamped(bool hasHitWall, bool isPed, bool hasSurface, bool surfaceInfiniteMass, bool surfaceFlagA08, bool secondShift);
// Impulse of a collision on the normal, from the elasticity type (1 normal, 2 upside down, 3 and 4 special), whether the entity already has a contact, its speed relative to the other entity, the minimum speed, the speed along the normal, the mass, whether it is a boat on a wooden or tilted surface and its elasticity (small relative speeds use fixed bouncing factors per type)
float Physical_CollisionImpulse(int elasticityType, bool hasContacted, float relX, float relY, float relZ, float minSpeed, float normalSpeed, float mass, bool isBoatOnWood, float elasticity);
// Factor an entity is shifted out of its collisions by: when every collision is averaged (1.5 in the second shift, 1.2 in the first) or when only the deepest one counts (0.4 in the second shift, 0.2 in the first)
float Physical_ShiftFactor(bool secondShift, bool deepestOnly);
// Speed difference reported to the collision audio: the larger of the squared turn speed and the squared move speed
float Physical_AudioSpeedDiff(float turnSpeedDiff, float moveSpeedDiff);
// Factor the speeds of an entity are damped by while it is shifted, per time step
float Physical_ShiftDamping(float damp, float timeStep);
// New gravity direction of a vehicle that sticks to surfaces: the previous direction blended towards the surface direction by a factor from the time step, normalised, or straight up when it ends up too short
void Physical_BlendGravityUp(const CVector *gravityUp, const CVector *surfaceUp, float timeStep, CVector *out);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
