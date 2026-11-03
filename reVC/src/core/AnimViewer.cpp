//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)

#include "Font.h"
#include "Pad.h"
#include "Text.h"
#include "main.h"
#include "Timer.h"
#include "DMAudio.h"
#include "FileMgr.h"
#include "Streaming.h"
#include "TxdStore.h"
#include "General.h"
#include "Camera.h"
#include "Vehicle.h"
#include "Bike.h"
#include "PlayerSkin.h"
#include "PlayerInfo.h"
#include "World.h"
#include "Renderer.h"
#include "AnimManager.h"
#include "AnimBlendAssocGroup.h"
#include "AnimViewer.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "References.h"
#include "PathFind.h"
#include "HandlingMgr.h"
#include "TempColModels.h"
#include "Particle.h"
#include "CdStream.h"
#include "Messages.h"
#include "CarCtrl.h"
#include "FileLoader.h"
#include "ModelIndices.h"
#include "Clock.h"
#include "Timecycle.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "Shadows.h"
#include "Radar.h"
#include "Hud.h"
#include "debugmenu.h"

//+ rouz edit (ChatGPT)
extern "C" void AnimViewer_HostFlushRequests(void)
{
    // Flush the native streaming request list
    Streaming_FlushRequestList(); // rouz edit (ChatGPT)
}
extern "C" void AnimViewer_HostRequestAnimation(int block,int flags)
{
    // Submit the selected native animation block request
    Streaming_RequestAnim(block,flags); // rouz edit (ChatGPT)
}
extern "C" void AnimViewer_HostLoadRequests(bool priority)
{
    // Load the selected native streaming requests
    CStreaming::LoadAllRequestedModels(priority);
}
extern "C" void AnimViewer_HostSetSpecialGroup(int modelId,int groupId)
{
    // Assign the selected group to the original native pedestrian model
    ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(modelId)))->m_animGroup=groupId; // rouz edit (ChatGPT)
}
extern "C" void AnimViewer_HostRequestSpecial(int lineId,const char *name,int flags)
{
    // Forward the selected special-character streaming request
    CStreaming::RequestSpecialChar(lineId,name,flags);
}
extern "C" void AnimViewer_HostRemoveTarget(CEntity *entity)
{
    // Remove the selected native entity from the world
    CWorld::Remove(entity);
}
extern "C" bool AnimViewer_HostTargetMatches(CEntity *entity,int kind)
{
    // Query the selected native entity category
    if (kind==0) return entity->IsVehicle();
    if (kind==1) return entity->IsPed();
    return entity->IsObject();
}
extern "C" void AnimViewer_HostDestroyTarget(CEntity *entity)
{
    // Invoke the original explicit base-entity destructor
    entity->~CEntity();
}
extern "C" void *AnimViewer_HostTargetSlot(CEntity *entity,int kind)
{
    // Convert the selected entity to its original native pool slot type
    if (kind==0) return (CVehicle*)entity;
    if (kind==1) return (CPed*)entity;
    return (CObject*)entity;
}
extern "C" bool AnimViewer_HostButton(void *input,int button)
{
    // Query the selected native pad button without changing its priority
    CPad *pad=(CPad*)input;
    switch (button) {
    case ANIMVIEWER_SQUARE: return pad->GetSquareJustDown();
    case ANIMVIEWER_CROSS: return pad->GetCrossJustDown();
    case ANIMVIEWER_CIRCLE: return pad->GetCircleJustDown();
    case ANIMVIEWER_UP: return pad->GetDPadUpJustDown();
    case ANIMVIEWER_DOWN: return pad->GetDPadDownJustDown();
    case ANIMVIEWER_START: return pad->GetStartJustDown();
    case ANIMVIEWER_SHOULDER: return pad->GetLeftShoulder1JustDown();
    case ANIMVIEWER_LEFT: return pad->GetDPadLeftJustDown();
    case ANIMVIEWER_RIGHT: return pad->GetDPadRightJustDown();
    default: return !!pad->NewState.Triangle;
    }
}
extern "C" void *AnimViewer_HostTargetClump(CEntity *target)
{
    // Borrow the selected native target clump
    return target->GetClump();
}
extern "C" void AnimViewer_HostMessage(const char *message,int duration)
{
    // Preserve the original shared Unicode buffer and message call
    AsciiToUnicode(message,gUString);
    Messages_AddMessage(gUString,duration,0);
}
extern "C" void AnimViewer_HostReloadVehicleColours(void)
{
    // Reload the original native vehicle colour table
    CVehicleModelInfo::LoadVehicleColours();
}
extern "C" void AnimViewer_HostAnimateCollision(void)
{
    // Animate the original live target collision model
    ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(AnimViewer_target->GetModelIndex())))->AnimatePedColModelSkinned(AnimViewer_target->GetClump()); // rouz edit (ChatGPT)
}
extern "C" void AnimViewer_HostProcessWorld(void)
{
    // Process the native game world
    CWorld::Process();
}
extern "C" void AnimViewer_HostProcessCamera(void)
{
    // Process the native viewer camera
    TheCamera.Process();
}
extern "C" void AnimViewer_HostHideHud(void)
{
    // Preserve the original HUD visibility assignment
    CHud::m_Wants_To_Draw_Hud=false;
}
extern "C" bool AnimViewer_HostIsPed(CEntity *entity)
{
    // Query the selected native entity type
    return entity->IsPed();
}
extern "C" void AnimViewer_HostUpdateAnimation(CEntity *entity)
{
    // Update the selected native pedestrian animation
    ((CPed*)entity)->UpdateRpHAnim();
}
extern "C" void AnimViewer_HostRender(CEntity *entity)
{
    // Render the selected native entity
    entity->Render();
}
extern "C" void AnimViewer_HostRenderNonRoad(CEntity *entity)
{
    // Pass the selected native entity to the renderer
    CRenderer::RenderOneNonRoad(entity);
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern "C" void AnimViewer_HostInitPools(void)
{
    // Initialize the native game pools
    CPools::Initialise();
}
extern "C" void AnimViewer_HostInitWorld(void)
{
    // Initialize the native camera, paths, collision, world, and handling systems
    TheCamera.Init();
    TheCamera.SetRwCamera(Scene.camera);
    TheCamera.Cams[TheCamera.ActiveCam].Distance=5.0f;
    ThePaths.Init();
    ThePaths.AllocatePathFindInfoMem(4500);
    CCollision::Init();
    CWorld::Initialise();
    mod_HandlingManager.Initialise();
    CTempColModels::Initialise();
}
extern "C" void AnimViewer_HostInitModelSystems(void)
{
    // Initialize native model, particle, and vehicle-control systems
    ModelRegistry_Initialise(); // rouz edit (ChatGPT)
    CParticle::Initialise();
    CCarCtrl::Init();
}
extern "C" void AnimViewer_HostLoadLevels(void)
{
    // Load the original image and viewer levels before streaming initialization
    CdStreamAddImage("MODELS\\GTA3.IMG");
    CFileLoader::LoadLevel("DATA\\DEFAULT.DAT");
    CFileLoader::LoadLevel("DATA\\ANIMVIEWER.DAT");
    CStreaming::Init();
}
extern "C" void AnimViewer_HostInitRenderer(void)
{
    // Load the original player models and initialize the native renderer
    CStreaming::LoadInitialPeds();
    CStreaming::RequestSpecialModel(MI_PLAYER,"player",STREAMFLAGS_DONT_REMOVE);
    CStreaming::LoadAllRequestedModels(false);
    CRenderer::Init();
    CVehicleModelInfo::LoadVehicleColours();
#ifdef FIX_BUGS
    CVehicleModelInfo::LoadEnvironmentMaps();
#endif
}
extern "C" void AnimViewer_HostInitPlayerSystems(void)
{
    // Set the player focus and initialize native weapon and pedestrian systems
    CWorld::PlayerInFocus=0;
    CWeapon::InitialiseWeapons();
    CPed::Initialise();
}
extern "C" void AnimViewer_HostInitCarControl(void)
{
    // Preserve the original native vehicle-control initialization
    CCarCtrl::Init();
}
extern "C" void AnimViewer_HostConstructPlayer(void *player)
{
    // Construct the native player in the selected pedestrian pool slot
    std::allocator<CPlayerPed>().construct((CPlayerPed*)((CPed*)player));
}
extern "C" void AnimViewer_HostPositionPlayer(void *player,float x,float y,float z)
{
    // Position the constructed native player
    ((CPlayerPed*)((CPed*)player))->SetPosition(x,y,z);
}
extern "C" void AnimViewer_HostAssignPlayer(void *player)
{
    // Assign the same native player to player zero
    CWorld::Players[0].m_pPed=(CPlayerPed*)((CPed*)player);
}
extern "C" void AnimViewer_HostSetLodDistance(void)
{
    // Preserve the viewer draw-distance assignment
    Draw_ms_fLODDistance=500.0f;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern "C" void AnimViewer_HostConvertModelAnimation(int modelId)
{
    // Requery the selected native model before converting its animation index
    ((CBaseModelInfo*)ModelRegistry_Get(modelId))->ConvertAnimFileIndex(); // rouz edit (ChatGPT)
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern "C" int AnimViewer_HostPedGroup(void *model)
{
    // Read the original pedestrian animation group
    return ((CPedModelInfo*)model)->m_animGroup;
}
extern "C" void *AnimViewer_HostUpdatePads(void)
{
    // Update native pads and process the optional debug menu in their original order
    CPad::UpdatePads();
    CPad *pad=CPad::GetPad(0);
#ifdef DEBUGMENU
    DebugMenuProcess();
#endif
    return pad;
}
extern "C" void AnimViewer_HostUpdateStreaming(void)
{
    // Process native streaming for the viewer
    CStreaming::UpdateForAnimViewer();
}
extern "C" void AnimViewer_HostRequestModel(int modelId)
{
    // Request the current native model
    Streaming_RequestModel(modelId,0); // rouz edit (ChatGPT)
}
extern "C" bool AnimViewer_HostModelLoaded(int modelId)
{
    // Check the current native model streaming status
    return Streaming_HasModelLoaded(modelId); // rouz edit (ChatGPT)
}
extern "C" void AnimViewer_HostConstructObject(CEntity *entity,int modelId)
{
    // Construct the selected native object in its allocated pool slot
    std::allocator<CObject>().construct((CObject*)entity,modelId,true);
}
extern "C" bool AnimViewer_HostIsTemporaryObject(void *object)
{
    // Check the original native object creation category
    return ((CObject*)object)->ObjectCreatedBy==TEMP_OBJECT;
}
extern "C" void AnimViewer_HostDestroyTemporaryObject(void *object)
{
    // Remove and destroy the selected native temporary object
    CObject *existing=(CObject*)object;
    CWorld::Remove(existing);
    existing->~CObject();
}
extern "C" CEntity *AnimViewer_HostObjectEntity(void *object)
{
    // Convert the native object slot to its entity base pointer
    return (CObject*)object;
}
extern "C" int AnimViewer_HostVehicleType(void *model)
{
    // Read the live native vehicle-model subtype
    return ((CVehicleModelInfo*)model)->m_vehicleType;
}
extern "C" CEntity *AnimViewer_HostVehicleEntity(void *slot)
{
    // Convert the native vehicle slot to its entity base pointer
    return (CVehicle*)slot;
}
extern "C" void AnimViewer_HostConstructCar(CEntity *target,int modelId)
{
    // Construct the native automobile in its allocated slot
    std::allocator<CAutomobile>().construct((CAutomobile*)target,modelId,RANDOM_VEHICLE);
}
extern "C" void AnimViewer_HostConstructBoat(CEntity *target,int modelId)
{
    // Construct the native boat in its allocated slot
    std::allocator<CBoat>().construct((CBoat*)target,modelId,RANDOM_VEHICLE);
}
extern "C" void AnimViewer_HostConstructBike(CEntity *target,int modelId)
{
    // Construct the native bike in its allocated slot
    std::allocator<CBike>().construct((CBike*)target,modelId,RANDOM_VEHICLE);
}
extern "C" void AnimViewer_HostSetFallbackCollision(void *model)
{
    // Set the original native fallback collision model
    ((CBaseModelInfo*)model)->SetColModel(&CTempColModels::ms_colModelWheel1);
}
extern "C" void AnimViewer_HostSetAbandoned(CEntity *target)
{
    // Mark the live native target as abandoned
    target->SetStatus(STATUS_ABANDONED);
}
extern "C" CEntity *AnimViewer_HostPedEntity(void *slot)
{
    // Convert the native pedestrian slot to its entity base pointer
    return (CPed*)slot;
}
extern "C" void AnimViewer_HostConstructPed(CEntity *target)
{
    // Construct the original native pedestrian in the allocated slot
    std::allocator<CPed>().construct((CPed*)target,PEDTYPE_CIVMALE);
}
extern "C" void AnimViewer_HostSetTargetModel(CEntity *target,int modelId)
{
    // Set the selected native target model after construction
    target->SetModelIndex(modelId);
}
extern "C" void AnimViewer_HostSetTargetPosition(CEntity *target,float x,float y,float z)
{
    // Set the selected native target position
    target->SetPosition(x,y,z);
}
extern "C" void AnimViewer_HostAddTarget(CEntity *target)
{
    // Add the selected native target to the world
    CWorld::Add(target);
}
extern "C" void AnimViewer_HostCameraTakeTarget(CEntity *target)
{
    // Attach the native camera using the original viewer settings
    TheCamera.TakeControl(target,CCam::MODE_MODELVIEW,JUMP_CUT,CAMCONTROL_SCRIPT);
}
extern "C" void AnimViewer_HostClearTargetVelocity(CEntity *entity)
{
    // Clear the selected native physical entity velocity
    ((CPhysical*)entity)->m_vecMoveSpeed=CVector(0.0f,0.0f,0.0f);
}
extern "C" void AnimViewer_HostSetTargetHeight(CEntity *entity,float height)
{
    // Set the selected native target height
    entity->GetMatrix().GetPosition().z=height;
}
extern "C" void AnimViewer_HostKeepPedInPlace(void)
{
    // Retain the original native pedestrian flag assignment
    ((CPed*)AnimViewer_target)->bKindaStayInSamePlace=true;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern "C" void *AnimViewer_HostPlayer(void)
{
    // Read the current native player pointer
    return CWorld::Players[0].m_pPed;
}
extern "C" void AnimViewer_HostDestroyPlayer(void *player)
{
    // Invoke the original native player destructor
    ((CPlayerPed*)player)->~CPlayerPed();
}
extern "C" void AnimViewer_HostShutdownWorld(void)
{
    // Shut down the native world subsystem
    CWorld::ShutDown();
}
extern "C" void AnimViewer_HostShutdownModels(void)
{
    // Shut down the native model subsystem
    ModelRegistry_Shutdown(); // rouz edit (ChatGPT)
}
extern "C" void AnimViewer_HostShutdownStreaming(void)
{
    // Shut down the native streaming subsystem
    CStreaming::Shutdown();
}
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
