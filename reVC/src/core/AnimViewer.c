//+ rouz edit (ChatGPT)
#include "SharedText.h"
#include "AnimViewer.h"
#include "ModelRegistry.h"
#include "ModelInfoType.h"
#include "config.h"
#include "TxdStore.h"
#include "AnimManager.h"
#include "RpAnimBlend.h"
#include "Timer.h"
#include "FileMgr.h"
#include "ModelIds.h"
#include "StreamingInfo.h"
#include "VehicleTypes.h"
#include "References.h"
#include "PedType.h"
#include "Messages.h"
#include "Clock.h"
#include "Timecycle.h"
#include "Draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef MASTER
extern void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#define ANIMVIEWER_ASSERT(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define ANIMVIEWER_ASSERT(expression) ((void)(expression))
#endif
#undef assert
#define assert(expression) ANIMVIEWER_ASSERT(expression)
#include "Pool.h"
#include "Pools.h"
extern bool AnimViewer_HostIsTemporaryObject(void *object);
extern void AnimViewer_HostDestroyTemporaryObject(void *object);
extern CEntity *AnimViewer_HostObjectEntity(void *object);
extern void AnimViewer_HostConstructObject(CEntity *entity,int modelId);
extern bool AnimViewer_HostButton(void *pad,int button);
extern void *AnimViewer_HostTargetClump(CEntity *target);
extern void AnimViewer_HostMessage(const char *message,int duration);
extern void AnimViewer_HostAnimateCollision(void);
extern void AnimViewer_HostReloadVehicleColours(void);
extern void AnimViewer_HostHideHud(void);
extern int AnimViewer_HostPedGroup(void *model);
extern void *AnimViewer_HostUpdatePads(void);
extern void AnimViewer_HostUpdateStreaming(void);
extern void AnimViewer_HostRequestModel(int modelId);
extern bool AnimViewer_HostModelLoaded(int modelId);
extern int AnimViewer_HostVehicleType(void *model);
extern CEntity *AnimViewer_HostVehicleEntity(void *slot);
extern void AnimViewer_HostConstructCar(CEntity *target,int modelId);
extern void AnimViewer_HostConstructBoat(CEntity *target,int modelId);
extern void AnimViewer_HostConstructBike(CEntity *target,int modelId);
extern void AnimViewer_HostSetFallbackCollision(void *model);
extern void AnimViewer_HostSetAbandoned(CEntity *target);
extern CEntity *AnimViewer_HostPedEntity(void *slot);
extern void AnimViewer_HostConstructPed(CEntity *target);
extern void AnimViewer_HostSetTargetModel(CEntity *target,int modelId);
extern void AnimViewer_HostSetTargetPosition(CEntity *target,float x,float y,float z);
extern void AnimViewer_HostAddTarget(CEntity *target);
extern void AnimViewer_HostCameraTakeTarget(CEntity *target);
extern void AnimViewer_HostClearTargetVelocity(CEntity *entity);
extern void AnimViewer_HostSetTargetHeight(CEntity *entity,float height);
extern void AnimViewer_HostKeepPedInPlace(void);
extern void AnimViewer_HostProcessWorld(void);
extern void AnimViewer_HostProcessCamera(void);
extern void AnimViewer_HostRemoveTarget(CEntity *entity);
extern bool AnimViewer_HostTargetMatches(CEntity *entity,int kind);
extern void AnimViewer_HostDestroyTarget(CEntity *entity);
extern void *AnimViewer_HostTargetSlot(CEntity *entity,int kind);
extern void AnimViewer_HostInitPools(void);
extern void AnimViewer_HostInitWorld(void);
extern void AnimViewer_HostInitModelSystems(void);
extern void AnimViewer_HostLoadLevels(void);
extern void AnimViewer_HostInitRenderer(void);
extern void AnimViewer_HostInitPlayerSystems(void);
extern void AnimViewer_HostInitCarControl(void);
extern void AnimViewer_HostConstructPlayer(void *player);
extern void AnimViewer_HostPositionPlayer(void *player,float x,float y,float z);
extern void AnimViewer_HostAssignPlayer(void *player);
extern void AnimViewer_HostSetLodDistance(void);
extern void AnimViewer_HostFlushRequests(void);
extern void AnimViewer_HostRequestAnimation(int block,int flags);
extern void AnimViewer_HostLoadRequests(bool priority);
extern void AnimViewer_HostSetSpecialGroup(int modelId,int groupId);
extern void AnimViewer_HostRequestSpecial(int lineId,const char *name,int flags);
extern void *AnimViewer_HostPlayer(void);
extern void AnimViewer_HostDestroyPlayer(void *player);
extern void AnimViewer_HostShutdownWorld(void);
extern void AnimViewer_HostShutdownModels(void);
extern void AnimViewer_HostShutdownStreaming(void);
extern void AnimViewer_HostConvertModelAnimation(int modelId);
extern bool AnimViewer_HostIsPed(CEntity *entity);
extern void AnimViewer_HostUpdateAnimation(CEntity *entity);
extern void AnimViewer_HostRender(CEntity *entity);
extern void AnimViewer_HostRenderNonRoad(CEntity *entity);

AnimViewerState AnimViewer_state={0,0,false};
int AnimViewer_animTxdSlot=0;
CEntity *AnimViewer_target=0;

void AnimViewer_Update(void)
{
    // Retain the original model snapshot and the shadowed pedestrian group read
    int animGroup=ASSOCGRP_STD;
    int nextModelId=AnimViewer_state.modelId;
    void *model=ModelRegistry_Get(AnimViewer_state.modelId);
    if ((ModelRegistry_Type(model)==MITYPE_PED)) {
        int pedGroup=AnimViewer_HostPedGroup(model);
        if (AnimViewer_state.animId>ANIM_STD_IDLE)
            pedGroup=ASSOCGRP_STD;
        (void)pedGroup;
        if (AnimViewer_state.reloadIFP) {
            AnimViewer_ReloadAnimations();
            AnimViewer_state.reloadIFP=false;
        }
    } else {
        animGroup=ASSOCGRP_STD;
    }
    // Update input and streaming before handling the loaded model
    void *pad=AnimViewer_HostUpdatePads();
    AnimViewer_HostUpdateStreaming();
    AnimViewer_HostRequestModel(AnimViewer_state.modelId);
    if (AnimViewer_HostModelLoaded(AnimViewer_state.modelId)) {
        if (!AnimViewer_target)
            AnimViewer_CreateTarget(model);
        AnimViewer_PrepareTarget();
        if ((ModelRegistry_Type(model)==MITYPE_PED)) {
            AnimViewer_HostKeepPedInPlace();
            AnimViewer_HandlePedInput(pad,animGroup,&nextModelId);
        } else if ((ModelRegistry_Type(model)==MITYPE_VEHICLE)) {
            AnimViewer_HandleVehicleInput(pad,&nextModelId);
        }
    }
    // Apply model changes before deciding whether to process the world
    AnimViewer_HandleModelNavigation(pad,&nextModelId);
    if (AnimViewer_ApplyModelChange(nextModelId))
        return;
    AnimViewer_UpdateWorld();
}
void AnimViewer_AllocateObject(void *objectPool)
{
    // Allocate a slot and preserve the fixed-build temporary-object fallback
    CPool *pool=(CPool*)objectPool;
    AnimViewer_target=AnimViewer_HostObjectEntity(CPool_New(pool));
#ifdef FIX_BUGS
    if (!AnimViewer_target)
        AnimViewer_ReclaimObject(pool);
    if (AnimViewer_target)
#endif
        AnimViewer_HostConstructObject(AnimViewer_target,AnimViewer_state.modelId);
    ANIMVIEWER_ASSERT(AnimViewer_target);
}
void AnimViewer_ReclaimObject(void *objectPool)
{
    // Scan occupied slots in order and reuse the first temporary object's handle
    CPool *pool=(CPool*)objectPool;
    for (int32_t i=0;i<CPool_GetSize(pool);++i) {
        void *existing=CPool_GetSlot(pool,i);
        if (existing && AnimViewer_HostIsTemporaryObject(existing)) {
            int32_t handle=CPool_GetIndex(pool,existing);
            AnimViewer_HostDestroyTemporaryObject(existing);
            CPool_Delete(pool,existing);
            AnimViewer_target=AnimViewer_HostObjectEntity(CPool_NewAt(pool,handle));
            break;
        }
    }
}
void AnimViewer_AllocateVehicle(int type)
{
    // Allocate and validate the vehicle slot before invoking the selected constructor
    CPool *pool=CPools_GetVehiclePool();
    AnimViewer_target=AnimViewer_HostVehicleEntity(CPool_New(pool));
    ANIMVIEWER_ASSERT(AnimViewer_target);
    // Dispatch to the native constructor selected by the C vehicle policy
    if (type==VEHICLE_TYPE_CAR)
        AnimViewer_HostConstructCar(AnimViewer_target,AnimViewer_state.modelId);
    else if (type==VEHICLE_TYPE_BOAT)
        AnimViewer_HostConstructBoat(AnimViewer_target,AnimViewer_state.modelId);
    else
        AnimViewer_HostConstructBike(AnimViewer_target,AnimViewer_state.modelId);
}
void AnimViewer_CreateObjectTarget(void *model)
{
    // Allocate the object and add the original collision fallback when needed
    AnimViewer_AllocateObject(CPools_GetObjectPool());
    if (!ModelRegistry_Collision(model))
        AnimViewer_HostSetFallbackCollision(model);
    AnimViewer_HostSetAbandoned(AnimViewer_target);
}
void AnimViewer_CreatePlayer(void)
{
    // Initialize vehicle control and allocate the viewer player from the pedestrian pool
    AnimViewer_HostInitCarControl();
    CPool *pool=CPools_GetPedPool();
    void *player=CPool_New(pool);
    ANIMVIEWER_ASSERT(player);
    // Construct, position, and assign the same native player object
    AnimViewer_HostConstructPlayer(player);
    AnimViewer_HostPositionPlayer(player,1000.0f,1000.0f,1000.0f);
    AnimViewer_HostAssignPlayer(player);
}
void AnimViewer_CreatePedTarget(void)
{
    // Allocate and validate the pedestrian slot before native construction
    CPool *pool=CPools_GetPedPool();
    AnimViewer_target=AnimViewer_HostPedEntity(CPool_New(pool));
    ANIMVIEWER_ASSERT(AnimViewer_target);
    AnimViewer_HostConstructPed(AnimViewer_target);
    // Reread the live target and model identifier after construction
    AnimViewer_HostSetTargetModel(AnimViewer_target,AnimViewer_state.modelId);
}
void AnimViewer_CreateVehicleTarget(void *model)
{
    // Preserve the original repeated vehicle-type reads and constructor selection
    if (AnimViewer_HostVehicleType(model)==VEHICLE_TYPE_CAR)
        AnimViewer_AllocateVehicle(VEHICLE_TYPE_CAR);
    else if (AnimViewer_HostVehicleType(model)==VEHICLE_TYPE_BOAT)
        AnimViewer_AllocateVehicle(VEHICLE_TYPE_BOAT);
    else if (AnimViewer_HostVehicleType(model)==VEHICLE_TYPE_BIKE)
        AnimViewer_AllocateVehicle(VEHICLE_TYPE_BIKE);
    else {
        // Construct an object and supply collision only when the model lacks it
        AnimViewer_AllocateObject(CPools_GetObjectPool());
        if (!ModelRegistry_Collision(model))
            AnimViewer_HostSetFallbackCollision(model);
    }
    AnimViewer_HostSetAbandoned(AnimViewer_target);
}
void AnimViewer_PlaceTarget(void)
{
    // Position, add, and attach the live target in the original order
    AnimViewer_HostSetTargetPosition(AnimViewer_target,0.0f,0.0f,0.0f);
    AnimViewer_HostAddTarget(AnimViewer_target);
    AnimViewer_HostCameraTakeTarget(AnimViewer_target);
}
void AnimViewer_CreateTarget(void *model)
{
    // Select the original native constructor path using the model type
    if ((ModelRegistry_Type(model)==MITYPE_VEHICLE))
        AnimViewer_CreateVehicleTarget(model);
    else if ((ModelRegistry_Type(model)==MITYPE_PED))
        AnimViewer_CreatePedTarget();
    else
        AnimViewer_CreateObjectTarget(model);
    // Place the resulting live target in the world and camera
    AnimViewer_PlaceTarget();
}
void AnimViewer_PrepareTarget(void)
{
    // Clear velocity only for the original physical target categories
    if (AnimViewer_HostTargetMatches(AnimViewer_target,0) || AnimViewer_HostTargetMatches(AnimViewer_target,1) || AnimViewer_HostTargetMatches(AnimViewer_target,2))
        AnimViewer_HostClearTargetVelocity(AnimViewer_target);
    // Preserve the fixed and legacy target heights
#ifdef FIX_BUGS
    AnimViewer_HostSetTargetHeight(AnimViewer_target,10.0f);
#else
    AnimViewer_HostSetTargetHeight(AnimViewer_target,0.0f);
#endif
}
void AnimViewer_UpdateWorld(void)
{
    // Update time and world state before checking the live viewer target
    TimeCycle_Update();
    AnimViewer_HostProcessWorld();
    if (AnimViewer_target)
        AnimViewer_HostProcessCamera();
}
void AnimViewer_Initialise(void)
{
    // Initialize world and model subsystems in the original order
    AnimViewer_InitTextures();
    AnimViewer_HostInitPools();
    References_Init();
    AnimViewer_HostInitWorld();
    AnimManager_Initialise();
    AnimViewer_HostInitModelSystems();
    PedStats_Initialise();
    Messages_Init();
    AnimViewer_HostLoadLevels();
    AnimViewer_ConvertModelAnimations();
    AnimViewer_HostInitRenderer();
    AnimManager_LoadAnimFiles();
    AnimViewer_HostInitPlayerSystems();
    Timer_Initialise();
    Clock_Initialise(60000);
    TimeCycle_Initialise();
    AnimViewer_CreatePlayer();
    Draw_SetFOV(120.0f);
    AnimViewer_HostSetLodDistance();
    AnimViewer_LoadSpecialCharacters();
    AnimViewer_LoadVehicleAnimations();
}
void AnimViewer_Shutdown(void)
{
    // Destroy the player and reread the live pointer before releasing its pool slot
    if (AnimViewer_HostPlayer()) {
        AnimViewer_HostDestroyPlayer(AnimViewer_HostPlayer());
        CPool *pool=CPools_GetPedPool();
        CPool_Delete(pool,AnimViewer_HostPlayer());
    }
    // Preserve subsystem shutdown order and the final live texture-slot read
    AnimViewer_HostShutdownWorld();
    AnimViewer_HostShutdownModels();
    AnimManager_Shutdown();
    Timer_Shutdown();
    AnimViewer_HostShutdownStreaming();
    TxdStore_RemoveTxdSlot(AnimViewer_animTxdSlot);
}
void AnimViewer_InitTextures(void)
{
    // Load the HUD dictionary and hide the HUD before creating viewer textures
    int hudSlot=TxdStore_AddTxdSlot("hud");
    TxdStore_LoadFile(hudSlot,"MODELS/HUD.TXD");
    AnimViewer_HostHideHud();
    // Create the generic dictionary and then load the particle dictionary
    AnimViewer_animTxdSlot=TxdStore_AddTxdSlot("generic");
    TxdStore_Create(AnimViewer_animTxdSlot);
    int particleSlot=TxdStore_AddTxdSlot("particle");
    TxdStore_LoadFile(particleSlot,"MODELS/PARTICLE.TXD");
    TxdStore_SetCurrentTxd(AnimViewer_animTxdSlot);
}
void AnimViewer_ReloadAnimations(void)
{
    // Remove the target before restarting the animation manager and loading its files
    AnimViewer_RemoveTarget();
    AnimManager_Shutdown();
    AnimManager_Initialise();
    AnimManager_LoadAnimFiles();
}
void AnimViewer_RemoveTarget(void)
{
    // Remove the live target before checking each original entity category
    if (AnimViewer_target) {
        AnimViewer_HostRemoveTarget(AnimViewer_target);
        int kind=-1;
        if (AnimViewer_HostTargetMatches(AnimViewer_target,0))
            kind=0;
        else if (AnimViewer_HostTargetMatches(AnimViewer_target,1))
            kind=1;
        else if (AnimViewer_HostTargetMatches(AnimViewer_target,2))
            kind=2;
        // Destroy and release the live target using the selected pool
        if (kind>=0) {
            AnimViewer_HostDestroyTarget(AnimViewer_target);
            CPool *pool=kind==0 ? CPools_GetVehiclePool() : kind==1 ? CPools_GetPedPool() : CPools_GetObjectPool();
            void *slot=AnimViewer_HostTargetSlot(AnimViewer_target,kind);
            CPool_Delete(pool,slot);
        }
    }
    AnimViewer_target=0;
}
void AnimViewer_HandleVehicleInput(void *pad,int *nextModelId)
{
    // Preserve the shoulder-button priority over the vehicle-colour reload
    if (AnimViewer_HostButton(pad,ANIMVIEWER_SHOULDER)) {
        *nextModelId=AnimViewer_LastPedModelId(AnimViewer_state.modelId);
        AnimViewer_HostMessage("Switched to peds",1000);
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_SQUARE)) {
        AnimViewer_HostReloadVehicleColours();
        AnimViewer_HostMessage("Carcols.dat reloaded",1000);
    }
}
void AnimViewer_HandleModelNavigation(void *pad,int *nextModelId)
{
    // Preserve left-button priority and the original shared message buffer
    int direction=0;
    if (AnimViewer_HostButton(pad,ANIMVIEWER_LEFT)) direction=-1;
    else if (AnimViewer_HostButton(pad,ANIMVIEWER_RIGHT)) direction=1;
    if (direction) {
        *nextModelId=AnimViewer_NextModelId(AnimViewer_state.modelId,direction);
        sprintf(gString,"Current model ID: %d",*nextModelId);
        AnimViewer_HostMessage(gString,1000);
    }
}
bool AnimViewer_ApplyModelChange(int nextModelId)
{
    // Update the persistent model and remove the target only when the selection changes
    if (nextModelId==AnimViewer_state.modelId)
        return false;
    AnimViewer_state.modelId=nextModelId;
    AnimViewer_RemoveTarget();
    return true;
}
void AnimViewer_HandlePedInput(void *pad,int animGroup,int *nextModelId)
{
    // Preserve the original button priority and associated actions
    if (AnimViewer_HostButton(pad,ANIMVIEWER_SQUARE)) {
        AnimViewer_state.reloadIFP=true;
        AnimViewer_HostMessage("IFP reloaded",1000);
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_CROSS)) {
        AnimViewer_PlayAnimation(AnimViewer_HostTargetClump(AnimViewer_target),animGroup,AnimViewer_state.animId);
        AnimViewer_HostMessage("Animation restarted",1000);
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_CIRCLE)) {
        AnimViewer_PlayAnimation(AnimViewer_HostTargetClump(AnimViewer_target),animGroup,ANIM_STD_IDLE);
        AnimViewer_HostMessage("Idle animation playing",1000);
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_UP)) {
        AnimViewer_PreviousAnimation();
        AnimViewer_PlayAnimation(AnimViewer_HostTargetClump(AnimViewer_target),animGroup,AnimViewer_state.animId);
        sprintf(gString,"Current anim: %d",AnimViewer_state.animId);
        AnimViewer_HostMessage(gString,1000);
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_DOWN)) {
        AnimViewer_NextAnimation();
        AnimViewer_PlayAnimation(AnimViewer_HostTargetClump(AnimViewer_target),animGroup,AnimViewer_state.animId);
        sprintf(gString,"Current anim: %d",AnimViewer_state.animId);
        AnimViewer_HostMessage(gString,1000);
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_START)) {
        // Preserve the original empty start-button branch
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_SHOULDER)) {
        *nextModelId=AnimViewer_FirstCarModelId(AnimViewer_state.modelId);
        AnimViewer_HostMessage("Switched to vehicles",1000);
    } else if (AnimViewer_HostButton(pad,ANIMVIEWER_TRIANGLE)) {
        AnimViewer_HostAnimateCollision();
        AnimViewer_HostMessage("Ped Col model will be animated as long as you hold the button",100);
    }
    // Clamp against the live association count after the selected action
    AnimViewer_ClampAnimation(AnimManager_GetAnimAssocGroups()[animGroup].numAssociations);
}
void AnimViewer_PreviousAnimation(void)
{
    // Preserve the original decrement and lower-bound wrap
    --AnimViewer_state.animId;
    if (AnimViewer_state.animId<0)
        AnimViewer_state.animId=ANIM_STD_NUM-1;
}
void AnimViewer_NextAnimation(void)
{
    // Preserve the original equality-only upper-bound wrap
    AnimViewer_state.animId=AnimViewer_state.animId==ANIM_STD_NUM-1 ? 0 : AnimViewer_state.animId+1;
}
void AnimViewer_ClampAnimation(int count)
{
    // Reset an animation index beyond the current association count
    if (count<=AnimViewer_state.animId)
        AnimViewer_state.animId=0;
}
void AnimViewer_PlayAnimation(void *clump,int animGroup,int anim)
{
    // Remove an existing partial association before changing blend deltas
    CAnimBlendAssociation *current=RpAnimBlendClumpGetAssociation(clump,(uint32_t)anim);
    if (current && AnimAssociation_IsPartial(current)) {
        AnimAssociation_Destroy(current);
        free(current);
    }
    // Blend, mark, rewind, and run the requested animation in order
    RpAnimBlendClumpSetBlendDeltas(clump,ASSOC_PARTIAL,-8.0f);
    CAnimBlendAssociation *association=AnimManager_BlendAnimation(clump,animGroup,anim,8.0f);
    association->flags|=ASSOC_DELETEFADEDOUT;
    AnimAssociation_SetCurrentTime(association,0.0f);
    AnimAssociation_SetRun(association);
}
void AnimViewer_LoadVehicleAnimations(void)
{
    // Resolve all five animation blocks before submitting any streaming requests
    const char *names[]={"van","bikes","bikev","bikeh","biked"};
    int blocks[5];
    for (int i=0;i<5;++i)
        blocks[i]=AnimManager_GetAnimationBlockIndex(names[i]);
    // Preserve the flush, request, load, and reference sequence
    AnimViewer_HostFlushRequests();
    for (int i=0;i<5;++i)
        AnimViewer_HostRequestAnimation(blocks[i],STREAMFLAGS_DEPENDENCY);
    AnimViewer_HostLoadRequests(false);
    for (int i=0;i<5;++i)
        AnimManager_AddAnimBlockRef(blocks[i]);
}
void AnimViewer_LoadSpecialCharacters(void)
{
    // Read the original special-character file and process each available line
    int fd=FileMgr_OpenFile("DATA\\SPECIAL.TXT","r");
    char animGroup[32],modelName[32];
    if (fd) {
        for (int lineId=0;lineId<NUM_OF_SPECIAL_CHARS;++lineId) {
            char *buffer=gString;
            if (!FileMgr_ReadLine(fd,buffer,255))
                break;
            sscanf(buffer,"%s %s",modelName,animGroup);
            // Preserve the first matching animation group and optional assignment
            int groupId;
            for (groupId=0;groupId<NUM_ANIM_ASSOC_GROUPS;++groupId)
                if (!strcmp(animGroup,AnimManager_GetAnimGroupName(groupId)))
                    break;
            if (groupId!=NUM_ANIM_ASSOC_GROUPS)
                AnimViewer_HostSetSpecialGroup(MI_SPECIAL01+lineId,groupId);
            AnimViewer_HostRequestSpecial(lineId,modelName,STREAMFLAGS_DONT_REMOVE);
        }
        FileMgr_CloseFile(fd);
    }
}
void AnimViewer_ConvertModelAnimations(void)
{
    // Visit every model slot and preserve the separate lookup used for conversion
    for (int i=0;i<MODELINFOSIZE;++i)
        if (ModelRegistry_Get(i))
            AnimViewer_HostConvertModelAnimation(i);
}
int AnimViewer_LastPedModelId(int modelId)
{
    // Search backward using the original upper-bound assertion and model lookup
    for (;;) {
        ANIMVIEWER_ASSERT(modelId < MODELINFOSIZE);
        // Check the live model category at this search position
        void *model=ModelRegistry_Get(modelId);
        if (model && ModelRegistry_Type(model)==MITYPE_PED)
            return modelId;
        --modelId;
    }
}
int AnimViewer_FirstCarModelId(int modelId)
{
    // Search forward using the original upper-bound assertion and model lookup
    for (;;) {
        ANIMVIEWER_ASSERT(modelId < MODELINFOSIZE);
        // Check the live model category at this search position
        void *model=ModelRegistry_Get(modelId);
        if (model && ModelRegistry_Type(model)==MITYPE_VEHICLE)
            return modelId;
        ++modelId;
    }
}
int AnimViewer_NextModelId(int modelId,int wantedChange)
{
    // Search in the requested direction and reverse at each table boundary
    int remaining=2;
    int original=modelId;
    while (remaining!=0) {
        modelId+=wantedChange;
        if (modelId<0 || modelId>=MODELINFOSIZE) {
            --remaining;
            wantedChange=-wantedChange;
        } else if (modelId!=5 && modelId!=6 && modelId!=405) {
            if (ModelRegistry_Get(modelId))
                return modelId;
        }
    }
    return original;
}
void AnimViewer_Render(void)
{
    // Preserve the original checks and live target reads across native calls
    if (AnimViewer_target && AnimViewer_target) {
#ifdef FIX_BUGS
        if (AnimViewer_HostIsPed(AnimViewer_target))
            AnimViewer_HostUpdateAnimation(AnimViewer_target);
#endif
        AnimViewer_HostRender(AnimViewer_target);
        AnimViewer_HostRenderNonRoad(AnimViewer_target);
    }
}
//- rouz edit (ChatGPT)
