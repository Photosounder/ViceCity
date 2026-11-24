//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "ObjectLayout.h"
#include "Pool.h"

#ifdef __cplusplus
extern "C" {
#endif
// Field initialisation done by the default object constructor
void Object_InitFields(CObjectStruct *self);
// Whether the object may be deleted by the cleanup code
bool Object_CanBeDeleted(const CObjectState *self);
// Objects of the pool created in a given way, in pool order
int Object_SelectByCreation(const CPool *objectPool, int creationMode, void **out, int maxOut);
// Objects the cleanup code may delete, walking the pool backwards
int Object_SelectDeletable(const CPool *objectPool, void **out, int maxOut);
// Temporary objects within a radius of a point, in pool order
int Object_SelectTempInArea(const CPool *objectPool, float x, float y, float z, float radius, void **out, int maxOut);

// Colours (red, green, blue, alpha) of the debris of a smashed newsstand, black bag or beach lounge from the collision damage effect, color2 is the same as color1 for the ones with a single colour
void ObjectLogic_SmashDebrisColors(int damageEffect, unsigned char color1[4], unsigned char color2[4]);
// Colour channel of the spray of a boat object from the directional and the ambient light
unsigned char ObjectLogic_RenderColorChannel(float directional, float ambient);
// Factor the speed of a radio controlled bomb is multiplied with in a frame, from its squared speed and the time step
float ObjectLogic_RCBombSpeedFactor(float speedSqr, float timeStep);
// Whether a beach ball is kicked by a player on foot who stands over it from the offset between them, the speed of the ball on the ground and the speed of the player
bool ObjectLogic_BeachballKickedByPlayer(float offsetZ, float distance2D, float ballSpeed2D, float playerSpeed);
// Whether a beach ball falling on the head of the player on foot is bounced from the offset between them, the vertical speed of the ball and the distance on the ground
bool ObjectLogic_BeachballHitsHead(float offsetZ, float ballSpeedZ, float distance2D);
// Angles the segments of a stinger that is deployed or taken back are at, from the angle the trap faces, the time in milliseconds since it started, whether it is being deployed (and not taken back) and the number of precomputed offsets: angles[0] for the even segments and angles[1] for the odd ones, and the index of the offset the even segments move by
void Stinger_SpikeAngles(float maxZ, uint32_t elapsed, bool deploying, int numPositions, float angles[2], int *positionIndex);
// Factor the speeds of an object are damped by in a frame (in water 0.97, a beach ball 0.95), a constant per time unit raised to the power of the time step
float ObjectLogic_SpeedDamping(float damping, float timeStep);
// Upward speed of a beach ball after it is kicked by a ped: the speed plus 0.3 but at least 0.2
float ObjectLogic_BeachballKickSpeed(float speedZ);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
