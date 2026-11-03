#include "AnimViewer.h"
#include "config.h"
#include "ModelIds.h"
#include "ModelInfoType.h"
#include "StreamingInfo.h"
#include "VehicleTypes.h"
#include "RpAnimBlend.h"
#include "AnimManager.h"
#include "Pool.h"
#include "AnimationId.h"
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static int selectedModel,modelQueries,modelConversions;
static int updateStep;
static bool fixtureUpdate,fixtureSearch;
static int expectedDirection,previousModel,modelMatches;
static bool allowAllModelIds;
void *ModelRegistry_Get(int modelId)
{
    // Distinguish update lookups from navigation and conversion searches
    if (fixtureUpdate) { assert(updateStep++==0 && modelId==0);return (void *)(uintptr_t)6; }
    if (fixtureSearch) {
        // Record the model search order and return a candidate for type checking
        if (modelMatches) assert(modelId==previousModel+expectedDirection);
        previousModel=modelId;++modelMatches;return (void *)(uintptr_t)1;
    }
    assert(allowAllModelIds || (modelId!=5 && modelId!=6 && modelId!=405));++modelQueries;
    return modelId==selectedModel ? (void *)(uintptr_t)1 : 0;
}
void AnimViewer_HostConvertModelAnimation(int modelId)
{
    // Check that only the selected populated slot is converted
    assert(modelId==selectedModel);++modelConversions;
}
void re3_assert(const char *expression,const char *file,unsigned int line,const char *function)
{
    // Fail the fixture if a valid model search reaches the asserted boundary
    (void)expression;(void)file;(void)line;(void)function;assert(false);
}
static void *player,*shutdownOriginalSlot,*shutdownOtherSlot;
static bool shutdownMode;
static int shutdownTrace;
static bool reloadMode;
static int reloadTrace;
void *AnimViewer_HostPlayer(void) { return player; }
void AnimViewer_HostDestroyPlayer(void *value)
{
    // Replace the live player pointer during destruction
    assert(value==shutdownOriginalSlot);shutdownTrace=shutdownTrace*10+1;player=shutdownOtherSlot;
}
void AnimViewer_HostShutdownWorld(void) { shutdownTrace=shutdownTrace*10+3;AnimViewer_animTxdSlot=77; }
void AnimViewer_HostShutdownModels(void) { shutdownTrace=shutdownTrace*10+4; }
void AnimManager_Shutdown(void) { if (reloadMode) { assert(!AnimViewer_target);reloadTrace=reloadTrace*10+1; } else shutdownTrace=shutdownTrace*10+5; }
void Timer_Shutdown(void) { shutdownTrace=shutdownTrace*10+6; }
void AnimViewer_HostShutdownStreaming(void) { shutdownTrace=shutdownTrace*10+7; }
void TxdStore_RemoveTxdSlot(int slot)
{
    // Verify the final shutdown reads the live texture slot
    assert(slot==77);shutdownTrace=shutdownTrace*10+8;
}
static int initStep;
void AnimViewer_HostInitPools(void) { assert(initStep++==0); }
void References_Init(void) { assert(initStep++==1); }
void AnimViewer_HostInitWorld(void) { assert(initStep++==2); }
void AnimManager_Initialise(void) { if (reloadMode) reloadTrace=reloadTrace*10+2; else assert(initStep++==3); }
void AnimViewer_HostInitModelSystems(void) { assert(initStep++==4); }
void PedStats_Initialise(void) { assert(initStep++==5); }
void Messages_Init(void) { assert(initStep++==6); }
void AnimViewer_HostLoadLevels(void) { assert(initStep++==7); }
void AnimViewer_HostInitRenderer(void) { assert(initStep++==8); }
void AnimManager_LoadAnimFiles(void) { if (reloadMode) reloadTrace=reloadTrace*10+3; else assert(initStep++==9); }
void AnimViewer_HostInitPlayerSystems(void) { assert(initStep++==10); }
void Timer_Initialise(void) { assert(initStep++==11); }
void Clock_Initialise(uint32_t scale) { assert(initStep++==12 && scale==60000); }
void TimeCycle_Initialise(void) { assert(initStep++==13); }
static void *constructedPlayer;
void AnimViewer_HostInitCarControl(void) { assert(initStep++==14); }
void AnimViewer_HostConstructPlayer(void *playerSlot) { assert(playerSlot);constructedPlayer=playerSlot; }
void AnimViewer_HostPositionPlayer(void *playerSlot,float x,float y,float z) { assert(playerSlot==constructedPlayer && x==1000 && y==1000 && z==1000); }
void AnimViewer_HostAssignPlayer(void *playerSlot) { assert(playerSlot==constructedPlayer); }
void Draw_SetFOV(float value) { assert(initStep++==15 && value==120.0f); }
void AnimViewer_HostSetLodDistance(void) { assert(initStep++==16); }
static int textureTrace,textureSlot;
int TxdStore_AddTxdSlot(const char *name)
{
    // Observe dictionary creation order and return distinct slots
    if (!strcmp(name,"hud")) { textureTrace=textureTrace*10+1;return 10; }
    if (!strcmp(name,"generic")) { textureTrace=textureTrace*10+4;return 20; }
    assert(!strcmp(name,"particle"));textureTrace=textureTrace*10+6;return 30;
}
bool TxdStore_LoadFile(int slot,const char *name)
{
    // Observe the original file and slot pairings
    if (slot==10) { assert(!strcmp(name,"MODELS/HUD.TXD"));textureTrace=textureTrace*10+2; }
    else { assert(slot==30 && !strcmp(name,"MODELS/PARTICLE.TXD"));textureTrace=textureTrace*10+7; }
    return false;
}
void AnimViewer_HostHideHud(void)
{
    // Check that HUD visibility changes after the HUD dictionary load
    textureTrace=textureTrace*10+3;
}
void TxdStore_Create(int slot)
{
    // Mutate the live current slot before it is read again
    assert(slot==20);textureTrace=textureTrace*10+5;AnimViewer_animTxdSlot=99;
}
void TxdStore_SetCurrentTxd(int slot)
{
    // Observe the live slot after all preceding callbacks
    textureTrace=textureTrace*10+8;textureSlot=slot;
}
static CAnimBlendAssociation *existingAssociation;
static CAnimBlendAssociation blendedAssociation;
static int playbackTrace;
CAnimBlendAssociation *RpAnimBlendClumpGetAssociation(void *clump,uint32_t id)
{
    // Retrieve the existing association before any blend changes
    assert(clump==(void *)(uintptr_t)1 && id==9);playbackTrace=playbackTrace*10+1;return existingAssociation;
}
bool AnimAssociation_IsPartial(CAnimBlendAssociation *association)
{
    // Identify the selected existing association
    assert(association==existingAssociation);playbackTrace=playbackTrace*10+2;return true;
}
void AnimAssociation_Destroy(CAnimBlendAssociation *association)
{
    // Record destruction before the C operation frees the association
    assert(association==existingAssociation);playbackTrace=playbackTrace*10+3;
}
void RpAnimBlendClumpSetBlendDeltas(void *clump,uint32_t mask,float delta)
{
    // Check the original partial fade parameters
    assert(clump==(void *)(uintptr_t)1 && mask==ASSOC_PARTIAL && delta==-8.0f);playbackTrace=playbackTrace*10+4;
}
CAnimBlendAssociation *AnimManager_BlendAnimation(void *clump,int32_t group,int32_t anim,float delta)
{
    // Supply the selected association after checking the blend request
    assert(clump==(void *)(uintptr_t)1 && group==3 && anim==9 && delta==8.0f);playbackTrace=playbackTrace*10+5;return &blendedAssociation;
}
void AnimAssociation_SetCurrentTime(CAnimBlendAssociation *association,float time)
{
    // Confirm the delete-on-fade flag is set before rewinding
    assert(association==&blendedAssociation && (association->flags & ASSOC_DELETEFADEDOUT) && time==0);playbackTrace=playbackTrace*10+6;
}
void AnimAssociation_SetRun(CAnimBlendAssociation *association)
{
    // Confirm running starts after the rewind
    assert(association==&blendedAssociation);playbackTrace=playbackTrace*10+7;
}
static int animationStage,animationLookups,animationRequests,animationRefs;
int32_t AnimManager_GetAnimationBlockIndex(const char *name)
{
    // Check the five block names are resolved before the flush
    const char *names[]={"van","bikes","bikev","bikeh","biked"};
    assert(animationStage==0 && animationLookups<5 && !strcmp(name,names[animationLookups]));return 10+animationLookups++;
}
void AnimViewer_HostFlushRequests(void) { assert(animationLookups==5);animationStage=1; }
void AnimViewer_HostRequestAnimation(int block,int flags)
{
    // Check request order and the dependency flag
    assert(animationStage==1 && block==10+animationRequests && flags==STREAMFLAGS_DEPENDENCY);++animationRequests;
}
void AnimViewer_HostLoadRequests(bool priority) { assert(animationStage==1 && animationRequests==5 && !priority);animationStage=2; }
void AnimManager_AddAnimBlockRef(int32_t block)
{
    // Check references are added only after loading completes
    assert(animationStage==2 && block==10+animationRefs);++animationRefs;
}
static char sharedLine[256];
static int fileEnabled,fileReads,fileClosed,specialRequests,specialGroups;
char *AnimViewer_HostStringBuffer(void) { return sharedLine; }
int FileMgr_OpenFile(const char *file,const char *mode)
{
    // Verify the original path and optionally simulate a missing file
    assert(!strcmp(file,"DATA\\SPECIAL.TXT") && !strcmp(mode,"r"));return fileEnabled ? 3 : 0;
}
bool FileMgr_ReadLine(int fd,char *buffer,int length)
{
    // Supply one known and one unknown animation group before end of file
    assert(fd==3 && buffer==sharedLine && length==255);
    if (fileReads==0) strcpy(buffer,"joe walk");
    else if (fileReads==1) strcpy(buffer,"amy unknown");
    else return false;
    ++fileReads;return true;
}
int FileMgr_CloseFile(int fd) { assert(fd==3);++fileClosed;return 0; }
const char *AnimManager_GetAnimGroupName(int32_t groupId) { return groupId==2 ? "walk" : "other"; }
void AnimViewer_HostSetSpecialGroup(int modelId,int groupId)
{
    // Check that only the known group is assigned
    assert(modelId==MI_SPECIAL01 && groupId==2);++specialGroups;
}
void AnimViewer_HostRequestSpecial(int lineId,const char *name,int flags)
{
    // Check both requests and the original streaming flag
    assert(lineId==specialRequests && !strcmp(name,lineId==0 ? "joe" : "amy") && flags==STREAMFLAGS_DONT_REMOVE);++specialRequests;
}
static int worldTrace;
static bool createTargetInWorld;
void TimeCycle_Update(void) { worldTrace=worldTrace*10+1; }
void AnimViewer_HostProcessWorld(void)
{
    // Set the live target during world processing when requested
    worldTrace=worldTrace*10+2;AnimViewer_target=createTargetInWorld ? (CEntity *)(uintptr_t)1 : 0;
}
void AnimViewer_HostProcessCamera(void) { worldTrace=worldTrace*10+3; }
static int targetTrace,targetKind;
static bool updateLoaded;
static int createCase=-1,createTrace;
void AnimViewer_HostRemoveTarget(CEntity *target)
{
    // Change the live target while removing it from the world
    assert((uintptr_t)target==1);targetTrace=targetTrace*10+1;AnimViewer_target=(CEntity *)(uintptr_t)2;
}
bool AnimViewer_HostTargetMatches(CEntity *target,int kind)
{
    // Test each category in order and change the target on a match
    if (updateLoaded) {
        assert((uintptr_t)target==8 && updateStep++==(kind==0 ? 11 : 12));return kind==1;
    }
    assert((uintptr_t)target==2);targetTrace=targetTrace*10+2;
    if (kind==targetKind) { AnimViewer_target=(CEntity *)(uintptr_t)3;return true; }
    return false;
}
void AnimViewer_HostDestroyTarget(CEntity *target)
{
    // Observe the selected live target and change it before pool release
    assert((uintptr_t)target==3);targetTrace=targetTrace*10+3;AnimViewer_target=(CEntity *)(uintptr_t)4;
}
static CPool *targetReleasePool;
static void *targetReleaseSlots[4];
static int targetReleaseIndex;
static bool releaseMode;
void *AnimViewer_HostTargetSlot(CEntity *target,int kind)
{
    // Convert the post-destruction target to the next controlled pool slot
    assert((uintptr_t)target==4 && kind==targetKind);targetTrace=targetTrace*10+4;
    return targetReleaseSlots[targetReleaseIndex++];
}
static int pressedButton,buttonQueries,messageCount;
static int expectedButtons[8];
static const char *expectedMessage="IFP reloaded";
static int colourReloads;
static CAnimBlendAssocGroup inputGroups[4];
bool AnimViewer_HostButton(void *pad,int button)
{
    // Check button queries remain in priority order
    assert(pad==(void *)(uintptr_t)9 && button==expectedButtons[buttonQueries++]);return button==pressedButton;
}
void *AnimViewer_HostTargetClump(CEntity *target) { (void)target;return (void *)(uintptr_t)1; }
void AnimViewer_HostMessage(const char *message,int duration)
{
    // Verify the reload message for the first-priority test
    assert(!strcmp(message,expectedMessage) && duration==1000);++messageCount;
}
void AnimViewer_HostAnimateCollision(void) { assert(false); }
void AnimViewer_HostReloadVehicleColours(void) { ++colourReloads; }
CAnimBlendAssocGroup *AnimManager_GetAnimAssocGroups(void) { return inputGroups; }
int ModelRegistry_Type(void *model)
{
    // Preserve separate model-type reads in each viewer control path
    if (fixtureSearch) {
        assert(model==(void *)(uintptr_t)1);
        return previousModel==selectedModel ? (expectedDirection<0 ? MITYPE_PED : MITYPE_VEHICLE) : MITYPE_SIMPLE;
    }
    assert(model==(void *)(uintptr_t)6);
    if (createCase>=0) {
        ++createTrace;
        return createCase==0 ? MITYPE_VEHICLE : createCase==1 ? MITYPE_PED : MITYPE_SIMPLE;
    }
    assert(updateStep==1 || (updateLoaded && (updateStep==7 || updateStep==8 || updateStep==15)));
    ++updateStep;return MITYPE_PED;
}
int AnimViewer_HostPedGroup(void *model) { assert(updateStep++==2 && model==(void *)(uintptr_t)6);return 3; }
void *AnimViewer_HostUpdatePads(void) { assert(updateStep++==3);return (void *)(uintptr_t)9; }
void AnimViewer_HostUpdateStreaming(void) { assert(updateStep++==4); }
void AnimViewer_HostRequestModel(int id) { assert(updateStep++==5 && id==0); }
bool AnimViewer_HostModelLoaded(int id) { assert(updateStep++==6 && id==0);return updateLoaded; }
static int vehicleSubtype,subtypeReads,constructedSubtype,abandoned;
static CPool *vehicleObjectPool,*viewerVehiclePool,*viewerPedPool;
static bool modelHasCollision;
int AnimViewer_HostVehicleType(void *model) { assert(model==(void *)(uintptr_t)6);++subtypeReads;return vehicleSubtype; }
CPool *CPools_GetVehiclePool(void) { return releaseMode ? targetReleasePool : viewerVehiclePool; }
CEntity *AnimViewer_HostVehicleEntity(void *slot) { return (CEntity*)slot; }
static void CheckVehicleConstruction(CEntity *target,int modelId,int type) {
    // Check the selected vehicle slot and record the constructor
    assert(target==AnimViewer_target && target && modelId==AnimViewer_state.modelId);constructedSubtype=type+1;if (createCase==0) createTrace+=10;
}
void AnimViewer_HostConstructCar(CEntity *target,int modelId) {
    // Record the car constructor selection
    CheckVehicleConstruction(target,modelId,VEHICLE_TYPE_CAR);
}
void AnimViewer_HostConstructBoat(CEntity *target,int modelId) {
    // Record the boat constructor selection
    CheckVehicleConstruction(target,modelId,VEHICLE_TYPE_BOAT);
}
void AnimViewer_HostConstructBike(CEntity *target,int modelId) {
    // Record the bike constructor selection
    CheckVehicleConstruction(target,modelId,VEHICLE_TYPE_BIKE);
}
CPool *CPools_GetObjectPool(void) { return releaseMode ? targetReleasePool : vehicleObjectPool; }
void *ModelRegistry_Collision(void *model) {
    // Return the configured collision pointer for the viewer model
    assert(model==(void *)(uintptr_t)6);return modelHasCollision ? (void *)(uintptr_t)1 : 0;
}
void AnimViewer_HostSetFallbackCollision(void *model) { assert(model==(void *)(uintptr_t)6);modelHasCollision=true; }
void AnimViewer_HostSetAbandoned(CEntity *target) { assert(target==AnimViewer_target && target);++abandoned; }
CPool *CPools_GetPedPool(void) { if (releaseMode) return targetReleasePool;if (shutdownMode) shutdownTrace=shutdownTrace*10+2;return viewerPedPool; }
CEntity *AnimViewer_HostPedEntity(void *slot) { assert(slot);return (CEntity *)(uintptr_t)8; }
void AnimViewer_HostConstructPed(CEntity *target) { assert((uintptr_t)target==8);if (createCase>=0) { assert(createCase==1);createTrace+=10;return; } assert(updateLoaded && updateStep++==9); }
void AnimViewer_HostSetTargetModel(CEntity *target,int modelId) { assert((uintptr_t)target==8 && modelId==AnimViewer_state.modelId); }
static bool placementMode;
static int placementTrace;
void AnimViewer_HostSetTargetPosition(CEntity *target,float x,float y,float z)
{
    // Observe the selected target and optionally replace it after positioning
    assert(target==AnimViewer_target && x==0 && y==0 && z==0);
    if (placementMode) { assert((uintptr_t)target==1);placementTrace=1;AnimViewer_target=(CEntity *)(uintptr_t)2; }
}
void AnimViewer_HostAddTarget(CEntity *target)
{
    // Observe the current target after the position callback
    assert(target==AnimViewer_target);
    if (placementMode) { assert((uintptr_t)target==2);placementTrace=12;AnimViewer_target=(CEntity *)(uintptr_t)3; }
}
void AnimViewer_HostCameraTakeTarget(CEntity *target)
{
    // Observe the current target after the world callback
    assert(target==AnimViewer_target);
    if (placementMode) { assert((uintptr_t)target==3);placementTrace=123; }
    else if (createCase>=0) createTrace+=100;
    else assert(updateLoaded && updateStep++==10);
}
void AnimViewer_HostClearTargetVelocity(CEntity *target) { assert(updateLoaded && updateStep++==13 && (uintptr_t)target==8); }
void AnimViewer_HostSetTargetHeight(CEntity *target,float height) { assert(updateLoaded && updateStep++==14 && (uintptr_t)target==8 && height==10); }
void AnimViewer_HostKeepPedInPlace(void) { assert(updateLoaded && updateStep++==16); }
static void *temporaryObject;
static int reclaimed,constructed;
bool AnimViewer_HostIsTemporaryObject(void *object) { return object==temporaryObject; }
void AnimViewer_HostDestroyTemporaryObject(void *object)
{
    // Observe destruction before the pool slot is reused
    assert(object==temporaryObject);++reclaimed;
}
CEntity *AnimViewer_HostObjectEntity(void *object) { return (CEntity*)object; }
void AnimViewer_HostConstructObject(CEntity *target,int modelId)
{
    // Verify construction receives the selected slot and live model identifier
    assert(target==AnimViewer_target && target && modelId==AnimViewer_state.modelId);++constructed;if (createCase==2) createTrace+=10;
}
static int trace;
static CEntity *entity(unsigned id) { return (CEntity *)(uintptr_t)id; }
bool AnimViewer_HostIsPed(CEntity *target)
{
    // Change the live target after classification
    assert(target==entity(1));trace=trace*10+1;AnimViewer_target=entity(2);return true;
}
void AnimViewer_HostUpdateAnimation(CEntity *target)
{
    // Observe the changed target and replace it before rendering
    assert(target==entity(2));trace=trace*10+2;AnimViewer_target=entity(3);
}
void AnimViewer_HostRender(CEntity *target)
{
    // Observe the post-animation target and change it for the renderer
    assert(target==entity(3));trace=trace*10+3;AnimViewer_target=entity(4);
}
void AnimViewer_HostRenderNonRoad(CEntity *target)
{
    // Observe the target after the entity render callback
    assert(target==entity(4));trace=trace*10+4;
}
int main(void)
{
    // Reclaim the first temporary object through the real C pool API
    CPool testPool;assert(CPool_Init(&testPool,2,sizeof(int),"viewer test"));
    constructed=0;AnimViewer_AllocateObject(&testPool);void *first=AnimViewer_target;
    temporaryObject=CPool_New(&testPool);assert(first && temporaryObject && constructed==1);
    int oldHandle=CPool_GetIndex(&testPool,temporaryObject);reclaimed=0;AnimViewer_target=0;
    AnimViewer_AllocateObject(&testPool);
    assert(constructed==2 && reclaimed==1 && (void*)AnimViewer_target==temporaryObject && CPool_GetIndex(&testPool,temporaryObject)==oldHandle);
    CPool_Flush(&testPool);AnimViewer_target=0;
    // Check subtype selection, repeated type reads, and fallback collision
    CPool vehiclePool,pedPool;assert(CPool_Init(&vehiclePool,4,sizeof(int),"viewer vehicles"));viewerVehiclePool=&vehiclePool;
    assert(CPool_Init(&pedPool,2,sizeof(int),"viewer pedestrians"));viewerPedPool=&pedPool;
    const int types[]={VEHICLE_TYPE_CAR,VEHICLE_TYPE_BOAT,VEHICLE_TYPE_BIKE};
    for (int i=0;i<3;++i) {
        vehicleSubtype=types[i];subtypeReads=0;constructedSubtype=0;abandoned=0;
        AnimViewer_CreateVehicleTarget((void *)(uintptr_t)6);
        assert(subtypeReads==i+1 && constructedSubtype==types[i]+1 && abandoned==1);
    }
    CPool fallbackPool;assert(CPool_Init(&fallbackPool,2,sizeof(int),"viewer fallback"));vehicleObjectPool=&fallbackPool;
    vehicleSubtype=VEHICLE_TYPE_HELI;subtypeReads=0;modelHasCollision=false;constructed=0;abandoned=0;
    AnimViewer_CreateVehicleTarget((void *)(uintptr_t)6);
    assert(subtypeReads==3 && constructed==1 && modelHasCollision && abandoned==1);
    vehicleSubtype=VEHICLE_TYPE_CAR;
    // Recheck the live target between each placement operation
    placementMode=true;placementTrace=0;AnimViewer_target=(CEntity *)(uintptr_t)1;
    AnimViewer_PlaceTarget();assert(placementTrace==123 && (uintptr_t)AnimViewer_target==3);placementMode=false;
    // Select vehicle, pedestrian, and object constructors before placement
    for (createCase=0;createCase<3;++createCase) {
        createTrace=0;AnimViewer_CreateTarget((void *)(uintptr_t)6);
        assert(createTrace==(createCase==0 ? 111 : 112));
    }
    createCase=-1;CPool_Flush(&vehiclePool);CPool_Flush(&fallbackPool);
    // Exercise the update path when the requested model is not loaded
    updateStep=0;AnimViewer_state.modelId=0;AnimViewer_state.reloadIFP=false;pressedButton=-1;
    expectedButtons[0]=ANIMVIEWER_LEFT;expectedButtons[1]=ANIMVIEWER_RIGHT;buttonQueries=0;createTargetInWorld=false;worldTrace=0;
    fixtureUpdate=true;AnimViewer_Update();fixtureUpdate=false;assert(updateStep==7 && buttonQueries==2 && worldTrace==12);
    // Exercise creation and pedestrian input when the model is loaded
    updateLoaded=true;updateStep=0;AnimViewer_state.modelId=0;AnimViewer_state.reloadIFP=false;AnimViewer_state.animId=0;AnimViewer_target=0;
    pressedButton=ANIMVIEWER_SQUARE;expectedButtons[0]=ANIMVIEWER_SQUARE;expectedButtons[1]=ANIMVIEWER_LEFT;expectedButtons[2]=ANIMVIEWER_RIGHT;buttonQueries=0;expectedMessage="IFP reloaded";inputGroups[0].numAssociations=10;worldTrace=0;
    fixtureUpdate=true;AnimViewer_Update();fixtureUpdate=false;assert(updateStep==17 && buttonQueries==3 && worldTrace==12 && AnimViewer_state.reloadIFP);
    updateLoaded=false;AnimViewer_state.reloadIFP=false;CPool_Flush(&pedPool);
    // Check that camera processing uses the target after the world callback
    createTargetInWorld=true;worldTrace=0;AnimViewer_target=0;AnimViewer_UpdateWorld();assert(worldTrace==123);
    createTargetInWorld=false;worldTrace=0;AnimViewer_UpdateWorld();assert(worldTrace==12);
    // Prepare occupied slots for the three cleanup categories and reload
    CPool releasePool;assert(CPool_Init(&releasePool,4,sizeof(int),"viewer cleanup"));targetReleasePool=&releasePool;
    for (int i=0;i<4;++i) { targetReleaseSlots[i]=CPool_New(&releasePool);assert(targetReleaseSlots[i]); }
    targetReleaseIndex=0;releaseMode=true;
    // Check each cleanup category and the live target rereads across callbacks
    for (targetKind=0;targetKind<3;++targetKind) {
        targetTrace=0;AnimViewer_target=(CEntity *)(uintptr_t)1;AnimViewer_RemoveTarget();
        assert(!AnimViewer_target && targetTrace==(targetKind==0 ? 1234 : targetKind==1 ? 12234 : 122234));
    }
    // Verify cleanup completes before animation manager reload
    reloadMode=true;reloadTrace=0;targetKind=0;targetTrace=0;AnimViewer_target=(CEntity *)(uintptr_t)1;
    AnimViewer_ReloadAnimations();assert(targetTrace==1234 && reloadTrace==123 && !AnimViewer_target && targetReleaseIndex==4);reloadMode=false;releaseMode=false;CPool_Flush(&releasePool);
    // Check partial-association destruction and the complete playback sequence
    existingAssociation=malloc(sizeof(*existingAssociation));assert(existingAssociation);playbackTrace=0;blendedAssociation.flags=0;
    AnimViewer_PlayAnimation((void *)(uintptr_t)1,3,9);assert(playbackTrace==1234567);
    // Check the original persistent update-state defaults
    assert(AnimViewer_state.modelId==0 && AnimViewer_state.animId==0 && !AnimViewer_state.reloadIFP);
    // Verify the first pressed button wins and the live association count clamps afterward
    int nextModel=4;pressedButton=0;expectedButtons[0]=0;buttonQueries=0;messageCount=0;AnimViewer_state.reloadIFP=false;AnimViewer_state.animId=5;inputGroups[0].numAssociations=5;
    AnimViewer_HandlePedInput((void *)(uintptr_t)9,0,&nextModel);
    assert(buttonQueries==1 && messageCount==1 && AnimViewer_state.reloadIFP && AnimViewer_state.animId==0 && nextModel==4);
    AnimViewer_state.reloadIFP=false;
    // Check vehicle input priority and model navigation through the C handlers
    pressedButton=ANIMVIEWER_SQUARE;expectedButtons[0]=ANIMVIEWER_SHOULDER;expectedButtons[1]=ANIMVIEWER_SQUARE;buttonQueries=0;colourReloads=0;expectedMessage="Carcols.dat reloaded";
    AnimViewer_HandleVehicleInput((void *)(uintptr_t)9,&nextModel);assert(buttonQueries==2 && colourReloads==1);
    AnimViewer_state.modelId=4;selectedModel=7;pressedButton=ANIMVIEWER_RIGHT;expectedButtons[0]=ANIMVIEWER_LEFT;expectedButtons[1]=ANIMVIEWER_RIGHT;buttonQueries=0;expectedMessage="Current model ID: 7";
    AnimViewer_HandleModelNavigation((void *)(uintptr_t)9,&nextModel);assert(nextModel==7 && buttonQueries==2);
    AnimViewer_target=0;assert(AnimViewer_ApplyModelChange(nextModel) && AnimViewer_state.modelId==7);assert(!AnimViewer_ApplyModelChange(7));
    AnimViewer_state.modelId=0;expectedMessage="IFP reloaded";
    // Verify both animation wraps and the association-count reset
    AnimViewer_state.animId=0;AnimViewer_PreviousAnimation();assert(AnimViewer_state.animId==ANIM_STD_NUM-1);
    AnimViewer_NextAnimation();assert(AnimViewer_state.animId==0);
    AnimViewer_state.animId=ANIM_STD_NUM;AnimViewer_NextAnimation();assert(AnimViewer_state.animId==ANIM_STD_NUM+1);
    AnimViewer_ClampAnimation(ANIM_STD_NUM);assert(AnimViewer_state.animId==0);
    // Check the complete initialization sequence with controlled subsystem calls
    CPool initPedPool;assert(CPool_Init(&initPedPool,1,sizeof(int),"viewer player"));viewerPedPool=&initPedPool;
    initStep=0;textureTrace=0;allowAllModelIds=true;selectedModel=42;modelQueries=0;modelConversions=0;
    animationStage=0;animationLookups=0;animationRequests=0;animationRefs=0;fileEnabled=0;
    AnimViewer_Initialise();assert(initStep==17 && modelQueries==MODELINFOSIZE && modelConversions==1 && animationRefs==5);CPool_Flush(&initPedPool);
    allowAllModelIds=false;animationStage=0;animationLookups=0;animationRequests=0;animationRefs=0;
    // Verify block lookups, requests, load, and references stay ordered
    AnimViewer_LoadVehicleAnimations();assert(animationLookups==5 && animationRequests==5 && animationRefs==5);
    // Check missing-file behavior and known versus unknown group handling
    fileEnabled=0;AnimViewer_LoadSpecialCharacters();assert(!fileReads && !fileClosed);
    fileEnabled=1;AnimViewer_LoadSpecialCharacters();assert(fileReads==2 && fileClosed==1 && specialGroups==1 && specialRequests==2);
    // Verify shutdown order and the player pointer reread after destruction
    CPool shutdownPool;assert(CPool_Init(&shutdownPool,2,sizeof(int),"viewer shutdown"));viewerPedPool=&shutdownPool;
    shutdownOriginalSlot=CPool_New(&shutdownPool);shutdownOtherSlot=CPool_New(&shutdownPool);assert(shutdownOriginalSlot && shutdownOtherSlot);
    player=shutdownOriginalSlot;shutdownMode=true;AnimViewer_Shutdown();shutdownMode=false;
    assert(shutdownTrace==12345678 && CPool_GetIsFree(&shutdownPool,1) && !CPool_GetIsFree(&shutdownPool,0));CPool_Flush(&shutdownPool);
    // Verify texture setup order and the later live slot read
    textureTrace=0;AnimViewer_InitTextures();assert(textureTrace==12345678 && textureSlot==99);
    // Check skipped IDs, boundary reversal, and the original fallback
    selectedModel=7;modelQueries=0;assert(AnimViewer_NextModelId(4,1)==7 && modelQueries==1);
    selectedModel=MODELINFOSIZE-2;modelQueries=0;assert(AnimViewer_NextModelId(MODELINFOSIZE-1,1)==MODELINFOSIZE-2 && modelQueries==2);
    selectedModel=-1;modelQueries=0;assert(AnimViewer_NextModelId(0,-1)==0 && modelQueries>0);
    // Visit every slot and convert only populated models
    allowAllModelIds=true;selectedModel=42;modelQueries=0;modelConversions=0;
    AnimViewer_ConvertModelAnimations();assert(modelQueries==MODELINFOSIZE && modelConversions==1);allowAllModelIds=false;
    // Check pedestrian and vehicle search directions and type selection
    fixtureSearch=true;expectedDirection=-1;selectedModel=8;modelMatches=0;assert(AnimViewer_LastPedModelId(10)==8 && modelMatches==3);
    expectedDirection=1;selectedModel=12;modelMatches=0;assert(AnimViewer_FirstCarModelId(10)==12 && modelMatches==3);fixtureSearch=false;
    // Check the empty target and the live target sequence
    AnimViewer_target=0;AnimViewer_Render();assert(trace==0);
    AnimViewer_target=entity(1);AnimViewer_Render();assert(trace==1234);
    return 0;
}
