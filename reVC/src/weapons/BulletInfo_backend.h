//+ rouz edit (ChatGPT)
#pragma once
#include "BulletInfo.h"
#include "ColPoint.h"
#ifdef __cplusplus
extern "C" {
#endif
bool BulletInfo_HostLineOfSight(const BulletInfoVector *start,const BulletInfoVector *end,CColPoint *point,CEntity **entity,bool buildings,bool vehicles,bool peds,bool objects,bool dummies,bool seeThrough,bool someObjects,bool shootThrough);
void BulletInfo_HostCheckOccupant(CEntity **entity,CColPoint *point,enum eWeaponType weapon,const BulletInfoVector *start,const BulletInfoVector *end);
bool BulletInfo_QueryCollision(const CBulletInfo *bullet,const BulletInfoVector *start,const BulletInfoVector *end,CColPoint *point,CEntity **entity);
void BulletInfo_HostStoreSniperStart(const BulletInfoVector *value);
void BulletInfo_HostStoreSniperEnd(const BulletInfoVector *value);
void BulletInfo_HostTestHeli(void);
void BulletInfo_FinishStep(CBulletInfo *bullet,const BulletInfoVector *newPosition);
bool BulletInfo_BeginFrame(void);
bool BulletInfo_IsSniperWeapon(enum eWeaponType weapon);
bool *BulletInfo_HostWorldDeadPeds(void);
bool *BulletInfo_HostWorldBikers(void);
bool *BulletInfo_HostWorldCarTyres(void);
CEntity **BulletInfo_HostWorldIgnoreEntity(void);
void BulletInfo_BeginWorldQuery(const CBulletInfo *bullet);
void BulletInfo_EndWorldQuery(void);
int32_t BulletInfo_HostEntityType(CEntity *entity);
int32_t BulletInfo_HostPedAudioId(CEntity *entity);
int32_t BulletInfo_HostVehicleAudioId(CEntity *entity);
void BulletInfo_HostPedSay(CEntity *entity,uint16_t sound);
void BulletInfo_HandleSniperAudio(const CBulletInfo *bullet,CEntity *entity);
bool BulletInfo_HostNastyGame(void);
CollisionPointRef BulletInfo_HostPedPosition(CEntity *entity);
bool BulletInfo_HostPedOnScreen(CEntity *entity);
int32_t BulletInfo_HostPedState(CEntity *entity);
void BulletInfo_HostBloodParticle(const CColPoint *point,const BulletInfoVector *direction);
bool BulletInfo_HandlePedEffects(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition);
CEntity *BulletInfo_HostPedAttachedTo(CEntity *entity);
int32_t BulletInfo_HostFirstWheelPiece(void);
int32_t BulletInfo_HostLastWheelPiece(void);
void BulletInfo_HostBurstTyre(CEntity *entity,uint8_t piece,bool physical);
void BulletInfo_HostVehicleDamage(CEntity *entity,CEntity *source,enum eWeaponType weapon,int16_t damage);
void BulletInfo_HostImpactParticle(const CColPoint *point,int32_t type);
bool BulletInfo_HandleVehicleHit(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition);
bool BulletInfo_HostIsVehicle(CEntity *entity);
void BulletInfo_HostGlassHit(CEntity *entity,const CColPoint *point);
void BulletInfo_HostBlowUpExplosive(CEntity *entity);
bool BulletInfo_HostIsObject(CEntity *entity);
bool BulletInfo_HostObjectInfiniteMass(CEntity *entity);
float BulletInfo_HostObjectDamageMultiplier(CEntity *entity);
bool BulletInfo_HostObjectIsStatic(CEntity *entity);
float BulletInfo_HostObjectUprootLimit(CEntity *entity);
void BulletInfo_HostObjectClearStatic(CEntity *entity);
void BulletInfo_HostObjectAddMoving(CEntity *entity);
void BulletInfo_HostObjectApplyForce(CEntity *entity,float x,float y,float z);
int32_t BulletInfo_HostObjectDamageEffect(CEntity *entity);
int32_t BulletInfo_HostSmashEffect(void);
void BulletInfo_HostObjectDamage(CEntity *entity,float amount);
bool BulletInfo_HandleOtherHit(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition);
void BulletInfo_HostClearPedAttack(CEntity *entity);
void *BulletInfo_HostPedClump(CEntity *entity);
void BulletInfo_HostDamagePed(CEntity *entity,const CBulletInfo *bullet,const CColPoint *point);
bool BulletInfo_HostPedIsCop(CEntity *entity);
bool BulletInfo_HandlePedDamage(CBulletInfo *bullet,CEntity *entity,const CColPoint *point,BulletInfoVector *newPosition);
bool BulletInfo_HostPedDyingOrDead(CEntity *entity);
bool BulletInfo_HostPedInControl(CEntity *entity);
bool BulletInfo_HostPedDucking(CEntity *entity);
bool BulletInfo_ShouldDamagePed(const CBulletInfo *bullet,CEntity *entity);
bool BulletInfo_ShouldAnimatePed(CEntity *entity);
bool BulletInfo_HostIsPed(CEntity *entity);
bool BulletInfo_HostPedValid(CEntity *entity);
bool BulletInfo_PrepareStep(CBulletInfo *bullet, BulletInfoVector *oldPosition, BulletInfoVector *newPosition);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
