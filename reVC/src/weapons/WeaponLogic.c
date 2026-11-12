//+ rouz edit (Claude)
#include "WeaponLogic.h"
#include "WeaponType.h"
#include "AnimGroups.h"

static bool IsShotgun(int weapon)
{
	return weapon == WEAPONTYPE_SHOTGUN || weapon == WEAPONTYPE_SPAS12_SHOTGUN || weapon == WEAPONTYPE_STUBBY_SHOTGUN;
}

void Weapon_Reload(CWeaponData *self, int32_t clipSize)
{
	// Nothing to load without ammo
	if (self->m_nAmmoTotal == 0)
		return;

	// A full clip or whatever is left
	if (self->m_nAmmoTotal >= clipSize)
		self->m_nAmmoInClip = clipSize;
	else
		self->m_nAmmoInClip = self->m_nAmmoTotal;
}

void Weapon_Init(CWeaponData *self, int type, int32_t ammo, int32_t clipSize)
{
	// Ready with at most 99999 rounds and the first clip loaded
	self->m_eWeaponType = type;
	self->m_eWeaponState = WEAPONSTATE_READY;
	self->m_nAmmoTotal = ammo < 99999 ? ammo : 99999;
	self->m_nAmmoInClip = 0;
	Weapon_Reload(self, clipSize);
	self->m_nTimer = 0;
}

void Weapon_ClearSlot(CWeaponData *self)
{
	// Unarmed and out of ammo
	self->m_eWeaponType = WEAPONTYPE_UNARMED;
	self->m_eWeaponState = WEAPONSTATE_READY;
	self->m_nAmmoInClip = 0;
	self->m_nAmmoTotal = 0;
	self->m_nTimer = 0;
}

bool Weapon_IsType2Handed(int type)
{
	return type == WEAPONTYPE_FLAMETHROWER || type == WEAPONTYPE_HELICANNON || type == WEAPONTYPE_M60 ||
		type == WEAPONTYPE_M4 || IsShotgun(type) ||
		type == WEAPONTYPE_RUGER || type == WEAPONTYPE_SNIPERRIFLE || type == WEAPONTYPE_LASERSCOPE;
}

bool Weapon_HasWeaponAmmoToBeUsed(const CWeaponData *self)
{
	// The first weapon types are melee
	if (self->m_eWeaponType <= WEAPONTYPE_CHAINSAW)
		return true;
	return self->m_nAmmoTotal != 0;
}
bool Weapon_SampleDue(uint32_t timer, uint32_t sampleTime, uint32_t prev, uint32_t now)
{
	// The frame that crosses the point in time of the sample
	uint32_t timePassed = timer - sampleTime;
	return prev < timePassed && now >= timePassed;
}

float Weapon_ReloadAnimSoundStart(int animGroup, const float *fractions)
{
	// Three quarters through unless the animation group has its own value
	float soundStart = 0.75f;
	switch (animGroup) {
	case ASSOCGRP_PYTHON:
		soundStart = fractions[0];
		break;
	case ASSOCGRP_COLT:
	case ASSOCGRP_TEC:
		soundStart = fractions[1];
		break;
	case ASSOCGRP_UZI:
		soundStart = fractions[2];
		break;
	case ASSOCGRP_RIFLE:
		soundStart = fractions[3];
		break;
	case ASSOCGRP_M60:
		soundStart = fractions[4];
		break;
	default:
		break;
	}
	return soundStart;
}

bool Weapon_UpdateFiring(CWeaponData *self, uint32_t now, bool isMelee)
{
	// Nothing changes until the timer ran out
	if (now > self->m_nTimer) {
		if (!isMelee && self->m_nAmmoTotal == 0) {
			self->m_eWeaponState = WEAPONSTATE_OUT_OF_AMMO;
			return true;
		}
		self->m_eWeaponState = WEAPONSTATE_READY;
	}
	return false;
}

void Weapon_UpdateReloading(CWeaponData *self, uint32_t now, int32_t clipSize)
{
	// Load the clip once the reload time is over
	if (now > self->m_nTimer) {
		Weapon_Reload(self, clipSize);
		self->m_eWeaponState = WEAPONSTATE_READY;
	}
}
// Cap the total ammo of a slot, by the maximum of its weapon or by 99999
static void CapAmmo(CWeaponData *self, int32_t maxAmmo)
{
	if (maxAmmo >= 0)
		self->m_nAmmoTotal = maxAmmo < self->m_nAmmoTotal ? maxAmmo : self->m_nAmmoTotal;
	else
		self->m_nAmmoTotal = 99999 < self->m_nAmmoTotal ? 99999 : self->m_nAmmoTotal;
}

void Weapon_GrantAmmo(CWeaponData *self, uint32_t ammo, int32_t maxAmmo)
{
	self->m_nAmmoTotal += ammo;
	CapAmmo(self, maxAmmo);

	if (self->m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO && self->m_nAmmoTotal > 0)
		self->m_eWeaponState = WEAPONSTATE_READY;
}

void Weapon_SetAmmo(CWeaponData *self, uint32_t ammo, int32_t maxAmmo)
{
	int32_t newClip;

	self->m_nAmmoTotal = ammo;
	CapAmmo(self, maxAmmo);

	// The clip never holds more than there is
	newClip = self->m_nAmmoTotal;
	if (newClip >= self->m_nAmmoInClip)
		newClip = self->m_nAmmoInClip;
	self->m_nAmmoInClip = newClip;

	if (self->m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO && self->m_nAmmoTotal > 0)
		self->m_eWeaponState = WEAPONSTATE_READY;
}
//- rouz edit (Claude)
