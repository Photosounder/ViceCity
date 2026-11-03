//+ rouz edit (ChatGPT)
#pragma once
#include "Explosion_access.h"
#include "CollisionVector.h"
#include "config.h"
#ifdef __cplusplus
class CEntity;
class CVector;
extern "C" {
#else
typedef struct CEntity CEntity;
typedef struct CVector CVector;
#endif
typedef struct ExplosionColor { uint8_t red,green,blue,alpha; } ExplosionColor;
extern ExplosionColor Explosion_colorMedium,Explosion_colorUpdate;
extern const ExplosionColor Explosion_colorAdd,Explosion_colorGrenade;
typedef struct ExplosionRecord {
    enum eExplosionType m_ExplosionType;
    float m_fRadius,m_fPropagationRate;
    CEntity *m_pCreatorEntity,*m_pVictimEntity;
    float m_fStopTime;
    uint8_t m_nIteration,m_nActiveCounter;
    bool m_bIsBoat,m_bMakeSound;
    float m_fStartTime;
    uint32_t m_nParticlesExpireTime;
    float m_fPower,m_fZshift;
} ExplosionRecord;
extern ExplosionRecord gaExplosion[NUM_EXPLOSIONS];
typedef struct ExplosionMutablePosition { float *x,*y,*z; } ExplosionMutablePosition;
ExplosionMutablePosition Explosion_HostMutablePosition(uint32_t index);
typedef struct ExplosionResetRefs {
    enum eExplosionType *type;
    float *x,*y,*z,*radius,*power,*propagation,*zshift;
    CEntity **creator,**victim;
    float *stopTime;
    uint8_t *iteration,*activeCounter;
    uint32_t *particlesExpireTime;
    float *startTime;
    bool *isBoat,*makeSound;
} ExplosionResetRefs;
void Explosion_HostResetRefs(uint32_t index,ExplosionResetRefs *refs);
const CVector *Explosion_HostNativePosition(uint32_t index);
bool Explosion_UpdateTiming(uint32_t index,int32_t *remainingTime);
void Explosion_AdvanceIteration(uint32_t index,int32_t remainingTime);
void Explosion_UpdateRecords(const ExplosionColor *nativeColor);
bool Explosion_AddNative(CEntity *victim,CEntity *creator,enum eExplosionType type,const CVector *nativePosition,CollisionPointRef position,uint32_t lifetime,bool makeSound,float overrideRadius);
bool Explosion_CreateType(uint32_t index,enum eExplosionType type,uint32_t lifetime,float overrideRadius,const CVector *nativePosition,CollisionPointRef position,const ExplosionColor *mediumColor,const ExplosionColor *color,ExplosionColor *grenadeColor);
bool Explosion_CreateMolotov(CollisionPointRef position);
void Explosion_CreateGroundEvent(CollisionPointRef position);
void Explosion_CreateGrenade(uint32_t index,CollisionPointRef position,ExplosionColor *color);
void Explosion_CreateRocket(uint32_t index,CollisionPointRef position,const ExplosionColor *nativeColor);
CollisionPointRef Explosion_HostCameraPosition(void);
float Explosion_CameraDistance(uint32_t index);
void Explosion_HostGrenadeParticle(uint32_t index,const ExplosionColor *color);
void Explosion_HostRocketParticle(uint32_t index,const ExplosionColor *nativeColor);
void Explosion_CreateVehicle(uint32_t index,CollisionPointRef position);
bool Explosion_HostIsVehicle(CEntity *entity);
bool Explosion_HostIsBoat(CEntity *entity);
void Explosion_HostCreationVehicleComponents(uint32_t index);
enum ExplosionVehiclePart { EXPLOSION_PART_BUMPER,EXPLOSION_PART_WHEEL,EXPLOSION_PART_BOOT,EXPLOSION_PART_BIKE_FORK };
void Explosion_CreateVehicleComponents(CEntity *victim,CVector *position);
bool Explosion_HostVehicleHasPart(CEntity *victim,enum ExplosionVehiclePart part);
void Explosion_HostVehicleComponentPosition(CEntity *victim,enum ExplosionVehiclePart part,CVector *position);
void Explosion_HostVehicleAveragePosition(CEntity *victim,CVector *position);
float Explosion_HostVectorX(const CVector *position);
void Explosion_HostCreationJet(const CVector *position,float size);
void Explosion_CreateHeliParticles(CollisionPointRef position,const ExplosionColor *nativeColor);
void Explosion_CreateBarrelParticles(CollisionPointRef position,const ExplosionColor *nativeColor);
void Explosion_HostHeliParticle(CollisionVector3 position,const ExplosionColor *nativeColor,uint32_t kind);
void Explosion_HostBarrelParticle(CollisionVector3 position,const ExplosionColor *nativeColor);
float Explosion_HostGroundZ(float x,float y,float z,bool *found);
bool Explosion_HostWaterLevel(float x,float y,float z,float *level);
void Explosion_UpdateMolotov(uint32_t index,const ExplosionColor *nativeColor);
void Explosion_HostSetPedsOnFire(float x,float y,float z,CEntity *creator);
void Explosion_HostSetCarsOnFire(float x,float y,float z,CEntity *creator);
bool Explosion_HostMolotovGround(CollisionVector3 start,float *groundZ);
void Explosion_HostMolotovParticle(CollisionVector3 position,const ExplosionColor *nativeColor);
void Explosion_UpdateVehicle(uint32_t index,int32_t remainingTime,const ExplosionColor *nativeColor);
void Explosion_VehicleJet(CEntity *victim);
bool Explosion_HostVehicleIsBike(CEntity *victim);
bool Explosion_HostVehicleHasJetComponent(CEntity *victim,bool bike);
void Explosion_HostVehicleJetComponent(CEntity *victim,bool bike);
CollisionPointRef Explosion_HostVictimPosition(CEntity *victim);
void Explosion_VehicleBurstLoop(CollisionVector3 position,const ExplosionColor *nativeColor);
void Explosion_HostVehicleBurstParticle(CollisionVector3 position,const ExplosionColor *nativeColor);
void Explosion_HostVehicleVictimSmoke(uint32_t index,const ExplosionColor *nativeColor);
void Explosion_HostVehicleSmoke(CollisionVector3 position,float size,const ExplosionColor *nativeColor);
void Explosion_UpdateBomb(uint32_t index,const ExplosionColor *nativeColor);
void Explosion_HostBombParticle(CollisionVector3 position,const ExplosionColor *nativeColor);
void Explosion_UpdateFlash(uint32_t index);
void Explosion_UpdateVehicleFlash(uint32_t index);
void Explosion_HostFlashCorona(uint32_t index,uint32_t offset,uint8_t red,uint8_t green,uint8_t blue,float size,float drawDistance,uint32_t textureIndex,bool star,bool reflection);
void Explosion_HostDebug(const char *message);
void Explosion_HostTriggerWorld(const CVector *position,float radius,float power,CEntity *creator,bool processBombTimer);
void Explosion_HostCameraShake(float strength,float x,float y,float z);
void Explosion_HostPadShake(int16_t duration,uint8_t intensity,float x,float y,float z);
bool Explosion_CompleteCreation(uint32_t index,enum eExplosionType type,bool dontExplode,const CVector *nativePosition,CollisionPointRef position,CEntity *creator);
void Explosion_HostRegisterReference(CEntity *entity,CEntity **slot);
void Explosion_SetupType(uint32_t index,enum eExplosionType type,uint32_t lifetime,float overrideRadius);
int32_t Explosion_BeginCreation(enum eExplosionType type,CollisionPointRef position,CEntity *creator,CEntity *victim,bool makeSound,uint32_t lifetime);
void Explosion_HostCreationShadow(CollisionVector3 position);
int32_t Explosion_FindFreeSlot(void);
void Explosion_InitialiseSlot(uint32_t index,enum eExplosionType type,CollisionPointRef position,CEntity *creator,CEntity *victim,bool makeSound,uint32_t lifetime);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
