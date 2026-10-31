//+ rouz edit (ChatGPT)
#include "common.h"
#include "Wanted_backend.h"
#include "Pools.h"
#include "CopPed.h"
#include "Darkel.h"
#include "Stats.h"
#include "main.h"
#include "World.h"
WantedVector Wanted_VectorFromNative(const CVector &value)
{
    // Copy the original by-value police-presence input at the native caller boundary
    WantedVector result={value.x,value.y,value.z};return result;
}
WantedPositionRef Wanted_ReferenceNative(const CVector &value)
{
    // Retain the actual native coordinate object until its original later read point
    WantedPositionRef result={NULL,&value};return result;
}
bool Wanted_BackendFrenzy(void)
{
    // Access only the original typed native primitive at its original policy access point
    return Darkel_FrenzyOnGoing();
}

int32_t * Wanted_BackendAttained(void)
{
    // Access only the original typed native primitive at its original policy access point
    return &Stats_WantedStarsAttained;
}

int32_t * Wanted_BackendEvaded(void)
{
    // Access only the original typed native primitive at its original policy access point
    return &Stats_WantedStarsEvaded;
}

CPool * Wanted_BackendPedPool(void)
{
    // Access only the original typed native primitive at its original policy access point
    return CPools::GetPedPool();
}

CPool * Wanted_BackendVehiclePool(void)
{
    // Access only the original typed native primitive at its original policy access point
    return CPools::GetVehiclePool();
}

int16_t Wanted_BackendPedModel(CPed *ped)
{
    // Access only the original typed native primitive at its original policy access point
    return ped->GetModelIndex();
}

int16_t Wanted_BackendVehicleModel(CVehicle *vehicle)
{
    // Access only the original typed native primitive at its original policy access point
    return vehicle->GetModelIndex();
}

WantedVector Wanted_BackendPedPosition(CPed *ped)
{
    // Access only the original typed native primitive at its original policy access point
    const CVector &v=ped->GetPosition();WantedVector result={v.x,v.y,v.z};return result;
}

WantedVector Wanted_BackendVehiclePosition(CVehicle *vehicle)
{
    // Access only the original typed native primitive at its original policy access point
    const CVector &v=vehicle->GetPosition();WantedVector result={v.x,v.y,v.z};return result;
}

bool Wanted_BackendLawEnforcer(CVehicle *vehicle)
{
    // Access only the original typed native primitive at its original policy access point
    return vehicle->bIsLawEnforcer;
}

uint8_t Wanted_BackendVehicleStatus(CVehicle *vehicle)
{
    // Access only the original typed native primitive at its original policy access point
    return vehicle->GetStatus();
}

CVehicle * Wanted_BackendPlayerVehicle(void)
{
    // Access only the original typed native primitive at its original policy access point
    return FindPlayerVehicle();
}

WantedVector Wanted_BackendPlayerCoors(void)
{
    // Access only the original typed native primitive at its original policy access point
    CVector v=FindPlayerCoors();WantedVector result={v.x,v.y,v.z};return result;
}

void Wanted_BackendClearCopState(CCopPed *cop)
{
    // Access only the original typed native primitive at its original policy access point
    cop->m_bIsInPursuit=false;cop->m_objective=OBJECTIVE_NONE;cop->m_prevObjective=OBJECTIVE_NONE;cop->m_nLastPedState=PED_NONE;
}

bool Wanted_BackendCopDying(CCopPed *cop)
{
    // Access only the original typed native primitive at its original policy access point
    return cop->DyingOrDead();
}

void Wanted_BackendCopWander(CCopPed *cop,float path)
{
    // Access only the original typed native primitive at its original policy access point
    cop->SetWanderPath(path);
}

WantedVector Wanted_BackendNativePosition(const CVector *value)
{
    // Access only the original typed native primitive at its original policy access point
    WantedVector result={value->x,value->y,value->z};return result;
}

void Wanted_BackendError(const char *message)
{
    // Access only the original typed native primitive at its original policy access point
    Error("%s",message);
}
//- rouz edit (ChatGPT)
