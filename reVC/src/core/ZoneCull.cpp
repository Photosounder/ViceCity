//+ rouz edit (ChatGPT)
#include "common.h"
#include "Building.h"
#include "Treadable.h"
#include "Train.h"
#include "Pools.h"
#include "Camera.h"
#include "World.h"
#include "ZoneCull.h"

extern "C" CullZonesPosition
CullZonesHost_GameCamera(void)
{
    // Read the original typed game input at its existing access point
    return CullZonesPosition_FromVector(TheCamera.GetGameCamPosition());
}

extern "C" CullZonesPosition
CullZonesHost_Player(void)
{
    // Read the original typed game input at its existing access point
    return CullZonesPosition_FromVector(FindPlayerCoors());
}

extern "C" float
CullZonesHost_CameraX(void)
{
    // Read the original typed game input at its existing access point
    return TheCamera.GetPosition().x;
}

extern "C" float
CullZonesHost_CameraY(void)
{
    // Read the original typed game input at its existing access point
    return TheCamera.GetPosition().y;
}

extern "C" CPool *
CullZonesHost_BuildingPool(void)
{
    // Read the original typed game input at its existing access point
    return CPools::GetBuildingPool();
}

extern "C" CPool *
CullZonesHost_TreadablePool(void)
{
    // Read the original typed game input at its existing access point
    return CPools::GetTreadablePool();
}

extern "C" CPool *
CullZonesHost_VehiclePool(void)
{
    // Read the original typed game input at its existing access point
    return CPools::GetVehiclePool();
}

extern "C" CEntity *
CullZonesHost_Building(void *slot)
{
    // Read the original typed game input at its existing access point
    return (CBuilding *)slot;
}

extern "C" CEntity *
CullZonesHost_Treadable(void *slot)
{
    // Read the original typed game input at its existing access point
    return (CTreadable *)slot;
}

extern "C" CVehicle *
CullZonesHost_Vehicle(void *slot)
{
    // Read the original typed game input at its existing access point
    return (CVehicle *)slot;
}

extern "C" bool
CullZonesHost_IsSubway(CEntity *entity)
{
    // Read the original typed game input at its existing access point
    return entity->bIsSubway;
}

extern "C" bool
CullZonesHost_IsTrain(CVehicle *vehicle)
{
    // Read the original typed game input at its existing access point
    return vehicle->IsTrain();
}

extern "C" uint8_t
CullZonesHost_Track(CVehicle *vehicle)
{
    // Read the original typed game input at its existing access point
    return ((CTrain *)vehicle)->m_nTrackId;
}

extern "C" void
CullZonesHost_SetEntityVisible(CEntity *entity, bool visible)
{
    // Update the original visibility bit after the C eligibility checks
    entity->bIsVisible = visible;
}

extern "C" void
CullZonesHost_SetVehicleVisible(CVehicle *entity, bool visible)
{
    // Update the original visibility bit after the C eligibility checks
    entity->bIsVisible = visible;
}

//- rouz edit (ChatGPT)
