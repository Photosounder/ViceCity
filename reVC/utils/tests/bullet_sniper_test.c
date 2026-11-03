#include "BulletInfo.h"
#include "BulletInfo_backend.h"
#include "WeaponInfo.h"
#include "ColLine.h"
#include "ColBox.h"
#include "AnimManager.h"
#include "EventList.h"
#include "RpAnimBlend.h"
#include "PedStates.h"
#include "ParticleType.h"
#include "Fire.h"
#include "DMAudio.h"
#include "AudioScriptObject.h"
#include "Pools.h"
#include "Stats.h"
#include "EntityTypes.h"
#include <assert.h>
uint32_t Timer_timeMs;
float Timer_timeStep;
static bool sourceIsPed, sourceValid;
static int sourceChecks, validityChecks;
static bool worldDead, worldBikers, worldTyres;
static CEntity *worldIgnore;
static unsigned worldOrder;
static CWeaponInfo info;
static int queryCalls, occupantCalls;
static bool pedDead, pedControl, pedDucking, changePedSource;
static int deadChecks, controlChecks, duckChecks;
static CBulletInfo *pedOwner;
static unsigned damageTrace;
static bool trackDamage;
bool BulletInfo_HostPedDyingOrDead(CEntity *entity)
{
    // Observe the state query and optionally change the live source during the call
    ++deadChecks; if (trackDamage) damageTrace=damageTrace*10+1;
    if (changePedSource) pedOwner->m_pSource = entity;
    return pedDead;
}
bool BulletInfo_HostPedInControl(CEntity *entity)
{
    // Observe the control query without changing ped state
    assert(entity); ++controlChecks; if (trackDamage) damageTrace=damageTrace*10+2; return pedControl;
}
bool BulletInfo_HostPedDucking(CEntity *entity)
{
    // Observe the ducking query only after the control query succeeds
    assert(entity); ++duckChecks; if (trackDamage) damageTrace=damageTrace*10+3; return pedDucking;
}
static bool effectsEnabled, effectsOnScreen, effectsFrontal, effectsReturnAnimation;
static int effectsState, effectsParticles, effectsBlends, effectsTimes;
static CollisionVector3 effectsPosition = {0,0,0};
static CAnimBlendAssociation effectsAnimation;
bool BulletInfo_HostNastyGame(void) { return effectsEnabled; }
CollisionPointRef BulletInfo_HostPedPosition(CEntity *entity) { assert(entity); return COLLISION_POINT_REF(effectsPosition); }
bool BulletInfo_HostPedOnScreen(CEntity *entity) { assert(entity); return effectsOnScreen; }
int32_t BulletInfo_HostPedState(CEntity *entity) { assert(entity); return effectsState; }
void BulletInfo_HostBloodParticle(const CColPoint *point,const BulletInfoVector *direction)
{
    // Verify stable direction and live collision-point reads across particle callbacks
    assert(direction->x==0.01f && direction->y==0.02f && direction->z==0.01f);
    assert(point->point.x==(effectsParticles ? 9 : 1));
    ++effectsParticles; ((CColPoint*)point)->point.x=9;
}
CAnimBlendAssociation *RpAnimBlendClumpGetFirstAssociationWithMask(void *clump,uint32_t mask)
{
    // Select the controlled frontal-association branch
    assert(clump==&info && mask==ASSOC_FRONTAL); return effectsFrontal ? &effectsAnimation : 0;
}
CAnimBlendAssociation *AnimManager_BlendAnimation(void *clump,int32_t groupId,int32_t animId,float delta)
{
    // Check the selected original floor animation
    assert(clump==&info && groupId==ASSOCGRP_STD && delta==8);
    assert(animId==(effectsFrontal ? ANIM_STD_HIT_FLOOR_FRONT : ANIM_STD_HIT_FLOOR));
    ++effectsBlends; return effectsReturnAnimation ? &effectsAnimation : 0;
}
void AnimAssociation_SetCurrentTime(CAnimBlendAssociation *animation,float time)
{
    // Observe the original time reset without modifying the controlled flags
    assert(animation==&effectsAnimation && time==0); ++effectsTimes;
}
static CEntity *pedAttachment;
static int tyreBursts, vehicleDamages, impactParticles, vehicleFires;
static bool changeVehicleDamage;
static CBulletInfo *vehicleOwner;
static CColPoint *vehiclePoint;
CFireManager gFireManager;
CEntity *BulletInfo_HostPedAttachedTo(CEntity *entity) { assert(entity); return pedAttachment; }
int32_t BulletInfo_HostFirstWheelPiece(void) { return 13; }
int32_t BulletInfo_HostLastWheelPiece(void) { return 16; }
void BulletInfo_HostBurstTyre(CEntity *entity,uint8_t piece,bool physical)
{
    // Observe the selected wheel before changing the live collision record
    assert(entity && piece==13 && physical); ++tyreBursts; vehiclePoint->point.x=9;
}
void BulletInfo_HostVehicleDamage(CEntity *entity,CEntity *source,enum eWeaponType weapon,int16_t damage)
{
    // Observe original damage arguments and optionally mutate later live bullet fields
    assert(entity && !source && weapon==WEAPONTYPE_COLT45 && damage==44); ++vehicleDamages;
    if (changeVehicleDamage) { vehicleOwner->m_eWeaponType=WEAPONTYPE_FLAMETHROWER; vehicleOwner->m_pSource=(CEntity*)&queryCalls; }
}
void BulletInfo_HostImpactParticle(const CColPoint *point,int32_t type)
{
    // Observe each actual live collision point and the selected particle kind
    assert(point->point.x==9 && (type==PARTICLE_SPARK || type==PARTICLE_BULLETHIT_SMOKE)); ++impactParticles;
}
CFire *FireManager_StartFireOn(CFireManager *manager,CEntity *entity,CEntity *source,float strength,uint8_t propagation)
{
    // Observe the post-damage live source and original fire options
    assert(manager==&gFireManager && entity && source==(CEntity*)&queryCalls && strength==0.8f && propagation==1); ++vehicleFires; return 0;
}
static bool objectHit, objectInfinite, objectStatic;
static float objectMultiplier, objectUproot;
static int objectStaticChecks, objectMovingCalls, objectForceCalls, objectDamageCalls;
bool BulletInfo_HostIsObject(CEntity *entity) { assert(entity); return objectHit; }
bool BulletInfo_HostObjectInfiniteMass(CEntity *entity) { assert(entity); return objectInfinite; }
float BulletInfo_HostObjectDamageMultiplier(CEntity *entity) { assert(entity); return objectMultiplier; }
bool BulletInfo_HostObjectIsStatic(CEntity *entity) { assert(entity); ++objectStaticChecks; return objectStatic; }
float BulletInfo_HostObjectUprootLimit(CEntity *entity) { assert(entity); return objectUproot; }
void BulletInfo_HostObjectClearStatic(CEntity *entity) { assert(entity); objectStatic=false; }
void BulletInfo_HostObjectAddMoving(CEntity *entity)
{
    // Change the state and collision point during the moving-list callback
    assert(entity); ++objectMovingCalls; objectStatic=true; vehiclePoint->point.x=12;
}
void BulletInfo_HostObjectApplyForce(CEntity *entity,float x,float y,float z)
{
    // Check the force computed from the actual collision normal
    assert(entity && x==-7.5f && y==15.0f && z==-3.75f); ++objectForceCalls;
}
int32_t BulletInfo_HostObjectDamageEffect(CEntity *entity) { assert(entity); return 20; }
int32_t BulletInfo_HostSmashEffect(void) { return 20; }
void BulletInfo_HostObjectDamage(CEntity *entity,float amount) { assert(entity && amount==50); ++objectDamageCalls; }
static int32_t audioType, audioTypeReads;
static uint8_t audioInitialised;
static int audioResets, audioCreates, audioPlays, audioSays;
static int16_t expectedScriptSound;
static cAudioScriptObject audioRecord;
static uint8_t audioFlag;
static CPool audioPool = {(uint8_t*)&audioRecord,&audioFlag,1,-1,sizeof(audioRecord)};
int32_t Stats_BulletsThatHit;
int32_t BulletInfo_HostEntityType(CEntity *entity) { assert(entity); ++audioTypeReads; return audioType; }
int32_t BulletInfo_HostPedAudioId(CEntity *entity) { assert(entity); return 111; }
int32_t BulletInfo_HostVehicleAudioId(CEntity *entity) { assert(entity); return 222; }
void BulletInfo_HostPedSay(CEntity *entity,uint16_t sound) { assert(entity && sound==SOUND_PED_BULLET_HIT); ++audioSays; }
uint8_t DMAudio_IsAudioInitialised(void) { return audioInitialised; }
CPool *CPools_GetAudioScriptObjectPool(void) { return &audioPool; }
void AudioScriptObject_Reset(cAudioScriptObject *object) { assert(object==&audioRecord); ++audioResets; }
void DMAudio_CreateOneShotScriptObject(cAudioScriptObject *object)
{
    // Verify both prior resets and the exact script sound and copied position
    assert(object==&audioRecord && object->AudioId==expectedScriptSound && object->AudioEntity==AEHANDLE_NONE);
    assert(object->Posn.x==3 && object->Posn.y==4 && object->Posn.z==5 && audioResets==(audioCreates+1)*2);
    ++audioCreates;
}
void DMAudio_PlayOneShot(int32_t audioEntity,uint16_t sound,float volume)
{
    // Verify the selected native audio entity and one-shot sound
    assert(volume==1 && ((audioType==ENTITY_TYPE_PED && audioEntity==111 && sound==SOUND_WEAPON_HIT_PED) || (audioType==ENTITY_TYPE_VEHICLE && audioEntity==222 && sound==SOUND_WEAPON_HIT_VEHICLE)));
    ++audioPlays;
}
static int finishTrace;
static CBulletInfo *finishOwner;
static BulletInfoVector observedStart, observedEnd;
void BulletInfo_HostStoreSniperStart(const BulletInfoVector *value)
{
    // Observe the actual bullet position before it is updated
    assert(value==&finishOwner->m_vecPosition); finishTrace=finishTrace*10+1; observedStart=*value;
}
void BulletInfo_HostStoreSniperEnd(const BulletInfoVector *value)
{
    // Observe the selected endpoint before the bullet position write
    finishTrace=finishTrace*10+2; observedEnd=*value;
}
void BulletInfo_HostTestHeli(void)
{
    // Verify the bullet position is stored before the unconditional helicopter query
    finishTrace=finishTrace*10+3; assert(finishOwner->m_vecPosition.x==observedEnd.x);
}
static bool hitIsVehicle;
static int glassCalls, explosiveCalls;
bool BulletInfo_HostIsVehicle(CEntity *entity) { assert(entity); return hitIsVehicle; }
void BulletInfo_HostGlassHit(CEntity *entity,const CColPoint *point)
{
    // Observe glass handling before world query state is reset
    assert(entity && point->point.x==21 && worldDead && worldBikers && worldTyres);
    ++glassCalls;
}
void BulletInfo_HostBlowUpExplosive(CEntity *entity)
{
    // Retain the original glass-before-explosive order
    assert(entity && glassCalls==1 && worldDead && worldBikers && worldTyres); ++explosiveCalls;
}
static bool queryResult;
static CBulletInfo *queryBullet;
bool BulletInfo_HostLineOfSight(const BulletInfoVector *start,const BulletInfoVector *end,CColPoint *point,CEntity **entity,bool buildings,bool vehicles,bool peds,bool objects,bool dummies,bool seeThrough,bool someObjects,bool shootThrough)
{
    // Observe the exact query inputs and write through the caller's real output slots
    ++queryCalls;
    assert(start->x == 1 && start->y == 2 && start->z == 3);
    assert(end->x == 4 && end->y == 5 && end->z == 6);
    assert(buildings && vehicles && peds && objects && dummies && !seeThrough && !someObjects && shootThrough);
    if (queryResult) { point->depth = 7; point->point=(CollisionVector3){1,2,3}; *entity = (CEntity*)&info; queryBullet->m_eWeaponType = WEAPONTYPE_SNIPERRIFLE; }
    return queryResult;
}
void BulletInfo_HostCheckOccupant(CEntity **entity,CColPoint *point,enum eWeaponType weapon,const BulletInfoVector *start,const BulletInfoVector *end)
{
    // Verify the live post-query weapon and replace both caller-owned outputs
    ++occupantCalls;
    assert(*entity == (CEntity*)&info && point->depth == 7 && weapon == WEAPONTYPE_SNIPERRIFLE);
    assert(start->x == 1 && end->z == 6);
    *entity = (CEntity*)&queryCalls; point->depth = 8;
}
void BulletInfo_HostClearPedAttack(CEntity *entity)
{
    // Record the original native attack-clear boundary
    assert(entity); damageTrace=damageTrace*10+4;
}
void *BulletInfo_HostPedClump(CEntity *entity)
{
    // Supply the actual controlled clump identity
    assert(entity); damageTrace=damageTrace*10+5; return &info;
}
CAnimBlendAssociation *AnimManager_AddAnimation(void *clump,int32_t groupId,int32_t animId)
{
    // Observe the original animation selection and return a typed controlled association
    assert(clump==&info && groupId==ASSOCGRP_STD && animId==ANIM_STD_HITBYGUN_FRONT);
    damageTrace=damageTrace*10+6; return (CAnimBlendAssociation*)&info;
}
void AnimAssociation_SetBlend(CAnimBlendAssociation *animation,float amount,float delta)
{
    // Observe the original blend values
    assert(animation==(CAnimBlendAssociation*)&info && amount==0 && delta==8); damageTrace=damageTrace*10+7;
}
void BulletInfo_HostDamagePed(CEntity *entity,const CBulletInfo *bullet,const CColPoint *point)
{
    // Change live values as a synchronous damage callback may do
    assert(entity && bullet==pedOwner); damageTrace=damageTrace*10+8;
    pedOwner->m_pSource=(CEntity*)&queryCalls; ((CColPoint*)point)->point.x=21;
}
bool BulletInfo_HostPedIsCop(CEntity *entity)
{
    // Observe the ped type after damage processing
    assert(entity); damageTrace=damageTrace*10+9; return true;
}
void Events_RegisterEntity(eEventType type,eEventEntity entityType,CEntity *entity,CPed *criminal,int32_t timeout)
{
    // Verify event inputs use the live source after the damage operation
    assert(type==EVENT_SHOOT_COP && entityType==EVENT_ENTITY_PED && entity && (void*)criminal==(void*)&queryCalls && timeout==1000);
    damageTrace=damageTrace*10;
}
bool *BulletInfo_HostWorldDeadPeds(void) { worldOrder = worldOrder*10+1; return &worldDead; }
bool *BulletInfo_HostWorldBikers(void) { worldOrder = worldOrder*10+2; return &worldBikers; }
bool *BulletInfo_HostWorldCarTyres(void) { worldOrder = worldOrder*10+3; return &worldTyres; }
CEntity **BulletInfo_HostWorldIgnoreEntity(void) { worldOrder = worldOrder*10+4; return &worldIgnore; }
bool BulletInfo_HostIsPed(CEntity *entity) { assert(entity); ++sourceChecks; return sourceIsPed; }
bool BulletInfo_HostPedValid(CEntity *entity) { assert(entity); ++validityChecks; return sourceValid; }
static bool result;
static int calls;
static CollisionVector3 start = {-2,0,0}, end = {2,0,0};
CWeaponInfo *WeaponInfo_GetWeaponInfo(enum eWeaponType weapon) { (void)weapon; return &info; }
void BulletInfo_HostDebug(const char *message) { (void)message; }
CollisionPointRef BulletInfo_HostSniperStart(void) { return COLLISION_POINT_REF(start); }
CollisionPointRef BulletInfo_HostSniperEnd(void) { return COLLISION_POINT_REF(end); }
#ifdef FIX_BUGS
void ColLine_Init(CColLine *line, CollisionPointRef first, CollisionPointRef last)
{
    // Capture the live input vector components at the original construction boundary
    line->p0 = (CollisionVector3){*first.x,*first.y,*first.z};
    line->p1 = (CollisionVector3){*last.x,*last.y,*last.z};
}
void ColBox_Set(CColBox *box, CollisionPointRef min, CollisionPointRef max, uint8_t surface, uint8_t piece)
{
    // Capture the exact bounds and surface supplied by the C operation
    box->bounds.min = (CollisionVector3){*min.x,*min.y,*min.z};
    box->bounds.max = (CollisionVector3){*max.x,*max.y,*max.z};
    box->surface = surface; box->piece = piece;
}
bool BulletInfo_HostTestLineBox(const CColLine *line, const CBox *box)
{
    // Verify full collision inputs before returning the controlled native result
    ++calls; assert(line->p0.x == -2 && line->p1.x == 2);
    assert(box->min.x == -1 && box->max.x == 1 && box->min.y == -1 && box->max.z == 1);
    return result;
}
#endif
int main(void)
{
    // Verify inactive shots skip collision and active shots retain the selected branch behavior
    assert(!BulletInfo_TestForSniperBullet(-1,1,-1,1,-1,1)); assert(calls == 0);
    bPlayerSniperBullet = true;
#ifdef FIX_BUGS
    result = true; assert(BulletInfo_TestForSniperBullet(-1,1,-1,1,-1,1));
    result = false; assert(!BulletInfo_TestForSniperBullet(-1,1,-1,1,-1,1)); assert(calls == 2);
#else
    assert(BulletInfo_TestForSniperBullet(-1,1,-1,1,-1,1));
    assert(!BulletInfo_TestForSniperBullet(3,-3,-1,1,-1,1));
#endif
    // Preserve the optional empty-frame return and both sniper weapon choices
    assert(BulletInfo_IsSniperWeapon(WEAPONTYPE_SNIPERRIFLE));
    assert(BulletInfo_IsSniperWeapon(WEAPONTYPE_LASERSCOPE));
    assert(!BulletInfo_IsSniperWeapon(WEAPONTYPE_COLT45));
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse = 0; bPlayerSniperBullet = true;
    assert(!BulletInfo_BeginFrame() && bPlayerSniperBullet);
    bulletInfoInUse = 1;
#endif
    bPlayerSniperBullet = true;
    assert(BulletInfo_BeginFrame() && !bPlayerSniperBullet);
    // Verify both collision outcomes and the exact output slots supplied by the caller
    BulletInfoVector queryStart = {1,2,3}, queryEnd = {4,5,6};
    CColPoint queryPoint = {0}; CEntity *queryEntity = 0;
    CBulletInfo queryOwner = {0}; queryBullet = &queryOwner;
    queryResult = false;
    assert(!BulletInfo_QueryCollision(&queryOwner,&queryStart,&queryEnd,&queryPoint,&queryEntity));
    assert(!queryEntity && queryPoint.depth == 0 && occupantCalls == 0);
    queryResult = true;
    assert(BulletInfo_QueryCollision(&queryOwner,&queryStart,&queryEnd,&queryPoint,&queryEntity));
    assert(queryEntity == (CEntity*)&queryCalls && queryPoint.depth == 8 && queryCalls == 2 && occupantCalls == 1);
    // Check ped short-circuiting and source changes during the native state query
    CBulletInfo pedBullet = {0}; CEntity *ped = (CEntity*)&info; pedOwner = &pedBullet;
    pedDead = true; assert(!BulletInfo_ShouldDamagePed(&pedBullet,ped));
    pedDead = false; changePedSource = true; assert(!BulletInfo_ShouldDamagePed(&pedBullet,ped));
    changePedSource = false; pedBullet.m_pSource = 0; assert(BulletInfo_ShouldDamagePed(&pedBullet,ped));
    assert(deadChecks == 3);
    pedControl = false; assert(!BulletInfo_ShouldAnimatePed(ped) && duckChecks == 0);
    pedControl = true; pedDucking = true; assert(!BulletInfo_ShouldAnimatePed(ped));
    pedDucking = false; assert(BulletInfo_ShouldAnimatePed(ped));
    assert(controlChecks == 3 && duckChecks == 2);
    // Compare the whole ped damage call order and retain callback-updated source and collision point
    CBulletInfo damageBullet = {0}; CColPoint damagePoint = {0}; BulletInfoVector hitPosition = {0};
    damageBullet.m_bInUse = true; damagePoint.point = (CollisionVector3){1,2,3}; pedOwner = &damageBullet;
    pedDead = false; pedControl = true; pedDucking = false; changePedSource = false; damageTrace = 0; trackDamage = true;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse = 3;
#endif
    assert(BulletInfo_HandlePedDamage(&damageBullet,ped,&damagePoint,&hitPosition));
    assert(damageTrace == 1234567890u && !damageBullet.m_bInUse && hitPosition.x == 21 && hitPosition.y == 2 && hitPosition.z == 3);
#ifdef SQUEEZE_PERFORMANCE
    assert(bulletInfoInUse == 2);
#endif
    trackDamage = false;
    // Exercise disabled effects and both floor animation branches with live particle inputs
    CBulletInfo effectBullet = {0}; CColPoint effectPoint = {0}; BulletInfoVector effectEnd = {0};
    effectBullet.m_bInUse = true; effectPoint.point = (CollisionVector3){1,2,3}; effectsEnabled = false;
    assert(!BulletInfo_HandlePedEffects(&effectBullet,ped,&effectPoint,&effectEnd) && effectBullet.m_bInUse);
    effectsEnabled = true; effectsOnScreen = true; effectsState = PED_DEAD; effectsFrontal = true; effectsReturnAnimation = true;
    effectsAnimation.flags = ASSOC_FADEOUTWHENDONE;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse = 3;
#endif
    assert(BulletInfo_HandlePedEffects(&effectBullet,ped,&effectPoint,&effectEnd));
    assert(effectsParticles==8 && effectsBlends==1 && effectsTimes==1 && effectEnd.x==9 && !effectBullet.m_bInUse);
    assert((effectsAnimation.flags & ASSOC_RUNNING) && !(effectsAnimation.flags & ASSOC_FADEOUTWHENDONE));
#ifdef SQUEEZE_PERFORMANCE
    assert(bulletInfoInUse==2);
#endif
    effectsOnScreen=false; effectsFrontal=false; effectsReturnAnimation=false;
    effectPoint.point=(CollisionVector3){1,2,3}; effectBullet.m_bInUse=true;
    assert(BulletInfo_HandlePedEffects(&effectBullet,ped,&effectPoint,&effectEnd));
    assert(effectsParticles==8 && effectsBlends==2 && effectsTimes==1 && effectEnd.x==1);
    // Check tyre, attached-source, post-damage fire, and spark branches
    CBulletInfo vehicleBullet = {0}; CColPoint vehicleHit = {0}; BulletInfoVector vehicleEnd = {0};
    CEntity *vehicle = (CEntity*)&info; vehicleOwner=&vehicleBullet; vehiclePoint=&vehicleHit;
    vehicleBullet.m_bInUse=true; vehicleBullet.m_eWeaponType=WEAPONTYPE_COLT45; vehicleBullet.m_nDamage=44;
    vehicleHit.pieceB=13; vehicleHit.point.x=1;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse=5;
#endif
#ifdef FIX_BUGS
    assert(BulletInfo_HandleVehicleHit(&vehicleBullet,vehicle,&vehicleHit,&vehicleEnd));
    assert(!vehicleBullet.m_bInUse && vehicleEnd.x==9);
#else
    assert(!BulletInfo_HandleVehicleHit(&vehicleBullet,vehicle,&vehicleHit,&vehicleEnd));
    assert(vehicleBullet.m_bInUse && vehicleEnd.x==0);
#endif
    assert(tyreBursts==1 && impactParticles==4);
    vehicleBullet.m_bInUse=true; vehicleBullet.m_pSource=(CEntity*)&queryCalls; sourceIsPed=true; pedAttachment=vehicle;
    BulletInfo_HandleVehicleHit(&vehicleBullet,vehicle,&vehicleHit,&vehicleEnd);
    assert(tyreBursts==1 && vehicleDamages==0 && impactParticles==4);
    vehicleBullet.m_bInUse=true; vehicleBullet.m_pSource=0; vehicleHit.pieceB=1; changeVehicleDamage=true;
    BulletInfo_HandleVehicleHit(&vehicleBullet,vehicle,&vehicleHit,&vehicleEnd);
    assert(vehicleDamages==1 && vehicleFires==1 && impactParticles==4);
    vehicleBullet.m_bInUse=true; vehicleBullet.m_pSource=0; vehicleBullet.m_eWeaponType=WEAPONTYPE_COLT45; changeVehicleDamage=false;
    BulletInfo_HandleVehicleHit(&vehicleBullet,vehicle,&vehicleHit,&vehicleEnd);
    assert(vehicleDamages==2 && vehicleFires==1 && impactParticles==20);
    // Verify object state is read twice around the moving-list callback
    CBulletInfo objectBullet = {0}; CColPoint objectPoint = {0}; BulletInfoVector objectEnd = {0};
    CEntity *object = (CEntity*)&info; objectBullet.m_bInUse=true; vehiclePoint=&objectPoint;
    objectPoint.point.x=9; objectPoint.normal=(CollisionVector3){1,-2,0.5f};
    objectHit=true; objectInfinite=false; objectMultiplier=1; objectStatic=false; objectUproot=0;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse=5;
#endif
#ifdef FIX_BUGS
    assert(BulletInfo_HandleOtherHit(&objectBullet,object,&objectPoint,&objectEnd));
    assert(!objectBullet.m_bInUse && objectEnd.x==12);
#else
    assert(!BulletInfo_HandleOtherHit(&objectBullet,object,&objectPoint,&objectEnd));
    assert(objectBullet.m_bInUse && objectEnd.x==0);
#endif
    assert(objectStaticChecks==2 && objectMovingCalls==1 && objectForceCalls==1 && impactParticles==28);
    // Retain the infinite-mass object damage path and its threshold
    objectBullet.m_bInUse=true; objectPoint.point.x=9; objectInfinite=true;
    BulletInfo_HandleOtherHit(&objectBullet,object,&objectPoint,&objectEnd);
    assert(objectDamageCalls==1 && objectStaticChecks==2 && impactParticles==36);
    // Retain the attached-source short circuit after all eight sparks
    objectBullet.m_bInUse=true; objectPoint.point.x=9; objectBullet.m_pSource=(CEntity*)&queryCalls;
    sourceIsPed=true; pedAttachment=object;
    BulletInfo_HandleOtherHit(&objectBullet,object,&objectPoint,&objectEnd);
    assert(objectDamageCalls==1 && impactParticles==44);
    // Verify the sniper guard, audio-disabled guard, three script sounds, and ped/vehicle sounds
    CBulletInfo audioBullet = {0}; audioBullet.m_eWeaponType=WEAPONTYPE_COLT45; audioType=ENTITY_TYPE_BUILDING;
    BulletInfo_HandleSniperAudio(&audioBullet,ped); assert(audioTypeReads==0);
    audioBullet.m_eWeaponType=WEAPONTYPE_SNIPERRIFLE; audioInitialised=0;
    BulletInfo_HandleSniperAudio(&audioBullet,ped); assert(audioTypeReads==1 && audioResets==0);
    audioInitialised=1; effectsPosition=(CollisionVector3){3,4,5};
    for (int type=ENTITY_TYPE_BUILDING;type<=ENTITY_TYPE_DUMMY;++type) {
        // Restrict script-object testing to the three original entity types
        if (type!=ENTITY_TYPE_BUILDING && type!=ENTITY_TYPE_OBJECT && type!=ENTITY_TYPE_DUMMY) continue;
        audioType=type; audioFlag=POOLFLAG_ISFREE; audioPool.m_allocPtr=-1;
        expectedScriptSound=type==ENTITY_TYPE_BUILDING ? SCRIPT_SOUND_BULLET_HIT_GROUND_1 : type==ENTITY_TYPE_OBJECT ? SCRIPT_SOUND_BULLET_HIT_GROUND_2 : SCRIPT_SOUND_BULLET_HIT_GROUND_3;
        BulletInfo_HandleSniperAudio(&audioBullet,ped);
    }
    assert(audioCreates==3 && audioResets==6);
    audioType=ENTITY_TYPE_PED; BulletInfo_HandleSniperAudio(&audioBullet,ped);
    audioType=ENTITY_TYPE_VEHICLE; BulletInfo_HandleSniperAudio(&audioBullet,ped);
    assert(Stats_BulletsThatHit==2 && audioPlays==2 && audioSays==1);
    // Verify sniper endpoint storage, position write, and helicopter query ordering
    CBulletInfo finishBullet = {0}; BulletInfoVector finishEnd={8,9,10};
    finishOwner=&finishBullet;finishBullet.m_eWeaponType=WEAPONTYPE_SNIPERRIFLE;finishBullet.m_vecPosition=(BulletInfoVector){1,2,3};
    finishTrace=0;bPlayerSniperBullet=false;BulletInfo_FinishStep(&finishBullet,&finishEnd);
    assert(finishTrace==123 && bPlayerSniperBullet && observedStart.x==1 && observedEnd.x==8 && finishBullet.m_vecPosition.x==8);
    // A non-sniper bullet leaves previous tracking intact but still invokes the helicopter query
    finishTrace=0;finishBullet.m_eWeaponType=WEAPONTYPE_COLT45;finishEnd.x=11;observedEnd.x=11;
    BulletInfo_FinishStep(&finishBullet,&finishEnd);assert(finishTrace==3 && bPlayerSniperBullet && finishBullet.m_vecPosition.x==11);
    // Check world flags and their exact original setup and reset order
    CBulletInfo query = {0}; query.m_pSource = (CEntity*)&info;
    BulletInfo_BeginWorldQuery(&query);
    assert(worldDead && worldBikers && worldTyres && worldIgnore == query.m_pSource);
    BulletInfo_EndWorldQuery();
    assert(!worldDead && !worldBikers && !worldTyres && worldIgnore == 0 && worldOrder == 12344132);
    // Check source validation before the inactive-slot return
    CBulletInfo bullet = {0}; BulletInfoVector oldPosition, newPosition;
    sourceChecks = validityChecks = 0;
    bullet.m_pSource = (CEntity*)&info; sourceIsPed = true; sourceValid = false;
    assert(!BulletInfo_PrepareStep(&bullet,&oldPosition,&newPosition));
    assert(bullet.m_pSource == 0 && sourceChecks == 1 && validityChecks == 1);
    // Check exact expiry and position updates with the original two scaling steps
    bullet.m_bInUse = true; bullet.m_fTimer = 100; bullet.m_vecPosition = (BulletInfoVector){1,2,3};
    bullet.m_vecSpeed = (BulletInfoVector){4,8,12}; Timer_timeMs = 100; Timer_timeStep = 0.5f;
    assert(BulletInfo_PrepareStep(&bullet,&oldPosition,&newPosition));
    assert(bullet.m_bInUse && oldPosition.x == 1 && newPosition.x == 2 && newPosition.y == 4 && newPosition.z == 6);
    Timer_timeMs = 101;
#ifdef SQUEEZE_PERFORMANCE
    bulletInfoInUse = 1;
#endif
    assert(BulletInfo_PrepareStep(&bullet,&oldPosition,&newPosition) && !bullet.m_bInUse);
#ifdef SQUEEZE_PERFORMANCE
    assert(bulletInfoInUse == 0);
#endif
    // Keep inclusive bounds rejection and its original counter behavior
    bullet.m_bInUse = true; bullet.m_fTimer = 200; bullet.m_vecPosition.x = 1600; bullet.m_vecSpeed.x = 0;
    assert(!BulletInfo_PrepareStep(&bullet,&oldPosition,&newPosition) && !bullet.m_bInUse);
    // Verify storage initialization, full-array rejection, and first-slot reuse
    gaBulletInfo[0].m_vecPosition.x=42;gaBulletInfo[0].m_nDamage=77;
    BulletInfo_Initialise();assert(gaBulletInfo[0].m_vecPosition.x==42 && gaBulletInfo[0].m_nDamage==77);
    Timer_timeMs=500;info.m_nDamage=34;BulletInfoVector storagePosition={1,2,3},storageSpeed={4,5,6};
    for (int i=0;i<BULLET_INFO_NUM_BULLETS;++i) assert(BulletInfo_AddBullet(0,WEAPONTYPE_COLT45,storagePosition,storageSpeed));
    assert(!BulletInfo_AddBullet(0,WEAPONTYPE_COLT45,storagePosition,storageSpeed));
    assert(gaBulletInfo[99].m_bInUse && gaBulletInfo[99].m_fTimer==1500 && gaBulletInfo[99].m_nDamage==34);
    gaBulletInfo[3].m_bInUse=false;storagePosition.x=9;
    assert(BulletInfo_AddBullet(0,WEAPONTYPE_COLT45,storagePosition,storageSpeed) && gaBulletInfo[3].m_vecPosition.x==9);
#ifdef SQUEEZE_PERFORMANCE
    assert(bulletInfoInUse==BULLET_INFO_NUM_BULLETS+1);
#endif
    BulletInfo_Shutdown();
    // Exercise complete miss and ped-hit update iterations with native callback observations
    for (int hit=0;hit<2;++hit) {
        // Reinitialize the real C array and create exactly one active bullet
        BulletInfo_Initialise();Timer_timeMs=500;Timer_timeStep=0.5f;
        BulletInfoVector updateStart={1,2,3},updateSpeed={12,12,12};
        assert(BulletInfo_AddBullet(0,WEAPONTYPE_COLT45,updateStart,updateSpeed));
        queryBullet=&gaBulletInfo[0];pedOwner=queryBullet;finishOwner=queryBullet;
        queryResult=hit!=0;sourceIsPed=true;pedDead=false;pedControl=false;changePedSource=false;
        effectsEnabled=false;audioType=ENTITY_TYPE_PED;worldOrder=0;finishTrace=0;glassCalls=explosiveCalls=0;
        observedEnd.x=hit ? 21 : 4;
        BulletInfo_Update();
        assert(worldOrder==12344132 && !worldDead && !worldBikers && !worldTyres && !worldIgnore);
        if (hit) {
            // Retain damage, audio, glass, explosive, and sniper endpoint behavior on a hit
            assert(glassCalls==1 && explosiveCalls==1 && !queryBullet->m_bInUse && queryBullet->m_vecPosition.x==21);
            assert(finishTrace==123 && bPlayerSniperBullet);
        } else {
            // Retain movement and the unconditional helicopter call on a miss
            assert(glassCalls==0 && explosiveCalls==0 && queryBullet->m_bInUse && queryBullet->m_vecPosition.x==4);
            assert(finishTrace==3 && !bPlayerSniperBullet);
        }
    }
    return 0;
}
