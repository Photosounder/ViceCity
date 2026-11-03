//+ rouz edit (ChatGPT)
#include "common.h"
#include "GameLogic.h"
#include "Hud.h"
#include "PlayerPed.h"
#include "Replay.h"
#include "Text.h"
#include "Vehicle.h"
#include "World.h"
#include "User_backend.h"

ZonesPosition UserDisplay_BackendPlayerPosition(void)
{
    // Capture the original player position once before either navigation-zone query
    CVector position=CWorld::Players[CWorld::PlayerInFocus].GetPos();
    return ZonesPosition_FromVector(position);
}
void UserDisplay_BackendSetZoneName(uint16_t *name)
{
    // Publish the selected original UTF-16 label through the native HUD owner
    CHud::SetZoneName(name);
}
uint16_t **UserDisplay_BackendLastZoneName(void)
{
    // Expose the original HUD history slot for the ordered C reset
    return &CHud::m_pLastZoneName;
}
uint32_t *UserDisplay_BackendZoneState(void)
{
    // Expose the original HUD state slot for the following C reset
    return &CHud::m_ZoneState;
}
bool UserDisplay_BackendInVehicle(void)
{
    // Query the live focused pedestrian at the original vehicle-state check
    return CWorld::Players[CWorld::PlayerInFocus].m_pPed->InVehicle();
}
CVehicle *UserDisplay_BackendVehicle(void)
{
    // Read the focused pedestrian again after the original predicate has returned
    return CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pMyVehicle;
}
CVehicle *UserDisplay_BackendShortCutTaxi(void)
{
    // Convert the original typed taxi pointer through its native vehicle base
    return CGameLogic::pShortCutTaxi;
}
int16_t UserDisplay_BackendModelIndex(const CVehicle *vehicle)
{
    // Read the model index through the actual native vehicle inheritance
    return vehicle->GetModelIndex();
}
char *UserDisplay_BackendGameName(int16_t model)
{
    // Resolve the original typed model record and borrow its existing game-name array
    return ((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(model)))->m_gameName; // rouz edit (ChatGPT)
}
void UserDisplay_BackendSetVehicleName(uint16_t *name)
{
    // Publish the original borrowed UTF-16 vehicle name through the native HUD owner
    CHud::SetVehicleName(name);
}

//- rouz edit (ChatGPT)
