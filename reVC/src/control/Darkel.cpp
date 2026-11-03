//+ rouz edit (ChatGPT)
#include "common.h"
#include "Darkel_backend.h"
#include "main.h"
#include "PlayerPed.h"
#include "Weapon.h"
#include "Vehicle.h"
#include "Replay.h"
#include "Population.h"
#include "GameLogic.h"
#include "Stats.h"
#include "Font.h"


void Darkel_BackendJustifyOff(void)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetJustifyOff();
}

void Darkel_BackendBackgroundOff(void)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetBackgroundOff();
}

void Darkel_BackendCentreSize(float size)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetCentreSize(size);
}

void Darkel_BackendCentreOn(void)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetCentreOn();
}

void Darkel_BackendPropOn(void)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetPropOn();
}

void Darkel_BackendScale(float x,float y)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetScale(x,y);
}

void Darkel_BackendColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetColor(CRGBA(r,g,b,a));
}

void Darkel_BackendFontStyle(int16_t style)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetFontStyle(style);
}

void Darkel_BackendPrint(float x,float y,uint16_t *text)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::PrintString(x,y,text);
}

void Darkel_BackendCentreOff(void)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetCentreOff();
}

void Darkel_BackendRightJustifyOn(void)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::SetRightJustifyOn();
}




int32_t Darkel_BackendVehicleModel(CVehicle *vehicle)
{
    // Access the original typed native primitive at its original C policy access point
    return vehicle->GetModelIndex();
}

int32_t Darkel_BackendVehicleAppearance(CVehicle *vehicle)
{
    // Access the original typed native primitive at its original C policy access point
    return vehicle->GetVehicleAppearance();
}

int32_t Darkel_BackendPedModel(CPed *ped)
{
    // Access the original typed native primitive at its original C policy access point
    return ped->GetModelIndex();
}

bool Darkel_BackendChrisCriminal(CPed *ped)
{
    // Access the original typed native primitive at its original C policy access point
    return ped->bChrisCriminal;
}

uint32_t Darkel_BackendPedType(CPed *ped)
{
    // Access the original typed native primitive at its original C policy access point
    return ped->m_nPedType;
}

int32_t * Darkel_BackendCarsExploded(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Stats_CarsExploded;
}

int32_t * Darkel_BackendHelisDestroyed(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Stats_HelisDestroyed;
}

int32_t * Darkel_BackendBoatsExploded(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Stats_BoatsExploded;
}

int32_t * Darkel_BackendPeopleKilledByPlayer(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Stats_PeopleKilledByPlayer;
}

int32_t * Darkel_BackendPedsKilledOfThisType(void)
{
    // Access the original typed native primitive at its original C policy access point
    return Stats_PedsKilledOfThisType;
}

int32_t * Darkel_BackendHeadsPopped(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Stats_HeadsPopped;
}

int32_t * Darkel_BackendKillsSinceLastCheckpoint(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Stats_KillsSinceLastCheckpoint;
}

int32_t * Darkel_BackendPeopleKilledByOthers(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Stats_PeopleKilledByOthers;
}

int32_t * Darkel_BackendRandomPedType(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &Population_allRandomPedsType; // rouz edit (ChatGPT)
}

void Darkel_BackendClearShortCut(void)
{
    // Access the original typed native primitive at its original C policy access point
    CGameLogic::ClearShortCut();
}

void Darkel_BackendRemoveDropOff(void)
{
    // Access the original typed native primitive at its original C policy access point
    CGameLogic::RemoveShortCutDropOffPointForMission();
}

CPlayerPed * Darkel_BackendPlayer(void)
{
    // Access the original typed native primitive at its original C policy access point
    return FindPlayerPed();
}

CVehicle * Darkel_BackendPlayerVehicle(void)
{
    // Access the original typed native primitive at its original C policy access point
    return FindPlayerVehicle();
}

CWeapon * Darkel_BackendCurrentWeapon(CPlayerPed *player)
{
    // Access the original typed native primitive at its original C policy access point
    return player->GetWeapon();
}

CWeapon * Darkel_BackendWeaponAt(CPlayerPed *player,uint8_t slot)
{
    // Access the original typed native primitive at its original C policy access point
    return &player->GetWeapon(slot);
}

int32_t Darkel_BackendWeaponSlot(CPlayerPed *player,enum eWeaponType type)
{
    // Access the original typed native primitive at its original C policy access point
    return player->GetWeaponSlot(type);
}

enum eWeaponType Darkel_BackendWeaponType(CWeapon *weapon)
{
    // Access the original typed native primitive at its original C policy access point
    return weapon->m_eWeaponType;
}

int32_t Darkel_BackendAmmoTotal(CWeapon *weapon)
{
    // Access the original typed native primitive at its original C policy access point
    return weapon->m_nAmmoTotal;
}

void Darkel_BackendRemoveWeaponAnims(CPlayerPed *player,int32_t type,float delta)
{
    // Access the original typed native primitive at its original C policy access point
    player->RemoveWeaponAnims(type,delta);
}

void Darkel_BackendGiveWeapon(CPlayerPed *player,enum eWeaponType type,uint32_t ammo,bool unused)
{
    // Access the original typed native primitive at its original C policy access point
    player->GiveWeapon(type,ammo,unused);
}

void Darkel_BackendCurrentType(CPlayerPed *player,enum eWeaponType type)
{
    // Access the original typed native primitive at its original C policy access point
    player->SetCurrentWeapon(type);
}

void Darkel_BackendCurrentSlot(CPlayerPed *player,int32_t slot)
{
    // Access the original typed native primitive at its original C policy access point
    player->SetCurrentWeapon(slot);
}

int8_t Darkel_BackendSelectedSlot(CPlayerPed *player)
{
    // Access the original typed native primitive at its original C policy access point
    return player->m_nSelectedWepSlot;
}

uint8_t Darkel_BackendCurrentSlotIndex(CPlayerPed *player)
{
    // Access the original typed native primitive at its original C policy access point
    return player->m_currentWeapon;
}

void Darkel_BackendMakeChanges(CPlayerPed *player,int32_t slot)
{
    // Access the original typed native primitive at its original C policy access point
    player->MakeChangesForNewWeapon(slot);
}

void Darkel_BackendSetAmmo(CPlayerPed *player,enum eWeaponType type,uint32_t ammo)
{
    // Access the original typed native primitive at its original C policy access point
    player->SetAmmo(type,ammo);
}

void Darkel_BackendClearTarget(CPlayerPed *player)
{
    // Access the original typed native primitive at its original C policy access point
    player->ClearWeaponTarget();
}

void Darkel_BackendAddRef(CBaseModelInfo *model)
{
    // Access the original typed native primitive at its original C policy access point
    model->AddRef();
}

void Darkel_BackendRemoveRef(CBaseModelInfo *model)
{
    // Access the original typed native primitive at its original C policy access point
    model->RemoveRef();
}

void Darkel_BackendRemoveModel(CPlayerPed *player,int32_t model)
{
    // Access the original typed native primitive at its original C policy access point
    player->RemoveWeaponModel(model);
}

void Darkel_BackendSetWeaponType(CWeapon *weapon,enum eWeaponType type)
{
    // Access the original typed native primitive at its original C policy access point
    weapon->m_eWeaponType=type;
}

void Darkel_BackendSetAmmoTotal(CWeapon *weapon,int32_t ammo)
{
    // Access the original typed native primitive at its original C policy access point
    weapon->m_nAmmoTotal=ammo;
}

void Darkel_BackendSetAmmoInClip(CWeapon *weapon,int32_t ammo)
{
    // Access the original typed native primitive at its original C policy access point
    weapon->m_nAmmoInClip=ammo;
}

void Darkel_BackendSetWeaponState(CWeapon *weapon,enum eWeaponState state)
{
    // Access the original typed native primitive at its original C policy access point
    weapon->m_eWeaponState=state;
}

void Darkel_BackendSetSelectedSlot(CPlayerPed *player,uint32_t slot)
{
    // Access the original typed native primitive at its original C policy access point
    player->m_nSelectedWepSlot=slot;
}

void Darkel_BackendRemoveDriveby(CPlayerPed *player)
{
    // Access the original typed native primitive at its original C policy access point
    player->RemoveDrivebyAnims();
}

void Darkel_BackendFrenzyPassed(void)
{
    // Access the original typed native primitive at its original C policy access point
    Stats_AnotherKillFrenzyPassed();
}

CWanted * Darkel_BackendWanted(CPlayerPed *player)
{
    // Access the original typed native primitive at its original C policy access point
    return player->m_pWanted;
}

void Darkel_BackendRemoveSlotModel(int32_t slot)
{
    // Access the original typed native primitive at its original C policy access point
    FindPlayerPed()->RemoveWeaponModel(FindPlayerPed()->GetWeapon(slot).GetInfo()->m_nModelId);
}
//- rouz edit (ChatGPT)
