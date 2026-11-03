//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "Automobile.h"
#include "Bike.h"
#include "Camera.h"
#include "Coronas.h"
#include "DMAudio.h"
#include "Entity.h"
#include "EventList.h"
#include "Explosion.h"
#include "Explosion_backend.h"
#include "CollisionVector_native.h"
#include "General.h"
#include "Fire.h"
#include "Pad.h"
#include "Particle.h"
#include "PointLights.h"
#include "Shadows.h"
#include "Timer.h"
#include "Vehicle.h"
#include "WaterLevel.h"
#include "World.h"

// Keep actual native vector objects for calls that borrow explosion positions
static CVector gaExplosionPositions[NUM_EXPLOSIONS];

//+ rouz edit (ChatGPT)
extern "C" ExplosionMutablePosition Explosion_HostMutablePosition(uint32_t index)
{
    // Borrow writable components of the actual native position object
    ExplosionMutablePosition position={&gaExplosionPositions[index].x,&gaExplosionPositions[index].y,&gaExplosionPositions[index].z};
    return position;
}
extern "C" const CVector *Explosion_HostNativePosition(uint32_t index)
{
    // Return the actual native vector object used by world collision processing
    return &gaExplosionPositions[index];
}
extern "C" void Explosion_HostFlashCorona(uint32_t index,uint32_t offset,uint8_t red,uint8_t green,uint8_t blue,float size,float drawDistance,uint32_t textureIndex,bool star,bool reflection)
{
    // Forward the selected corona using the actual live native position and original record identity
    ExplosionRecord &explosion=gaExplosion[index];
    CCoronas::RegisterCorona((uintptr)&explosion+offset,red,green,blue,255,gaExplosionPositions[index],size,drawDistance,gpCoronaTexture[textureIndex],star ? CCoronas::TYPE_STAR : CCoronas::TYPE_NORMAL,reflection ? CCoronas::REFLECTION_ON : CCoronas::REFLECTION_OFF,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_OFF,0.0f);
}
extern "C" void Explosion_HostTriggerWorld(const CVector *position,float radius,float power,CEntity *creator,bool processBombTimer)
{
    // Pass the actual original native vector object without creating a snapshot
    CWorld::TriggerExplosion(*position,radius,power,creator,processBombTimer);
}
extern "C" void Explosion_HostCameraShake(float strength,float x,float y,float z)
{
    // Forward the original camera call with C-selected arguments
    TheCamera.CamShake(strength,x,y,z);
}
extern "C" void Explosion_HostPadShake(int16_t duration,uint8_t intensity,float x,float y,float z)
{
    // Resolve the original pad at the original controller-call boundary
    CPad::GetPad(0)->StartShake_Distance(duration,intensity,x,y,z);
}
extern "C" void Explosion_HostRegisterReference(CEntity *entity,CEntity **slot)
{
    // Register the actual native entity reference at its original assignment boundary
    entity->RegisterReference(slot);
}
extern "C" void Explosion_HostDebug(const char *message)
{
    // Preserve the native diagnostic output at the original call boundary
    debug("%s",message);
}
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern "C" CollisionPointRef Explosion_HostCameraPosition(void)
{
    // Borrow the actual native camera position components
    return COLLISION_POINT_REF(TheCamera.GetPosition());
}
extern "C" void Explosion_HostGrenadeParticle(uint32_t index,const ExplosionColor *color)
{
    // Create the native grenade particle from the C color snapshot
    RwRGBA nativeColor={color->red,color->green,color->blue,color->alpha};
    CParticle::AddParticle(PARTICLE_EXPLOSION_LFAST,gaExplosionPositions[index],CVector(0.0f,0.0f,0.0f),nil,4.5f,nativeColor);
}
extern "C" void Explosion_HostRocketParticle(uint32_t index,const ExplosionColor *nativeColor)
{
    // Create the native rocket particle with the shared color snapshot
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    CParticle::AddParticle(PARTICLE_EXPLOSION_LFAST,gaExplosionPositions[index],CVector(0.0f,0.0f,0.0f),nil,5.5f,color);
}
extern "C" bool Explosion_HostIsVehicle(CEntity *entity)
{
    // Check the native entity type before accessing vehicle behavior
    return entity->IsVehicle();
}
extern "C" bool Explosion_HostIsBoat(CEntity *entity)
{
    // Check whether the selected native vehicle is a boat
    return ((CVehicle*)entity)->IsBoat();
}
extern "C" void Explosion_HostCreationVehicleComponents(uint32_t index)
{
    // Retain the original uninitialized native vector and selected victim object
    CVector position;
    Explosion_CreateVehicleComponents(gaExplosion[index].m_pVictimEntity,&position);
}
extern "C" bool Explosion_HostVehicleHasPart(CEntity *victim,enum ExplosionVehiclePart part)
{
    // Map the C-selected car component to the original native identifier
    int component=part==EXPLOSION_PART_BUMPER ? CAR_BUMP_REAR : part==EXPLOSION_PART_WHEEL ? CAR_WHEEL_LB : CAR_BOOT;
    return ((CVehicle*)victim)->IsComponentPresent(component);
}
extern "C" void Explosion_HostVehicleComponentPosition(CEntity *victim,enum ExplosionVehiclePart part,CVector *position)
{
    // Resolve the selected bike fork or car boot into the actual native vector
    ((CVehicle*)victim)->GetComponentWorldPosition(part==EXPLOSION_PART_BIKE_FORK ? BIKE_FORKS_REAR : CAR_BOOT,*position);
}
extern "C" void Explosion_HostVehicleAveragePosition(CEntity *victim,CVector *position)
{
    // Retain the original native bumper and wheel averaging sequence
    CVector other;
    CVehicle *vehicle=(CVehicle*)victim;
    vehicle->GetComponentWorldPosition(CAR_BUMP_REAR,*position);
    vehicle->GetComponentWorldPosition(CAR_WHEEL_LB,other);
    *position+=other;
    *position/=2.0f;
}
extern "C" float Explosion_HostVectorX(const CVector *position)
{
    // Read the original native vector component
    return position->x;
}
extern "C" void Explosion_HostCreationJet(const CVector *position,float size)
{
    // Emit a native jet using the original component position object
    CParticle::AddJetExplosion(*position,size,0.5f);
}
extern "C" void Explosion_HostHeliParticle(CollisionVector3 position,const ExplosionColor *nativeColor,uint32_t kind)
{
    // Create the selected native helicopter effect at the C-calculated position
    CVector point(position.x,position.y,position.z);
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    if (kind==0)
        CParticle::AddParticle(PARTICLE_EXPLOSION_MFAST,point,CVector(0.0f,0.0f,0.0f),nil,2.5f,color);
    else if (kind==1)
        CParticle::AddParticle(PARTICLE_EXPLOSION_LFAST,point,CVector(0.0f,0.0f,0.0f),nil,5.0f,color);
    else
        CParticle::AddJetExplosion(point,1.4f,3.0f);
}
extern "C" void Explosion_HostBarrelParticle(CollisionVector3 position,const ExplosionColor *nativeColor)
{
    // Create the native barrel particle at the C-calculated position
    CVector point(position.x,position.y,position.z);
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM,point,CVector(0.0f,0.0f,0.0f),nil,0.0f,color);
}
extern "C" float Explosion_HostGroundZ(float x,float y,float z,bool *found)
{
    // Query the original native ground collision routine
    return CWorld::FindGroundZFor3DCoord(x,y,z,found);
}
extern "C" bool Explosion_HostWaterLevel(float x,float y,float z,float *level)
{
    // Query the original native water level without waves
    return CWaterLevel::GetWaterLevelNoWaves(x,y,z,level);
}
extern "C" void Explosion_HostCreationShadow(CollisionVector3 shadowPosition)
{
    // Construct the native shadow position and preserve the original shadow arguments
    CVector position(shadowPosition.x,shadowPosition.y,shadowPosition.z);
#ifdef FIX_BUGS
    CShadows::AddPermanentShadow(SHADOWTEX_CAR,gpShadowHeliTex,&position,8.0f,0.0f,0.0f,-8.0f,200,0,0,0,10.0f,30000,1.0f);
#else
    CShadows::AddPermanentShadow(SHADOWTEX_CAR,gpShadowHeliTex,&position,8.0f,0.0f,0.0f,-8.0f,200,0,0,0,10.0f,1,30000.0f);
#endif
}
//- rouz edit (ChatGPT)

bool
#ifdef SIMPLER_MISSIONS
Explosion_AddExplosion(CEntity* explodingEntity, CEntity* culprit, eExplosionType type, const CVector& pos, uint32 lifetime, bool makeSound, float radius)
#else
Explosion_AddExplosion(CEntity *explodingEntity, CEntity *culprit, eExplosionType type, const CVector &pos, uint32 lifetime, bool makeSound)
#endif
{
#ifdef SIMPLER_MISSIONS
    float explosionOverrideRadius=radius;
#else
    float explosionOverrideRadius=-1.0f;
#endif
    // Run creation in C with the actual native position
    return Explosion_AddNative(explodingEntity,culprit,type,&pos,COLLISION_POINT_REF(pos),lifetime,makeSound,explosionOverrideRadius);
}

extern "C" void Explosion_AddAtPosition(CEntity *source,enum eExplosionType type,ExplosionPosition position)
{
    // Construct the original temporary native vector for by-value C callers
    Explosion_AddExplosion(nil,source,type,CVector(position.x,position.y,position.z),0);
}
extern "C" void Explosion_HostSetPedsOnFire(float x,float y,float z,CEntity *creator)
{
    // Apply the pedestrian fire effect at the selected live position
    CWorld::SetPedsOnFire(x,y,z,6.0f,creator);
}
extern "C" void Explosion_HostSetCarsOnFire(float x,float y,float z,CEntity *creator)
{
    // Apply the vehicle fire effect at the selected live position
    CWorld::SetCarsOnFire(x,y,z,6.0f,creator);
}
extern "C" bool Explosion_HostMolotovGround(CollisionVector3 start,float *groundZ)
{
    // Perform the original native vertical collision query
    CVector point(start.x,start.y,start.z);
    CColPoint colPoint;
    CEntity *entity;
    if (!CWorld::ProcessVerticalLine(point,-1000.0f,colPoint,entity,true,false,false,false,true,false,nil))
        return false;
    *groundZ=colPoint.point.z;
    return true;
}
extern "C" void Explosion_HostMolotovParticle(CollisionVector3 position,const ExplosionColor *nativeColor)
{
    // Preserve the native particle call and its original random rotation draws
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    CVector point(position.x,position.y,position.z);
    CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM,point,CVector(0.0f,0.0f,0.0f),nil,0.0f,color,General_GetRandomNumberInRangeFloat(-3.0f,3.0f),General_GetRandomNumberInRangeFloat(-180.0f,180.0f));
}
extern "C" bool Explosion_HostVehicleIsBike(CEntity *victim)
{
    // Classify the selected native vehicle
    return ((CVehicle*)victim)->IsBike();
}
extern "C" bool Explosion_HostVehicleHasJetComponent(CEntity *victim,bool bike)
{
    // Check the selected native rear component
    return ((CVehicle*)victim)->IsComponentPresent(bike ? BIKE_FORKS_REAR : CAR_WING_LR);
}
extern "C" void Explosion_HostVehicleJetComponent(CEntity *victim,bool bike)
{
    // Resolve the selected component position and emit the native jet effect
    CVehicle *vehicle=(CVehicle*)victim;
    CVector position;
    vehicle->GetComponentWorldPosition(bike ? BIKE_FORKS_REAR : CAR_WING_LR,position);
    CParticle::AddJetExplosion(position,0.5f,0.0f);
}
extern "C" CollisionPointRef Explosion_HostVictimPosition(CEntity *victim)
{
    // Borrow the selected native victim position for the C snapshot
    return COLLISION_POINT_REF(victim->GetPosition());
}
extern "C" void Explosion_HostVehicleBurstParticle(CollisionVector3 position,const ExplosionColor *nativeColor)
{
    // Emit one native particle at the original position snapshot
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    CVector point(position.x,position.y,position.z);
    CParticle::AddParticle(PARTICLE_EXPLOSION_MEDIUM,point,CVector(0.0f,0.0f,0.0f),nil,3.5f,color);
}
extern "C" void Explosion_HostVehicleVictimSmoke(uint32_t index,const ExplosionColor *nativeColor)
{
    // Emit smoke at the live victim using the original random size
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2,gaExplosion[index].m_pVictimEntity->GetPosition(),CVector(0.0f,0.0f,0.0f),nil,General_GetRandomNumberInRangeFloat(0.5f,0.8f),color);
}
extern "C" void Explosion_HostVehicleSmoke(CollisionVector3 position,float size,const ExplosionColor *nativeColor)
{
    // Emit native smoke at the selected snapshot and size
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    CVector point(position.x,position.y,position.z);
    CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2,point,CVector(0.0f,0.0f,0.11f),nil,size,color);
}
extern "C" void Explosion_HostBombParticle(CollisionVector3 position,const ExplosionColor *nativeColor)
{
    // Preserve the native particle call and original random rotation draws
    RwRGBA color={nativeColor->red,nativeColor->green,nativeColor->blue,nativeColor->alpha};
    CVector point(position.x,position.y,position.z);
    CParticle::AddParticle(PARTICLE_EXPLOSION_LARGE,point,CVector(0.0f,0.0f,0.0f),nil,0.0f,color,General_GetRandomNumberInRangeFloat(-3.0f,3.0f),General_GetRandomNumberInRangeFloat(-180.0f,180.0f));
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
