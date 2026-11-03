#include "Explosion_access.h"
#include "ExplosionTypes.h"
#include "CollisionVector.h"
#include "config.h"
#include "Explosion_backend.h"
#include "DMAudio.h"
#include "PointLights.h"
#include "Fire.h"
#include "EventList.h"
#include <string.h>
#include <assert.h>
#include <math.h>
static uint8_t counters[NUM_EXPLOSIONS];
static enum eExplosionType types[NUM_EXPLOSIONS];
static bool sounds[NUM_EXPLOSIONS];
static unsigned counterReads, typeReads, soundReads;
uint8_t *Explosion_HostCounter(uint8_t id) { assert(id<NUM_EXPLOSIONS); ++counterReads; return &counters[id]; }
enum eExplosionType *Explosion_HostType(uint8_t id) { assert(id<NUM_EXPLOSIONS); ++typeReads; return &types[id]; }
bool *Explosion_HostMakeSound(uint8_t id) { assert(id<NUM_EXPLOSIONS); ++soundReads; return &sounds[id]; }
static uint8_t iterations[NUM_EXPLOSIONS];
static CollisionVector3 positions[NUM_EXPLOSIONS];
uint8_t *Explosion_HostIteration(uint32_t index) { assert(index<NUM_EXPLOSIONS); return &iterations[index]; }
CollisionPointRef Explosion_HostPosition(uint32_t index) { assert(index<NUM_EXPLOSIONS); return COLLISION_POINT_REF(positions[index]); }
typedef struct TestResetFields {
    float radius,power,propagation,zshift,stopTime,startTime;
    CEntity *creator,*victim;
    bool isBoat;
} TestResetFields;
static TestResetFields resetFields[NUM_EXPLOSIONS];
static int audioResult, audioTrace, resetCount;
static uint32_t expiryTimes[NUM_EXPLOSIONS];
uint32_t Timer_timeMs;
uint32_t Timer_frameCounter;
float Timer_timeStep;
static int referenceCalls;
static CEntity *expectedCreator,*expectedVictim;
static CollisionVector3 inputPosition;
extern int AudioHandle;
void Explosion_HostResetRefs(uint32_t index,ExplosionResetRefs *refs)
{
    // Supply pointers to the actual controlled record fields
    assert(index<NUM_EXPLOSIONS); ++resetCount; TestResetFields *field=&resetFields[index];
    refs->type=&types[index];refs->x=&positions[index].x;refs->y=&positions[index].y;refs->z=&positions[index].z;
    refs->radius=&field->radius;refs->power=&field->power;refs->propagation=&field->propagation;refs->zshift=&field->zshift;
    refs->creator=&field->creator;refs->victim=&field->victim;refs->stopTime=&field->stopTime;
    refs->iteration=&iterations[index];refs->activeCounter=&counters[index];refs->particlesExpireTime=&expiryTimes[index];refs->startTime=&field->startTime;refs->isBoat=&field->isBoat;refs->makeSound=&sounds[index];
}
void Explosion_HostRegisterReference(CEntity *entity,CEntity **slot)
{
    // Observe original registration order and mutate later live inputs during callbacks
    ++referenceCalls; assert(*slot==entity);
    if (referenceCalls==1) {
        assert(entity==expectedCreator);inputPosition.x=99;resetFields[0].victim=expectedCreator;
    } else {
        assert(referenceCalls==2 && entity==expectedVictim);iterations[0]=7;counters[0]=8;
    }
}
static int creationVehicleTrace;
static bool creationIsVehicle,creationIsBoat,clearVictimOnEvent;
bool Explosion_HostIsVehicle(CEntity *entity)
{
    // Check entity classification occurs before any boat lookup
    assert(entity==expectedVictim);creationVehicleTrace=creationVehicleTrace*10+1;return creationIsVehicle;
}
bool Explosion_HostIsBoat(CEntity *entity)
{
    // Check the selected entity is the current live victim
    assert(entity==expectedVictim);creationVehicleTrace=creationVehicleTrace*10+2;return creationIsBoat;
}
void Explosion_HostCreationVehicleComponents(uint32_t index)
{
    // Observe component selection after event callbacks
    assert(index==0 && resetFields[0].victim==expectedVictim && !resetFields[0].isBoat);creationVehicleTrace=creationVehicleTrace*10+4;
}
void Events_RegisterEntity(eEventType type,eEventEntity entityType,CEntity *entity,CPed *criminal,int32_t timeout)
{
    // Record the victim event and optionally remove the live victim during the callback
    assert(type==EVENT_EXPLOSION && entityType==EVENT_ENTITY_VEHICLE && entity==expectedVictim && criminal==0 && timeout==1000);
    creationVehicleTrace=creationVehicleTrace*10+3;if (clearVictimOnEvent) resetFields[0].victim=0;
}
static int heliParticles,barrelParticles;
static bool verifyCreationSnapshot;
static ExplosionColor fixtureColor;
static const ExplosionColor *creationColor;
void Explosion_HostHeliParticle(CollisionVector3 position,const ExplosionColor *color,uint32_t kind)
{
    // Check the repeating effect sequence and mutate the input before the final event
    assert(position.x==1.0f-128.0f/20.0f && position.y==2.0f-128.0f/20.0f && position.z==3.0f-128.0f/20.0f && color==creationColor && kind==(uint32_t)(heliParticles%3));
    if (++heliParticles==30) inputPosition.x=42;
}
void Explosion_HostBarrelParticle(CollisionVector3 position,const ExplosionColor *color)
{
    // Count the independently selected barrel particles
    if (verifyCreationSnapshot) {
        assert(position.x==1.0f-128.0f/50.0f && color->red==12);
    } else {
        assert(position.x==42.0f-128.0f/50.0f && color==creationColor);
    }
    assert(position.y==2.0f-128.0f/50.0f && position.z==3.0f-128.0f/25.0f);++barrelParticles;
}
CFireManager gFireManager;
static bool creationGroundFound,creationWaterFound;
static float creationGroundHeight,creationWaterHeight;
static int creationTrace;
static int unknownTypeCalls;
static int creationEventMode;
static float cameraDistance;
float Explosion_HostGroundZ(float x,float y,float z,bool *found)
{
    // Verify the snapshotted input and change the original during the query
    assert(x==1 && y==2 && z==((found || creationEventMode==1) ? 6 : 7));creationTrace=creationTrace*10+1;inputPosition.x=99;
    if (found) *found=creationGroundFound;
    return creationGroundHeight;
}
bool Explosion_HostWaterLevel(float x,float y,float z,float *level)
{
    // Observe the snapshot and the selected ground height after the query
    assert(x==1 && y==2 && z==(creationGroundFound ? creationGroundHeight : 3));creationTrace=creationTrace*10+2;
    *level=creationWaterHeight;return creationWaterFound;
}
void Events_RegisterPosition(eEventType type,EventsVector position,int32_t timeout)
{
    // Verify the event retains the snapshot from before the discarded ground query
    assert(type==EVENT_EXPLOSION && position.x==(timeout==1000 ? 42 : 1) && position.y==2 && position.z==(creationEventMode==1 ? creationGroundHeight : 3) && (timeout==250 || timeout==1000));
    creationTrace=creationTrace*10+2;
}
CollisionPointRef Explosion_HostCameraPosition(void)
{
    // Supply a live camera position at the selected distance after event registration
    static CollisionVector3 camera;
    creationTrace=creationTrace*10+3;camera=positions[0];camera.x+=cameraDistance;
    return COLLISION_POINT_REF(camera);
}
static ExplosionColor grenadeColor;
static int grenadeDraws;
int32_t General_GetRandomNumberInRangeInt(int32_t low,int32_t high)
{
    // Return the selected grenade color adjustment
    assert(low==0 && high==64);++grenadeDraws;return 0;
}
void Explosion_HostGrenadeParticle(uint32_t index,const ExplosionColor *color)
{
    // Check the C color adjustment precedes native particle creation
    assert(index==0 && color==&grenadeColor && color->green==192 && color->blue==32);creationTrace=creationTrace*10+4;
}
void Explosion_HostRocketParticle(uint32_t index,const ExplosionColor *color)
{
    // Check the rocket effect receives its color after the distance test
    assert(index==0 && color==creationColor);creationTrace=creationTrace*10+5;
}
void FireManager_StartFireAt(CFireManager *manager,FireVector position,float size,uint8_t propagation)
{
    // Verify fire creation uses the selected snapshot and original parameters
    assert(manager==&gFireManager && position.x==1 && position.y==2 && position.z==creationGroundHeight && size==1.8f && !propagation);
    creationTrace=creationTrace*10+3;
}
static int shadowCalls;
static bool preserveShadowInput;
void Explosion_HostCreationShadow(CollisionVector3 position)
{
    // Verify shadow order and change the live input before slot initialization
    assert(position.x==inputPosition.x && position.y==inputPosition.y && position.z==inputPosition.z+5);
    ++shadowCalls;if (!preserveShadowInput) inputPosition.x=42;
    if (verifyCreationSnapshot) Explosion_colorMedium.red=123;
}
static int effectTrace;
static const CVector *worldNativeExpected;
static bool timingMutation;
static bool expectedBombTimer;
static float expectedShake;
static CEntity *expectedEffectCreator;
void Explosion_HostTriggerWorld(const CVector *position,float radius,float power,CEntity *creator,bool processBombTimer)
{
    // Check native position identity and mutate the live scalar source during the callback
    assert((const void*)position==(worldNativeExpected ? (const void*)worldNativeExpected : (const void*)&inputPosition) && radius==9 && power==300);
    assert(creator==expectedEffectCreator && processBombTimer==expectedBombTimer);
    effectTrace=effectTrace*10+1;inputPosition.x=8;
    if (timingMutation) { assert(expiryTimes[0]==0); iterations[0]=9; expiryTimes[0]=77; }
}
void Explosion_HostCameraShake(float strength,float x,float y,float z)
{
    // Check the post-world live position and change it before any controller call
    assert(strength==expectedShake && x==inputPosition.x && y==inputPosition.y && z==inputPosition.z);
    effectTrace=effectTrace*10+2;inputPosition.y=9;
}
void Explosion_HostPadShake(int16_t duration,uint8_t intensity,float x,float y,float z)
{
    // Check that the controller call observes the post-camera live position
    assert(duration==300 && intensity==128 && x==inputPosition.x && y==9 && z==inputPosition.z);
    effectTrace=effectTrace*10+3;
}
const CVector *Explosion_HostNativePosition(uint32_t index)
{
    // Return the controlled native object identity for the timed world call
    assert(index<NUM_EXPLOSIONS);return (const CVector*)&positions[index];
}
static int dispatchTrace;
static const ExplosionColor *expectedColor;
static int molotovTrace,particleCount;
static bool groundHit;
void Explosion_HostSetPedsOnFire(float x,float y,float z,CEntity *creator)
{
    // Verify live input and change the later fire call and iteration decisions
    assert(x==1 && y==2 && z==3 && creator==expectedCreator);
    molotovTrace=molotovTrace*10+1;positions[0].x=4;
}
void Explosion_HostSetCarsOnFire(float x,float y,float z,CEntity *creator)
{
    // Observe changes made by the pedestrian callback
    assert(x==4 && y==2 && z==3 && creator==expectedCreator);
    molotovTrace=molotovTrace*10+2;types[1]=EXPLOSION_BOAT;
}
bool Explosion_HostMolotovGround(CollisionVector3 start,float *groundZ)
{
    // Mutate live fallback height and iteration while returning the selected collision result
    assert(start.x==4 && start.y==2 && start.z==8);molotovTrace=molotovTrace*10+3;positions[0].z=7;iterations[0]=4;*groundZ=11;
    return groundHit;
}
void Explosion_HostMolotovParticle(CollisionVector3 position,const ExplosionColor *color)
{
    // Verify the count uses the post-collision fixed spread and the shared update color
    assert(position.x==4 && position.y==4.2f && position.z==(groundHit ? 11.5f : 7.5f) && color==expectedColor);
    ++particleCount;
}
static int vehicleTrace,randomReads;
static uint32_t smokeIndex;
static uint16_t randomValue;
static const uint16_t *randomSequence;
static int burstParticles;
uint16_t General_GetRandomNumber(void)
{
    // Supply a deterministic value and record each vehicle selection draw
    int index=randomReads++;return randomSequence ? randomSequence[index] : randomValue;
}
static bool jetBike,jetComponentPresent;
static int jetTrace;
bool Explosion_HostVehicleIsBike(CEntity *victim)
{
    // Classify the same victim before checking its selected component
    assert(victim==expectedVictim);jetTrace=jetTrace*10+1;return jetBike;
}
bool Explosion_HostVehicleHasJetComponent(CEntity *victim,bool bike)
{
    // Check the selected component and preserve the original vehicle identity
    assert(victim==expectedVictim && bike==jetBike);jetTrace=jetTrace*10+2;return jetComponentPresent;
}
void Explosion_HostVehicleJetComponent(CEntity *victim,bool bike)
{
    // Emit the jet before the timed burst and mutate its threshold
    assert(victim==expectedVictim && bike==jetBike);jetTrace=jetTrace*10+3;vehicleTrace=vehicleTrace*10+1;resetFields[0].startTime=0;
}
static CollisionVector3 burstExpectedPosition;
void Explosion_HostVehicleBurstParticle(CollisionVector3 position,const ExplosionColor *color)
{
    // Check each particle receives the original victim position snapshot
    assert(position.x==burstExpectedPosition.x && position.y==burstExpectedPosition.y && position.z==burstExpectedPosition.z && color==expectedColor);++burstParticles;
}
CollisionPointRef Explosion_HostVictimPosition(CEntity *victim)
{
    // Observe the updated start time before borrowing the current victim position
    assert(victim==expectedVictim && resetFields[0].startTime==175+(randomValue & 0x7f));vehicleTrace=vehicleTrace*10+2;
    burstExpectedPosition=inputPosition;return COLLISION_POINT_REF(inputPosition);
}
void Explosion_HostVehicleVictimSmoke(uint32_t index,const ExplosionColor *color)
{
    // Record the victim smoke before the explosion smoke
    assert((index==0 || index==1) && color==expectedColor);vehicleTrace=vehicleTrace*10+3;positions[index].x=19;smokeIndex=index;
}
float General_GetRandomNumberInRangeFloat(float low,float high)
{
    // Change the live position after the smoke snapshot is taken
    assert(low==0.5f && high==2.0f);positions[smokeIndex].x=20;return 1.25f;
}
void Explosion_HostVehicleSmoke(CollisionVector3 position,float size,const ExplosionColor *color)
{
    // Check the snapshot sees victim smoke but precedes the random callback mutation
    assert(color==expectedColor && position.x==19 && position.z==positions[smokeIndex].z+1 && size==1.25f);
    assert(positions[smokeIndex].x==20);vehicleTrace=vehicleTrace*10+4;
    if (smokeIndex==1) dispatchTrace=dispatchTrace*10+2;
}
static int componentTrace,creationJets;
static bool partBumper,partWheel,partBoot;
static float componentX;
bool Explosion_HostVehicleHasPart(CEntity *victim,enum ExplosionVehiclePart part)
{
    // Observe the original short-circuit component queries
    assert(victim==expectedVictim);componentTrace=componentTrace*10+(int)part+1;
    return part==EXPLOSION_PART_BUMPER ? partBumper : part==EXPLOSION_PART_WHEEL ? partWheel : partBoot;
}
void Explosion_HostVehicleComponentPosition(CEntity *victim,enum ExplosionVehiclePart part,CVector *position)
{
    // Check the selected component uses the supplied native vector identity
    assert(victim==expectedVictim && (void*)position==(void*)&inputPosition);
    componentTrace=componentTrace*10+(part==EXPLOSION_PART_BIKE_FORK ? 4 : 5);
}
void Explosion_HostVehicleAveragePosition(CEntity *victim,CVector *position)
{
    // Check the paired component branch retains the supplied native vector
    assert(victim==expectedVictim && (void*)position==(void*)&inputPosition);componentTrace=componentTrace*10+6;
}
float Explosion_HostVectorX(const CVector *position)
{
    // Return the selected original vector component
    assert((const void*)position==(const void*)&inputPosition);return componentX;
}
void Explosion_HostCreationJet(const CVector *position,float size)
{
    // Verify each jet uses the same position object and the selected random size
    assert((const void*)position==(const void*)&inputPosition && size==1.5f);++creationJets;
}
static int bombParticles;
static bool mutateBombIteration;
static float expectedBombSpread;
void Explosion_HostBombParticle(CollisionVector3 position,const ExplosionColor *color)
{
    // Check the fixed spread and shared color while changing the live iteration
    float inverse=1.0f/sqrtf(32769.0f);
    float scale=(iterations[2]+1)*expectedBombSpread/5.0f;
    assert(color==expectedColor && position.x==(-128.0f*inverse)*scale+positions[2].x);
    assert(position.y==(-128.0f*inverse)*scale+positions[2].y && position.z==inverse*scale+positions[2].z+0.5f);
    if (bombParticles++==0) dispatchTrace=dispatchTrace*10+3;
    if (mutateBombIteration) iterations[2]=9;
}
static int flashTrace;
static bool vehicleFlash;
void PointLights_AddLight(uint8_t type,PointLightsVector position,PointLightsVector direction,float radius,float red,float green,float blue,uint8_t fog,bool shadows)
{
    // Check the copied position and mutate the live record before the first corona
    assert(type==POINTLIGHTS_LIGHT_POINT && position.x==1 && position.y==2 && position.z==3);
    assert(direction.x==0 && direction.y==0 && direction.z==0 && radius==(vehicleFlash ? 15 : 20) && red==1 && green==(vehicleFlash ? 0 : 1) && blue==(vehicleFlash ? 0 : 0.5f) && fog==POINTLIGHTS_FOG_NONE && shadows);
    flashTrace=flashTrace*10+1;positions[0].x=4;
}
void Explosion_HostFlashCorona(uint32_t index,uint32_t offset,uint8_t red,uint8_t green,uint8_t blue,float size,float drawDistance,uint32_t textureIndex,bool star,bool reflection)
{
    // Check call order and observe live position and radius changes between renderer calls
    assert(index==0);
    assert(drawDistance==(vehicleFlash ? 80 : 120));
    if (offset==0) {
        assert(positions[0].x==((Timer_frameCounter & 1) ? 4 : 1));
        if (vehicleFlash) {
            assert(red==((Timer_frameCounter & 1) ? 200 : 128) && green==((Timer_frameCounter & 1) ? 100 : 0) && blue==0);
            assert(size==((Timer_frameCounter & 1) ? 6 : 8));
        } else {
            assert(red==((Timer_frameCounter & 1) ? 255 : 128) && green==red && blue==((Timer_frameCounter & 1) ? 200 : 100) && size==8);
        }
        assert(textureIndex==0 && !star && reflection);
        flashTrace=flashTrace*10+2;positions[0].x=5;resetFields[0].radius=13;
    } else {
        assert(offset==1 && positions[0].x==5 && red==30 && green==(vehicleFlash ? 15 : 30) && blue==(vehicleFlash ? 0 : 25));
        assert(size==13 && textureIndex==7 && star && !reflection);
        flashTrace=flashTrace*10+3;
    }
}
void Explosion_HostDebug(const char *message)
{
    // Record the exact diagnostic sequence without opening a window
    if (!strcmp(message,"Undefined explosion type, AddExplosion, Explosion.cpp")) ++unknownTypeCalls;
    else if (!strcmp(message,"Initialising CExplosion...\n")) audioTrace=audioTrace*10+1;
    else if (!strcmp(message,"CExplosion ready\n")) audioTrace=audioTrace*10+4;
    else if (!strcmp(message,"Shutting down CExplosion...\n")) audioTrace=audioTrace*10+5;
    else { assert(!strcmp(message,"CExplosion shut down\n")); audioTrace=audioTrace*10+7; }
}
int32_t DMAudio_CreateEntity(enum eAudioType type,void *owner)
{
    // Verify the complete reset precedes the original audio creation call
    assert(type==AUDIOTYPE_EXPLOSION && owner==(void*)(uintptr_t)1 && resetCount==NUM_EXPLOSIONS);
    audioTrace=audioTrace*10+2; return audioResult;
}
void DMAudio_SetEntityStatus(int32_t entity,uint8_t status) { assert(entity==audioResult && status==1); audioTrace=audioTrace*10+3; }
void DMAudio_DestroyEntity(int32_t entity) { assert(entity==audioResult); audioTrace=audioTrace*10+6; }
int main(void)
{
    // Verify the original mutable and constant explosion color defaults
    assert(Explosion_colorMedium.red==0 && Explosion_colorUpdate.alpha==0);
    assert(Explosion_colorAdd.red==160 && Explosion_colorAdd.alpha==255 && Explosion_colorGrenade.blue==96 && Explosion_colorGrenade.alpha==255);
    // Copy the current live position without exposing a native vector
    positions[0]=(CollisionVector3){1,2,3};ExplosionPosition copied=Explosion_GetExplosionPosition(0);
    positions[0].x=9;assert(copied.x==1 && copied.y==2 && copied.z==3 && Explosion_GetExplosionPosition(0).x==9);
    // Observe the same live fields and original signed byte conversion
    counters[0]=255; counters[1]=7; types[0]=EXPLOSION_ROCKET; sounds[0]=true;
    assert(Explosion_GetExplosionActiveCounter(0)==-1);
    assert(Explosion_GetExplosionType(0)==EXPLOSION_ROCKET && Explosion_DoesExplosionMakeSound(0));
    // Verify reset affects only the selected record and later reads remain live
    Explosion_ResetExplosionActiveCounter(0); assert(counters[0]==0 && counters[1]==7);
    types[0]=EXPLOSION_BOAT; sounds[0]=false;
    assert(Explosion_GetExplosionType(0)==EXPLOSION_BOAT && !Explosion_DoesExplosionMakeSound(0));
    assert(counterReads==2 && typeReads==2 && soundReads==2);
    // Check active filtering, type matching, and inclusive area boundaries
    iterations[1]=1; types[1]=EXPLOSION_ROCKET; positions[1]=(CollisionVector3){1,2,3};
    assert(Explosion_TestForExplosionInArea(EXPLOSION_ROCKET,1,1,2,2,3,3));
    assert(!Explosion_TestForExplosionInArea(EXPLOSION_BOAT,1,1,2,2,3,3));
    assert(!Explosion_TestForExplosionInArea(EXPLOSION_ROCKET,1.1f,2,2,2,3,3));
    // Keep the strict radius boundary and avoid changing inactive records
    iterations[2]=1; positions[2]=(CollisionVector3){3,4,0};
    iterations[3]=1; positions[3]=(CollisionVector3){0,0,0};
    ExplosionPosition origin={0,0,0}; Explosion_RemoveAllExplosionsInArea(origin,5);
    assert(iterations[1]==0 && iterations[2]==1 && iterations[3]==0 && iterations[4]==0);
    // Keep partial initialization, negative audio handles, and every diagnostic boundary
    counters[0]=77;positions[0]=(CollisionVector3){8,9,10};iterations[0]=4;resetFields[0].radius=3;
    resetFields[0].creator=(CEntity*)&counters[0];resetFields[0].isBoat=true;sounds[0]=false;
    audioResult=-12;audioTrace=resetCount=0;Explosion_Initialise();
    assert(audioTrace==124 && AudioHandle==-12 && counters[0]==77 && resetCount==NUM_EXPLOSIONS);
    assert(positions[0].x==0 && positions[0].y==0 && positions[0].z==0 && iterations[0]==0);
    assert(resetFields[0].radius==1 && !resetFields[0].creator && !resetFields[0].isBoat && sounds[0]);
    Explosion_Shutdown();assert(audioTrace==12457 && AudioHandle==-12);
    // Create and destroy a valid audio entity with the original status ordering
    audioResult=7;audioTrace=resetCount=0;Explosion_Initialise();Explosion_Shutdown();
    assert(audioTrace==1234567 && AudioHandle==AEHANDLE_NONE);
    // Select the first free slot and preserve reference callbacks before final field writes
    for (unsigned i=0;i<NUM_EXPLOSIONS;++i) iterations[i]=1;
    iterations[3]=0;assert(Explosion_FindFreeSlot()==3);
#ifdef FIX_BUGS
    iterations[3]=1;assert(Explosion_FindFreeSlot()==-1);
#endif
    iterations[0]=0;expectedCreator=(CEntity*)&counters[1];expectedVictim=(CEntity*)&counters[2];
    inputPosition=(CollisionVector3){1,2,3};referenceCalls=0;Timer_timeMs=UINT32_MAX-4;
    Explosion_InitialiseSlot(0,EXPLOSION_ROCKET,COLLISION_POINT_REF(inputPosition),expectedCreator,expectedVictim,false,10);
    assert(referenceCalls==2 && positions[0].x==1 && inputPosition.x==99);
    assert(resetFields[0].creator==expectedCreator && resetFields[0].victim==expectedVictim);
    assert(types[0]==EXPLOSION_ROCKET && iterations[0]==1 && counters[0]==1 && !sounds[0] && expiryTimes[0]==5);
    // A zero lifetime does not read or incorporate the timer
    inputPosition=(CollisionVector3){4,5,6};Explosion_InitialiseSlot(1,EXPLOSION_BOAT,COLLISION_POINT_REF(inputPosition),0,0,true,0);
    assert(expiryTimes[1]==0 && iterations[1]==1 && counters[1]==1 && sounds[1]);
    // Verify every original type-specific scalar setup and timer offset
    static const float radii[]={9,6,10,9,9,9,6,12,10,7,10,8};
    static const float powers[]={300,0,300,300,300,300,300,500,150,150,150,50};
    static const uint32_t durations[]={750,3000,750,4250,4250,4250,750,750,750,750,750,750};
    Timer_timeMs=1000;
    for (int type=EXPLOSION_GRENADE;type<=EXPLOSION_HELI_BOMB;++type) {
        // Retain unchanged start times for all types that do not initialize them
        resetFields[0].startTime=42;Explosion_SetupType(0,(enum eExplosionType)type,10,4.0f);
#ifdef SIMPLER_MISSIONS
        assert(resetFields[0].radius==(type==EXPLOSION_GRENADE ? 4.0f : radii[type]));
#else
        assert(resetFields[0].radius==radii[type]);
#endif
        assert(resetFields[0].power==powers[type] && resetFields[0].stopTime==(float)(1010+durations[type]) && resetFields[0].propagation==0.5f);
        assert(resetFields[0].startTime==((type>=EXPLOSION_CAR && type<=EXPLOSION_HELI2) ? 1000.0f : 42.0f));
    }
#ifdef SIMPLER_MISSIONS
    // Preserve the sentinel radius fallback for simpler missions
    Explosion_SetupType(0,EXPLOSION_GRENADE,0,-1.0f);assert(resetFields[0].radius==9.0f);
#endif
    // Preserve cancellation, native position identity, and live reads across world and camera callbacks
    inputPosition=(CollisionVector3){1,2,3};iterations[0]=1;resetFields[0].power=300;resetFields[0].radius=9;expiryTimes[0]=0;
    expectedEffectCreator=(CEntity*)&counters[3];expectedBombTimer=true;expectedShake=0.6f;effectTrace=0;
    assert(!Explosion_CompleteCreation(0,EXPLOSION_ROCKET,true,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),expectedEffectCreator));
    assert(iterations[0]==0 && effectTrace==0);
    assert(Explosion_CompleteCreation(0,EXPLOSION_ROCKET,false,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),expectedEffectCreator));
    assert(effectTrace==123 && inputPosition.x==8 && inputPosition.y==9);
    // Delay suppresses the world call while Molotov retains camera-only feedback
    inputPosition=(CollisionVector3){1,2,3};expiryTimes[0]=7;expectedShake=0.2f;effectTrace=0;
    assert(Explosion_CompleteCreation(0,EXPLOSION_MOLOTOV,false,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),0));
    assert(effectTrace==2 && inputPosition.y==9);
    // A non-bomb type with zero power still performs both shake calls
    inputPosition=(CollisionVector3){1,2,3};expiryTimes[0]=0;resetFields[0].power=0;expectedShake=0.6f;effectTrace=0;
    assert(Explosion_CompleteCreation(0,EXPLOSION_GRENADE,false,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),0));
    assert(effectTrace==23);
    // Keep inactive and delayed timing separate from normal updates
    int32_t remaining=-123;iterations[0]=0;expiryTimes[0]=0;
    assert(!Explosion_UpdateTiming(0,&remaining) && remaining==-123);
    iterations[0]=1;expiryTimes[0]=100;Timer_timeMs=100;
    assert(!Explosion_UpdateTiming(0,&remaining) && expiryTimes[0]==100);
    // Retain the native position identity and callback changes after an expired delay
    Timer_timeMs=101;types[0]=EXPLOSION_ROCKET;resetFields[0].radius=9;resetFields[0].power=300;
    resetFields[0].creator=expectedEffectCreator;expectedBombTimer=true;worldNativeExpected=(const CVector*)&positions[0];timingMutation=true;effectTrace=0;
    assert(!Explosion_UpdateTiming(0,&remaining) && effectTrace==1 && iterations[0]==9 && expiryTimes[0]==77);
    // Apply normal radius and truncated remaining-time updates before later effects
    timingMutation=false;worldNativeExpected=0;expiryTimes[0]=0;Timer_timeMs=50;Timer_timeStep=2;
    resetFields[0].radius=1;resetFields[0].propagation=0.5f;resetFields[0].stopTime=75.75f;
    assert(Explosion_UpdateTiming(0,&remaining) && resetFields[0].radius==2 && remaining==25);
    iterations[0]=255;Explosion_AdvanceIteration(0,remaining);assert(iterations[0]==0);
    iterations[0]=5;Explosion_AdvanceIteration(0,0);assert(iterations[0]==0);
    // Exercise both frame parities and live changes made by earlier renderer calls
    positions[0]=(CollisionVector3){1,2,3};resetFields[0].radius=9;Timer_frameCounter=1;flashTrace=0;
    Explosion_UpdateFlash(0);assert(flashTrace==123);
    positions[0]=(CollisionVector3){1,2,3};resetFields[0].radius=9;Timer_frameCounter=2;flashTrace=0;
    Explosion_UpdateFlash(0);assert(flashTrace==23);
    // Exercise vehicle flash parity and mutations between the light and corona calls
    vehicleFlash=true;positions[0]=(CollisionVector3){1,2,3};resetFields[0].radius=9;Timer_frameCounter=1;flashTrace=0;
    Explosion_UpdateVehicleFlash(0);assert(flashTrace==123);
    positions[0]=(CollisionVector3){1,2,3};resetFields[0].radius=9;Timer_frameCounter=2;flashTrace=0;
    Explosion_UpdateVehicleFlash(0);assert(flashTrace==23);
    // Dispatch creation through C and leave fields untouched for an unknown type
    inputPosition=(CollisionVector3){1,2,3};creationColor=&fixtureColor;creationTrace=0;creationEventMode=0;cameraDistance=39;Timer_timeMs=50;
    assert(!Explosion_CreateType(0,EXPLOSION_ROCKET,0,-1,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),creationColor,creationColor,&grenadeColor));
    assert(creationTrace==235 && resetFields[0].radius==10 && resetFields[0].power==300);
    resetFields[0].radius=17;unknownTypeCalls=0;
    assert(!Explosion_CreateType(0,(enum eExplosionType)100,0,-1,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),creationColor,creationColor,&grenadeColor));
    assert(unknownTypeCalls==1 && resetFields[0].radius==17);
    // Check grenade and rocket event ordering at the camera-distance boundary
    inputPosition=(CollisionVector3){1,2,3};creationColor=&fixtureColor;creationTrace=0;creationEventMode=1;creationGroundHeight=4;cameraDistance=39;grenadeColor.green=0;grenadeColor.blue=96;grenadeDraws=0;
    Explosion_CreateGrenade(0,COLLISION_POINT_REF(inputPosition),&grenadeColor);assert(creationTrace==1234 && grenadeDraws==1);
    inputPosition=(CollisionVector3){1,2,3};creationTrace=0;cameraDistance=40;
    Explosion_CreateGrenade(0,COLLISION_POINT_REF(inputPosition),&grenadeColor);assert(creationTrace==123 && grenadeDraws==1);
    inputPosition=(CollisionVector3){1,2,3};creationTrace=0;creationEventMode=0;cameraDistance=39;
    Explosion_CreateRocket(0,COLLISION_POINT_REF(inputPosition),creationColor);assert(creationTrace==235);
    // Verify victim event, boat short circuit, and the live victim recheck
    inputPosition=(CollisionVector3){1,2,3};expectedVictim=(CEntity*)&counters[3];resetFields[0].victim=expectedVictim;
    resetFields[0].isBoat=false;creationIsVehicle=true;creationIsBoat=false;clearVictimOnEvent=false;creationVehicleTrace=0;
    Explosion_CreateVehicle(0,COLLISION_POINT_REF(inputPosition));assert(creationVehicleTrace==1234);
    resetFields[0].victim=expectedVictim;resetFields[0].isBoat=false;creationIsBoat=true;creationVehicleTrace=0;
    Explosion_CreateVehicle(0,COLLISION_POINT_REF(inputPosition));assert(creationVehicleTrace==123 && resetFields[0].isBoat);
    resetFields[0].victim=expectedVictim;resetFields[0].isBoat=false;creationIsVehicle=false;clearVictimOnEvent=true;creationVehicleTrace=0;
    Explosion_CreateVehicle(0,COLLISION_POINT_REF(inputPosition));assert(creationVehicleTrace==13 && !resetFields[0].victim);
    clearVictimOnEvent=false;
    // Verify helicopter effect order, its later live event, and barrel particle count
    inputPosition=(CollisionVector3){1,2,3};creationColor=&fixtureColor;heliParticles=0;creationTrace=0;randomValue=0;randomReads=0;
    Explosion_CreateHeliParticles(COLLISION_POINT_REF(inputPosition),creationColor);
    assert(heliParticles==30 && creationTrace==2 && randomReads==90);
    barrelParticles=0;Explosion_CreateBarrelParticles(COLLISION_POINT_REF(inputPosition),creationColor);assert(barrelParticles==6 && randomReads==108);
    // Preserve the event snapshot when the discarded ground query changes the input
    inputPosition=(CollisionVector3){1,2,3};creationTrace=0;
    Explosion_CreateGroundEvent(COLLISION_POINT_REF(inputPosition));
    assert(creationTrace==12 && inputPosition.x==99);
    // Cover water interval boundaries, missing ground, and the unchanged input snapshot
    creationGroundFound=true;creationGroundHeight=4;creationWaterFound=true;
    for (unsigned scenario=0;scenario<5;++scenario) {
        inputPosition=(CollisionVector3){1,2,3};creationTrace=0;
        creationWaterHeight=scenario==0 ? 5 : scenario==1 ? 4 : scenario==2 ? 10 : 11;
        creationGroundFound=scenario!=4;
        bool cancelled=Explosion_CreateMolotov(COLLISION_POINT_REF(inputPosition));
        assert(cancelled==(scenario==0) && creationTrace==(scenario==0 || scenario==4 ? 12 : 123));
    }
    // Run complete creation through a water-cancelled Molotov path
    memset(iterations,0,sizeof(iterations));inputPosition=(CollisionVector3){1,2,3};preserveShadowInput=true;shadowCalls=0;
    creationGroundFound=true;creationGroundHeight=4;creationWaterFound=true;creationWaterHeight=5;creationTrace=0;effectTrace=0;
    assert(!Explosion_AddNative(0,0,EXPLOSION_MOLOTOV,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),0,true,-1));
    assert(shadowCalls==1 && creationTrace==12 && iterations[0]==0 && types[0]==EXPLOSION_MOLOTOV && effectTrace==0);
    preserveShadowInput=false;
    // Keep the medium color snapshot when the shadow callback changes its source
    memset(iterations,0,sizeof(iterations));inputPosition=(CollisionVector3){1,2,3};preserveShadowInput=true;verifyCreationSnapshot=true;
    Explosion_colorMedium.red=12;randomValue=0;barrelParticles=0;creationTrace=0;expectedShake=0.6f;effectTrace=0;
    assert(Explosion_AddNative(0,0,EXPLOSION_BARREL,(const CVector*)&inputPosition,COLLISION_POINT_REF(inputPosition),1,true,-1));
    assert(Explosion_colorMedium.red==123 && barrelParticles==6 && creationTrace==12 && effectTrace==23);
    Explosion_colorMedium.red=0;verifyCreationSnapshot=false;preserveShadowInput=false;
    // Create the shadow before slot selection and copy the input after its callback
    memset(iterations,0,sizeof(iterations));inputPosition=(CollisionVector3){1,2,3};shadowCalls=0;
    int32_t created=Explosion_BeginCreation(EXPLOSION_ROCKET,COLLISION_POINT_REF(inputPosition),0,0,true,0);
    assert(created==0 && shadowCalls==1 && positions[0].x==42 && iterations[0]==1 && types[0]==EXPLOSION_ROCKET);
#ifdef FIX_BUGS
    // Preserve shadow creation even when every fixed-search slot is occupied
    memset(iterations,1,sizeof(iterations));inputPosition.x=1;shadowCalls=0;
    assert(Explosion_BeginCreation(EXPLOSION_ROCKET,COLLISION_POINT_REF(inputPosition),0,0,true,0)==-1 && shadowCalls==1);
#endif
    // Exercise both collision outcomes and the live fallback position
    for (unsigned hit=0;hit<2;++hit) {
        groundHit=hit!=0;positions[0]=(CollisionVector3){1,2,3};iterations[0]=1;resetFields[0].creator=expectedCreator;
        molotovTrace=0;particleCount=0;expectedColor=&fixtureColor;
        Explosion_UpdateMolotov(0,expectedColor);
        assert(molotovTrace==123 && particleCount==11);
    }
    // Check bomb iteration boundaries and retain the original count across callback changes
    expectedColor=&fixtureColor;mutateBombIteration=true;randomValue=0;randomSequence=0;
    for (unsigned iteration=0;iteration<=5;++iteration) {
        iterations[2]=(uint8_t)iteration;bombParticles=0;expectedBombSpread=(float)iteration*0.65f;
        Explosion_UpdateBomb(2,expectedColor);
        assert(bombParticles==(iteration==0 || iteration==5 ? 0 : iteration==1 ? 7 : iteration==2 ? 13 : iteration==3 ? 20 : 26));
    }
    mutateBombIteration=false;expectedBombSpread=0.65f;bombParticles=0;
    // Draw again for each burst loop condition and preserve the native position identity
    const uint16_t burstDraws[]={1,1,0};randomSequence=burstDraws;randomReads=0;burstParticles=0;expectedColor=&fixtureColor;
    burstExpectedPosition=inputPosition;Explosion_VehicleBurstLoop(inputPosition,expectedColor);
    assert(randomReads==3 && burstParticles==2);randomSequence=0;
    // Exercise creation component branches, short circuits, and random jet count
    expectedVictim=(CEntity*)&counters[3];randomValue=0;randomSequence=0;componentX=1;creationJets=0;jetBike=true;componentTrace=0;
    Explosion_CreateVehicleComponents(expectedVictim,(CVector*)&inputPosition);assert(componentTrace==4 && creationJets==1);
    jetBike=false;partBumper=true;partWheel=true;partBoot=false;componentTrace=0;creationJets=0;
    Explosion_CreateVehicleComponents(expectedVictim,(CVector*)&inputPosition);assert(componentTrace==126 && creationJets==1);
    partBumper=false;partBoot=true;componentTrace=0;componentX=0;creationJets=0;
    Explosion_CreateVehicleComponents(expectedVictim,(CVector*)&inputPosition);assert(componentTrace==135 && creationJets==0);
    // Select the right rear component for both vehicle kinds and skip missing parts
    expectedVictim=(CEntity*)&counters[3];jetBike=true;jetComponentPresent=true;jetTrace=0;vehicleTrace=0;
    Explosion_VehicleJet(expectedVictim);assert(jetTrace==123);
    jetBike=false;jetComponentPresent=false;jetTrace=0;Explosion_VehicleJet(expectedVictim);assert(jetTrace==12);
    jetComponentPresent=true;
    // Check vehicle timing, draw order, boat suppression, and smoke ordering
    expectedColor=&fixtureColor;Timer_timeMs=50;resetFields[0].victim=expectedVictim;resetFields[0].isBoat=false;
    positions[0]=(CollisionVector3){1,2,3};resetFields[0].startTime=999;randomValue=0;randomReads=0;vehicleTrace=0;Timer_frameCounter=2;vehicleFlash=true;flashTrace=0;
    Explosion_UpdateVehicle(0,3500,expectedColor);
    assert(vehicleTrace==12 && randomReads==4 && flashTrace==23);
    positions[0]=(CollisionVector3){1,2,3};resetFields[0].startTime=999;resetFields[0].isBoat=true;randomReads=0;vehicleTrace=0;flashTrace=0;
    Explosion_UpdateVehicle(0,3500,expectedColor);
    assert(vehicleTrace==0 && randomReads==1 && flashTrace==23);
    iterations[0]=1;vehicleTrace=0;Explosion_UpdateVehicle(0,3499,expectedColor);assert(vehicleTrace==34);
    iterations[0]=2;vehicleTrace=0;Explosion_UpdateVehicle(0,3499,expectedColor);assert(vehicleTrace==0);
    // Verify loop ordering, live type changes, time filtering, and post-effect iteration changes
    memset(iterations,0,sizeof(iterations));memset(expiryTimes,0,sizeof(expiryTimes));
    Timer_timeMs=50;Timer_timeStep=1;dispatchTrace=0;expectedColor=&fixtureColor;
    positions[0]=(CollisionVector3){1,2,3};resetFields[0].creator=expectedCreator;resetFields[1].victim=expectedVictim;molotovTrace=0;particleCount=0;vehicleTrace=0;
    for (unsigned i=0;i<4;++i) { iterations[i]=1;resetFields[i].stopTime=70;resetFields[i].propagation=0; }
    types[0]=EXPLOSION_MOLOTOV;types[1]=EXPLOSION_GRENADE;types[2]=EXPLOSION_HELI_BOMB;types[3]=EXPLOSION_MOLOTOV;
    expiryTimes[3]=100;Explosion_UpdateRecords(expectedColor);
    assert(dispatchTrace==23 && molotovTrace==123 && particleCount==11 && iterations[0]==5 && iterations[1]==2 && iterations[2]==2 && iterations[3]==1);
    // Exercise the public update with no active records using its C-owned color snapshot
    memset(iterations,0,sizeof(iterations));Explosion_Update();
    return 0;
}
