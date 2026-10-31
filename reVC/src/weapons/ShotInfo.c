//+ rouz edit (ChatGPT)
#include "ShotInfo.h"
#include "ShotInfo_backend.h"
#include "WeaponInfo.h"
#include "General.h"
#include "Timer.h"
#include "Fire.h"
#include <math.h>
#include <stddef.h>
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Max(a,b) ((a)>(b)?(a):(b))
#ifndef MASTER
#define ShotInfo_Assert(expression) ((void)((!!(expression)) || (ShotInfo_HostAssert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define ShotInfo_Assert(expression) (expression)
#endif

static ShotInfoVector ShotInfo_Difference(ShotInfoVector end,ShotInfoVector start)
{
    // Preserve the original component subtraction before storing the entire vector
    ShotInfoVector result={end.x-start.x,end.y-start.y,end.z-start.z};return result;
}
static void ShotInfo_Scale(ShotInfoVector *value,float scale)
{
    // Convert the scalar once before the original component multiplication order
    value->x*=scale;value->y*=scale;value->z*=scale;
}
static void ShotInfo_Advance(ShotInfoVector *position,float step,const ShotInfoVector *speed)
{
    // Form the original complete temporary vector before adding it into position
    ShotInfoVector delta={step*speed->x,step*speed->y,step*speed->z};position->x+=delta.x;position->y+=delta.y;position->z+=delta.z;
}
static float ShotInfo_DistanceSquared(ShotInfoVector position,const ShotInfoVector *center)
{
    // Retain float subtraction and squared magnitude without changing the caller threshold
    ShotInfoVector delta=ShotInfo_Difference(position,*center);return delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
}
//+ rouz edit (ChatGPT)


ShotInfo gaShotInfo[NUMSHOTINFOS];
float ShotInfo_ms_afRandTable[20];

#ifdef SQUEEZE_PERFORMANCE
uint32_t shotInfoInUse;
#endif

/*
	Used for flamethrower. I don't know why it's name is CShotInfo.
	Has no relation with any visual, just calculates the area fire affects
	(including spreading and slowing of fire) and make entities burn/flee.
*/

void
ShotInfo_Initialise(void)
{
    // Execute the complete original shot operation with C-owned state
	ShotInfo_HostDebug("Initialising CShotInfo...\n");
	for(int i=0; i<ARRAY_SIZE(gaShotInfo); i++) {
		gaShotInfo[i].m_inUse = false;
		gaShotInfo[i].m_weapon = WEAPONTYPE_COLT45;
		gaShotInfo[i].m_startPos = (ShotInfoVector){0.0f, 0.0f, 0.0f};
		gaShotInfo[i].m_areaAffected = (ShotInfoVector){0.0f, 0.0f, 0.0f};
		gaShotInfo[i].m_radius = 1.0f;
		gaShotInfo[i].m_sourceEntity = NULL;
		gaShotInfo[i].m_timeout = 0;
	}

	// Not random for sure
	float nextVal = -0.05f;
	for (int i = 0; i < ARRAY_SIZE(ShotInfo_ms_afRandTable); i++) {
		ShotInfo_ms_afRandTable[i] = nextVal;
		nextVal += 0.005f;
	}
	ShotInfo_HostDebug("CShotInfo ready\n");
#ifdef SQUEEZE_PERFORMANCE
	shotInfoInUse = 0;
#endif
}

bool
ShotInfo_AddShot(CEntity *sourceEntity, enum eWeaponType weapon, ShotInfoVector startPos, ShotInfoVector endPos)
{
    // Execute the complete original shot operation with C-owned state
	CWeaponInfo *weaponInfo = WeaponInfo_GetWeaponInfo(weapon);

	int slot;
	for (slot = 0; slot < ARRAY_SIZE(gaShotInfo) && gaShotInfo[slot].m_inUse; slot++);

	if (slot == ARRAY_SIZE(gaShotInfo))
		return false;

#ifdef SQUEEZE_PERFORMANCE
	shotInfoInUse++;
#endif

	gaShotInfo[slot].m_inUse = true;
	gaShotInfo[slot].m_weapon = weapon;
	gaShotInfo[slot].m_startPos = startPos;
	gaShotInfo[slot].m_areaAffected = ShotInfo_Difference(endPos,startPos);
	gaShotInfo[slot].m_radius = weaponInfo->m_fRadius;

	if (weaponInfo->m_fSpread != 0.0f) {
		gaShotInfo[slot].m_areaAffected.x += ShotInfo_ms_afRandTable[General_GetRandomNumber() % ARRAY_SIZE(ShotInfo_ms_afRandTable)] * weaponInfo->m_fSpread;
		gaShotInfo[slot].m_areaAffected.y += ShotInfo_ms_afRandTable[General_GetRandomNumber() % ARRAY_SIZE(ShotInfo_ms_afRandTable)] * weaponInfo->m_fSpread;
		gaShotInfo[slot].m_areaAffected.z += ShotInfo_ms_afRandTable[General_GetRandomNumber() % ARRAY_SIZE(ShotInfo_ms_afRandTable)];
	}
	ShotInfo_HostNormalise(&gaShotInfo[slot].m_areaAffected);
	if (WeaponInfo_IsFlagSet(weaponInfo,WEAPONFLAG_RAND_SPEED))
		ShotInfo_Scale(&gaShotInfo[slot].m_areaAffected,ShotInfo_ms_afRandTable[General_GetRandomNumber() % ARRAY_SIZE(ShotInfo_ms_afRandTable)] + weaponInfo->m_fSpeed);
	else
		ShotInfo_Scale(&gaShotInfo[slot].m_areaAffected,weaponInfo->m_fSpeed);

	gaShotInfo[slot].m_sourceEntity = sourceEntity;
	gaShotInfo[slot].m_timeout = (*Timer_GetTimeInMilliseconds()) + weaponInfo->m_fLifespan;

	return true;
}

void
ShotInfo_Shutdown(void)
{
    // Execute the complete original shot operation with C-owned state
	ShotInfo_HostDebug("Shutting down CShotInfo...\n");
	ShotInfo_HostDebug("CShotInfo shut down\n");
}

void
ShotInfo_Update(void)
{
    // Execute the complete original shot operation with C-owned state
#ifdef SQUEEZE_PERFORMANCE
	if (shotInfoInUse == 0)
		return;
#endif
	for (int slot = 0; slot < ARRAY_SIZE(gaShotInfo); slot++) {
		ShotInfo *shot = &gaShotInfo[slot];
		if (shot->m_sourceEntity && ShotInfo_HostIsPed(shot->m_sourceEntity) && !ShotInfo_HostValid(shot->m_sourceEntity))
			shot->m_sourceEntity = NULL;

		if (!shot->m_inUse)
			continue;

		CWeaponInfo *weaponInfo = WeaponInfo_GetWeaponInfo(shot->m_weapon);
		if ((*Timer_GetTimeInMilliseconds()) > shot->m_timeout) {
#ifdef SQUEEZE_PERFORMANCE
			shotInfoInUse--;
#endif
			shot->m_inUse = false;
		}

		if (WeaponInfo_IsFlagSet(weaponInfo,WEAPONFLAG_SLOWS_DOWN))
			ShotInfo_Scale(&shot->m_areaAffected,(float)pow(0.96, (*Timer_GetTimeStep()))); // FRAMERATE

		if (WeaponInfo_IsFlagSet(weaponInfo,WEAPONFLAG_EXPANDS))
			shot->m_radius += 0.075f * (*Timer_GetTimeStep());

		ShotInfo_Advance(&shot->m_startPos,(*Timer_GetTimeStep()),&shot->m_areaAffected);
		if (shot->m_sourceEntity) {
			ShotInfo_Assert(ShotInfo_HostIsPed(shot->m_sourceEntity));
			CEntity *ped = shot->m_sourceEntity;
			float radius = Max(1.0f, shot->m_radius);

			for (int i = 0; i < ShotInfo_HostNearCount(ped); ++i) {
				CEntity *nearPed = ShotInfo_HostNearPed(ped,i);
				if (ShotInfo_HostValid(nearPed)) {
					if (ShotInfo_HostInControl(nearPed) && ShotInfo_DistanceSquared(ShotInfo_HostPosition(nearPed),&shot->m_startPos) < radius && !ShotInfo_HostFireProof(nearPed)) {

						if (!ShotInfo_HostIsPlayer(nearPed)) {
							ShotInfo_HostFlee(nearPed,shot->m_sourceEntity,10000);
							ShotInfo_HostSprint(nearPed);
						}
						FireManager_StartFireOn(&gFireManager,nearPed, shot->m_sourceEntity, 0.8f, true);
					}
				}
			}
		}
		if (!(((*Timer_GetFrameCounter()) + slot) & 3))
			ShotInfo_HostCarsOnFire(shot->m_startPos.x, shot->m_startPos.y, shot->m_startPos.z, 4.0f, shot->m_sourceEntity);
	}
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
