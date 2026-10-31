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
static_assert(int(EVENTS_MISSION_CHAR)==int(MISSION_CHAR),"Original native pedestrian creation code");
bool Events_BackendIsPlayer(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->IsPlayer();
}

uint8_t Events_BackendCreatedBy(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->CharCreatedBy;
}

bool Events_BackendScorched(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->bRenderScorched;
}

bool Events_BackendBodyPart(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->bBodyPartJustCameOff;
}

bool Events_BackendAllowRevive(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->bAllowMedicsToReviveMe;
}

bool Events_BackendInWater(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->bIsInWater;
}

bool Events_BackendHasSurface(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->m_pCurrentPhysSurface!=nil;
}

EventsVector Events_BackendPedPosition(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    const CVector &v=ped->GetPosition();EventsVector result={v.x,v.y,v.z};return result;
}

float Events_BackendPedZ(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->GetPosition().z;
}

bool Events_BackendVerticalLine(EventsVector position,float endZ)
{
    // Access the original typed native primitive only at its original C policy access point
    CColPoint collision;CEntity *entity;return CWorld::ProcessVerticalLine(CVector(position.x,position.y,position.z),endZ,collision,entity,true,false,false,false,false,false,nil);
}

void Events_BackendSetLastAccident(CPed *ped,CAccident *accident)
{
    // Access the original typed native primitive only at its original C policy access point
    ped->m_lastAccident=accident;
}

CPed * Events_BackendPed(int32_t reference)
{
    // Access the original typed native primitive only at its original C policy access point
    return CPools::GetPed(reference);
}

float Events_BackendHealth(CPed *ped)
{
    // Access the original typed native primitive only at its original C policy access point
    return ped->m_fHealth;
}
//- rouz edit (ChatGPT)
