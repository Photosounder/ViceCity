//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "WeaponInfo_backend.h"
#include "main.h"
#include "AnimManager.h"
#include "Weapon.h"
#include "ModelInfo.h"
CVector WeaponInfo_VectorToNative(WeaponInfoVector value)
{
    // Supply the original native vector value to each remaining native consumer
    return CVector(value.x,value.y,value.z);
}
bool WeaponInfo_BackendIsShotgun(int32_t weapon)
{
    // Enter the original typed native operation or exact diagnostic call
    return CWeapon::IsShotgun(weapon);
}
void WeaponInfo_BackendSetModelWeapon(int32_t model,int32_t weapon)
{
    // Enter the original typed native operation or exact diagnostic call
    ((CWeaponModelInfo *)((CBaseModelInfo*)ModelRegistry_Get(model)))->m_atomics[2]=(RpAtomic*)weapon; // rouz edit (ChatGPT)
}
const char * WeaponInfo_BackendAnimGroupName(enum AssocGroupId group)
{
    // Enter the original typed native operation or exact diagnostic call
    // Use the complete C animation manager at the original call boundary
    return AnimManager_GetAnimGroupName(group);
}
void WeaponInfo_BackendInitialising(void)
{
    // Enter the original typed native operation or exact diagnostic call
    debug("Initialising CWeaponInfo...\n");
}
void WeaponInfo_BackendLoading(void)
{
    // Enter the original typed native operation or exact diagnostic call
    debug("Loading weapon data...\n");
}
void WeaponInfo_BackendReady(void)
{
    // Enter the original typed native operation or exact diagnostic call
    debug("CWeaponInfo ready\n");
}
void WeaponInfo_BackendShuttingDown(void)
{
    // Enter the original typed native operation or exact diagnostic call
    debug("Shutting down CWeaponInfo...\n");
}
void WeaponInfo_BackendShutDown(void)
{
    // Enter the original typed native operation or exact diagnostic call
    debug("CWeaponInfo shut down\n");
}
void WeaponInfo_BackendUnknownFireType(void)
{
    // Enter the original typed native operation or exact diagnostic call
    Error("Unknown weapon fire type, WeaponInfo.cpp");
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
