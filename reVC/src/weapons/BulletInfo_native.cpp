//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "CollisionVector_native.h"

#include "BulletInfo.h"
#include "BulletInfo_backend.h"

#include "AnimBlendAssociation.h"
#include "DMAudio.h"
#include "AudioScriptObject.h"
#include "Pools.h" // rouz edit (ChatGPT)
#ifdef FIX_BUGS
#include "Collision.h"
#endif
#include "RpAnimBlend.h"
#include "Entity.h"
#include "EventList.h"
#include "Fire.h"
#include "Glass.h"
#include "Particle.h"
#include "Ped.h"
#include "Object.h"
#include "Stats.h"
#include "Timer.h"
#include "Vehicle.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "World.h"
#include "SurfaceTable.h"
#include "Heli.h"


#define NUM_PED_BLOOD_PARTICLES (8)
#define BLOOD_PARTICLE_OFFSET (CVector(0.0f, 0.0f, 0.0f))
#define NUM_VEHICLE_SPARKS (16)
#define NUM_TYRE_POP_SMOKES (4)
#define NUM_OTHER_SPARKS (8)
#define BULLET_HIT_FORCE (7.5f)

#define BULLET_BOUNDARY_MIN_X -2400.0f
#define BULLET_BOUNDARY_MAX_X 1600.0f
#define BULLET_BOUNDARY_MIN_Y -2000.0f
#define BULLET_BOUNDARY_MAX_Y 2000.0f


CVector PlayerSniperBulletStart;
CVector PlayerSniperBulletEnd;
//+ rouz edit (ChatGPT)
extern "C" void BulletInfo_HostStoreSniperStart(const BulletInfoVector *value)
{
    // Update the actual native start vector without changing its identity
    PlayerSniperBulletStart = CVector(value->x,value->y,value->z);
}
extern "C" void BulletInfo_HostStoreSniperEnd(const BulletInfoVector *value)
{
    // Update the actual native end vector without changing its identity
    PlayerSniperBulletEnd = CVector(value->x,value->y,value->z);
}
extern "C" void BulletInfo_HostTestHeli(void)
{
    // Pass the original live native endpoint objects to the helicopter operation
    CHeli::TestSniperCollision(&PlayerSniperBulletStart,&PlayerSniperBulletEnd);
}
extern "C" CollisionPointRef BulletInfo_HostSniperStart(void)
{
    // Borrow the actual native start vector components
    return COLLISION_POINT_REF(PlayerSniperBulletStart);
}
extern "C" CollisionPointRef BulletInfo_HostSniperEnd(void)
{
    // Borrow the actual native end vector components
    return COLLISION_POINT_REF(PlayerSniperBulletEnd);
}
#ifdef FIX_BUGS
extern "C" bool BulletInfo_HostTestLineBox(const CColLine *line, const CBox *box)
{
    // Call the separately owned native collision operation with its real C records
    return CCollision::TestLineBox(*line, *box);
}
#endif
//- rouz edit (ChatGPT)


//+ rouz edit (ChatGPT)
extern "C" bool BulletInfo_HostLineOfSight(const BulletInfoVector *start,const BulletInfoVector *end,CColPoint *point,CEntity **entity,bool buildings,bool vehicles,bool peds,bool objects,bool dummies,bool seeThrough,bool someObjects,bool shootThrough)
{
    // Construct actual native input vectors and retain the original typed output objects
    CVector nativeStart(start->x,start->y,start->z), nativeEnd(end->x,end->y,end->z);
    return CWorld::ProcessLineOfSight(nativeStart,nativeEnd,*point,*entity,buildings,vehicles,peds,objects,dummies,seeThrough,someObjects,shootThrough);
}
extern "C" void BulletInfo_HostCheckOccupant(CEntity **entity,CColPoint *point,enum eWeaponType weapon,const BulletInfoVector *start,const BulletInfoVector *end)
{
    // Invoke the existing weapon operation with native vectors and the original output slots
    CVector nativeStart(start->x,start->y,start->z), nativeEnd(end->x,end->y,end->z);
    CWeapon::CheckForShootingVehicleOccupant(entity,point,weapon,nativeStart,nativeEnd);
}
extern "C" bool *BulletInfo_HostWorldDeadPeds(void)
{
    // Borrow the actual live world query flag
    return &CWorld::bIncludeDeadPeds;
}
extern "C" bool *BulletInfo_HostWorldBikers(void)
{
    // Borrow the actual live biker query flag
    return &CWorld::bIncludeBikers;
}
extern "C" bool *BulletInfo_HostWorldCarTyres(void)
{
    // Borrow the actual live tyre query flag
    return &CWorld::bIncludeCarTyres;
}
extern "C" CEntity **BulletInfo_HostWorldIgnoreEntity(void)
{
    // Borrow the actual live ignored entity slot
    return &CWorld::pIgnoreEntity;
}
extern "C" int32_t BulletInfo_HostEntityType(CEntity *entity)
{
    // Read the actual hit entity type once at the original switch boundary
    return entity->GetType();
}
extern "C" int32_t BulletInfo_HostPedAudioId(CEntity *entity)
{
    // Read the actual ped audio entity at the playback boundary
    return ((CPed*)entity)->m_audioEntityId;
}
extern "C" int32_t BulletInfo_HostVehicleAudioId(CEntity *entity)
{
    // Read the actual vehicle audio entity at the playback boundary
    return ((CVehicle*)entity)->m_audioEntityId;
}
extern "C" void BulletInfo_HostPedSay(CEntity *entity,uint16_t sound)
{
    // Invoke the actual ped speech operation with the C-selected sound
    ((CPed*)entity)->Say(sound);
}
extern "C" bool BulletInfo_HostNastyGame(void)
{
    // Read the actual live game effects flag
    return CGame::nastyGame;
}
extern "C" CollisionPointRef BulletInfo_HostPedPosition(CEntity *entity)
{
    // Borrow the actual native position components
    const CVector &position = entity->GetPosition();
    return COLLISION_POINT_REF(position);
}
extern "C" bool BulletInfo_HostPedOnScreen(CEntity *entity)
{
    // Query the actual entity visibility
    return entity->GetIsOnScreen();
}
extern "C" int32_t BulletInfo_HostPedState(CEntity *entity)
{
    // Read the actual ped state at the original boundary
    return ((CPed*)entity)->GetPedState();
}
extern "C" void BulletInfo_HostBloodParticle(const CColPoint *point,const BulletInfoVector *direction)
{
    // Call the native particle operation with actual vector temporaries
    CVector nativeDirection(direction->x,direction->y,direction->z);
    CParticle::AddParticle(PARTICLE_BLOOD_SMALL,CollisionVector_ToNative(point->point) + BLOOD_PARTICLE_OFFSET,nativeDirection);
}
extern "C" CEntity *BulletInfo_HostPedAttachedTo(CEntity *entity)
{
    // Read the actual live ped attachment
    return ((CPed*)entity)->m_attachedTo;
}
extern "C" int32_t BulletInfo_HostFirstWheelPiece(void)
{
    // Return the native first wheel identity
    return CAR_PIECE_WHEEL_LF;
}
extern "C" int32_t BulletInfo_HostLastWheelPiece(void)
{
    // Return the native last wheel identity
    return CAR_PIECE_WHEEL_RR;
}
extern "C" void BulletInfo_HostBurstTyre(CEntity *entity,uint8_t piece,bool physical)
{
    // Forward the original tyre operation to the actual vehicle
    ((CVehicle*)entity)->BurstTyre(piece,physical);
}
extern "C" void BulletInfo_HostVehicleDamage(CEntity *entity,CEntity *source,enum eWeaponType weapon,int16_t damage)
{
    // Forward the original damage operation to the actual vehicle
    ((CVehicle*)entity)->InflictDamage(source,weapon,damage);
}
extern "C" void BulletInfo_HostImpactParticle(const CColPoint *point,int32_t type)
{
    // Construct the original native particle arguments from the live collision record
    CParticle::AddParticle((tParticleType)type,CollisionVector_ToNative(point->point),CollisionVector_ToNative(point->normal) / 20);
}
extern "C" bool BulletInfo_HostIsVehicle(CEntity *entity)
{
    // Query the actual native entity classification
    return entity->IsVehicle();
}
extern "C" void BulletInfo_HostGlassHit(CEntity *entity,const CColPoint *point)
{
    // Pass the live collision point to the existing native glass operation
    CGlass::WasGlassHitByBullet(entity,CollisionVector_ToNative(point->point));
}
extern "C" void BulletInfo_HostBlowUpExplosive(CEntity *entity)
{
    // Invoke the existing native explosive-object operation
    CWeapon::BlowUpExplosiveThings(entity);
}
extern "C" bool BulletInfo_HostIsObject(CEntity *entity)
{
    // Access the actual native entity operation or field
    return entity->IsObject();
}
extern "C" bool BulletInfo_HostObjectInfiniteMass(CEntity *entity)
{
    // Access the actual native entity operation or field
    return ((CObject*)entity)->bInfiniteMass;
}
extern "C" float BulletInfo_HostObjectDamageMultiplier(CEntity *entity)
{
    // Access the actual native entity operation or field
    return ((CObject*)entity)->m_fCollisionDamageMultiplier;
}
extern "C" bool BulletInfo_HostObjectIsStatic(CEntity *entity)
{
    // Access the actual native entity operation or field
    return ((CObject*)entity)->GetIsStatic();
}
extern "C" float BulletInfo_HostObjectUprootLimit(CEntity *entity)
{
    // Access the actual native entity operation or field
    return ((CObject*)entity)->m_fUprootLimit;
}
extern "C" void BulletInfo_HostObjectClearStatic(CEntity *entity)
{
    // Access the actual native entity operation or field
    ((CObject*)entity)->bIsStatic = false;
}
extern "C" void BulletInfo_HostObjectAddMoving(CEntity *entity)
{
    // Access the actual native entity operation or field
    ((CObject*)entity)->AddToMovingList();
}
extern "C" void BulletInfo_HostObjectApplyForce(CEntity *entity,float x,float y,float z)
{
    // Access the actual native entity operation or field
    ((CObject*)entity)->ApplyMoveForce(x,y,z);
}
extern "C" int32_t BulletInfo_HostObjectDamageEffect(CEntity *entity)
{
    // Access the actual native entity operation or field
    return ((CObject*)entity)->m_nCollisionDamageEffect;
}
extern "C" int32_t BulletInfo_HostSmashEffect(void)
{
    // Access the actual native entity operation or field
    return DAMAGE_EFFECT_SMASH_COMPLETELY;
}
extern "C" void BulletInfo_HostObjectDamage(CEntity *entity,float amount)
{
    // Access the actual native entity operation or field
    ((CObject*)entity)->ObjectDamage(amount);
}
extern "C" void BulletInfo_HostClearPedAttack(CEntity *entity)
{
    // Clear the actual ped attack at the original animation boundary
    ((CPed*)entity)->ClearAttackByRemovingAnim();
}
extern "C" void *BulletInfo_HostPedClump(CEntity *entity)
{
    // Borrow the actual ped clump for the C animation manager
    return ((CPed*)entity)->GetClump();
}
extern "C" void BulletInfo_HostDamagePed(CEntity *entity,const CBulletInfo *bullet,const CColPoint *point)
{
    // Preserve the native damage call and its live argument expressions
    CPed *ped = (CPed*)entity;
    ped->InflictDamage(bullet->m_pSource,bullet->m_eWeaponType,bullet->m_nDamage,(ePedPieceTypes)point->pieceB,ped->GetLocalDirection(ped->GetPosition() - CollisionVector_ToNative(point->point)));
}
extern "C" bool BulletInfo_HostPedIsCop(CEntity *entity)
{
    // Read the actual live ped type after damage processing
    return ((CPed*)entity)->m_nPedType == PEDTYPE_COP;
}
extern "C" bool BulletInfo_HostPedDyingOrDead(CEntity *entity)
{
    // Query the actual ped state at the original decision boundary
    return ((CPed*)entity)->DyingOrDead();
}
extern "C" bool BulletInfo_HostPedInControl(CEntity *entity)
{
    // Query the actual ped control state
    return ((CPed*)entity)->IsPedInControl();
}
extern "C" bool BulletInfo_HostPedDucking(CEntity *entity)
{
    // Read the actual live ducking flag
    return ((CPed*)entity)->bIsDucking;
}
extern "C" bool BulletInfo_HostIsPed(CEntity *entity)
{
    // Query the actual source entity without changing update policy
    return entity->IsPed();
}
extern "C" bool BulletInfo_HostPedValid(CEntity *entity)
{
    // Check the actual ped pointer at the original validation boundary
    return ((CPed*)entity)->IsPointerValid();
}
extern "C" void BulletInfo_HostDebug(const char *message)
{
    // Preserve the native diagnostic output at the original call boundary
    debug("%s", message);
}
//- rouz edit (ChatGPT)




//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
