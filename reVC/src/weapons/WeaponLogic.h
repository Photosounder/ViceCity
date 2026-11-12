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
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
