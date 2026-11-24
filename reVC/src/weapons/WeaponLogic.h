//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "WeaponData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Set up a weapon slot with an ammo count and load the first clip, clipSize is the ammunition of a full clip
void Weapon_Init(CWeaponData *self, int type, int32_t ammo, int32_t clipSize);
// Load as much of the remaining ammo as fits in a clip
void Weapon_Reload(CWeaponData *self, int32_t clipSize);
// Empty the slot after its models were released
void Weapon_ClearSlot(CWeaponData *self);
// Weapons held with both hands
bool Weapon_IsType2Handed(int type);
// Melee weapons need no ammo, the others need some left
bool Weapon_HasWeaponAmmoToBeUsed(const CWeaponData *self);
// Whether a reload or shotgun pump sample has to play now, the time passed is the point in time of the sample and prev and now are the game times of the last and the current frame
bool Weapon_SampleDue(uint32_t timer, uint32_t sampleTime, uint32_t prev, uint32_t now);
// How far through the reload animation the reload sound starts, fractions holds the values for the python, colt and tec, uzi, rifle and m60 groups
float Weapon_ReloadAnimSoundStart(int animGroup, const float *fractions);
// A weapon that fired becomes ready again after its timer, or runs dry, returns true when it ran out of ammo and the pickups of that weapon have to go
bool Weapon_UpdateFiring(CWeaponData *self, uint32_t now, bool isMelee);
// A reloading weapon is ready again once its timer ran out, with a full clip
void Weapon_UpdateReloading(CWeaponData *self, uint32_t now, int32_t clipSize);
// Add ammo to a slot, capped by the maximum of the weapon (a negative maximum means the general cap of 99999), a slot that was out of ammo gets ready again
void Weapon_GrantAmmo(CWeaponData *self, uint32_t ammo, int32_t maxAmmo);
// Set the ammo of a slot with the same cap, the clip can not hold more than the ammo
void Weapon_SetAmmo(CWeaponData *self, uint32_t ammo, int32_t maxAmmo);
// Whether the auto aiming skips an entity because of its status (eEntityStatus): trains, helicopters and planes, and with includeWrecked also wrecked vehicles
bool WeaponLogic_AutoAimIgnoresStatus(int status, bool includeWrecked);
// Whether a victim is in the cone the auto aiming looks into: its height difference times the factor is below its distance on the ground
bool WeaponLogic_AutoAimWithinCone(float factor, float distZ, float dist2D);
// Distance to a victim from the distance on the ground and the height difference
float WeaponLogic_AutoAimDist(float dist2D, float distZ);
// Height a victim is aimed at: a little above its position, lower for a ducking ped
float WeaponLogic_AutoAimVictimZ(float victimPosZ, bool victimDuckingPed);
// Height of the target of a shot that is turned towards a victim along the line from the source
float WeaponLogic_AutoAimTargetZ(float distToTarget, float distToSource, float victimZ, float sourceZ);
// Score of a ped for the drive by auto aiming, the lower the better: helicopters weigh the distance from the line by the distance from the helicopter, other vehicles add a part of the distance from the driver
float WeaponLogic_DriveByPedDist(bool isHeli, float lineDist, float distFromVehicle, float distFromDriver);
// Largest score that still gets auto aimed, helicopters use the tangent of their aiming angle in degrees
float WeaponLogic_DriveByMaxAimDist(bool isHunter, bool isSeaSparrowOrSparrow, float hunterAngle, float seaSparrowAngle, float defaultMax);

// Unit vector on the ground that points in the direction of an angle (x is minus the sine and y the cosine), ahead[0] is x and ahead[1] is y
void Weapon_AheadFromAngle(float angle, float ahead[2]);
// Whether the bullet of a ped that points a gun at someone hit what it aimed at, from the dot product of the directions to the target and to the hit, and the distances to both
bool Weapon_BulletMatchesAim(float dotProd, float distWithBullet, float distWithAim);
// Inaccuracy of the shots of the player, from the aim scale, the attack button counter, the base inaccuracy, the distance the aim scale applies up to, the distance to the target and whether the player ducks
float Weapon_PlayerInaccuracy(float aimScale, float attackButtonCounter, float inaccuracy, float aimScaleDist, float distToTarget, bool isDucking);
// Inaccuracy of the shots of the AI at the player, widened by the speed of the player (limited to 0.33)
float Weapon_SpeedInaccuracy(float inaccuracy, float playerSpeed);
// Frequency of the shake of the pad when the player fires, from the firing rate of the weapon
uint8_t Weapon_ShakeFrequency(int firingRate);
// Box around a shot from a source to a target, 2 units larger on every side, in which peds jump away
void Weapon_ShotBox(const float source[3], const float target[3], float boxMin[3], float boxMax[3]);
// Whether a position is strictly inside a box
bool Weapon_PositionInBox(const float pos[3], const float boxMin[3], const float boxMax[3]);
// Offset on the ground along the facing direction of an angle (-sin, cos) scaled by a length
void Weapon_AngleOffset(float angle, float scale, float offset[2]);
// Power a projectile is thrown with at a seek target, from the distance to it (0.2 to 1)
float Weapon_ThrowPower(float distToTarget);
// End of the bullet trace for the effects, at most 30 units from the fire source towards the target
void Weapon_TraceTarget(const float fireSource[3], const float target[3], float range, float traceTarget[3]);
// Distance a bullet effect is pulled back from the hit point along the shot: a fifth of the distance of the shot but at least the given minimum
float Weapon_EffectBackOff(float distance, float minimum);
// Distance to a threatened ped target with a minimum of 0.01 so the range scaling never divides by zero
float Weapon_ThreatTargetDistance(float magnitude);
// Distance the smoke of a bullet hitting a wall is pulled back: a tenth of the distance of the shot but at least 0.2
float Weapon_WallSmokeBackOff(float distance);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
