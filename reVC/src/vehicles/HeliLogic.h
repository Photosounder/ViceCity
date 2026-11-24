//+ rouz edit (Claude)
#pragma once
#include "HeliLayout.h"
#include "HeliStatus.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Offset of the nth swat rope position relative to the helicopter, zero for an unknown index
void Heli_FindSwatPositionRelativeToHeli(int n, float *x, float *y, float *z);
void Heli_ClearNodes(CHeliData *self);
// Bring a helicopter down, it spins one way or the other and explodes at the given game time
void Heli_ShootDown(CHeliData *self, bool spinPositive, uint32_t explosionTime);
// Add the damage of a bullet and tell whether the helicopter has taken more than it can bear, the Catalina one less than the others
bool Heli_AddBulletDamage(CHeliData *self, int32_t damage);
// Where a bullet that passes a helicopter on a line hits it, a bit before the helicopter along the line
void Heli_BulletPosition(const float *line0, const float *line1, const float *heliPos, float *out);
// Speed on the ground a helicopter flies towards the player with, from the distance to it, it stands still when hovering or shot down
float Heli_ChaseSpeed(float targetDist, int heliStatus);
// Moves the speed on the ground of a helicopter closer to the target speed by a step that depends on the time step
void Heli_UpdateGroundSpeed(float targetSpeedX, float targetSpeedY, float *moveSpeedX, float *moveSpeedY, float timeStep);
// Vertical speed of a helicopter after it gets closer to the speed that leads it to the target height
float Heli_UpdateVerticalSpeed(float targetZ, float posZ, float speedZ, float timeStep, int heliStatus);
// Turns a helicopter towards the direction of the target when it is far enough, angularSpeed and rotation get updated
void Heli_UpdateRotation(float *angularSpeed, float *rotation, float targetDist, float targetDirX, float targetDirY, float timeStep);
// How much the search light of a helicopter lights the ground at the distance it is from the helicopter
float Heli_SearchLightIntensity(float searchLightDist);
// Time in milliseconds a helicopter waits to shoot the player, from the wanted level for the random ones
int Heli_ShootTimeout(bool isRandomHeli, int wantedLevel, bool noPolice);
// Distance to the player below which a hovering helicopter keeps hovering, the ones in a hurry stay further
float Heli_HoverTargetHeight(bool isCatalina, int heliId);
// Distance to the player below which a chasing helicopter starts to hover
float Heli_ChaseTargetHeight(bool isCatalina, int heliId);
// Height a helicopter flies to, from its status, the height of the ground below it, its current target height and its target offset: hovering above the ground or the target and 8 units up (12 when flying away or chasing)
float Heli_TargetZ(int status, float groundZ, float targetZ, float targetOffset);
// Position of a police helicopter that spawns 250 units away from the player in a random direction, on the opposite side when the first one is out of the map
void Heli_SpawnPosition(float playerX, float playerY, float angle, float *x, float *y);
// Pushes a new target position into the history of the search light (6 entries): the older ones move one place back
void Heli_PushSearchLight(float *historyX, float *historyY, float newX, float newY);
// Rotation of the rotor of a helicopter after a frame, wrapped around a full turn
float Heli_NextRotorRotation(float rotation);
// Strength of the dust a helicopter blows up, from the time step: it drops with time but not below 2
float Heli_DustStrength(float timeStep);
// Direction on the ground a helicopter searches in (-cos, -sin) from its rotation
void Heli_SearchDirection(float rotation, float dir[2]);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
