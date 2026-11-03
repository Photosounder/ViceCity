//+ rouz edit (ChatGPT)
#include "BulletInfo.h"
#include "BulletInfo_backend.h"
#include "WeaponInfo.h"
#include "Timer.h"
#include <stddef.h>
#include "ColLine.h"
#include "ColBox.h"
#include "SurfaceTypes.h"
#include "AnimManager.h"
#include "EventList.h"
#include "RpAnimBlend.h"
#include "PedStates.h"
#include "ParticleType.h"
#include "Fire.h"
#include "EntityTypes.h"
#include "DMAudio.h"
#include "AudioScriptObject.h"
#include "Pools.h"
#include "Stats.h"
#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
extern CollisionPointRef BulletInfo_HostSniperStart(void);
extern CollisionPointRef BulletInfo_HostSniperEnd(void);
#ifdef FIX_BUGS
extern bool BulletInfo_HostTestLineBox(const CColLine *,const CBox *);
#endif

extern void BulletInfo_HostDebug(const char *message);
CBulletInfo gaBulletInfo[BULLET_INFO_NUM_BULLETS];
bool bPlayerSniperBullet;
#ifdef SQUEEZE_PERFORMANCE
uint32_t bulletInfoInUse;
#endif
#define BULLET_LIFETIME (1000)

void BulletInfo_Initialise(void)
{
	// Retain the original initialization order and diagnostic messages
	BulletInfo_HostDebug("Initialising CBulletInfo...\n");
	// Use the shared C-style bullet capacity
	for (int i = 0; i < BULLET_INFO_NUM_BULLETS; i++) { // rouz edit (ChatGPT)
		gaBulletInfo[i].m_bInUse = false;
		gaBulletInfo[i].m_eWeaponType = WEAPONTYPE_COLT45;
		gaBulletInfo[i].m_fTimer = 0.0f;
		gaBulletInfo[i].m_pSource = NULL;
	}
	BulletInfo_HostDebug("CBulletInfo ready\n");
#ifdef SQUEEZE_PERFORMANCE
	bulletInfoInUse = 0;
#endif
}

void BulletInfo_Shutdown(void)
{
	// Retain the original shutdown diagnostic messages
	BulletInfo_HostDebug("Shutting down CBulletInfo...\n");
	BulletInfo_HostDebug("CBulletInfo shut down\n");
}

bool BulletInfo_AddBullet(CEntity* pSource, enum eWeaponType type, BulletInfoVector vecPosition, BulletInfoVector vecSpeed)
{
	// Preserve first-free-slot allocation and original field updates
	int i;
	// Use the shared C-style bullet capacity
	for (i = 0; i < BULLET_INFO_NUM_BULLETS; i++) { // rouz edit (ChatGPT)
		if (!gaBulletInfo[i].m_bInUse)
			break;
	}
	// Use the shared C-style bullet capacity
	if (i == BULLET_INFO_NUM_BULLETS) // rouz edit (ChatGPT)
		return false;
	gaBulletInfo[i].m_pSource = pSource;
	gaBulletInfo[i].m_eWeaponType = type;
	gaBulletInfo[i].m_nDamage = WeaponInfo_GetWeaponInfo(type)->m_nDamage;
	// Copy the by-value C-style position into the bullet record
	gaBulletInfo[i].m_vecPosition = vecPosition; // rouz edit (ChatGPT)
	// Copy the by-value C-style speed into the bullet record
	gaBulletInfo[i].m_vecSpeed = vecSpeed; // rouz edit (ChatGPT)
	gaBulletInfo[i].m_fTimer = (*Timer_GetTimeInMilliseconds()) + BULLET_LIFETIME;
	gaBulletInfo[i].m_bInUse = true;

#ifdef SQUEEZE_PERFORMANCE
	bulletInfoInUse++;
#endif
	return true;
}

bool BulletInfo_QueryCollision(const CBulletInfo *bullet,const BulletInfoVector *start,const BulletInfoVector *end,CColPoint *point,CEntity **entity)
{
    // Keep the original line-of-sight options and check occupants only after a hit
    if (!BulletInfo_HostLineOfSight(start,end,point,entity,true,true,true,true,true,false,false,true))
        return false;
    // Read the live weapon after the world query and preserve both caller-owned output slots
    BulletInfo_HostCheckOccupant(entity,point,bullet->m_eWeaponType,start,end);
    return true;
}

bool BulletInfo_ShouldDamagePed(const CBulletInfo *bullet,CEntity *entity)
{
    // Query ped state before comparing against the live source entity
    return !BulletInfo_HostPedDyingOrDead(entity) && entity != bullet->m_pSource;
}

bool BulletInfo_ShouldAnimatePed(CEntity *entity)
{
    // Preserve short-circuit evaluation of control and ducking state
    return BulletInfo_HostPedInControl(entity) && !BulletInfo_HostPedDucking(entity);
}

bool BulletInfo_HandlePedDamage(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition)
{
    // Retain the eligibility and animation decisions before applying damage
    if (!BulletInfo_ShouldDamagePed(bullet,entity))
        return false;
    if (BulletInfo_ShouldAnimatePed(entity)) {
        // Keep the native attack clear and C animation calls in their original order
        BulletInfo_HostClearPedAttack(entity);
        CAnimBlendAssociation *animation = AnimManager_AddAnimation(BulletInfo_HostPedClump(entity),ASSOCGRP_STD,ANIM_STD_HITBYGUN_FRONT);
        AnimAssociation_SetBlend(animation,0.0f,8.0f);
    }
    // Read live bullet and collision fields through the original damage call boundary
    BulletInfo_HostDamagePed(entity,bullet,point);
    Events_RegisterEntity(BulletInfo_HostPedIsCop(entity) ? EVENT_SHOOT_COP : EVENT_SHOOT_PED,EVENT_ENTITY_PED,entity,(CPed*)bullet->m_pSource,1000);
    bullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse--;
#endif
    newPosition->x = point->point.x;
    newPosition->y = point->point.y;
    newPosition->z = point->point.z;
    return true;
}

bool BulletInfo_HandlePedEffects(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition)
{
    // Skip all effects and state changes when the original game flag is disabled
    if (!BulletInfo_HostNastyGame())
        return false;
    // Retain the original float subtraction and particle direction
    CollisionPointRef position = BulletInfo_HostPedPosition(entity);
    BulletInfoVector direction = {(point->point.x - *position.x)*0.01f,(point->point.y - *position.y)*0.01f,0.01f};
    if (BulletInfo_HostPedOnScreen(entity)) {
        // Keep all eight particle calls and read the live collision point each time
        for (int i=0;i<8;++i)
            BulletInfo_HostBloodParticle(point,&direction);
    }
    if (BulletInfo_HostPedState(entity) == PED_DEAD) {
        // Retain both original clump reads and the selected floor animation
        CAnimBlendAssociation *animation;
        if (RpAnimBlendClumpGetFirstAssociationWithMask(BulletInfo_HostPedClump(entity),ASSOC_FRONTAL))
            animation = AnimManager_BlendAnimation(BulletInfo_HostPedClump(entity),ASSOCGRP_STD,ANIM_STD_HIT_FLOOR_FRONT,8.0f);
        else
            animation = AnimManager_BlendAnimation(BulletInfo_HostPedClump(entity),ASSOCGRP_STD,ANIM_STD_HIT_FLOOR,8.0f);
        if (animation) {
            // Preserve time and flag changes only for a returned association
            AnimAssociation_SetCurrentTime(animation,0.0f);
            animation->flags |= ASSOC_RUNNING;
            animation->flags &= ~ASSOC_FADEOUTWHENDONE;
        }
    }
    // Keep the original repeated deactivation and optional counter decrement
    bullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse--;
#endif
    newPosition->x=point->point.x;newPosition->y=point->point.y;newPosition->z=point->point.z;
    return true;
}

bool BulletInfo_HandleVehicleHit(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition)
{
    // Capture the original source once before the attachment checks
    CEntity *source = bullet->m_pSource;
    if (!source || !BulletInfo_HostIsPed(source) || BulletInfo_HostPedAttachedTo(source) != entity) {
        // Preserve the original wheel range and live collision-piece read
        if (point->pieceB >= BulletInfo_HostFirstWheelPiece() && point->pieceB <= BulletInfo_HostLastWheelPiece()) {
            // Burst the selected tyre before creating four smoke particles
            BulletInfo_HostBurstTyre(entity,point->pieceB,true);
            for (int i=0;i<4;++i)
                BulletInfo_HostImpactParticle(point,PARTICLE_BULLETHIT_SMOKE);
        } else {
            // Read the live bullet fields for damage, then select fire or sixteen sparks
            BulletInfo_HostVehicleDamage(entity,source,bullet->m_eWeaponType,bullet->m_nDamage);
            if (bullet->m_eWeaponType == WEAPONTYPE_FLAMETHROWER)
                FireManager_StartFireOn(&gFireManager,entity,bullet->m_pSource,0.8f,1);
            else
                for (int i=0;i<16;++i)
                    BulletInfo_HostImpactParticle(point,PARTICLE_SPARK);
        }
    }
#ifdef FIX_BUGS
    // Preserve optional deactivation and the live collision point after all effects
    bullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse--;
#endif
    newPosition->x=point->point.x;newPosition->y=point->point.y;newPosition->z=point->point.z;
    return true;
#else
    // Retain the legacy path where this branch leaves the bullet active and position unchanged
    (void)newPosition;
    return false;
#endif
}

bool BulletInfo_HandleOtherHit(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition)
{
    // Emit the original eight sparks before reading the source entity
    for (int i=0;i<8;++i)
        BulletInfo_HostImpactParticle(point,PARTICLE_SPARK);
    CEntity *source = bullet->m_pSource;
    if (!source || !BulletInfo_HostIsPed(source) || BulletInfo_HostPedAttachedTo(source) != entity) {
        // Preserve object-only handling and the original short-circuit field reads
        if (BulletInfo_HostIsObject(entity)) {
            if (!BulletInfo_HostObjectInfiniteMass(entity) && BulletInfo_HostObjectDamageMultiplier(entity) < 99.9f) {
                bool notStatic = !BulletInfo_HostObjectIsStatic(entity);
                if (notStatic && BulletInfo_HostObjectUprootLimit(entity) <= 0.0f) {
                    // Clear the original static flag before adding the object to the moving list
                    BulletInfo_HostObjectClearStatic(entity);
                    BulletInfo_HostObjectAddMoving(entity);
                }
                // Read static state again because the moving-list operation can change it
                notStatic = !BulletInfo_HostObjectIsStatic(entity);
                if (!notStatic) {
                    BulletInfoVector force = {point->normal.x * -7.5f,point->normal.y * -7.5f,point->normal.z * -7.5f};
                    BulletInfo_HostObjectApplyForce(entity,force.x,force.y,force.z);
                }
            } else if (BulletInfo_HostObjectDamageEffect(entity) >= BulletInfo_HostSmashEffect()) {
                // Preserve the original object damage threshold and amount
                BulletInfo_HostObjectDamage(entity,50.0f);
            }
        }
    }
#ifdef FIX_BUGS
    // Retain fixed-build deactivation after all particle and object operations
    bullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse--;
#endif
    newPosition->x=point->point.x;newPosition->y=point->point.y;newPosition->z=point->point.z;
    return true;
#else
    // Retain the legacy branch where the bullet and position remain unchanged
    (void)newPosition;
    return false;
#endif
}

void BulletInfo_HandleSniperAudio(const CBulletInfo *bullet,CEntity *entity)
{
    // Keep the weapon guard before querying the hit entity type
    if (!BulletInfo_IsSniperWeapon(bullet->m_eWeaponType))
        return;
    int32_t type = BulletInfo_HostEntityType(entity);
    switch (type) {
    case ENTITY_TYPE_BUILDING:
    case ENTITY_TYPE_OBJECT:
    case ENTITY_TYPE_DUMMY: {
        // Retain the audio-initialized guard before allocating a script object
        if (!DMAudio_IsAudioInitialised())
            break;
        cAudioScriptObject *audio = (cAudioScriptObject*)CPool_New(CPools_GetAudioScriptObjectPool());
        if (audio)
            AudioScriptObject_Reset(audio);
        if (audio)
            AudioScriptObject_Reset(audio);
        // Copy the original position after both resets and retain the original allocation behavior
        CollisionPointRef position = BulletInfo_HostPedPosition(entity);
        BulletInfoVector positionCopy = {*position.x,*position.y,*position.z};
        audio->Posn.x=positionCopy.x;audio->Posn.y=positionCopy.y;audio->Posn.z=positionCopy.z;
        audio->AudioId = type == ENTITY_TYPE_BUILDING ? SCRIPT_SOUND_BULLET_HIT_GROUND_1 : type == ENTITY_TYPE_OBJECT ? SCRIPT_SOUND_BULLET_HIT_GROUND_2 : SCRIPT_SOUND_BULLET_HIT_GROUND_3;
        audio->AudioEntity = AEHANDLE_NONE;
        DMAudio_CreateOneShotScriptObject(audio);
        break;
    }
    case ENTITY_TYPE_PED:
        // Retain the original hit statistic, sound, and speech order
        ++Stats_BulletsThatHit;
        DMAudio_PlayOneShot(BulletInfo_HostPedAudioId(entity),SOUND_WEAPON_HIT_PED,1.0f);
        BulletInfo_HostPedSay(entity,SOUND_PED_BULLET_HIT);
        break;
    case ENTITY_TYPE_VEHICLE:
        // Retain the original vehicle statistic and sound
        ++Stats_BulletsThatHit;
        DMAudio_PlayOneShot(BulletInfo_HostVehicleAudioId(entity),SOUND_WEAPON_HIT_VEHICLE,1.0f);
        break;
    default:
        break;
    }
}

void BulletInfo_FinishStep(CBulletInfo *bullet,const BulletInfoVector *newPosition)
{
    // Update the original native sniper endpoints only for the selected weapon types
    if (BulletInfo_IsSniperWeapon(bullet->m_eWeaponType)) {
        bPlayerSniperBullet = true;
        BulletInfo_HostStoreSniperStart(&bullet->m_vecPosition);
        BulletInfo_HostStoreSniperEnd(newPosition);
    }
    // Store the current bullet position before the unconditional helicopter query
    bullet->m_vecPosition = *newPosition;
    BulletInfo_HostTestHeli();
}

bool BulletInfo_BeginFrame(void)
{
    // Keep the optional early return ahead of the sniper flag reset
#ifdef SQUEEZE_PERFORMANCE
    if (bulletInfoInUse == 0)
        return false;
#endif
    bPlayerSniperBullet = false;
    return true;
}

bool BulletInfo_IsSniperWeapon(enum eWeaponType weapon)
{
    // Preserve both original weapon choices for sniper tracking
    return weapon == WEAPONTYPE_SNIPERRIFLE || weapon == WEAPONTYPE_LASERSCOPE;
}

void BulletInfo_BeginWorldQuery(const CBulletInfo *bullet)
{
    // Apply the original flags and read the source at its original assignment boundary
    *BulletInfo_HostWorldDeadPeds() = true;
    *BulletInfo_HostWorldBikers() = true;
    *BulletInfo_HostWorldCarTyres() = true;
    *BulletInfo_HostWorldIgnoreEntity() = bullet->m_pSource;
}

void BulletInfo_EndWorldQuery(void)
{
    // Restore the original values in their original order after the collision query
    *BulletInfo_HostWorldIgnoreEntity() = NULL;
    *BulletInfo_HostWorldDeadPeds() = false;
    *BulletInfo_HostWorldCarTyres() = false;
    *BulletInfo_HostWorldBikers() = false;
}

bool BulletInfo_PrepareStep(CBulletInfo *bullet, BulletInfoVector *oldPosition, BulletInfoVector *newPosition)
{
    // Validate the source before the inactive check, as in the original update
    if (bullet->m_pSource && BulletInfo_HostIsPed(bullet->m_pSource) && !BulletInfo_HostPedValid(bullet->m_pSource))
        bullet->m_pSource = NULL;
    if (!bullet->m_bInUse)
        return false;
    if ((*Timer_GetTimeInMilliseconds()) > bullet->m_fTimer) {
        // Retain the original expiration update without skipping the rest of this frame
        bullet->m_bInUse = false;
#ifdef SQUEEZE_PERFORMANCE
        bulletInfoInUse--;
#endif
    }
    // Form each original float vector temporary before applying the world bounds
    *oldPosition = bullet->m_vecPosition;
    float step = *Timer_GetTimeStep();
    BulletInfoVector scaled = { bullet->m_vecSpeed.x * step, bullet->m_vecSpeed.y * step, bullet->m_vecSpeed.z * step };
    scaled.x *= 0.5f; scaled.y *= 0.5f; scaled.z *= 0.5f;
    newPosition->x = bullet->m_vecPosition.x + scaled.x;
    newPosition->y = bullet->m_vecPosition.y + scaled.y;
    newPosition->z = bullet->m_vecPosition.z + scaled.z;
    if (newPosition->x <= -2400.0f || newPosition->x >= 1600.0f || newPosition->y <= -2000.0f || newPosition->y >= 2000.0f) {
        // Preserve the original boundary rejection and its counter behavior
        bullet->m_bInUse = false;
        return false;
    }
    return true;
}

void BulletInfo_Update(void)
{
    // Preserve the optional frame guard and traverse the original fixed-capacity array
    if (!BulletInfo_BeginFrame())
        return;
    for (int i=0;i<BULLET_INFO_NUM_BULLETS;++i) {
        // Keep validation, expiry, movement, and boundary checks ahead of the world query
        CBulletInfo *bullet = &gaBulletInfo[i];
        BulletInfoVector oldPosition,newPosition;
        if (!BulletInfo_PrepareStep(bullet,&oldPosition,&newPosition))
            continue;
        BulletInfo_BeginWorldQuery(bullet);
        CColPoint point;
        CEntity *hitEntity;
        if (BulletInfo_QueryCollision(bullet,&oldPosition,&newPosition,&point,&hitEntity)) {
            // Dispatch the original entity branches in their original short-circuit order
            if (BulletInfo_HostIsPed(hitEntity)) {
                BulletInfo_HandlePedDamage(bullet,hitEntity,&point,&newPosition);
                BulletInfo_HandlePedEffects(bullet,hitEntity,&point,&newPosition);
            } else if (BulletInfo_HostIsVehicle(hitEntity)) {
                BulletInfo_HandleVehicleHit(bullet,hitEntity,&point,&newPosition);
            } else {
                BulletInfo_HandleOtherHit(bullet,hitEntity,&point,&newPosition);
            }
            // Preserve sniper audio, glass, and explosive handling after each collision
            BulletInfo_HandleSniperAudio(bullet,hitEntity);
            BulletInfo_HostGlassHit(hitEntity,&point);
            BulletInfo_HostBlowUpExplosive(hitEntity);
        }
        // Reset world query state before endpoint tracking and the helicopter operation
        BulletInfo_EndWorldQuery();
        BulletInfo_FinishStep(bullet,&newPosition);
    }
}

bool BulletInfo_TestForSniperBullet(float x1, float x2, float y1, float y2, float z1, float z2)
{
	// Retain the original sniper flag and live vector component reads
	if (!bPlayerSniperBullet)
		return false;
	CollisionPointRef start = BulletInfo_HostSniperStart();
	CollisionPointRef end = BulletInfo_HostSniperEnd();
#ifdef FIX_BUGS // original code is not going work anyway...
	// Construct the original line and box before the native collision test
	CColLine line;
	ColLine_Init(&line,start,end);
	CColBox box;
	CollisionVector3 minimum = {x1, y1, z1}, maximum = {x2, y2, z2};
	ColBox_Set(&box,COLLISION_POINT_REF(minimum),COLLISION_POINT_REF(maximum),SURFACE_DEFAULT,0);
	return BulletInfo_HostTestLineBox(&line,&box.bounds);
#else
	float minP = 0.0f;
	float maxP = 1.0f;
	float minX = Min((*start.x), (*end.x));
	float maxX = Max((*start.x), (*end.x));
	if (minX < x2 || maxX > x1) {
		if (minX < x1)
			minP = Min(minP, (x1 - minX) / (maxX - minX));
		if (maxX > x2)
			maxP = Max(maxP, (maxX - x2) / (maxX - minX));
	}
	else
		return false;
	float minY = Min((*start.y), (*end.y));
	float maxY = Max((*start.y), (*end.y));
	if (minY < y2 || maxY > y1) {
		if (minY < y1)
			minP = Min(minP, (y1 - minY) / (maxY - minY));
		if (maxY > y2)
			maxP = Max(maxP, (maxY - y2) / (maxY - minY));
	}
#ifdef FIX_BUGS
	else
		return false;
#endif
	float minZ = Min((*start.z), (*end.z));
	float maxZ = Max((*start.z), (*end.z));
	if (minZ < z2 || maxZ > z1) {
		if (minZ < z1)
			minP = Min(minP, (z1 - minZ) / (maxZ - minZ));
		if (maxZ > z2)
			maxP = Max(maxP, (maxZ - z2) / (maxZ - minZ));
	}
	else
		return false;
	return minP <= maxP;
#endif
}
//- rouz edit (ChatGPT)
