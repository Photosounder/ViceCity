//+ rouz edit (ChatGPT)
#include "common.h"
#include "WeaponEffects_backend.h"
#include "PlayerPed.h"
#include "World.h"
int32_t WeaponEffects_HostWeaponType(void)
{
    // Query the actual player weapon at the original rendering boundary
    return FindPlayerPed()->GetWeapon()->m_eWeaponType;
}
int32_t WeaponEffects_HostConstant(int32_t identity)
{
    // Translate SDK tokens while keeping every render-state decision in C
    switch(identity){case WEAPON_EFFECTS_CONST_rwBLENDINVDESTALPHA:return rwBLENDINVDESTALPHA;case WEAPON_EFFECTS_CONST_rwBLENDINVSRCALPHA:return rwBLENDINVSRCALPHA;case WEAPON_EFFECTS_CONST_rwBLENDSRCALPHA:return rwBLENDSRCALPHA;case WEAPON_EFFECTS_CONST_rwRENDERSTATEDESTBLEND:return rwRENDERSTATEDESTBLEND;case WEAPON_EFFECTS_CONST_rwRENDERSTATESRCBLEND:return rwRENDERSTATESRCBLEND;case WEAPON_EFFECTS_CONST_rwRENDERSTATETEXTURERASTER:return rwRENDERSTATETEXTURERASTER;case WEAPON_EFFECTS_CONST_rwRENDERSTATEVERTEXALPHAENABLE:return rwRENDERSTATEVERTEXALPHAENABLE;case WEAPON_EFFECTS_CONST_rwRENDERSTATEZTESTENABLE:return rwRENDERSTATEZTESTENABLE;case WEAPON_EFFECTS_CONST_rwRENDERSTATEZWRITEENABLE:return rwRENDERSTATEZWRITEENABLE;}return 0;
}
//- rouz edit (ChatGPT)
