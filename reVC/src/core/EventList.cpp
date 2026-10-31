//+ rouz edit (ChatGPT)
#include "common.h"
#include "Events_backend.h"
#include "main.h"
#include "Game.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "World.h"
#include "Wanted.h"
EventsVector Events_VectorFromNative(const CVector &v)
{
    // Capture original caller vector values explicitly without assuming a native object representation
    EventsVector result={v.x,v.y,v.z};return result;
}
void Events_BackendDebug(const char *message)
{
    // Access the original typed native primitive only at its original C policy access point
    debug("%s",message);
}

void Events_BackendError(const char *message)
{
    // Access the original typed native primitive only at its original C policy access point
    Error("%s",message);
}

int32_t Events_BackendPedRef(CEntity *entity)
{
    // Access the original typed native primitive only at its original C policy access point
    return CPools::GetPedRef((CPed*)entity);
}

int32_t Events_BackendVehicleRef(CEntity *entity)
{
    // Access the original typed native primitive only at its original C policy access point
    return CPools::GetVehicleRef((CVehicle*)entity);
}

int32_t Events_BackendObjectRef(CEntity *entity)
{
    // Access the original typed native primitive only at its original C policy access point
    return CPools::GetObjectRef((CObject*)entity);
}

CPed * Events_BackendPedFromEntity(CEntity *entity)
{
    // Access the original typed native primitive only at its original C policy access point
    return (CPed*)entity;
}

EventsVector Events_BackendEntityPosition(CEntity *entity)
{
    // Access the original typed native primitive only at its original C policy access point
    const CVector &v=entity->GetPosition();EventsVector result={v.x,v.y,v.z};return result;
}

void Events_BackendRegisterReference(CPed *ped,CPed **slot)
{
    // Access the original typed native primitive only at its original C policy access point
    ped->RegisterReference((CEntity**)slot);
}

CPed * Events_BackendPlayer(void)
{
    // Access the original typed native primitive only at its original C policy access point
    return FindPlayerPed();
}

bool Events_BackendPedValid(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return IsPedPointerValid(ped);
}

int32_t Events_BackendWantedLevel(void)
{
    // Access the original typed native primitive only at its original C policy access point
    return Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted);
}

bool Events_BackendChased(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->bBeingChasedByPolice;
}

bool Events_BackendDyingOrDead(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->DyingOrDead();
}

int32_t * Events_BackendMoney(void)
{
    // Access the original typed native primitive only at its original C policy access point
    return &CWorld::Players[CWorld::PlayerInFocus].m_nMoney;
}

EventsVector Events_BackendPlayerCoors(void)
{
    // Access the original typed native primitive only at its original C policy access point
    CVector v=FindPlayerCoors();EventsVector result={v.x,v.y,v.z};return result;
}

EventsVector Events_BackendPlayerPedPosition(void)
{
    // Access the original typed native primitive only at its original C policy access point
    const CVector &v=FindPlayerPed()->GetPosition();EventsVector result={v.x,v.y,v.z};return result;
}

int32_t Events_BackendPolicePresence(EventsVector position,float radius)
{
    // Access the original typed native primitive only at its original C policy access point
    return Wanted_WorkOutPolicePresence(Wanted_VectorFromNative(CVector(position.x,position.y,position.z)),radius);
}

bool Events_BackendGermanGame(void)
{
    // Access the original typed native primitive only at its original C policy access point
    return CGame::germanGame;
}

void Events_BackendCrime(enum eCrimeType type,EventsVector position,uint32_t id,bool copsDontCare)
{
    // Access the original typed native primitive only at its original C policy access point
    Wanted_RegisterCrime(FindPlayerPed()->m_pWanted,type,Wanted_ReferenceNative(CVector(position.x,position.y,position.z)),id,copsDontCare);
}

void Events_BackendImmediateCrime(enum eCrimeType type,EventsVector position,uint32_t id,bool copsDontCare)
{
    // Access the original typed native primitive only at its original C policy access point
    Wanted_RegisterCrime_Immediately(FindPlayerPed()->m_pWanted,type,Wanted_ReferenceNative(CVector(position.x,position.y,position.z)),id,copsDontCare);
}

void Events_BackendWantedNoDrop(int32_t level)
{
    // Access the original typed native primitive only at its original C policy access point
    Wanted_SetWantedLevelNoDrop(FindPlayerPed()->m_pWanted,level);
}

void Events_BackendPedWantedNoDrop(int32_t level)
{
    // Access the original typed native primitive only at its original C policy access point
    FindPlayerPed()->SetWantedLevelNoDrop(level);
}
//- rouz edit (ChatGPT)
