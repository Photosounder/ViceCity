//+ rouz edit (ChatGPT)
#include "Explosion_access.h"
#include "Explosion_backend.h"
#include "DMAudio.h"
#include "Timer.h"
#include "PointLights.h"
#include "General.h"
#include "Fire.h"
#include "EventList.h"
#include <stddef.h>
#include <math.h>
#include "ExplosionTypes.h"
#include "config.h"
#include "CollisionVector.h"
extern uint8_t *Explosion_HostIteration(uint32_t index);
extern CollisionPointRef Explosion_HostPosition(uint32_t index);
extern uint8_t *Explosion_HostCounter(uint8_t id);
extern enum eExplosionType *Explosion_HostType(uint8_t id);
extern bool *Explosion_HostMakeSound(uint8_t id);
// Retain the original mutable and constant color values in C storage
ExplosionColor Explosion_colorMedium={0,0,0,0};
ExplosionColor Explosion_colorUpdate={0,0,0,0};
const ExplosionColor Explosion_colorAdd={160,160,160,255};
const ExplosionColor Explosion_colorGrenade={96,96,96,255};
int AudioHandle = AEHANDLE_NONE;
void Explosion_Update(void)
{
    // Snapshot the C update color once before processing all explosion records
    ExplosionColor color=Explosion_colorUpdate;
    Explosion_UpdateRecords(&color);
}
void Explosion_UpdateRecords(const ExplosionColor *nativeColor)
{
    // Process each live record in order and dispatch only after its timing update
    for (uint32_t i=0;i<NUM_EXPLOSIONS;++i) {
        int32_t remainingTime;
        if (!Explosion_UpdateTiming(i,&remainingTime))
            continue;
        switch (*Explosion_HostType((uint8_t)i)) {
        case EXPLOSION_GRENADE: case EXPLOSION_ROCKET: case EXPLOSION_HELI:
        case EXPLOSION_HELI2: case EXPLOSION_MINE: case EXPLOSION_BARREL:
            Explosion_UpdateFlash(i);
            break;
        case EXPLOSION_MOLOTOV:
            Explosion_UpdateMolotov(i,nativeColor);
            break;
        case EXPLOSION_CAR: case EXPLOSION_CAR_QUICK: case EXPLOSION_BOAT:
            Explosion_UpdateVehicle(i,remainingTime,nativeColor);
            break;
        case EXPLOSION_TANK_GRENADE: case EXPLOSION_HELI_BOMB:
            Explosion_UpdateBomb(i,nativeColor);
            break;
        default:
            break;
        }
        // Advance the live iteration after the selected effect has completed
        Explosion_AdvanceIteration(i,remainingTime);
    }
}
void Explosion_VehicleBurstLoop(CollisionVector3 position,const ExplosionColor *nativeColor)
{
    // Draw a fresh random value for every original loop condition
    for (int i=0;i<(General_GetRandomNumber() & 1)+1;++i)
        Explosion_HostVehicleBurstParticle(position,nativeColor);
}
void Explosion_VehicleJet(CEntity *victim)
{
    // Select the rear bike fork or car wing before checking component availability
    bool bike=Explosion_HostVehicleIsBike(victim);
    if (Explosion_HostVehicleHasJetComponent(victim,bike))
        Explosion_HostVehicleJetComponent(victim,bike);
}
void Explosion_UpdateVehicle(uint32_t index,int32_t remainingTime,const ExplosionColor *nativeColor)
{
    // Preserve the early vehicle effects and random draw before testing the boat flag
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    if (remainingTime>=3500) {
        if (*refs.victim != NULL) {
            if ((General_GetRandomNumber() & 0xF)==0 && !*refs.isBoat)
                Explosion_VehicleJet(*refs.victim);
            if ((*Timer_GetTimeInMilliseconds()) > *refs.startTime) {
                *refs.startTime=(*Timer_GetTimeInMilliseconds())+125+(General_GetRandomNumber() & 0x7F);
                // Snapshot the current victim position once before the burst loop
                CollisionPointRef victimPosition=Explosion_HostVictimPosition(*refs.victim);
                CollisionVector3 point={*victimPosition.x,*victimPosition.y,*victimPosition.z};
                Explosion_VehicleBurstLoop(point,nativeColor);
            }
        }
        // Register the flash after any victim effects
        Explosion_UpdateVehicleFlash(index);
    } else if (*refs.iteration & 1) {
        // Apply victim smoke before reading the live explosion position for its own smoke
        if (*refs.victim != NULL)
            Explosion_HostVehicleVictimSmoke(index,nativeColor);
        // Snapshot the live explosion position before drawing the smoke size
        CollisionVector3 point={*refs.x,*refs.y,*refs.z};
        point.z+=1.0f;
        float size=General_GetRandomNumberInRangeFloat(0.5f,2.0f);
        Explosion_HostVehicleSmoke(point,size,nativeColor);
    }
}
void Explosion_UpdateBomb(uint32_t index,const ExplosionColor *nativeColor)
{
    // Compute the original particle count once from the current iteration
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    if (*refs.iteration < 5) {
        float spread=(float)*refs.iteration * 0.65f;
        for (int i=0;i<10*spread;++i) {
            // Preserve byte narrowing and normalize the original random direction
            uint8_t x=(uint8_t)General_GetRandomNumber(),y=(uint8_t)General_GetRandomNumber(),z=(uint8_t)General_GetRandomNumber();
            CollisionVector3 point={(float)(x-128),(float)(y-128),(float)((z%128)+1)};
            float square=point.x*point.x+point.y*point.y+point.z*point.z;
            if (square>0.0f) {
                float inverse=1.0f/sqrtf(square);
                point.x*=inverse;point.y*=inverse;point.z*=inverse;
            } else {
                point.x=1.0f;
            }
            // Read the live iteration and position after the random draws
            float scale=(*refs.iteration+1)*spread/5.0f;
            point.x*=scale;point.y*=scale;point.z*=scale;
            point.x+=*refs.x;point.y+=*refs.y;point.z+=*refs.z;
            point.z+=0.5f;
            Explosion_HostBombParticle(point,nativeColor);
        }
    }
}
bool Explosion_AddNative(CEntity *victim,CEntity *creator,enum eExplosionType type,const CVector *nativePosition,CollisionPointRef position,uint32_t lifetime,bool makeSound,float overrideRadius)
{
    // Snapshot the three original colors before any shadow or slot callbacks
    ExplosionColor mediumColor=Explosion_colorMedium;
    ExplosionColor color=Explosion_colorAdd;
    ExplosionColor grenadeColor=Explosion_colorGrenade;
    // Preserve shadow creation, slot initialization, and failure before type effects
    int32_t index=Explosion_BeginCreation(type,position,creator,victim,makeSound,lifetime);
    if (index < 0)
        return false;
    // Dispatch the selected effects before the final world and camera decisions
    bool cancelled=Explosion_CreateType((uint32_t)index,type,lifetime,overrideRadius,nativePosition,position,&mediumColor,&color,&grenadeColor);
    return Explosion_CompleteCreation((uint32_t)index,type,cancelled,nativePosition,position,creator);
}
bool Explosion_CreateType(uint32_t index,enum eExplosionType type,uint32_t lifetime,float overrideRadius,const CVector *nativePosition,CollisionPointRef position,const ExplosionColor *mediumColor,const ExplosionColor *color,ExplosionColor *grenadeColor)
{
    // Apply scalar setup only for the recognized creation types
    if ((unsigned)type>(unsigned)EXPLOSION_HELI_BOMB) {
        Explosion_HostDebug("Undefined explosion type, AddExplosion, Explosion.cpp");
        return false;
    }
    Explosion_SetupType(index,type,lifetime,overrideRadius);
    // Preserve each original effect sequence and Molotov cancellation result
    switch (type) {
    case EXPLOSION_GRENADE: Explosion_CreateGrenade(index,position,grenadeColor);break;
    case EXPLOSION_MOLOTOV: return Explosion_CreateMolotov(position);
    case EXPLOSION_ROCKET: Explosion_CreateRocket(index,position,color);break;
    case EXPLOSION_CAR: case EXPLOSION_CAR_QUICK: case EXPLOSION_BOAT:
        Explosion_CreateVehicle(index,position);break;
    case EXPLOSION_HELI: case EXPLOSION_HELI2:
        Explosion_CreateHeliParticles(position,color);break;
    case EXPLOSION_BARREL:
        Explosion_CreateBarrelParticles(position,mediumColor);
        Explosion_CreateGroundEvent(position);break;
    case EXPLOSION_MINE: case EXPLOSION_TANK_GRENADE: case EXPLOSION_HELI_BOMB:
        Explosion_CreateGroundEvent(position);break;
    default: break;
    }
    return false;
}
float Explosion_CameraDistance(uint32_t index)
{
    // Subtract the live explosion position from the live camera position
    CollisionPointRef camera=Explosion_HostCameraPosition();
    CollisionPointRef position=Explosion_HostPosition(index);
    float x=*camera.x-*position.x,y=*camera.y-*position.y,z=*camera.z-*position.z;
    return sqrtf(x*x+y*y+z*z);
}
void Explosion_CreateGrenade(uint32_t index,CollisionPointRef position,ExplosionColor *color)
{
    // Register the grounded snapshot before testing the live explosion distance
    EventsVector eventPosition={*position.x,*position.y,*position.z};
    eventPosition.z=Explosion_HostGroundZ(eventPosition.x,eventPosition.y,eventPosition.z+3.0f,NULL);
    Events_RegisterPosition(EVENT_EXPLOSION,eventPosition,250);
    if (Explosion_CameraDistance(index)<40.0f) {
        // Apply the original byte-wrapped color adjustment before particle creation
        uint8_t adjustment=(uint8_t)(General_GetRandomNumberInRangeInt(0,64)-64);
        color->green=(uint8_t)(color->green+adjustment);
        color->blue=(uint8_t)(color->blue+adjustment);
        Explosion_HostGrenadeParticle(index,color);
    }
}
void Explosion_CreateRocket(uint32_t index,CollisionPointRef position,const ExplosionColor *nativeColor)
{
    // Register the input position before testing the live explosion distance
    EventsVector eventPosition={*position.x,*position.y,*position.z};
    Events_RegisterPosition(EVENT_EXPLOSION,eventPosition,250);
    if (Explosion_CameraDistance(index)<40.0f)
        Explosion_HostRocketParticle(index,nativeColor);
}
void Explosion_CreateVehicleComponents(CEntity *victim,CVector *position)
{
    // Select the original bike or car component sequence using one live native vector
    if (Explosion_HostVehicleIsBike(victim)) {
        Explosion_HostVehicleComponentPosition(victim,EXPLOSION_PART_BIKE_FORK,position);
    } else if (Explosion_HostVehicleHasPart(victim,EXPLOSION_PART_BUMPER) && Explosion_HostVehicleHasPart(victim,EXPLOSION_PART_WHEEL)) {
        Explosion_HostVehicleAveragePosition(victim,position);
    } else if (Explosion_HostVehicleHasPart(victim,EXPLOSION_PART_BOOT)) {
        Explosion_HostVehicleComponentPosition(victim,EXPLOSION_PART_BOOT,position);
    }
    // Preserve the original vector read, random count, and per-jet random sizes
    if (Explosion_HostVectorX(position)!=0.0f) {
        int count=(General_GetRandomNumber() & 1)+1;
        for (int i=0;i<count;++i)
            Explosion_HostCreationJet(position,(General_GetRandomNumber() & 7)/7.0f+1.5f);
    }
}
void Explosion_CreateVehicle(uint32_t index,CollisionPointRef position)
{
    // Register the victim event or the current input position in the original order
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    if (*refs.victim != NULL) {
        if (Explosion_HostIsVehicle(*refs.victim) && Explosion_HostIsBoat(*refs.victim))
            *refs.isBoat=true;
        Events_RegisterEntity(EVENT_EXPLOSION,EVENT_ENTITY_VEHICLE,*refs.victim,NULL,1000);
    } else {
        EventsVector eventPosition={*position.x,*position.y,*position.z};
        Events_RegisterPosition(EVENT_EXPLOSION,eventPosition,1000);
    }
    // Recheck the live victim and boat flag after event registration
    if (*refs.victim != NULL && !*refs.isBoat)
        Explosion_HostCreationVehicleComponents(index);
}
void Explosion_CreateHeliParticles(CollisionPointRef position,const ExplosionColor *nativeColor)
{
    // Draw and transform each position before emitting the three effects in order
    for (int i=0;i<10;++i) {
        for (uint32_t kind=0;kind<3;++kind) {
            CollisionVector3 point;
            point.x=General_GetRandomNumber();point.y=General_GetRandomNumber();point.z=General_GetRandomNumber();
            point.x-=128.0f;point.y-=128.0f;point.z-=128.0f;
            point.x/=20.0f;point.y/=20.0f;point.z/=20.0f;
            point.x+=*position.x;point.y+=*position.y;point.z+=*position.z;
            Explosion_HostHeliParticle(point,nativeColor,kind);
        }
    }
    // Register the later live position after all effects
    EventsVector eventPosition={*position.x,*position.y,*position.z};
    Events_RegisterPosition(EVENT_EXPLOSION,eventPosition,1000);
}
void Explosion_CreateBarrelParticles(CollisionPointRef position,const ExplosionColor *nativeColor)
{
    // Draw and transform six independent positions using the original axis scales
    for (int i=0;i<6;++i) {
        CollisionVector3 point;
        point.x=General_GetRandomNumber();point.y=General_GetRandomNumber();point.z=General_GetRandomNumber();
        point.x-=128.0f;point.y-=128.0f;point.z-=128.0f;
        point.x/=50.0f;point.y/=50.0f;point.z/=25.0f;
        point.x+=*position.x;point.y+=*position.y;point.z+=*position.z;
        Explosion_HostBarrelParticle(point,nativeColor);
    }
}
void Explosion_CreateGroundEvent(CollisionPointRef position)
{
    // Preserve the original position snapshot and discarded ground result
    EventsVector eventPosition={*position.x,*position.y,*position.z};
    Explosion_HostGroundZ(*position.x,*position.y,*position.z+4.0f,NULL);
    Events_RegisterPosition(EVENT_EXPLOSION,eventPosition,250);
}
bool Explosion_CreateMolotov(CollisionPointRef position)
{
    // Snapshot the input before the ground and water queries
    FireVector ground={*position.x,*position.y,*position.z};
    bool found;
    float height=Explosion_HostGroundZ(ground.x,ground.y,ground.z+3.0f,&found);
    if (found)
        ground.z=height;
    // Cancel only when the ground lies strictly within the original water interval
    float waterLevel;
    if (Explosion_HostWaterLevel(ground.x,ground.y,ground.z,&waterLevel) && ground.z<waterLevel && waterLevel-6.0f<ground.z)
        return true;
    if (found)
        FireManager_StartFireAt(&gFireManager,ground,1.8f,false);
    return false;
}
void Explosion_UpdateMolotov(uint32_t index,const ExplosionColor *nativeColor)
{
    // Apply fire effects in order while rereading the live position and creator
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    Explosion_HostSetPedsOnFire(*refs.x,*refs.y,*refs.z,*refs.creator);
    Explosion_HostSetCarsOnFire(*refs.x,*refs.y,*refs.z,*refs.creator);
    if (*refs.iteration < 10) {
        if (*refs.iteration == 1) {
            // Snapshot the vertical line start before querying native collision
            CollisionVector3 start={*refs.x,*refs.y,*refs.z};
            start.z+=5.0f;
            float groundZ;
            // Use the collision result or the live position after the collision callback
            if (Explosion_HostMolotovGround(start,&groundZ))
                *refs.zshift=groundZ;
            else
                *refs.zshift=*refs.z;
        }
        // Keep the original fixed spread and particle count for this update
        float spread=(float)*refs.iteration * 0.55f;
        for (int i=0;i<5*spread;++i) {
            float angle=General_GetRandomNumber()/256.0f*6.28f;
            CollisionVector3 point={*refs.x,*refs.y,*refs.z};
            point.x+=spread*sinf(angle);
            point.y+=spread*cosf(angle);
            point.z=*refs.zshift+0.5f;
            Explosion_HostMolotovParticle(point,nativeColor);
        }
    }
}
void Explosion_UpdateFlash(uint32_t index)
{
    // Preserve the alternating light and first corona before registering the radius-dependent star
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    if ((*Timer_GetFrameCounter()) & 1) {
        PointLightsVector position={*refs.x,*refs.y,*refs.z};
        PointLightsVector zero={0.0f,0.0f,0.0f};
        PointLights_AddLight(POINTLIGHTS_LIGHT_POINT,position,zero,20.0f,1.0f,1.0f,0.5f,POINTLIGHTS_FOG_NONE,true);
        Explosion_HostFlashCorona(index,0,255,255,200,8.0f,120.0f,0,false,true);
    } else {
        Explosion_HostFlashCorona(index,0,128,128,100,8.0f,120.0f,0,false,true);
    }
    // Read the live radius after the preceding rendering calls
    Explosion_HostFlashCorona(index,1,30,30,25,*refs.radius,120.0f,7,true,false);
}
void Explosion_UpdateVehicleFlash(uint32_t index)
{
    // Preserve the vehicle flash colors, sizes, and optional light for each frame parity
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    if ((*Timer_GetFrameCounter()) & 1) {
        PointLightsVector position={*refs.x,*refs.y,*refs.z};
        PointLightsVector zero={0.0f,0.0f,0.0f};
        PointLights_AddLight(POINTLIGHTS_LIGHT_POINT,position,zero,15.0f,1.0f,0.0f,0.0f,POINTLIGHTS_FOG_NONE,true);
        Explosion_HostFlashCorona(index,0,200,100,0,6.0f,80.0f,0,false,true);
    } else {
        Explosion_HostFlashCorona(index,0,128,0,0,8.0f,80.0f,0,false,true);
    }
    // Read the live radius after the preceding rendering calls
    Explosion_HostFlashCorona(index,1,30,15,0,*refs.radius,80.0f,7,true,false);
}
bool Explosion_UpdateTiming(uint32_t index,int32_t *remainingTime)
{
    // Preserve the inactive and delayed branches before normal radius updates
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    if (*refs.iteration==0)
        return false;
    if (*refs.particlesExpireTime!=0) {
        if ((*Timer_GetTimeInMilliseconds()) > *refs.particlesExpireTime) {
            // Clear the timer before the world call and retain the actual native position object
            *refs.particlesExpireTime=0;
            if (*refs.power!=0.0f) {
                bool processBombTimer = *refs.type==EXPLOSION_ROCKET || *refs.type==EXPLOSION_CAR_QUICK || *refs.type==EXPLOSION_MINE || *refs.type==EXPLOSION_BARREL || *refs.type==EXPLOSION_TANK_GRENADE || *refs.type==EXPLOSION_HELI_BOMB;
                Explosion_HostTriggerWorld(Explosion_HostNativePosition(index),*refs.radius,*refs.power,*refs.creator,processBombTimer);
            }
        }
        return false;
    }
    // Update radius and calculate the original truncated remaining time
    *refs.radius += *refs.propagation * (*Timer_GetTimeStep());
    *remainingTime = (int32_t)(*refs.stopTime - (*Timer_GetTimeInMilliseconds()));
    return true;
}
void Explosion_AdvanceIteration(uint32_t index,int32_t remainingTime)
{
    // Read the live iteration after effects and preserve the original byte increment or reset
    uint8_t *iteration=Explosion_HostIteration(index);
    if (remainingTime > 0)
        ++*iteration;
    else
        *iteration=0;
}

bool Explosion_CompleteCreation(uint32_t index,enum eExplosionType type,bool dontExplode,const CVector *nativePosition,CollisionPointRef position,CEntity *creator)
{
    // Retain the original cancellation before world and camera effects
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    if (dontExplode) {
        *refs.iteration=0;
        return false;
    }
    // Keep the original power and delayed-particle guards and native position identity
    if (*refs.power != 0.0f && *refs.particlesExpireTime == 0) {
        bool processBombTimer = type==EXPLOSION_ROCKET || type==EXPLOSION_CAR_QUICK || type==EXPLOSION_MINE || type==EXPLOSION_BARREL || type==EXPLOSION_TANK_GRENADE || type==EXPLOSION_HELI_BOMB;
        Explosion_HostTriggerWorld(nativePosition,*refs.radius,*refs.power,creator,processBombTimer);
    }
    // Read the live position again for each original camera and controller call
    if (type==EXPLOSION_MOLOTOV) {
        Explosion_HostCameraShake(0.2f,*position.x,*position.y,*position.z);
    } else {
        Explosion_HostCameraShake(0.6f,*position.x,*position.y,*position.z);
        Explosion_HostPadShake(300,128,*position.x,*position.y,*position.z);
    }
    return true;
}

void Explosion_SetupType(uint32_t index,enum eExplosionType type,uint32_t lifetime,float overrideRadius)
{
    // Select the original scalar values while retaining the optional grenade radius
    ExplosionResetRefs refs;
    float radius,power;uint32_t duration;bool setStart=false;
    Explosion_HostResetRefs(index,&refs);
    (void)overrideRadius;
    switch (type) {
    case EXPLOSION_GRENADE:
#ifdef SIMPLER_MISSIONS
        radius=overrideRadius == -1.0f ? 9.0f : overrideRadius;
#else
        radius=9.0f;
#endif
        power=300.0f;duration=750;break;
    case EXPLOSION_MOLOTOV: radius=6.0f;power=0.0f;duration=3000;break;
    case EXPLOSION_ROCKET: radius=10.0f;power=300.0f;duration=750;break;
    case EXPLOSION_CAR: case EXPLOSION_CAR_QUICK: case EXPLOSION_BOAT:
        radius=9.0f;power=300.0f;duration=4250;setStart=true;break;
    case EXPLOSION_HELI: radius=6.0f;power=300.0f;duration=750;setStart=true;break;
    case EXPLOSION_HELI2: radius=12.0f;power=500.0f;duration=750;setStart=true;break;
    case EXPLOSION_MINE: case EXPLOSION_TANK_GRENADE: radius=10.0f;power=150.0f;duration=750;break;
    case EXPLOSION_BARREL: radius=7.0f;power=150.0f;duration=750;break;
    case EXPLOSION_HELI_BOMB: radius=8.0f;power=50.0f;duration=750;break;
    default: return;
    }
    // Preserve each original field write and both live timer reads in order
    *refs.radius=radius;
    *refs.power=power;
    *refs.stopTime=lifetime+(*Timer_GetTimeInMilliseconds())+duration;
    *refs.propagation=0.5f;
    if (setStart)
        *refs.startTime=(*Timer_GetTimeInMilliseconds());
}

int32_t Explosion_BeginCreation(enum eExplosionType type,CollisionPointRef position,CEntity *creator,CEntity *victim,bool makeSound,uint32_t lifetime)
{
    // Create the original shadow before selecting and initializing a live slot
    CollisionVector3 shadowPosition={*position.x,*position.y,*position.z};
    shadowPosition.z+=5.0f;
    Explosion_HostCreationShadow(shadowPosition);
    int32_t index=Explosion_FindFreeSlot();
    if (index < 0)
        return -1;
    Explosion_InitialiseSlot((uint32_t)index,type,position,creator,victim,makeSound,lifetime);
    return index;
}
int32_t Explosion_FindFreeSlot(void)
{
    // Preserve the fixed and legacy evaluation order of the original slot search
    int32_t index=0;
#ifdef FIX_BUGS
    while (index < NUM_EXPLOSIONS && *Explosion_HostIteration((uint32_t)index) != 0)
#else
    while (*Explosion_HostIteration((uint32_t)index) != 0 && index < NUM_EXPLOSIONS)
#endif
        ++index;
    return index == NUM_EXPLOSIONS ? -1 : index;
}
void Explosion_InitialiseSlot(uint32_t index,enum eExplosionType type,CollisionPointRef position,CEntity *creator,CEntity *victim,bool makeSound,uint32_t lifetime)
{
    // Preserve assignment and native entity-reference registration order on the live record
    ExplosionResetRefs refs;
    Explosion_HostResetRefs(index,&refs);
    *refs.type=type;
    *refs.x=*position.x;*refs.y=*position.y;*refs.z=*position.z;
    *refs.radius=1.0f;*refs.zshift=0.0f;
    *refs.creator=creator;
    if (creator != NULL)
        Explosion_HostRegisterReference(creator,refs.creator);
    *refs.victim=victim;
    if (victim != NULL)
        Explosion_HostRegisterReference(victim,refs.victim);
    *refs.iteration=1;*refs.activeCounter=1;*refs.isBoat=false;*refs.makeSound=makeSound;
    *refs.particlesExpireTime = lifetime != 0 ? (*Timer_GetTimeInMilliseconds()) + lifetime : 0;
}
void Explosion_ClearAllExplosions(void)
{
    // Reset only the original fields in the original per-record order
    for (uint32_t i=0;i<NUM_EXPLOSIONS;++i) {
        ExplosionResetRefs refs;
        Explosion_HostResetRefs(i,&refs);
        *refs.type=EXPLOSION_GRENADE;
        *refs.x=0.0f;*refs.y=0.0f;*refs.z=0.0f;
        *refs.radius=1.0f;*refs.propagation=0.0f;*refs.zshift=0.0f;
        *refs.creator=NULL;*refs.victim=NULL;*refs.stopTime=0.0f;
        *refs.iteration=0;*refs.startTime=0.0f;*refs.isBoat=false;*refs.makeSound=true;
    }
}
void Explosion_Initialise(void)
{
    // Preserve reset, audio creation, conditional status, and diagnostic order
    Explosion_HostDebug("Initialising CExplosion...\n");
    Explosion_ClearAllExplosions();
    AudioHandle=DMAudio_CreateEntity(AUDIOTYPE_EXPLOSION,(void*)(uintptr_t)1);
    if (AudioHandle>=0)
        DMAudio_SetEntityStatus(AudioHandle,true);
    Explosion_HostDebug("CExplosion ready\n");
}
void Explosion_Shutdown(void)
{
    // Preserve the original negative-handle branch and diagnostic order
    Explosion_HostDebug("Shutting down CExplosion...\n");
    if (AudioHandle>=0) {
        DMAudio_DestroyEntity(AudioHandle);
        AudioHandle=AEHANDLE_NONE;
    }
    Explosion_HostDebug("CExplosion shut down\n");
}
ExplosionPosition Explosion_GetExplosionPosition(uint8_t id)
{
    // Copy the current components of the live position for the audio consumer
    CollisionPointRef position=Explosion_HostPosition(id);
    ExplosionPosition result={*position.x,*position.y,*position.z};
    return result;
}
int8_t Explosion_GetExplosionActiveCounter(uint8_t id)
{
    // Return the original byte counter through the actual live storage
    return *Explosion_HostCounter(id);
}
void Explosion_ResetExplosionActiveCounter(uint8_t id)
{
    // Reset the same live counter without changing other explosion fields
    *Explosion_HostCounter(id) = 0;
}
uint8_t Explosion_GetExplosionType(uint8_t id)
{
    // Preserve the original narrowing conversion from the stored enumeration
    return *Explosion_HostType(id);
}
bool Explosion_DoesExplosionMakeSound(uint8_t id)
{
    // Read the original live sound flag
    return *Explosion_HostMakeSound(id);
}
bool Explosion_TestForExplosionInArea(enum eExplosionType type,float x1,float x2,float y1,float y2,float z1,float z2)
{
    // Preserve iteration order, short-circuit comparisons, and live component reads
    for (uint32_t i=0;i<NUM_EXPLOSIONS;++i) {
        if (*Explosion_HostIteration(i) != 0 && type == *Explosion_HostType((uint8_t)i)) {
            CollisionPointRef position=Explosion_HostPosition(i);
            if (*position.x >= x1 && *position.x <= x2 && *position.y >= y1 && *position.y <= y2 && *position.z >= z1 && *position.z <= z2)
                return true;
        }
    }
    return false;
}
void Explosion_RemoveAllExplosionsInArea(ExplosionPosition position,float radius)
{
    // Preserve the original strict squared-distance comparison and live iteration writes
    for (uint32_t i=0;i<NUM_EXPLOSIONS;++i) {
        uint8_t *iteration=Explosion_HostIteration(i);
        if (*iteration != 0) {
            CollisionPointRef target=Explosion_HostPosition(i);
            float x=position.x-*target.x,y=position.y-*target.y,z=position.z-*target.z;
            if (x*x+y*y+z*z < radius*radius)
                *iteration=0;
        }
    }
}
//- rouz edit (ChatGPT)
