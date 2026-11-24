//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Score of a candidate for the neighbouring target selection, the higher the better, angle is the limited angle between the candidate and the camera direction
float PlayerPedLogic_NeighbourCloseness(float angleBetweenUs, bool lookToLeft);
// Score of a candidate for the target selection from its flat distance and angle, priority targets get a bonus
float PlayerPedLogic_TargetCloseness(float dist, float angleBetweenUs, bool priority);
// A target further than the weapon range has to be dropped
bool PlayerPedLogic_DoesTargetHaveToBeBroken(float distance, float weaponRange);
// Sprinting uses up stamina unless the player has infinite sprint or adrenaline, and long sprints raise the maximum
void PlayerPedLogic_UseSprintEnergy(float *currentStamina, float *staminaProgress, float *maxStamina, bool infiniteSprint, bool adrenalineActive, float timeStep);
// Offset on the ground of a melee attack position around a ped, for one of 6 directions (0 ahead, then clockwise every 60 degrees) at a distance
void PlayerPedLogic_MeleeAttackOffset(int dir, float dist, float *dx, float *dy);
// Angle a player looks towards when the camera looks behind, from the camera angle, the rotation of the player, the current look direction (999999 for none) and whether the player ducks: returns whether to make the player look (the angle is turned around the player when the camera is almost directly behind)
bool PlayerPedLogic_LookBehindAngle(float camAngle, float rotationCur, float lookDirection, bool isDucking, float *lookAngle);
// Direction (0 to 5, each 60 degrees wide) a victim at a distance vector from the player is attacked from
int PlayerPedLogic_MeleeWantedDir(float distX, float distY);
// Free melee direction closest to the preferred one (-99 when none is free), from which of the 6 directions are occupied
int PlayerPedLogic_ClosestFreeMeleeDir(const bool occupied[6], int preferredDir);
// Horizontal push applied to a ped that jumps and is stuck in the air, along its facing direction
void PlayerPedLogic_JumpPush(float rotationCur, float *x, float *y);
// Move speed of the player when walking in first person or around, accelerating by 0.07 per time step up to the speed the pad asks for
float PlayerPedLogic_MoveSpeedRamp(float padMoveInGameUnit, float moveSpeed, float timeStep);
// Spin speed of the minigun while it fires, speeding up to 0.45
float PlayerPedLogic_GunSpinUp(float spinSpeed, float timeStep);
// Spin speed of the minigun while it winds down, slowing to 0
float PlayerPedLogic_GunSpinDown(float spinSpeed, float timeStep);
// Whether the player can reach a target with the weapon, from the angle between the facing of the player and the target, whether the angle matters, whether the weapon can aim with the arm, the height difference and the distance on the ground
bool PlayerPedLogic_CanReachTarget(float angleDiff, bool zRotImportant, bool canAimWithArm, float dz, float dist2D);
// Move speed of the player when walking to a seek position: twice the distance on the ground, limited to 2
float PlayerPedLogic_SeekMoveSpeed(float dist2D);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
