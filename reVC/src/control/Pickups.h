#pragma once
#include "Weapon.h"

#include "PickupData.h"

class CEntity;
class CObject;
class CVehicle;
class CPlayerPed;

class CPickup : public CPickupData
{
public:

	CObject *GiveUsAPickUpObject(CObject **object, CObject **extraObject, int32 handle, int32 extraHandle);
	bool Update(CPlayerPed *player, CVehicle *vehicle, int playerId);
	void GetRidOfObjects();
	void ExtractAmmoFromPickup(CPlayerPed *player);
	void ProcessGunShot(CVector *vec1, CVector *vec2);
private:
	inline bool IsMine() { return m_eType >= PICKUP_MINE_INACTIVE && m_eType <= PICKUP_FLOATINGPACKAGE_FLOATING; }
	inline bool CanBePickedUp(CPlayerPed *player, int playerId);
	inline void Remove();
};

VALIDATE_SIZE(CPickup, 0x1C);

struct tPickupMessage
{
	CVector2D m_pos;
	eWeaponType m_weaponType;
	CVector2D m_dist;
	CRGBA m_color;
	uint8 m_bOutOfStock;
	uint8 m_quantity;
	uint16 money;
};

//+ rouz edit (Claude)
// Formerly the static class CPickups, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern int32 Pickups_aPickUpsCollected[NUMCOLLECTEDPICKUPS];
extern int16 Pickups_CollectedPickUpIndex;
extern int16 Pickups_NumMessages;
extern tPickupMessage Pickups_aMessages[NUMPICKUPMESSAGES];
extern int32 Pickups_PlayerOnWeaponPickup;
void Pickups_Init();
void Pickups_Update();
void Pickups_RenderPickUpText();
void Pickups_DoCollectableEffects(CEntity *ent);
void Pickups_DoMoneyEffects(CEntity *ent);
void Pickups_DoMineEffects(CEntity *ent);
void Pickups_DoPickUpEffects(CEntity *ent);
int32 Pickups_GenerateNewOne_WeaponType(CVector pos, eWeaponType weaponType, uint8 type, uint32 quantity);
void Pickups_RemovePickUp(int32 pickupIndex);
void Pickups_AddToCollectedPickupsArray(int32 index);
bool Pickups_IsPickUpPickedUp(int32 pickupId);
int32 Pickups_ModelForWeapon(eWeaponType weaponType);
enum eWeaponType Pickups_WeaponForModel(int32 model);
int32 Pickups_GetActualPickupIndex(int32 index);
int32 Pickups_GetNewUniquePickupIndex(int32 slot);
void Pickups_PassTime(uint32 time);
bool Pickups_GivePlayerGoodiesWithPickUpMI(int16 modelIndex, int playerIndex);
bool Pickups_TestForPickupsInBubble(CVector pos, float range);
bool Pickups_TryToMerge_WeaponType(CVector pos, eWeaponType weapon, uint8 type, uint32 quantity, bool unused);
void Pickups_CreateSomeMoney(CVector, int);
void Pickups_DetonateMinesHitByGunShot(CVector *vec1, CVector *vec2);
void Pickups_Load(uint8 *buf, uint32 size);
void Pickups_Save(uint8 *buf, uint32 *size);
extern bool Pickups_bPickUpcamActivated;
extern CVehicle * Pickups_pPlayerVehicle;
extern CVector Pickups_StaticCamCoors;
extern uint32 Pickups_StaticCamStartTime;
void Pickups_RemoveAllPickupsOfACertainWeaponGroupWithNoAmmo(eWeaponType);
CPickup *Pickups_FindPickUpForThisObject(CEntity*);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
int32 Pickups_GenerateNewOne(CVector pos, uint32 modelIndex, uint8 type, uint32 quantity, uint32 rate = 0, bool highPriority = false, char* pText = nil);
void Pickups_RemoveUnnecessaryPickups(const CVector& center, float radius);
extern CPickup Pickups_aPickUps[NUMPICKUPS];
#endif
//- rouz edit (Claude)

extern uint16 AmmoForWeapon[WEAPONTYPE_TOTALWEAPONS + 1];
extern uint16 AmmoForWeapon_OnStreet[WEAPONTYPE_TOTALWEAPONS + 1];
extern uint16 CostOfWeapon[WEAPONTYPE_TOTALWEAPONS + 3];

extern int32 CollectPickupBuffer;

enum ePacmanPickupType
{
	PACMAN_NONE,
	PACMAN_SCRAMBLE,
	PACMAN_RACE,
};

class CPacManPickup
{
public:
	CVector m_vecPosn;
	CObject *m_pObject;
	uint8 m_eType;

	void Update();
};

//+ rouz edit (Claude)
// Formerly the static class CPacManPickups, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern CVector PacManPickups_LastPickUpCoors;
extern int PacManPickups_PillsEatenInRace;
extern bool PacManPickups_bPMActive;
void PacManPickups_Init(void);
void PacManPickups_Update(void);
void PacManPickups_GeneratePMPickUps(CVector, float, int16, uint8);
void PacManPickups_GeneratePMPickUpsForRace(int32);
void PacManPickups_GenerateOnePMPickUp(CVector);
void PacManPickups_Render(void);
void PacManPickups_StartPacManRace(int32);
void PacManPickups_StartPacManRecord(void);
uint32 PacManPickups_QueryPowerPillsEatenInRace(void);
void PacManPickups_ResetPowerPillsEatenInRace(void);
void PacManPickups_ClearPMPickUps(void);
void PacManPickups_CleanUpPacManStuff(void);
void PacManPickups_StartPacManScramble(CVector, float, int16);
uint32 PacManPickups_QueryPowerPillsCarriedByPlayer(void);
void PacManPickups_ResetPowerPillsCarriedByPlayer(void);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CPacManPickup PacManPickups_aPMPickUps[NUMPACMANPICKUPS];
#endif
//- rouz edit (Claude)
