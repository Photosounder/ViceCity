//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "WeaponInfo_backend.h"
#include "WorkBuffer.h"
#include "FileMgr.h"
#include "ModelIndexData.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
_Static_assert(sizeof(CWeaponInfo)==100,"Original complete weapon metadata record");
uint16_t WeaponInfo_ms_aReloadSampleTime[WEAPONTYPE_TOTALWEAPONS] =
{
	0,			// UNARMED
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			// GRENADE
	0,			// DETONATEGRENADE
	0,			// TEARGAS
	0,			// MOLOTOV
	0,			// ROCKET
	250,		// COLT45
	250,		// PYTHON
	650,		// SHOTGUN
	650,		// SPAS12 SHOTGUN
	650,		// STUBBY SHOTGUN
	400,		// TEC9
	400,		// UZIhec
	400,		// SILENCED_INGRAM
	400,		// MP5
	300,		// M16
	300,		// AK47
	423,		// SNIPERRIFLE
	423,		// LASERSCOPE
	400,		// ROCKETLAUNCHER
	0,			// FLAMETHROWER
	0,			// M60
	0,			// MINIGUN
	0,			// DETONATOR
	0,			// HELICANNON
	0			// CAMERA
};

// Yeah...
int32_t WeaponInfo_ms_aMaxAmmoForWeapon[WEAPONTYPE_TOTALWEAPONS] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1
};

CWeaponInfo aWeaponInfo[WEAPONTYPE_TOTALWEAPONS];
static char WeaponInfo_names[WEAPONTYPE_TOTALWEAPONS][32] =
{
	"Unarmed",
	"BrassKnuckle",
	"ScrewDriver",
	"GolfClub",
	"NightStick",
	"Knife",
	"BaseballBat",
	"Hammer",
	"Cleaver",
	"Machete",
	"Katana",
	"Chainsaw",
	"Grenade",
	"DetonateGrenade",
	"TearGas",
	"Molotov",
	"Rocket",
	"Colt45",
	"Python",
	"Shotgun",
	"Spas12Shotgun",
	"StubbyShotgun",
	"Tec9",
	"Uzi",
	"SilencedIngram",
	"Mp5",
	"m4",
	"Ruger",
	"SniperRifle",
	"LaserScope",
	"RocketLauncher",
	"FlameThrower",
	"M60",
	"Minigun",
	"Detonator",
	"HeliCannon",
	"Camera",
};

CWeaponInfo*
WeaponInfo_GetWeaponInfo(enum eWeaponType weaponType)
{
    // Preserve the complete original GetWeaponInfo operation and ordered live accesses
	return &aWeaponInfo[weaponType];
}

void
WeaponInfo_Initialise(void)
{
    // Preserve the complete original Initialise operation and ordered live accesses
	WeaponInfo_BackendInitialising();
	for (int i = 0; i < WEAPONTYPE_TOTALWEAPONS; i++) {
		aWeaponInfo[i].m_eWeaponFire = WEAPON_FIRE_INSTANT_HIT;
		aWeaponInfo[i].m_fRange = 0.0f;
		aWeaponInfo[i].m_nFiringRate = 0;
		aWeaponInfo[i].m_nReload = 0;
		aWeaponInfo[i].m_nAmountofAmmunition = 0;
		aWeaponInfo[i].m_nDamage = 0;
		aWeaponInfo[i].m_fSpeed = 0.0f;
		aWeaponInfo[i].m_fRadius = 0.0f;
		aWeaponInfo[i].m_fLifespan = 0.0f;
		aWeaponInfo[i].m_fSpread = 0.0f;
		aWeaponInfo[i].m_vecFireOffset = (WeaponInfoVector){0.0f,0.0f,0.0f};
		aWeaponInfo[i].m_AnimToPlay = ASSOCGRP_UNARMED;
		aWeaponInfo[i].m_fAnimLoopStart = 0.0f;
		aWeaponInfo[i].m_fAnimLoopEnd = 0.0f;
		aWeaponInfo[i].m_fAnimFrameFire = 0.0f;
		aWeaponInfo[i].m_fAnim2LoopStart = 0.0f;
		aWeaponInfo[i].m_fAnim2LoopEnd = 0.0f;
		aWeaponInfo[i].m_fAnim2FrameFire = 0.0f;
		aWeaponInfo[i].m_fAnimBreakout = 0.0f;
		aWeaponInfo[i].m_Flags = WEAPONFLAG_USE_GRAVITY | WEAPONFLAG_SLOWS_DOWN | WEAPONFLAG_RAND_SPEED | WEAPONFLAG_EXPANDS | WEAPONFLAG_EXPLODES;
		aWeaponInfo[i].m_nWeaponSlot = WEAPONSLOT_UNARMED;
	}
	WeaponInfo_BackendLoading();
	WeaponInfo_LoadWeaponData();
	WeaponInfo_BackendReady();
}

void
WeaponInfo_LoadWeaponData(void)
{
    // Preserve the complete original LoadWeaponData operation and ordered live accesses
    uint8_t (*buffer)[WORK_BUFFER_SIZE]=&work_buff;
	float spread, speed, lifeSpan, radius;
	float range, fireOffsetX, fireOffsetY, fireOffsetZ;
	float anim2LoopStart, anim2LoopEnd, delayBetweenAnim2AndFire, animBreakout;
	float delayBetweenAnimAndFire, animLoopStart, animLoopEnd;
	int flags, ammoAmount, damage, reload, weaponType;
	int firingRate, modelId, modelId2, weaponSlot;
	char line[256], weaponName[32], fireType[32];
	char animToPlay[32];

	size_t bp, buflen;
	int lp, linelen;
		
	FileMgr_SetDir("DATA");
	buflen = FileMgr_LoadFile("WEAPON.DAT", *buffer, sizeof(*buffer), "r");

	for (bp = 0; bp < buflen; ) {
		// read file line by line
		for (linelen = 0; (*buffer)[bp] != '\n' && bp < buflen; bp++) {
			line[linelen++] = (*buffer)[bp];
		}
		bp++;
		line[linelen] = '\0';

		// skip white space
		for (lp = 0; line[lp] <= ' ' && line[lp] != '\0'; lp++);

		if (line[lp] == '\0' || line[lp] == '#')
			continue;

		spread = 0.0f;
		flags = 0;
		speed = 0.0f;
		ammoAmount = 0;
		lifeSpan = 0.0f;
		radius = 0.0f;
		range = 0.0f;
		damage = 0;
		reload = 0;
		firingRate = 0;
		fireOffsetX = 0.0f;
		weaponName[0] = '\0';
		fireType[0] = '\0';
		fireOffsetY = 0.0f;
		fireOffsetZ = 0.0f;
		sscanf(
			&line[lp],
			"%s %s %f %d %d %d %d %f %f %f %f %f %f %f %s %f %f %f %f %f %f %f %d %d %x %d",
			weaponName,
			fireType,
			&range,
			&firingRate,
			&reload,
			&ammoAmount,
			&damage,
			&speed,
			&radius,
			&lifeSpan,
			&spread,
			&fireOffsetX,
			&fireOffsetY,
			&fireOffsetZ,
			animToPlay,
			&animLoopStart,
			&animLoopEnd,
			&delayBetweenAnimAndFire,
			&anim2LoopStart,
			&anim2LoopEnd,
			&delayBetweenAnim2AndFire,
			&animBreakout,
			&modelId,
			&modelId2,
			&flags,
			&weaponSlot);

		if (strncmp(weaponName, "ENDWEAPONDATA", 13) == 0)
			return;

		weaponType = WeaponInfo_FindWeaponType(weaponName);

		WeaponInfoVector vecFireOffset={fireOffsetX,fireOffsetY,fireOffsetZ};

		aWeaponInfo[weaponType].m_eWeaponFire = WeaponInfo_FindWeaponFireType(fireType);
		aWeaponInfo[weaponType].m_fRange = range;
		aWeaponInfo[weaponType].m_nFiringRate = firingRate;
		aWeaponInfo[weaponType].m_nReload = reload;
		aWeaponInfo[weaponType].m_nAmountofAmmunition = ammoAmount;
		aWeaponInfo[weaponType].m_nDamage = damage;
		aWeaponInfo[weaponType].m_fSpeed = speed;
		aWeaponInfo[weaponType].m_fRadius = radius;
		aWeaponInfo[weaponType].m_fLifespan = lifeSpan;
		aWeaponInfo[weaponType].m_fSpread = spread;
		aWeaponInfo[weaponType].m_vecFireOffset = vecFireOffset;
		aWeaponInfo[weaponType].m_fAnimLoopStart = animLoopStart / 30.0f;
		aWeaponInfo[weaponType].m_fAnimLoopEnd = animLoopEnd / 30.0f;
		aWeaponInfo[weaponType].m_fAnim2LoopStart = anim2LoopStart / 30.0f;
		aWeaponInfo[weaponType].m_fAnim2LoopEnd = anim2LoopEnd / 30.0f;
		aWeaponInfo[weaponType].m_fAnimFrameFire = delayBetweenAnimAndFire / 30.0f;
		aWeaponInfo[weaponType].m_fAnim2FrameFire = delayBetweenAnim2AndFire / 30.0f;
		aWeaponInfo[weaponType].m_fAnimBreakout = animBreakout / 30.0f;
		aWeaponInfo[weaponType].m_nModelId = modelId;
		aWeaponInfo[weaponType].m_nModel2Id = modelId2;
		aWeaponInfo[weaponType].m_Flags = flags;
		aWeaponInfo[weaponType].m_nWeaponSlot = weaponSlot;

		if (animLoopEnd < 98.0f && weaponType != WEAPONTYPE_FLAMETHROWER && !WeaponInfo_BackendIsShotgun(weaponType))
			aWeaponInfo[weaponType].m_nFiringRate = ((aWeaponInfo[weaponType].m_fAnimLoopEnd - aWeaponInfo[weaponType].m_fAnimLoopStart) * 900.0f);

		if (weaponType == WEAPONTYPE_DETONATOR || weaponType == WEAPONTYPE_HELICANNON)
			modelId = -1;
		else if (weaponType == WEAPONTYPE_DETONATOR_GRENADE)
			modelId = MI_BOMB;

		if (modelId != -1)
			WeaponInfo_BackendSetModelWeapon(modelId,weaponType);

		for (int i = 0; i < NUM_ANIM_ASSOC_GROUPS; i++) {
			if (!strcmp(animToPlay, WeaponInfo_BackendAnimGroupName((enum AssocGroupId)i))) {
				aWeaponInfo[weaponType].m_AnimToPlay = (enum AssocGroupId)i;
				break;
			}
		}
	}
}

enum eWeaponType
WeaponInfo_FindWeaponType(char *name)
{
    // Preserve the complete original FindWeaponType operation and ordered live accesses
	for (int i = 0; i < WEAPONTYPE_TOTALWEAPONS; i++) {
		if (strcmp(WeaponInfo_names[i], name) == 0) {
			return (enum eWeaponType)i;
		}
	}
	return WEAPONTYPE_UNARMED;
}

enum eWeaponFire
WeaponInfo_FindWeaponFireType(char *name)
{
    // Preserve the complete original FindWeaponFireType operation and ordered live accesses
	if (strcmp(name, "MELEE") == 0) return WEAPON_FIRE_MELEE;
	if (strcmp(name, "INSTANT_HIT") == 0) return WEAPON_FIRE_INSTANT_HIT;
	if (strcmp(name, "PROJECTILE") == 0) return WEAPON_FIRE_PROJECTILE;
	if (strcmp(name, "AREA_EFFECT") == 0) return WEAPON_FIRE_AREA_EFFECT;
	if (strcmp(name, "CAMERA") == 0) return WEAPON_FIRE_CAMERA;
	WeaponInfo_BackendUnknownFireType();
	return WEAPON_FIRE_INSTANT_HIT;
}

void
WeaponInfo_Shutdown(void)
{
    // Preserve the complete original Shutdown operation and ordered live accesses
	WeaponInfo_BackendShuttingDown();
	WeaponInfo_BackendShutDown();
}
bool WeaponInfo_IsWeaponSlotAmmoMergeable(uint32_t slot)
{
    // Preserve the complete original static inline slot predicate
 return slot == WEAPONSLOT_SHOTGUN || slot == WEAPONSLOT_SUBMACHINEGUN || slot == WEAPONSLOT_RIFLE; 
}
bool WeaponInfo_IsFlagSet(const CWeaponInfo *info,uint32_t flag)
{
    // Preserve the complete original const record predicate with an explicit record pointer
 return (info->m_Flags & flag) != 0; 
}
//- rouz edit (ChatGPT)
