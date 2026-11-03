//+ rouz edit (ChatGPT)
#pragma once
#include "Darkel.h"
#include "Wanted.h"
#include "WeaponInfo.h"
#ifdef __cplusplus
class CPlayerPed;class CWeapon;class CBaseModelInfo;
extern "C" {
#else
typedef struct CPlayerPed CPlayerPed;typedef struct CWeapon CWeapon;typedef struct CBaseModelInfo CBaseModelInfo;
#endif
void Darkel_BackendJustifyOff(void);
void Darkel_BackendBackgroundOff(void);
void Darkel_BackendCentreSize(float size);
void Darkel_BackendCentreOn(void);
void Darkel_BackendPropOn(void);
void Darkel_BackendScale(float x,float y);
void Darkel_BackendColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void Darkel_BackendFontStyle(int16_t style);
void Darkel_BackendPrint(float x,float y,uint16_t *text);
void Darkel_BackendCentreOff(void);
void Darkel_BackendRightJustifyOn(void);
int32_t Darkel_BackendVehicleModel(CVehicle *vehicle);
int32_t Darkel_BackendVehicleAppearance(CVehicle *vehicle);
int32_t Darkel_BackendPedModel(CPed *ped);
bool Darkel_BackendChrisCriminal(CPed *ped);
uint32_t Darkel_BackendPedType(CPed *ped);
int32_t * Darkel_BackendCarsExploded(void);
int32_t * Darkel_BackendHelisDestroyed(void);
int32_t * Darkel_BackendBoatsExploded(void);
int32_t * Darkel_BackendPeopleKilledByPlayer(void);
int32_t * Darkel_BackendPedsKilledOfThisType(void);
int32_t * Darkel_BackendHeadsPopped(void);
int32_t * Darkel_BackendKillsSinceLastCheckpoint(void);
int32_t * Darkel_BackendPeopleKilledByOthers(void);
int32_t * Darkel_BackendRandomPedType(void);
void Darkel_BackendClearShortCut(void);
void Darkel_BackendRemoveDropOff(void);
CPlayerPed * Darkel_BackendPlayer(void);
CVehicle * Darkel_BackendPlayerVehicle(void);
CWeapon * Darkel_BackendCurrentWeapon(CPlayerPed *player);
CWeapon * Darkel_BackendWeaponAt(CPlayerPed *player,uint8_t slot);
int32_t Darkel_BackendWeaponSlot(CPlayerPed *player,enum eWeaponType type);
enum eWeaponType Darkel_BackendWeaponType(CWeapon *weapon);
int32_t Darkel_BackendAmmoTotal(CWeapon *weapon);
void Darkel_BackendRemoveWeaponAnims(CPlayerPed *player,int32_t type,float delta);
void Darkel_BackendGiveWeapon(CPlayerPed *player,enum eWeaponType type,uint32_t ammo,bool unused);
void Darkel_BackendCurrentType(CPlayerPed *player,enum eWeaponType type);
void Darkel_BackendCurrentSlot(CPlayerPed *player,int32_t slot);
int8_t Darkel_BackendSelectedSlot(CPlayerPed *player);
uint8_t Darkel_BackendCurrentSlotIndex(CPlayerPed *player);
void Darkel_BackendMakeChanges(CPlayerPed *player,int32_t slot);
void Darkel_BackendSetAmmo(CPlayerPed *player,enum eWeaponType type,uint32_t ammo);
void Darkel_BackendClearTarget(CPlayerPed *player);
void Darkel_BackendAddRef(CBaseModelInfo *model);
void Darkel_BackendRemoveRef(CBaseModelInfo *model);
void Darkel_BackendRemoveModel(CPlayerPed *player,int32_t model);
void Darkel_BackendSetWeaponType(CWeapon *weapon,enum eWeaponType type);
void Darkel_BackendSetAmmoTotal(CWeapon *weapon,int32_t ammo);
void Darkel_BackendSetAmmoInClip(CWeapon *weapon,int32_t ammo);
void Darkel_BackendSetWeaponState(CWeapon *weapon,enum eWeaponState state);
void Darkel_BackendSetSelectedSlot(CPlayerPed *player,uint32_t slot);
void Darkel_BackendRemoveDriveby(CPlayerPed *player);
void Darkel_BackendFrenzyPassed(void);
CWanted * Darkel_BackendWanted(CPlayerPed *player);
void Darkel_BackendRemoveSlotModel(int32_t slot);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
