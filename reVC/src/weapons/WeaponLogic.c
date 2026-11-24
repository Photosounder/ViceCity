//+ rouz edit (Claude)
#include "WeaponLogic.h"
#include <math.h>
#include "EntityStatus.h"
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
bool WeaponLogic_AutoAimIgnoresStatus(int status, bool includeWrecked)
{
	return status == STATUS_TRAIN_MOVING
		|| status == STATUS_TRAIN_NOT_MOVING
		|| status == STATUS_HELI
		|| status == STATUS_PLANE
		|| (includeWrecked && status == STATUS_WRECKED);
}

bool WeaponLogic_AutoAimWithinCone(float factor, float distZ, float dist2D)
{
	return factor*distZ < dist2D;
}

float WeaponLogic_AutoAimDist(float dist2D, float distZ)
{
	return sqrtf((dist2D*dist2D) + (distZ*distZ));
}

float WeaponLogic_AutoAimVictimZ(float victimPosZ, bool victimDuckingPed)
{
	float victimZ = victimPosZ + 0.3f;
	if (victimDuckingPed)
		victimZ -= 0.8f;
	return victimZ;
}

float WeaponLogic_AutoAimTargetZ(float distToTarget, float distToSource, float victimZ, float sourceZ)
{
	return (distToTarget / distToSource) * (victimZ - sourceZ) + sourceZ;
}

float WeaponLogic_DriveByPedDist(bool isHeli, float lineDist, float distFromVehicle, float distFromDriver)
{
	if (isHeli) {
		float larger = 5.f > distFromVehicle ? 5.f : distFromVehicle;
		return lineDist / larger;
	}
	return 0.15f * distFromDriver + lineDist;
}

float WeaponLogic_DriveByMaxAimDist(bool isHunter, bool isSeaSparrowOrSparrow, float hunterAngle, float seaSparrowAngle, float defaultMax)
{
	const float pi = (float)3.14159265358979323846;

	if (isHunter)
		return tanf(hunterAngle * pi / 180.0f);
	if (isSeaSparrowOrSparrow)
		return tanf(seaSparrowAngle * pi / 180.0f);
	return defaultMax;
}

void Weapon_AheadFromAngle(float angle, float ahead[2])
{
	float x = -sinf(angle);
	float y = cosf(angle);
	float sq = x * x + y * y;

	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		x *= invsqrt;
		y *= invsqrt;
	} else
		x = 1.0f;
	ahead[0] = x;
	ahead[1] = y;
}
bool Weapon_BulletMatchesAim(float dotProd, float distWithBullet, float distWithAim)
{
	const float pi = (float)3.14159265358979323846;
	float aimAndBulletAngle;

	if (dotProd <= 0.35f)
		aimAndBulletAngle = pi;
	else
		aimAndBulletAngle = acosf(dotProd);

	return aimAndBulletAngle <= 45.0f * pi / 180.0f &&
		(aimAndBulletAngle <= 15.0f * pi / 180.0f || distWithBullet / distWithAim >= 0.75f) &&
		distWithBullet / distWithAim >= 0.99f;
}
float Weapon_PlayerInaccuracy(float aimScale, float attackButtonCounter, float inaccuracy, float aimScaleDist, float distToTarget, bool isDucking)
{
	float ratio = aimScaleDist / distToTarget;
	float newInaccuracy = aimScale * attackButtonCounter * (inaccuracy * (1.f < ratio ? 1.f : ratio));

	if (isDucking)
		newInaccuracy *= 0.4f;
	return newInaccuracy;
}

float Weapon_SpeedInaccuracy(float inaccuracy, float playerSpeed)
{
	float speed = 0.33f < playerSpeed ? 0.33f : playerSpeed;

	return inaccuracy * (0.3f * speed * 100.f / 33.f + 0.8f);
}

uint8_t Weapon_ShakeFrequency(int firingRate)
{
	/* Yes, double */
	double notFiringRate = (20.0 - firingRate) / 80.0;
	double clamped = 0.0 > notFiringRate ? 0.0 : notFiringRate;
	double raisedNotFiringRate = 1.0 > clamped ? 1.0 : clamped;

	return (uint8_t)(80.0 * raisedNotFiringRate + 130.0);
}
void Weapon_ShotBox(const float source[3], const float target[3], float boxMin[3], float boxMax[3])
{
	int k;

	for (k = 0; k < 3; k++) {
		boxMin[k] = (source[k] < target[k] ? source[k] : target[k]) - 2.0f;
		boxMax[k] = (source[k] > target[k] ? source[k] : target[k]) + 2.0f;
	}
}

bool Weapon_PositionInBox(const float pos[3], const float boxMin[3], const float boxMax[3])
{
	return pos[0] > boxMin[0] && pos[0] < boxMax[0]
		&& pos[1] > boxMin[1] && pos[1] < boxMax[1]
		&& pos[2] > boxMin[2] && pos[2] < boxMax[2];
}
void Weapon_AngleOffset(float angle, float scale, float offset[2])
{
	offset[0] = -sinf(angle) * scale;
	offset[1] = cosf(angle) * scale;
}

float Weapon_ThrowPower(float distToTarget)
{
	float v = (distToTarget - 10.0f) * 0.02f;

	return v < 0.2f ? 0.2f : v > 1.0f ? 1.0f : v;
}

void Weapon_TraceTarget(const float fireSource[3], const float target[3], float range, float traceTarget[3])
{
	float m = range < 30.0f ? range : 30.0f;
	int k;

	for (k = 0; k < 3; k++)
		traceTarget[k] = fireSource[k] + (target[k] - fireSource[k]) * m / range;
}
float Weapon_EffectBackOff(float distance, float minimum)
{
	float v = 0.2f * distance;

	return v > minimum ? v : minimum;
}
float Weapon_ThreatTargetDistance(float magnitude)
{
	return magnitude > 0.01f ? magnitude : 0.01f;
}
float Weapon_WallSmokeBackOff(float distance)
{
	float v = distance / 10.0f;

	return v > 0.2f ? v : 0.2f;
}
//- rouz edit (Claude)
