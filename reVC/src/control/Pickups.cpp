//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"
#include "CollisionVector_native.h"

#include "main.h"

#include "Camera.h"
#include "Coronas.h"
#include "Darkel.h"
#include "Entity.h"
#include "Explosion.h"
#include "Font.h"
#include "Garages.h"
#include "General.h"
#include "ModelIndices.h"
#include "Object.h"
#include "Pad.h"
#include "Pickups.h"
#include "PickupLogic.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "DMAudio.h"
#include "Fire.h"
#include "PointLights.h"
#include "Pools.h"
#ifdef FIX_BUGS
#include "Replay.h"
#endif
#include "SaveBuf.h"
#include "Script.h"
#include "Shadows.h"
#include "SpecialFX.h"
#include "Sprite.h"
#include "Timer.h"
#include "WaterLevel.h"
#include "World.h"
#include "Hud.h"
#include "Messages.h"
#include "Streaming.h"
#include "SaveBuf.h"

#ifdef COMPATIBLE_SAVES
#define PICKUPS_SAVE_SIZE 0x4440
#else
#define PICKUPS_SAVE_SIZE sizeof(Pickups_aPickUps)
#endif

CPickup Pickups_aPickUps[NUMPICKUPS];
int16 Pickups_NumMessages;
int32 Pickups_aPickUpsCollected[NUMCOLLECTEDPICKUPS];
int16 Pickups_CollectedPickUpIndex;

int32 Pickups_PlayerOnWeaponPickup;
int32 CollectPickupBuffer;

// unused
bool Pickups_bPickUpcamActivated;
CVehicle *Pickups_pPlayerVehicle;
CVector Pickups_StaticCamCoors;
uint32 Pickups_StaticCamStartTime;

tPickupMessage Pickups_aMessages[NUMPICKUPMESSAGES];

uint16 AmmoForWeapon[WEAPONTYPE_TOTALWEAPONS + 1] = { 0,  1,  1,  1,   1,   1,   1,   1,   1,   1,  1,  1, 8,   8,   8,    8, 8,   68, 24,
													 32, 28, 20, 200, 120, 120, 120, 120, 120, 40, 28, 8, 300, 200, 1000, 1, 400, 36, 0 };

uint16 AmmoForWeapon_OnStreet[WEAPONTYPE_TOTALWEAPONS + 1] = { 0,  1,  1,  1,   1,  1,  1,  1,  1,  1,  1,  1, 4,   4,   4,   4, 4,   34, 12,
															  16, 14, 10, 100, 60, 60, 60, 60, 60, 20, 14, 4, 150, 100, 500, 1, 400, 36, 0 };

uint16 CostOfWeapon[WEAPONTYPE_TOTALWEAPONS + 3] = { 0,     10,    10,   10,   10,   10,    10,   10,    10,  10,  10,  10,   1000, 1000,
													1000,  500,   8000, 250,  400,  1200,  1250, 1250,  800, 800, 650, 1200, 5000, 400,
													10000, 10000, 8000, 8000, 8000, 10000, 1000, 11000, 500, 20,  10,  0 };

struct
{
	uint8 r,g,b;
	float unk;
} aPickupColors[] = {
	{ 128, 128, 128, 1.0f },
	{ 128, 128, 128, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 97, 194, 247, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 27, 89, 130, 1.0f },
	{ 149, 194, 24, 1.0f },
	{ 149, 194, 24, 1.0f },
	{ 45, 155, 90, 1.0f },
	{ 45, 155, 90, 1.0f },
	{ 45, 155, 90, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 255, 227, 79, 1.0f },
	{ 254, 137, 0, 1.0f },
	{ 254, 137, 0, 1.0f },
	{ 249, 131, 215, 1.0f },
	{ 249, 131, 215, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 164, 40, 178, 1.0f },
	{ 69, 69, 69, 1.0f },
	{ 69, 69, 69, 1.0f },
	{ 69, 69, 69, 1.0f },
	{ 255, 100, 100, 1.0f },
	{ 128, 255, 128, 1.0f },
	{ 100, 100, 255, 1.0f },
	{ 255, 255, 100, 1.0f },
	{ 255, 100, 100, 1.0f },
	{ 100, 255, 100, 1.0f },
	{ 255, 255, 255, 1.0f }
};


void
ModifyStringLabelForControlSetting(char *str)
{
	int len = (int)strlen(str);
	if (len <= 2)
		return;

	if (str[len - 2] != '_')
		return;

	switch (CPad::GetPad(0)->Mode) {
		case 0:
		case 1:
			str[len - 1] = 'L';
			break;
		case 2:
			str[len - 1] = 'T';
			break;
		case 3:
			str[len - 1] = 'C';
			break;
		default:
			return;
	}
}

void
CPickup::ExtractAmmoFromPickup(CPlayerPed *player)
{
	eWeaponType weaponType = Pickups_WeaponForModel(m_pObject->GetModelIndex());
	
	if (m_eType == PICKUP_IN_SHOP || !WeaponInfo_IsWeaponSlotAmmoMergeable(WeaponInfo_GetWeaponInfo(weaponType)->m_nWeaponSlot))
		return;

	uint32 ammo = m_nQuantity;
	if (ammo == 0) {
		if (!m_bWasAmmoCollected)
			ammo = AmmoForWeapon_OnStreet[weaponType];
		else
			goto removeAmmo;
	}
	player->GrantAmmo(weaponType, ammo);
	DMAudio_PlayOneShot(player->m_audioEntityId, SOUND_WEAPON_RELOAD, weaponType); // BUG? weapon type as volume, wtf?
removeAmmo:
	m_nQuantity = 0;
	m_bWasAmmoCollected = true;
}

void
CPickup::Remove()
{
	GetRidOfObjects();
	m_bRemoved = true;
	m_eType = PICKUP_NONE;
}

CObject *
CPickup::GiveUsAPickUpObject(CObject **ppObject, CObject **ppExtraObject, int32 handle, int32 extraHandle)
{
	CObject *&object = *ppObject;
	CObject *&extraObject = *ppExtraObject;

	object = extraObject = nil;

	int32 modelId = -1;
	if (((CBaseModelInfo*)ModelRegistry_Get(m_eModelIndex))->GetModelType() == MITYPE_WEAPON) { // rouz edit (ChatGPT)
		CWeaponInfo *weaponInfo = WeaponInfo_GetWeaponInfo((eWeaponType)(uintptr)((CWeaponModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(m_eModelIndex)))->m_atomics[2]); // rouz edit (ChatGPT)
		modelId = weaponInfo->m_nModelId;
		if (modelId == m_eModelIndex)
			modelId = weaponInfo->m_nModel2Id;
	}

	if (handle >= 0) {
		Pools_MakeSureSlotInObjectPoolIsEmpty(handle);
		if (extraHandle >= 0)
			Pools_MakeSureSlotInObjectPoolIsEmpty(extraHandle);
		if (object == nil) {
			// Allocate the handled pickup object without invoking C++ new.
						// Access raw storage through the C store or pool API
			object = ((CObject*)CPool_NewAt(Pools_GetObjectPool(), handle));
						assert(object);
			std::allocator<CObject>().construct(object, m_eModelIndex, false);
		}

		if (extraHandle >= 0 && modelId != -1 && extraObject == nil) {
			// Allocate the handled extra pickup object without invoking C++ new.
						// Access raw storage through the C store or pool API
			extraObject = ((CObject*)CPool_NewAt(Pools_GetObjectPool(), extraHandle));
						assert(extraObject);
			std::allocator<CObject>().construct(extraObject, modelId, false);
		}
	} else {
		// Allocate the pickup object without invoking C++ new.
		CObjectPool *objectPool = Pools_GetObjectPool();
				// Access raw storage through the C store or pool API
		object = ((CObject*)CPool_New(objectPool));
		#ifdef FIX_BUGS
		if (!object) {
						// Access raw storage through the C store or pool API
			for (int32 i = 0; i < CPool_GetSize(objectPool); i++) {
				// Access raw storage through the C store or pool API
				CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
							if (existing && existing->ObjectCreatedBy == TEMP_OBJECT) {
										// Access raw storage through the C store or pool API
					int32 tempHandle = CPool_GetIndex(objectPool, existing);
										World_Remove(existing);
					existing->~CObject();
										// Access raw storage through the C store or pool API
					CPool_Delete(objectPool, existing);
					object = ((CObject*)CPool_NewAt(objectPool, tempHandle));
										break;
				}
			}
		}
		if (object)
#endif
			std::allocator<CObject>().construct(object, m_eModelIndex, false);
		if (modelId != -1) {
			// Allocate the extra pickup object without invoking C++ new.
						// Access raw storage through the C store or pool API
			extraObject = ((CObject*)CPool_New(objectPool));
			#ifdef FIX_BUGS
			if (!extraObject) {
								// Access raw storage through the C store or pool API
				for (int32 i = 0; i < CPool_GetSize(objectPool); i++) {
					// Access raw storage through the C store or pool API
					CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
									if (existing && existing->ObjectCreatedBy == TEMP_OBJECT) {
												// Access raw storage through the C store or pool API
						int32 tempHandle = CPool_GetIndex(objectPool, existing);
												World_Remove(existing);
						existing->~CObject();
												// Access raw storage through the C store or pool API
						CPool_Delete(objectPool, existing);
						extraObject = ((CObject*)CPool_NewAt(objectPool, tempHandle));
												break;
					}
				}
			}
			if (extraObject)
#endif
				std::allocator<CObject>().construct(extraObject, modelId, false);
		}
	}

	if (object == nil) return nil;
	object->ObjectCreatedBy = MISSION_OBJECT;
	object->SetPosition(m_vecPos);
	object->SetOrientation(0.0f, 0.0f, -HALFPI);
	object->GetMatrix().UpdateRW();
	object->UpdateRwFrame();

	object->bAffectedByGravity = false;
	object->bExplosionProof = true;
	object->bUsesCollision = false;
	object->bIsPickup = true;
	object->bAmmoCollected = m_bWasAmmoCollected;
	object->bHasPreRenderEffects = true;

	if (extraObject) {
		extraObject->ObjectCreatedBy = MISSION_OBJECT;
		extraObject->SetPosition(m_vecPos);
		extraObject->SetOrientation(0.0f, 0.0f, -HALFPI);
		extraObject->GetMatrix().UpdateRW();
		extraObject->UpdateRwFrame();

		extraObject->bAffectedByGravity = false;
		extraObject->bExplosionProof = true;
		extraObject->bUsesCollision = false;
		extraObject->bIsPickup = true;
		extraObject->bAmmoCollected = true;
		extraObject->bHasPreRenderEffects = true;
		extraObject->m_nBonusValue = 0;
		extraObject->bPickupObjWithMessage = false;
		extraObject->bOutOfStock = false;
	}

	object->m_nBonusValue = (m_eModelIndex == MI_PICKUP_BONUS || m_eModelIndex == MI_PICKUP_CLOTHES) ? m_nQuantity : 0;

	switch (m_eType)
	{
	case PICKUP_IN_SHOP:
		object->bPickupObjWithMessage = true;
		object->bOutOfStock = false;
		if (m_eModelIndex == MI_PICKUP_HEALTH || m_eModelIndex == MI_PICKUP_ADRENALINE)
			object->m_nCostValue = 0;
		else
			object->m_nCostValue = CostOfWeapon[Pickups_WeaponForModel(m_eModelIndex)];
		break;
	case PICKUP_ON_STREET:
	case PICKUP_ONCE:
	case PICKUP_ONCE_TIMEOUT:
	case PICKUP_COLLECTABLE1:
	case PICKUP_MONEY:
	case PICKUP_MINE_INACTIVE:
	case PICKUP_MINE_ARMED:
	case PICKUP_NAUTICAL_MINE_INACTIVE:
	case PICKUP_NAUTICAL_MINE_ARMED:
	case PICKUP_FLOATINGPACKAGE:
	case PICKUP_ON_STREET_SLOW:
		object->bPickupObjWithMessage = false;
		object->bOutOfStock = false;
		break;
	case PICKUP_IN_SHOP_OUT_OF_STOCK:
		object->bPickupObjWithMessage = false;
		object->bOutOfStock = true;
		object->bRenderScorched = true;
		break;
	case PICKUP_FLOATINGPACKAGE_FLOATING:
	default:
		break;
	}
	return object;
}

bool
CPickup::CanBePickedUp(CPlayerPed *player, int playerId)
{
	assert(m_pObject != nil);
	bool cannotBePickedUp =
		(m_pObject->GetModelIndex() == MI_PICKUP_BODYARMOUR && player->m_fArmour > World_Players[playerId].m_nMaxArmour - 0.2f)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_HEALTH && player->m_fHealth > World_Players[playerId].m_nMaxHealth - 0.2f)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_BRIBE && Wanted_GetWantedLevel(player->m_pWanted) == 0)
		|| (m_pObject->GetModelIndex() == MI_PICKUP_KILLFRENZY && (TheScripts_IsPlayerOnAMission() || Darkel_FrenzyOnGoing() || !Game_nastyGame))
		|| (m_eType == PICKUP_ASSET_REVENUE && m_fRevenue < 10.0f);
	return !cannotBePickedUp;
}

bool
CPickup::Update(CPlayerPed *player, CVehicle *vehicle, int playerId)
{
	bool result = false;
	float waterLevel;

	if (m_pObject) {
		m_pObject->GetMatrix().GetPosition() = m_vecPos;
		if (m_pExtraObject)
			m_pExtraObject->GetMatrix().GetPosition() = m_vecPos;
	}
	if (m_eType == PICKUP_ASSET_REVENUE) {
		uint32 timePassed = (*Timer_GetTimeInMilliseconds()) - m_nTimer;
		m_nTimer = (*Timer_GetTimeInMilliseconds());

		if (Distance(FindPlayerCoors(), m_vecPos) > 10.0f)
			m_fRevenue += float(timePassed * m_nMoneySpeed) / SQR(1200.0f);

		m_fRevenue = Min(m_fRevenue, m_nQuantity);

		m_pObject->m_nCostValue = m_fRevenue < 10 ? 0 : m_fRevenue;
	}

	if (m_bRemoved) {
		if ((*Timer_GetTimeInMilliseconds()) > m_nTimer) {
			// respawn pickup if we're far enough
			float dist = (FindPlayerCoors().x - m_vecPos.x) * (FindPlayerCoors().x - m_vecPos.x) + (FindPlayerCoors().y - m_vecPos.y) * (FindPlayerCoors().y - m_vecPos.y);
			if (dist > 100.0f || m_eType == PICKUP_IN_SHOP && dist > 2.4f) {
				m_pObject = GiveUsAPickUpObject(&m_pObject, &m_pExtraObject, -1, -1);
				if (m_pObject) {
					World_Add(m_pObject);
					m_bRemoved = false;
				}
			}
		}
		return false;
	}

	if (!m_pObject) {
		GiveUsAPickUpObject(&m_pObject, &m_pExtraObject, -1, -1);
		if (m_pObject)
			World_Add(m_pObject);
		if (m_pExtraObject)
			World_Add(m_pExtraObject);
	}

	if (!m_pObject) return false;

	if (!IsMine()) {
		// let's check if we touched the pickup
		bool isPickupTouched = false;
		if (m_pObject->GetModelIndex() == MI_PICKUP_BRIBE) {
			if (vehicle != nil) {
				if (vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f))
					isPickupTouched = true;
			}
			else {
				// Delegate the touch test of the player on foot to the C implementation
				if (Pickups_OnFootTouches(player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z))
					isPickupTouched = true;
			}
		} else if (m_pObject->GetModelIndex() == MI_PICKUP_CAMERA) {
			if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f)) {
				isPickupTouched = true;
			}
		} else if (vehicle == nil) {
			// Delegate the touch test of the player on foot to the C implementation
			if (Pickups_OnFootTouches(player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z))
				isPickupTouched = true;
		}

		if (isPickupTouched) {
			eWeaponType weaponType = Pickups_WeaponForModel(m_pObject->GetModelIndex());
			if (weaponType < WEAPONTYPE_TOTALWEAPONS && Darkel_FrenzyOnGoing()) {
				isPickupTouched = false;
				m_bWasControlMessageShown = false;
			} else if (weaponType < WEAPONTYPE_TOTALWEAPONS && weaponType != WEAPONTYPE_UNARMED) {
				uint32 slot = WeaponInfo_GetWeaponInfo(weaponType)->m_nWeaponSlot;
				eWeaponType plrWeaponSlot = FindPlayerPed()->GetWeapon(slot).m_eWeaponType;
				if (plrWeaponSlot != weaponType) {
					if (Streaming_modelInfo[m_pObject->GetModelIndex()].m_loadState == STREAMSTATE_LOADED) { // rouz edit (ChatGPT)
						if (plrWeaponSlot == WEAPONTYPE_UNARMED || (FindPlayerPed()->GetWeapon(slot).m_nAmmoTotal == 0 && !WeaponInfo_IsWeaponSlotAmmoMergeable(slot))) {
							if ((*Timer_GetTimeInMilliseconds()) - FindPlayerPed()->m_nPadDownPressedInMilliseconds < 1500) {
								Pickups_PlayerOnWeaponPickup = 6;
								isPickupTouched = false;
							}
						} else {
							Pickups_PlayerOnWeaponPickup = 6;
							if (WeaponInfo_IsWeaponSlotAmmoMergeable(slot)) {
								if (m_eType == PICKUP_ONCE_TIMEOUT || m_eType == PICKUP_ONCE || m_eType == PICKUP_ON_STREET) {
									ExtractAmmoFromPickup(player);
									FindPlayerPed()->GetWeapon(slot).Reload();
								}
							}
							if (!m_bWasControlMessageShown) {
								switch (CPad::GetPad(0)->Mode)
								{
								case 0:
								case 1:
									Hud_SetHelpMessage(Text_Get(&TheText,"PU_CF1"), false);
									break;
								case 2:
									Hud_SetHelpMessage(Text_Get(&TheText,"PU_CF3"), false);
									break;
								case 3:
									Hud_SetHelpMessage(Text_Get(&TheText,"PU_CF4"), false);
									break;
								default:
									break;
								}
								m_bWasControlMessageShown = true;
							}
							if (CollectPickupBuffer == 0)
								isPickupTouched = false;
							if ((*Timer_GetTimeInMilliseconds()) - FindPlayerPed()->m_nPadDownPressedInMilliseconds < 1500)
								isPickupTouched = false;
						}
					} else
						isPickupTouched = false;
				}
			}
		} else
			m_bWasControlMessageShown = false;

		// if we didn't then we've got nothing to do
		if (isPickupTouched && CanBePickedUp(player, playerId)) {
			if (m_pObject->GetModelIndex() != MI_PICKUP_PROPERTY && m_pObject->GetModelIndex() != MI_PICKUP_PROPERTY_FORSALE)
				CPad::GetPad(0)->StartShake(120, 100);

			eWeaponType weaponType = Pickups_WeaponForModel(m_pObject->GetModelIndex());
			switch (m_eType)
			{
			case PICKUP_IN_SHOP:
				if (World_Players[playerId].m_nMoney < CostOfWeapon[weaponType])
					Garages_TriggerMessage("PU_MONY", -1, 6000, -1);
				else {
					World_Players[playerId].m_nMoney -= CostOfWeapon[weaponType];
					if (!Pickups_GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
						if (!player->DoesPlayerWantNewWeapon(weaponType, false))
							break; 
						player->GiveWeapon(weaponType, AmmoForWeapon[weaponType]);
						player->m_nSelectedWepSlot = player->GetWeaponSlot(weaponType);
						DMAudio_PlayFrontEndSound(SOUND_PICKUP_WEAPON_BOUGHT, m_pObject->GetModelIndex() - MI_GRENADE);
					}
					result = true;
					Remove();
				}
				break;
			case PICKUP_ON_STREET:
			case PICKUP_ON_STREET_SLOW:
				if (!Pickups_GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
					if (!player->DoesPlayerWantNewWeapon(weaponType, false))
						break;
					if (weaponType != WEAPONTYPE_UNARMED) {
						player->GiveWeapon(weaponType, m_nQuantity != 0 ? m_nQuantity : (m_bWasAmmoCollected ? 0 : AmmoForWeapon_OnStreet[weaponType]), true);

						if (player->m_nSelectedWepSlot == player->GetWeaponSlot(WEAPONTYPE_UNARMED)) 
							player->m_nSelectedWepSlot = player->GetWeaponSlot(weaponType);

						DMAudio_PlayFrontEndSound(SOUND_PICKUP_WEAPON, m_pObject->GetModelIndex() - MI_GRENADE);
					} else if (m_pObject->GetModelIndex() == MI_PICKUP_CAMERA && vehicle != nil) {
						DMAudio_PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
						Pickups_bPickUpcamActivated = true;
						Pickups_pPlayerVehicle = FindPlayerVehicle();
						Pickups_StaticCamCoors = m_pObject->GetPosition();
						Pickups_StaticCamStartTime = (*Timer_GetTimeInMilliseconds());
					}
				}
				if (m_eType == PICKUP_ON_STREET)
					m_nTimer = (*Timer_GetTimeInMilliseconds()) + 30000;
				else if (m_eType == PICKUP_ON_STREET_SLOW) {
					if (MI_PICKUP_BRIBE == m_pObject->GetModelIndex())
						m_nTimer = (*Timer_GetTimeInMilliseconds()) + 300000;
					else
						m_nTimer = (*Timer_GetTimeInMilliseconds()) + 720000;
				}

				result = true;
				GetRidOfObjects();
				m_bRemoved = true;
				break;
			case PICKUP_ONCE:
			case PICKUP_ONCE_TIMEOUT:
			case PICKUP_ONCE_TIMEOUT_SLOW:
				if (!Pickups_GivePlayerGoodiesWithPickUpMI(m_pObject->GetModelIndex(), playerId)) {
					if (!player->DoesPlayerWantNewWeapon(weaponType, false)) {
						ExtractAmmoFromPickup(player);
						break;
					}

					if (weaponType != WEAPONTYPE_UNARMED) {
						player->GiveWeapon(weaponType, m_nQuantity != 0 ? m_nQuantity : (m_bWasAmmoCollected ? 0 : AmmoForWeapon[weaponType]), true);
						if (player->m_nSelectedWepSlot == player->GetWeaponSlot(WEAPONTYPE_UNARMED))
							player->m_nSelectedWepSlot = player->GetWeaponSlot(weaponType);
					}
					if (MI_PICKUP_SAVEGAME != m_pObject->GetModelIndex())
						DMAudio_PlayFrontEndSound(SOUND_PICKUP_WEAPON, m_pObject->GetModelIndex() - MI_GRENADE);
				}
				result = true;
				Remove();
				break;
			case PICKUP_COLLECTABLE1:
				World_Players[playerId].m_nCollectedPackages++;
				World_Players[playerId].m_nMoney += 100;

				if (World_Players[playerId].m_nCollectedPackages == World_Players[playerId].m_nTotalPackages) {
					printf("All collectables have been picked up\n");
					Garages_TriggerMessage("CO_ALL", -1, 5000, -1);
					World_Players[World_PlayerInFocus].m_nMoney += 100000;
				} else
					Garages_TriggerMessage("CO_ONE", World_Players[World_PlayerInFocus].m_nCollectedPackages, 5000, World_Players[World_PlayerInFocus].m_nTotalPackages);

				result = true;
				Remove();
				DMAudio_PlayFrontEndSound(SOUND_PICKUP_HIDDEN_PACKAGE, 0);
				break;
			case PICKUP_MONEY:
				World_Players[playerId].m_nMoney += m_nQuantity;
				sprintf(gString, "$%d", m_nQuantity);
#ifdef MONEY_MESSAGES
				// Enter the C money-message algorithm with the original arguments
				MoneyMessages_RegisterOneLegacy(m_vecPos + CVector(0.0f, 0.0f, 1.0f), gString, 0, 255, 0, 0.5f, 0.5f);
#endif
				result = true;
				Remove();
				DMAudio_PlayFrontEndSound(SOUND_PICKUP_MONEY, 0);
				player->Say(SOUND_PED_MUGGING);
				break;
			case PICKUP_ASSET_REVENUE:
				World_Players[World_PlayerInFocus].m_nMoney += m_fRevenue;
				m_fRevenue = 0.0f;
				DMAudio_PlayFrontEndSound(SOUND_PICKUP_MONEY, 0);
				break;
			case PICKUP_PROPERTY_LOCKED:
				if (!m_bWasControlMessageShown) {
					m_bWasControlMessageShown = true;
					Hud_SetHelpMessage(Text_Get(&TheText,m_sTextKey), false);
				}
				break;
			case PICKUP_PROPERTY_FORSALE:
				ModifyStringLabelForControlSetting(m_sTextKey);
				Messages_InsertNumberInString(Text_Get(&TheText,m_sTextKey), m_nQuantity,
					0, 0, 0, 0, 0, gUString);
				if (!Hud_IsHelpMessageBeingDisplayed())
					Hud_SetHelpMessage(gUString, false);
				if (CollectPickupBuffer == 0)
					break;
				if (TheScripts_IsPlayerOnAMission())
					Hud_SetHelpMessage(Text_Get(&TheText,"PROP_2"), true);
				else {
					if (World_Players[World_PlayerInFocus].m_nMoney >= m_nQuantity) {
						World_Players[World_PlayerInFocus].m_nMoney -= m_nQuantity;
						Hud_SetHelpMessage(nil, true);
						result = true;
						Remove();
						break;
					}
					Hud_SetHelpMessage(Text_Get(&TheText,"PROP_1"), true);
				}
				break;
			default:
				break;
			}
		}
	} else {
		switch (m_eType)
		{
		case PICKUP_MINE_INACTIVE:
			if (vehicle != nil && !vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f)) {
				m_eType = PICKUP_MINE_ARMED;
				m_nTimer = (*Timer_GetTimeInMilliseconds()) + 10000;
			}
			break;
		case PICKUP_NAUTICAL_MINE_INACTIVE:
		{
			if (WaterLevel_GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false))
				m_pObject->GetMatrix().GetPosition().z = waterLevel + 0.6f;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();

			bool touched = false;
						// Access raw storage through the C store or pool API
			for (int32 i = CPool_GetSize(Pools_GetVehiclePool())-1; i >= 0; i--) {
				// Access raw storage through the C store or pool API
				CVehicle *vehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
							if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 1.5f)) {
					touched = true;
#ifdef FIX_BUGS
					break; // added break here
#endif
				}
			}

			if (!touched) {
				m_eType = PICKUP_NAUTICAL_MINE_ARMED;
				m_nTimer = (*Timer_GetTimeInMilliseconds()) + 10000;
			}
			break;
		}
		case PICKUP_NAUTICAL_MINE_ARMED:
			if (WaterLevel_GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false))
				m_pObject->GetMatrix().GetPosition().z = waterLevel + 0.6f;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			// no break here
		case PICKUP_MINE_ARMED:
		{
			bool explode = false;
			if ((*Timer_GetTimeInMilliseconds()) > m_nTimer)
				explode = true;
#ifdef FIX_BUGS
			else// added else here since vehicle lookup is useless
#endif
			{
								// Access raw storage through the C store or pool API
				for (int32 i = CPool_GetSize(Pools_GetVehiclePool())-1; i >= 0; i--) {
					// Access raw storage through the C store or pool API
					CVehicle *vehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
									if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 1.5f)) {
						explode = true;
#ifdef FIX_BUGS
						break; // added break here
#endif
					}
				}
			}
			if (explode) {
				Explosion_AddExplosion(nil, nil, EXPLOSION_MINE, m_pObject->GetPosition(), 0);
				Remove();
			}
			break;
		}
		case PICKUP_FLOATINGPACKAGE:
			m_pObject->m_vecMoveSpeed.z -= 0.01f * (*Timer_GetTimeStep());
			m_pObject->GetMatrix().GetPosition() += m_pObject->GetMoveSpeed() * (*Timer_GetTimeStep());

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			if (WaterLevel_GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false) && waterLevel >= m_pObject->GetPosition().z)
				m_eType = PICKUP_FLOATINGPACKAGE_FLOATING;
			break;
		case PICKUP_FLOATINGPACKAGE_FLOATING:
			if (WaterLevel_GetWaterLevel(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z + 5.0f, &waterLevel, false))
				m_pObject->GetMatrix().GetPosition().z = waterLevel;

			m_pObject->GetMatrix().UpdateRW();
			m_pObject->UpdateRwFrame();
			if (vehicle != nil && vehicle->IsSphereTouchingVehicle(m_pObject->GetPosition().x, m_pObject->GetPosition().y, m_pObject->GetPosition().z, 2.0f)) {
				Remove();
				result = true;
				DMAudio_PlayFrontEndSound(SOUND_PICKUP_FLOAT_PACKAGE, 0);
			}
			break;
		default: break;
		}
	}

	if (!m_bRemoved && (m_eType == PICKUP_ONCE_TIMEOUT || m_eType == PICKUP_ONCE_TIMEOUT_SLOW || m_eType == PICKUP_MONEY) && (*Timer_GetTimeInMilliseconds()) > m_nTimer)
		Remove();
	
	return result;
}

void
CPickup::ProcessGunShot(CVector *vec1, CVector *vec2)
{
	// Retain the line record at its original construction boundary
	CColLine line;
	// Initialize the line at the former parameterized constructor boundary
	ColLine_Init(&line,COLLISION_POINT_REF(*vec1),COLLISION_POINT_REF(*vec2));
	if (m_pObject) {
		CColSphere sphere;
		sphere.bounds.radius = 4.0f;
		sphere.bounds.center = CollisionVector_FromNative(m_pObject->GetPosition());
		if (Collision_TestLineSphere(line, sphere)) {
			Explosion_AddExplosion(nil, nil, EXPLOSION_MINE, m_pObject->GetPosition(), 0);
			World_Remove(m_pObject);
			// Destroy and release the pickup object without invoking C++ delete.
			m_pObject->~CObject();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetObjectPool(), m_pObject);
						m_pObject = nil;
			m_bRemoved = true;
			m_eType = PICKUP_NONE;
		}
	}
}

void
CPickup::GetRidOfObjects()
{
	if (m_pObject) {
		World_Remove(m_pObject);
		// Destroy and release the pickup object without invoking C++ delete.
		m_pObject->~CObject();
				// Access raw storage through the C store or pool API
		CPool_Delete(Pools_GetObjectPool(), m_pObject);
				m_pObject = nil;
	}
	if (m_pExtraObject) {
		World_Remove(m_pExtraObject);
		// Destroy and release the extra pickup object without invoking C++ delete.
		m_pExtraObject->~CObject();
				// Access raw storage through the C store or pool API
		CPool_Delete(Pools_GetObjectPool(), m_pExtraObject);
				m_pExtraObject = nil;
	}
}

void
Pickups_Init(void)
{
	Pickups_NumMessages = 0;
	for (int i = 0; i < NUMPICKUPS; i++) {
		Pickups_aPickUps[i].m_eType = PICKUP_NONE;
		Pickups_aPickUps[i].m_nIndex = 1;
		Pickups_aPickUps[i].m_pObject = nil;
		Pickups_aPickUps[i].m_pExtraObject = nil;
	}

	for (int i = 0; i < NUMCOLLECTEDPICKUPS; i++)
		Pickups_aPickUpsCollected[i] = 0;

	Pickups_CollectedPickUpIndex = 0;
}

bool
Pickups_TestForPickupsInBubble(CVector pos, float range)
{
	// Delegate to the C implementation
	return PickupLogic_TestForPickupsInBubble(Pickups_aPickUps, NUMPICKUPS, pos.x, pos.y, pos.z, range);
}

bool
Pickups_TryToMerge_WeaponType(CVector pos, eWeaponType weapon, uint8 type, uint32 quantity, bool unused) {
	// Delegate to the C implementation
	return Pickups_TryToMerge(Pickups_aPickUps, NUMPICKUPS, pos.x, pos.y, pos.z, type, Pickups_ModelForWeapon(weapon), quantity);
}

bool
Pickups_IsPickUpPickedUp(int32 pickupId)
{
	// Delegate to the C implementation
	return PickupLogic_IsPickUpPickedUp((int32_t*)Pickups_aPickUpsCollected, NUMCOLLECTEDPICKUPS, pickupId);
}

void
Pickups_PassTime(uint32 time)
{
	// Delegate to the C implementation
	PickupLogic_PassTime(Pickups_aPickUps, NUMPICKUPS, time);
}

int32
Pickups_GetActualPickupIndex(int32 index)
{
	// Delegate to the C implementation
	return PickupLogic_GetActualPickupIndex(Pickups_aPickUps, index);
}

bool
Pickups_GivePlayerGoodiesWithPickUpMI(int16 modelIndex, int playerIndex)
{
	CPlayerPed *player;

	if (playerIndex <= 0) player = World_Players[World_PlayerInFocus].m_pPed;
	else player = World_Players[playerIndex].m_pPed;

	if (modelIndex == MI_PICKUP_ADRENALINE) {
		player->m_bAdrenalineActive = true;
		player->m_nAdrenalineTime = (*Timer_GetTimeInMilliseconds()) + 20000;
		player->m_fCurrentStamina = player->m_fMaxStamina;
		DMAudio_PlayFrontEndSound(SOUND_PICKUP_ADRENALINE, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BODYARMOUR) {
		player->m_fArmour = World_Players[playerIndex].m_nMaxArmour;
		DMAudio_PlayFrontEndSound(SOUND_PICKUP_ARMOUR, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_INFO) {
		DMAudio_PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_HEALTH) {
		player->m_fHealth = World_Players[playerIndex].m_nMaxHealth;
		DMAudio_PlayFrontEndSound(SOUND_PICKUP_HEALTH, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BONUS) {
		DMAudio_PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_BRIBE) {
		int32 level = Max(Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) - 1, 0);
		player->SetWantedLevel(level);
		DMAudio_PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	} else if (modelIndex == MI_PICKUP_KILLFRENZY) {
		DMAudio_PlayFrontEndSound(SOUND_PICKUP_BONUS, 0);
		return true;
	}
	return false;
}

void
Pickups_RemovePickUp(int32 pickupIndex)
{
	int32 index = Pickups_GetActualPickupIndex(pickupIndex);
	if (index == -1) return;

	if (Pickups_aPickUps[index].m_pObject) {
		World_Remove(Pickups_aPickUps[index].m_pObject);
		// Destroy and release the pickup object without invoking C++ delete.
		Pickups_aPickUps[index].m_pObject->~CObject();
				// Access raw storage through the C store or pool API
		CPool_Delete(Pools_GetObjectPool(), Pickups_aPickUps[index].m_pObject);
				Pickups_aPickUps[index].m_pObject = nil;
	}
	if (Pickups_aPickUps[index].m_pExtraObject) {
		World_Remove(Pickups_aPickUps[index].m_pExtraObject);
		// Destroy and release the extra pickup object without invoking C++ delete.
		Pickups_aPickUps[index].m_pExtraObject->~CObject();
				// Access raw storage through the C store or pool API
		CPool_Delete(Pools_GetObjectPool(), Pickups_aPickUps[index].m_pExtraObject);
				Pickups_aPickUps[index].m_pExtraObject = nil;
	}
	Pickups_aPickUps[index].m_eType = PICKUP_NONE;
	Pickups_aPickUps[index].m_bRemoved = true;
}

int32
Pickups_GenerateNewOne(CVector pos, uint32 modelIndex, uint8 type, uint32 quantity, uint32 rate, bool highPriority, char* pText)
{
	bool bFreeFound = false;
	int32 slot = 0;

	if (type == PICKUP_FLOATINGPACKAGE || type == PICKUP_NAUTICAL_MINE_INACTIVE || highPriority) {
		for (slot = NUMPICKUPS-1; slot >= 0; slot--) {
			if (Pickups_aPickUps[slot].m_eType == PICKUP_NONE) {
				bFreeFound = true;
				break;
			}
		}
	}
	if (!bFreeFound) {
		for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
			if (Pickups_aPickUps[slot].m_eType == PICKUP_NONE) {
				bFreeFound = true;
				break;
			}
		}
	}

	if (!bFreeFound) {
		for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
			if (Pickups_aPickUps[slot].m_eType == PICKUP_MONEY) break;
		}

		if (slot >= NUMGENERALPICKUPS) {
			for (slot = 0; slot < NUMGENERALPICKUPS; slot++) {
				if (Pickups_aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT || Pickups_aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT_SLOW) break;
			}

			if (slot >= NUMGENERALPICKUPS) return -1;
			Pickups_aPickUps[slot].GetRidOfObjects();
		}
	}

	if (slot >= NUMPICKUPS) return -1;

	Pickups_aPickUps[slot].m_eType = type;
	Pickups_aPickUps[slot].m_bRemoved = false;
	Pickups_aPickUps[slot].m_nQuantity = quantity;
	Pickups_aPickUps[slot].m_nMoneySpeed = rate;
	Pickups_aPickUps[slot].m_fRevenue = 0.0f;
	Pickups_aPickUps[slot].m_nTimer = (*Timer_GetTimeInMilliseconds());
	Pickups_aPickUps[slot].m_bWasAmmoCollected = highPriority;
	Pickups_aPickUps[slot].m_bWasControlMessageShown = false;
	if (type == PICKUP_ONCE_TIMEOUT)
		Pickups_aPickUps[slot].m_nTimer = (*Timer_GetTimeInMilliseconds()) + 20000;
	else if (type == PICKUP_ONCE_TIMEOUT_SLOW)
		Pickups_aPickUps[slot].m_nTimer = (*Timer_GetTimeInMilliseconds()) + 120000;
	else if (type == PICKUP_MONEY)
		Pickups_aPickUps[slot].m_nTimer = (*Timer_GetTimeInMilliseconds()) + 30000;
	else if (type == PICKUP_MINE_INACTIVE || type == PICKUP_MINE_ARMED) {
		Pickups_aPickUps[slot].m_eType = PICKUP_MINE_INACTIVE;
		Pickups_aPickUps[slot].m_nTimer = (*Timer_GetTimeInMilliseconds()) + 1500;
	} else if (type == PICKUP_NAUTICAL_MINE_INACTIVE || type == PICKUP_NAUTICAL_MINE_ARMED) {
		Pickups_aPickUps[slot].m_eType = PICKUP_NAUTICAL_MINE_INACTIVE;
		Pickups_aPickUps[slot].m_nTimer = (*Timer_GetTimeInMilliseconds()) + 1500;
	}
	Pickups_aPickUps[slot].m_eModelIndex = modelIndex;
	if (pText)
		strncpy(Pickups_aPickUps[slot].m_sTextKey, pText, 8);
	else
		Pickups_aPickUps[slot].m_sTextKey[0] = '\0';

	Pickups_aPickUps[slot].m_vecPos = pos;
	Pickups_aPickUps[slot].m_pObject = Pickups_aPickUps[slot].GiveUsAPickUpObject(&Pickups_aPickUps[slot].m_pObject, &Pickups_aPickUps[slot].m_pExtraObject, -1, -1);
	if (Pickups_aPickUps[slot].m_pObject)
		World_Add(Pickups_aPickUps[slot].m_pObject);
	if (Pickups_aPickUps[slot].m_pExtraObject)
		World_Add(Pickups_aPickUps[slot].m_pExtraObject);
	return Pickups_GetNewUniquePickupIndex(slot);
}

int32
Pickups_GenerateNewOne_WeaponType(CVector pos, eWeaponType weaponType, uint8 type, uint32 quantity)
{
	return Pickups_GenerateNewOne(pos, Pickups_ModelForWeapon(weaponType), type, quantity);
}

int32
Pickups_GetNewUniquePickupIndex(int32 slot)
{
	if (Pickups_aPickUps[slot].m_nIndex >= 0xFFFE)
		Pickups_aPickUps[slot].m_nIndex = 1;
	else
		Pickups_aPickUps[slot].m_nIndex++;
	return slot | (Pickups_aPickUps[slot].m_nIndex << 16);
}

int32
Pickups_ModelForWeapon(eWeaponType weaponType)
{
	return WeaponInfo_GetWeaponInfo(weaponType)->m_nModelId;
}

eWeaponType
Pickups_WeaponForModel(int32 model)
{
	if (model == MI_PICKUP_BODYARMOUR) return WEAPONTYPE_ARMOUR;
	if (model == MI_PICKUP_HEALTH) return WEAPONTYPE_HEALTH;
	if (model == MI_PICKUP_ADRENALINE) return WEAPONTYPE_ARMOUR;
	if (model == -1) return WEAPONTYPE_UNARMED;
	return (eWeaponType)(uintptr)((CWeaponModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(model)))->m_atomics[2]; // rouz edit (ChatGPT)
}

void
Pickups_AddToCollectedPickupsArray(int32 index)
{
	Pickups_aPickUpsCollected[Pickups_CollectedPickUpIndex++] = index | (Pickups_aPickUps[index].m_nIndex << 16);
	if (Pickups_CollectedPickUpIndex >= NUMCOLLECTEDPICKUPS)
		Pickups_CollectedPickUpIndex = 0;
}

void
Pickups_Update()
{
#ifdef FIX_BUGS // RIP speedrunning (solution from SA)
	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;
#endif
#ifdef CAMERA_PICKUP
	if ( Pickups_bPickUpcamActivated ) // taken from PS2
	{
		float dist = Distance2D(Pickups_StaticCamCoors, FindPlayerCoors());
		float mult;
		if ( dist < 10.0f )
			mult = 1.0f - (dist / 10.0f );
		else
			mult = 0.0f;

		CVector pos = Pickups_StaticCamCoors;
		pos.z += (CollisionVector_ToNative(Box_GetSize(&Pickups_pPlayerVehicle->GetColModel()->boundingBox)).z + 2.0f) * mult;

		if ( ((*Timer_GetTimeInMilliseconds()) - Pickups_StaticCamStartTime) > 750 )
		{
			TheCamera.SetCamPositionForFixedMode(pos, CVector(0.0f, 0.0f, 0.0f));
			TheCamera.TakeControl(FindPlayerVehicle(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_SCRIPT);
		}

		if ( FindPlayerVehicle() != Pickups_pPlayerVehicle || Distance(Pickups_StaticCamCoors, FindPlayerCoors()) > 40.0f
			|| (((*Timer_GetTimeInMilliseconds()) - Pickups_StaticCamStartTime) > 60000) )
		{
			TheCamera.RestoreWithJumpCut();
			Pickups_bPickUpcamActivated = false;
		}
	}
#endif
	if (CPad::GetPad(0)->CollectPickupJustDown())
		CollectPickupBuffer = 6;
	else
		CollectPickupBuffer = General_MaxInt(0, CollectPickupBuffer - 1);

	if (Pickups_PlayerOnWeaponPickup)
		Pickups_PlayerOnWeaponPickup = General_MaxInt(0, Pickups_PlayerOnWeaponPickup - 1);

#define PICKUPS_FRAME_SPAN (6)
#ifdef FIX_BUGS
	for (uint32 i = NUMGENERALPICKUPS * ((*Timer_GetFrameCounter()) % PICKUPS_FRAME_SPAN) / PICKUPS_FRAME_SPAN; i < NUMGENERALPICKUPS * ((*Timer_GetFrameCounter()) % PICKUPS_FRAME_SPAN + 1) / PICKUPS_FRAME_SPAN; i++) {
#else // BUG: this code can only reach 318 out of 320 pickups
	for (uint32 i = NUMGENERALPICKUPS / PICKUPS_FRAME_SPAN * ((*Timer_GetFrameCounter()) % PICKUPS_FRAME_SPAN); i < NUMGENERALPICKUPS / PICKUPS_FRAME_SPAN * ((*Timer_GetFrameCounter()) % PICKUPS_FRAME_SPAN + 1); i++) {
#endif
		if (Pickups_aPickUps[i].m_eType != PICKUP_NONE && Pickups_aPickUps[i].Update(FindPlayerPed(), FindPlayerVehicle(), World_PlayerInFocus))
			Pickups_AddToCollectedPickupsArray(i);
	}
#undef PICKUPS_FRAME_SPAN
	for (uint32 i = NUMGENERALPICKUPS; i < NUMPICKUPS; i++) {
		if (Pickups_aPickUps[i].m_eType != PICKUP_NONE && Pickups_aPickUps[i].Update(FindPlayerPed(), FindPlayerVehicle(), World_PlayerInFocus))
			Pickups_AddToCollectedPickupsArray(i);
	}
}

CPickup*
Pickups_FindPickUpForThisObject(CEntity *object)
{
	for (uint32 i = 0; i < NUMPICKUPS; i++) {
		if (Pickups_aPickUps[i].m_eType != PICKUP_NONE && (Pickups_aPickUps[i].m_pObject == object || Pickups_aPickUps[i].m_pExtraObject == object)) {
			return &Pickups_aPickUps[i];
		}
	}
	return &Pickups_aPickUps[0];
}

void
Pickups_DoPickUpEffects(CEntity *entity)
{
	CPickup *pickup = Pickups_FindPickUpForThisObject(entity);

	if (entity->GetModelIndex() == MI_PICKUP_KILLFRENZY)
		entity->bDoNotRender = TheScripts_IsPlayerOnAMission() || Darkel_FrenzyOnGoing() || !Game_nastyGame;

	if (!entity->bDoNotRender) {
		// Delegate the sine of the pulse to the C implementation
		float modifiedSin = 0.3f * (Pickups_PulseSin((uint32)((*Timer_GetTimeInMilliseconds()) + (uintptr)entity), 0x7FF) + 1.0f);

#ifdef FIX_BUGS
		int16 colorId = 0;
#else
		int16 colorId;
#endif
		bool doInnerGlow = false;
		bool doOuterGlow = true;

		if (entity->GetModelIndex() == MI_PICKUP_ADRENALINE || entity->GetModelIndex() == MI_PICKUP_CAMERA) {
			colorId = WEAPONTYPE_TOTALWEAPONS;
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_BODYARMOUR) {
			colorId = WEAPONTYPE_ARMOUR;
		} else if (entity->GetModelIndex() == MI_PICKUP_BRIBE) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_INFO || entity->GetModelIndex() == MI_PICKUP_KILLFRENZY) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_HEALTH || entity->GetModelIndex() == MI_PICKUP_BONUS) {
			colorId = WEAPONTYPE_HEALTH;
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_PROPERTY) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_PROPERTY_FORSALE) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_REVENUE) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_SAVEGAME) {
			doInnerGlow = true;
			doOuterGlow = false;
		} else if (entity->GetModelIndex() == MI_PICKUP_CLOTHES) {
			colorId = WEAPONTYPE_TOTALWEAPONS;
			doOuterGlow = false;
			doInnerGlow = true;
		} else
			colorId = Pickups_WeaponForModel(entity->GetModelIndex());

		const CVector& pos = pickup->m_vecPos;
		if (doOuterGlow) {
			bool corona1 = false;
			bool corona2 = false;
			int timerVal = ((*Timer_GetTimeInMilliseconds()) >> 9) & 7;

			// Delegate the state of the flickering to the C implementation
			int phase1 = Pickups_CoronaPhase(timerVal);
			if (phase1 == 1)
				corona1 = (General_GetRandomNumber() & 3) != 0;
			else
				corona1 = phase1 == 2;

			timerVal = (timerVal - 1) & 7;
			int phase2 = Pickups_CoronaPhase(timerVal);
			if (phase2 == 1)
				corona2 = (General_GetRandomNumber() & 3) != 0;
			else
				corona2 = phase2 == 2;

			if (((CObject*)entity)->bAmmoCollected) {
				corona2 = false;
				corona1 = false;
			}

			if (corona1) {
				CCoronas::RegisterCorona((uintptr)entity,
					aPickupColors[colorId].r * 0.45f, aPickupColors[colorId].g * 0.45f, aPickupColors[colorId].b * 0.45f,
					255, pos, 0.76f, 65.0f,
					CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF,
					0.0f, false, -0.4f);
				CShadows::StoreStaticShadow((uintptr)entity,
					SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0,
					aPickupColors[colorId].r * 0.3f, aPickupColors[colorId].g * 0.3f, aPickupColors[colorId].b * 0.3f,
					4.0f, 1.0f, 40.0f, false, 0.0f);
				float radius = Pickups_LightRadius(General_GetRandomNumber());
				// Enter the C point-light algorithm with the original owner expressions
				PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_POINT, pos, CVector(0.0f, 0.0f, 0.0f), radius, aPickupColors[colorId].r / 256.0f, aPickupColors[colorId].g / 256.0f, aPickupColors[colorId].b / 256.0f, POINTLIGHTS_FOG_NONE, true);
			} else
				CCoronas::RegisterCorona((uintptr)entity, 0, 0, 0, 255, pos, 0.57f, 65.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);

			if (corona2) {
				CCoronas::RegisterCorona(
					(uintptr)entity + 1,
					aPickupColors[colorId].r * 0.55f, aPickupColors[colorId].g * 0.55f, aPickupColors[colorId].b * 0.55f,
					255,
					pos,
					0.6f,
					65.0f,
					CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF,
					0.0f, false, -0.4f);
				if (!corona1)
					CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0,
						aPickupColors[colorId].r * 0.25f, aPickupColors[colorId].g * 0.25f, aPickupColors[colorId].b * 0.25f,
						4.0f, 1.0f, 40.0f, false, 0.0f);
			} else
				CCoronas::RegisterCorona((uintptr)entity + 1, 0, 0, 0, 255, pos, 0.45f, 65.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
		}

		CObject *object = (CObject*)entity;
		if (object->bPickupObjWithMessage || object->bOutOfStock || object->m_nBonusValue || object->m_nCostValue) {
			
			float dist = Distance2D(pos, TheCamera.GetPosition());
			const float MAXDIST = 14.0f;

			if (dist < MAXDIST && Pickups_NumMessages < NUMPICKUPMESSAGES) {
				RwV3d vecOut;
				float fDistX, fDistY;
				if (([](const RwV3d &spriteInput,RwV3d *spriteOutput,float *spriteWidth,float *spriteHeight,bool spriteFarClip){
    // Keep actual native input and output subobjects alive through the complete synchronous C projection
    // Use the complete C sprite operation with the original argument values
    return Sprite_CalcScreenCoors(Sprite_VectorRef(&spriteInput.x,&spriteInput.y,&spriteInput.z),Sprite_VectorOut(&spriteOutput->x,&spriteOutput->y,&spriteOutput->z),spriteWidth,spriteHeight,spriteFarClip);
})(entity->GetPosition() + CVector(0.0f, 0.0f, 0.7f), &vecOut, &fDistX, &fDistY, true)) {
					Pickups_aMessages[Pickups_NumMessages].m_pos.x = vecOut.x;
					Pickups_aMessages[Pickups_NumMessages].m_pos.y = vecOut.y;
					Pickups_aMessages[Pickups_NumMessages].m_dist.x = fDistX;
					Pickups_aMessages[Pickups_NumMessages].m_dist.y = fDistY;
					Pickups_aMessages[Pickups_NumMessages].m_weaponType = Pickups_WeaponForModel(entity->GetModelIndex());
					Pickups_aMessages[Pickups_NumMessages].m_color.red = aPickupColors[colorId].r;
					Pickups_aMessages[Pickups_NumMessages].m_color.green = aPickupColors[colorId].g;
					Pickups_aMessages[Pickups_NumMessages].m_color.blue = aPickupColors[colorId].b;
					Pickups_aMessages[Pickups_NumMessages].m_color.alpha = Pickups_MessageAlpha(dist, MAXDIST);
					Pickups_aMessages[Pickups_NumMessages].m_bOutOfStock = object->bOutOfStock;
					Pickups_aMessages[Pickups_NumMessages].m_quantity = object->m_nBonusValue;
					Pickups_aMessages[Pickups_NumMessages].money = object->m_nCostValue;
					Pickups_NumMessages++;
				}
			}
		}

		uint32 model = entity->GetModelIndex();
		CColModel *colModel = entity->GetColModel();
		CVector colLength = CollisionVector_ToNative(colModel->boundingBox.max) - CollisionVector_ToNative(colModel->boundingBox.min);
		float maxDimension = General_MaxFloat(colLength.x, General_MaxFloat(colLength.y, colLength.z));

		// Delegate the scale and the angle to the C implementation
		float scale = Pickups_ModelScale(maxDimension);
		if (model == MI_MINIGUN || model == MI_MINIGUN2)
			scale = 1.2f;

		float angle = Pickups_SpinAngle((*Timer_GetTimeInMilliseconds()), 0x7FF);
		// Delegate the cosine and sine of the spin to the C implementation
		float c, s;
		Pickups_SpinCosSin(angle, scale, &c, &s);

		// we know from SA they were setting each field manually like this
		entity->GetMatrix().rx = c;
		entity->GetMatrix().ry = s;
		entity->GetMatrix().rz = 0.0f;
		entity->GetMatrix().fx = -s;
		entity->GetMatrix().fy = c;
		entity->GetMatrix().fz = 0.0f;
		entity->GetMatrix().ux = 0.0f;
		entity->GetMatrix().uy = 0.0f;
		entity->GetMatrix().uz = scale;

		if (entity->GetModelIndex() == MI_MINIGUN2) {
			CMatrix matrix1;
			CMatrix matrix2; // unused
			entity->SetPosition(pickup->m_vecPos);
			matrix1.SetRotateX(0.0f);
			matrix1.Rotate(DEGTORAD(4.477f), DEGTORAD(-29.731f), DEGTORAD(-1.064f));
			matrix1.Translate(CVector(0.829f, -0.001f, 0.226f));
			entity->GetMatrix() *= matrix1;
		}

		if (doOuterGlow) {
			CVector scale(0.0f, 0.0f, 0.0f);
			if (colLength.x == maxDimension)
				scale.x = colLength.x;
			else if (colLength.y == maxDimension)
				scale.y = colLength.y;
			else
				scale.z = colLength.z;

			for (int i = 0; i < 4; i++) {
				CVector pos = entity->GetMatrix() * (scale * ((float)i / 3.0f));
				CCoronas::RegisterCorona(
					(uintptr)entity + 8 + i,
					aPickupColors[colorId].r * 0.15f,
					aPickupColors[colorId].g * 0.15f,
					aPickupColors[colorId].b * 0.15f,
					255,
					pos,
					1.0f,
					65.0f,
					CCoronas::TYPE_STAR, CCoronas::FLARE_NONE,
					CCoronas::REFLECTION_OFF,
					CCoronas::LOSCHECK_OFF,
					CCoronas::STREAK_OFF,
					0.0f,
					false,
					-0.5f);
			}
		}

		if (doInnerGlow)
			CCoronas::RegisterCorona(
#ifdef FIX_BUGS
				(uintptr)entity + 8 + 4,
#else
				(uintptr)entity + 9,
#endif
				126, 69, 121, 255, entity->GetPosition(), 1.2f, 50.0f,
				CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
	}
}

void
Pickups_DoMineEffects(CEntity *entity)
{
	const CVector &pos = entity->GetPosition();
	float dist = Distance(pos, TheCamera.GetPosition());
	const float MAXDIST = 20.0f;

	if (dist < MAXDIST) {
		// Delegate the pulse and the glow to the C implementation
		float s = Pickups_PulseSin((uint32)((*Timer_GetTimeInMilliseconds()) + (uintptr)entity), 0x1FF);

		int32 red = Pickups_GlowChannel(MAXDIST, dist, s, 0.5f, 0.5f, 64.0f);
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0, red, 0, 0, 4.0f, 1.0f, 40.0f,
		                            false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, red, 0, 0, 255, pos, 0.6f, 60.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	// Delegate the angle to the C implementation
	entity->GetMatrix().SetRotateZOnly(Pickups_SpinAngle((*Timer_GetTimeInMilliseconds()), 0x3FF));
}

void
Pickups_DoMoneyEffects(CEntity *entity)
{
	const CVector &pos = entity->GetPosition();
	float dist = Distance(pos, TheCamera.GetPosition());
	const float MAXDIST = 20.0f;

	if (dist < MAXDIST) {
		// Delegate the pulse and the glow to the C implementation
		float s = Pickups_PulseSin((uint32)((*Timer_GetTimeInMilliseconds()) + (uintptr)entity), 0x3FF);

		int32 green = Pickups_GlowChannel(MAXDIST, dist, s, 0.2f, 0.3f, 64.0f);
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0, 0, green, 0, 4.0f, 1.0f,
		                            40.0f, false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, 0, green, 0, 255, pos, 0.4f, 40.0f, CCoronas::TYPE_RING, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	// Delegate the angle to the C implementation
	entity->GetMatrix().SetRotateZOnly(Pickups_SpinAngle((*Timer_GetTimeInMilliseconds()), 0x7FF));
}

void
Pickups_DoCollectableEffects(CEntity *entity)
{
	const CVector &pos = entity->GetPosition();
	float dist = Distance(pos, TheCamera.GetPosition());
	const float MAXDIST = 14.0f;

	if (dist < MAXDIST) {
		// Delegate the pulse and the glow to the C implementation
		float s = Pickups_PulseSin((uint32)((*Timer_GetTimeInMilliseconds()) + (uintptr)entity), 0x7FF);

		int32 color = Pickups_GlowChannel(MAXDIST, dist, s, 0.5f, 0.5f, 255.0f);
		CShadows::StoreStaticShadow((uintptr)entity, SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &pos, 2.0f, 0.0f, 0.0f, -2.0f, 0, color, color, color, 4.0f,
		                            1.0f, 40.0f, false, 0.0f);
		CCoronas::RegisterCorona((uintptr)entity, color, color, color, 255, pos, 0.6f, 40.0f, CCoronas::TYPE_HEX, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
	}

	// Delegate the angle to the C implementation
	entity->GetMatrix().SetRotateZOnly(Pickups_SpinAngle((*Timer_GetTimeInMilliseconds()), 0xFFF));
}

void
Pickups_RenderPickUpText()
{
	wchar *strToPrint;
	for (int32 i = 0; i < Pickups_NumMessages; i++) {

		if (Pickups_aMessages[i].money != 0) {
			sprintf(gString, "$%d", Pickups_aMessages[i].money);
			AsciiToUnicode(gString, gUString);
			strToPrint = gUString;
		} else {
			switch (Pickups_aMessages[i].m_quantity) // could use some enum maybe
			{
			case 0:
				if (Pickups_aMessages[i].m_weaponType == WEAPONTYPE_HEALTH || Pickups_aMessages[i].m_weaponType == WEAPONTYPE_ARMOUR) {
					strToPrint = nil;
				} else {
					if (Pickups_aMessages[i].m_bOutOfStock)
						strToPrint = Text_Get(&TheText,"STOCK");
					else {
						sprintf(gString, "$%d", CostOfWeapon[Pickups_aMessages[i].m_weaponType]);
						AsciiToUnicode(gString, gUString);
						strToPrint = gUString;
					}
				}
				break;
			case 1:
				strToPrint = Text_Get(&TheText,"OUTFT1");
				break;
			case 2:
				strToPrint = Text_Get(&TheText,"OUTFT2");
				break;
			case 3:
				strToPrint = Text_Get(&TheText,"OUTFT3");
				break;
			case 4:
				strToPrint = Text_Get(&TheText,"OUTFT4");
				break;
			case 5:
				strToPrint = Text_Get(&TheText,"OUTFT5");
				break;
			case 6:
				strToPrint = Text_Get(&TheText,"OUTFT6");
				break;
			case 7:
				strToPrint = Text_Get(&TheText,"OUTFT7");
				break;
			case 8:
				strToPrint = Text_Get(&TheText,"OUTFT8");
				break;
			case 9:
				strToPrint = Text_Get(&TheText,"OUTFT9");
				break;
			case 10:
				strToPrint = Text_Get(&TheText,"OUTFT10");
				break;
			case 11:
				strToPrint = Text_Get(&TheText,"OUTFT11");
				break;
			case 12:
				strToPrint = Text_Get(&TheText,"OUTFT12");
				break;
			case 13:
				strToPrint = Text_Get(&TheText,"OUTFT13");
				break;
			default:
				break;
			}
		}
		if (strToPrint == nil)
			continue;
		Font_SetPropOn();
		Font_SetBackgroundOff();

#ifdef FIX_BUGS
		const float MAX_SCALE = SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH;
#else
		float MAX_SCALE = RsGlobal.width / DEFAULT_SCREEN_WIDTH;
#endif

		// Delegate the scale of the text to the C implementation
		float fScaleY = Pickups_MessageScale(Pickups_aMessages[i].m_dist.y, MAX_SCALE);

		float fScaleX = Pickups_MessageScale(Pickups_aMessages[i].m_dist.x, MAX_SCALE);

		Font_SetScale(fScaleX, fScaleY); // this shouldn't be scaled
		Font_SetCentreOn();
		Font_SetCentreSize(SCREEN_WIDTH);
		Font_SetJustifyOff();

		Font_SetColor(CRGBA(Pickups_aMessages[i].m_color.red, Pickups_aMessages[i].m_color.green, Pickups_aMessages[i].m_color.blue, Pickups_aMessages[i].m_color.alpha));
		Font_SetBackGroundOnlyTextOff();
		Font_SetFontStyle(FONT_STANDARD);
		Font_PrintString(Pickups_aMessages[i].m_pos.x, Pickups_aMessages[i].m_pos.y, strToPrint);
	}
	Pickups_NumMessages = 0;
}

void
Pickups_CreateSomeMoney(CVector pos, int money)
{
	bool found;

	// Delegate the number of pickups to the C implementation
	int pickupCount = Pickups_MoneyPickupCount(money);
	int moneyPerPickup = money / pickupCount;

	for (int i = 0; i < pickupCount; i++) {
		// (CGeneral::GetRandomNumber() % 256) * PI / 128 gives a float up to something TWOPI-ish.
		// Delegate the offsets on the ring to the C implementation
		pos.x += Pickups_MoneyRingOffset(General_GetRandomNumber(), false);
		pos.y += Pickups_MoneyRingOffset(General_GetRandomNumber(), true);
		pos.z = World_FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &found) + 0.5f;
		if (found) {
			Pickups_GenerateNewOne(CVector(pos.x, pos.y, pos.z), MI_MONEY, PICKUP_MONEY, moneyPerPickup + (General_GetRandomNumber() & 3));
		}
	}
}

void
Pickups_RemoveAllPickupsOfACertainWeaponGroupWithNoAmmo(eWeaponType weaponType)
{
	uint32 weaponSlot = WeaponInfo_GetWeaponInfo(weaponType)->m_nWeaponSlot;
	if (WeaponInfo_IsWeaponSlotAmmoMergeable(weaponSlot)) {
		for (int slot = 0; slot < NUMPICKUPS; slot++) {
			if (Pickups_aPickUps[slot].m_eType == PICKUP_ONCE || Pickups_aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT || Pickups_aPickUps[slot].m_eType == PICKUP_ONCE_TIMEOUT_SLOW) {
				if (Pickups_aPickUps[slot].m_pObject) {
					if (WeaponInfo_GetWeaponInfo(Pickups_WeaponForModel(Pickups_aPickUps[slot].m_pObject->GetModelIndex()))->m_nWeaponSlot == weaponSlot &&
						Pickups_aPickUps[slot].m_nQuantity == 0) {
						World_Remove(Pickups_aPickUps[slot].m_pObject);
						// Destroy and release the empty weapon pickup without invoking C++ delete.
						Pickups_aPickUps[slot].m_pObject->~CObject();
												// Access raw storage through the C store or pool API
						CPool_Delete(Pools_GetObjectPool(), Pickups_aPickUps[slot].m_pObject);
												Pickups_aPickUps[slot].m_bRemoved = true;
						Pickups_aPickUps[slot].m_pObject = nil;
						Pickups_aPickUps[slot].m_eType = PICKUP_NONE;
					}
				}
			}
		}
	}
}

void
Pickups_DetonateMinesHitByGunShot(CVector *vec1, CVector *vec2)
{
	for (int i = 0; i < NUMGENERALPICKUPS; i++) {
		if (Pickups_aPickUps[i].m_eType == PICKUP_NAUTICAL_MINE_ARMED)
			Pickups_aPickUps[i].ProcessGunShot(vec1, vec2);
	}
}

void
Pickups_RemoveUnnecessaryPickups(const CVector& center, float radius)
{
	// The C implementation picks the pickups, getting rid of them stays here
	int selected[NUMPICKUPS];
	int numSelected = Pickups_SelectUnnecessary(Pickups_aPickUps, NUMPICKUPS, center.x, center.y, center.z, radius, selected, NUMPICKUPS);
	for (int s = 0; s < numSelected; s++) {
		int i = selected[s];
		Pickups_aPickUps[i].GetRidOfObjects();
		Pickups_aPickUps[i].m_bRemoved = true;
		Pickups_aPickUps[i].m_eType = PICKUP_NONE;
	}
}

void
Pickups_Load(uint8 *buf, uint32 size)
{
INITSAVEBUF

	for (int32 i = 0; i < NUMPICKUPS; i++) {
#ifdef COMPATIBLE_SAVES
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Pickups_aPickUps[i].m_vecPos, &buf, sizeof(Pickups_aPickUps[i].m_vecPos));
		ReadSaveBuf(&Pickups_aPickUps[i].m_fRevenue, &buf, sizeof(Pickups_aPickUps[i].m_fRevenue));
				int32 tmp_pObject;
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&tmp_pObject, &buf, sizeof(tmp_pObject));
				int32 tmp_pExtraObject;
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&tmp_pExtraObject, &buf, sizeof(tmp_pExtraObject));
		ReadSaveBuf(&Pickups_aPickUps[i].m_nQuantity, &buf, sizeof(Pickups_aPickUps[i].m_nQuantity));
		ReadSaveBuf(&Pickups_aPickUps[i].m_nTimer, &buf, sizeof(Pickups_aPickUps[i].m_nTimer));
		ReadSaveBuf(&Pickups_aPickUps[i].m_nMoneySpeed, &buf, sizeof(Pickups_aPickUps[i].m_nMoneySpeed));
		ReadSaveBuf(&Pickups_aPickUps[i].m_eModelIndex, &buf, sizeof(Pickups_aPickUps[i].m_eModelIndex));
		ReadSaveBuf(&Pickups_aPickUps[i].m_nIndex, &buf, sizeof(Pickups_aPickUps[i].m_nIndex));
				memcpy(Pickups_aPickUps[i].m_sTextKey, buf, sizeof(Pickups_aPickUps[i].m_sTextKey));
				// Transfer save data through the C buffer API with explicit sizes
		SkipSaveBuf(&buf, sizeof(Pickups_aPickUps[i].m_sTextKey));
		ReadSaveBuf(&Pickups_aPickUps[i].m_eType, &buf, sizeof(Pickups_aPickUps[i].m_eType));
		ReadSaveBuf(&Pickups_aPickUps[i].m_bRemoved, &buf, sizeof(Pickups_aPickUps[i].m_bRemoved));
				uint8 flags;
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&flags, &buf, sizeof(flags));
				Pickups_aPickUps[i].m_bWasAmmoCollected = !!(flags & BIT(0));
		Pickups_aPickUps[i].m_bWasControlMessageShown = !!(flags & BIT(1));
				// Transfer save data through the C buffer API with explicit sizes
		SkipSaveBuf(&buf, 3);
		
				// Access raw storage through the C store or pool API
		Pickups_aPickUps[i].m_pObject = Pickups_aPickUps[i].m_eType != PICKUP_NONE && tmp_pObject != 0 ? ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), tmp_pObject - 1)) : nil;
		Pickups_aPickUps[i].m_pExtraObject = Pickups_aPickUps[i].m_eType != PICKUP_NONE && tmp_pExtraObject != 0 ? ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), tmp_pExtraObject - 1)) : nil;
		#else
				// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		Pickups_aPickUps[i] = *(CPickup*)buf;
		SkipSaveBuf(&buf, sizeof(Pickups_aPickUps[i]));
		
		if (Pickups_aPickUps[i].m_eType != PICKUP_NONE) {
			if (Pickups_aPickUps[i].m_pObject != nil)
								// Access raw storage through the C store or pool API
				Pickups_aPickUps[i].m_pObject = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), (uintptr)Pickups_aPickUps[i].m_pObject - 1));
							if (Pickups_aPickUps[i].m_pExtraObject != nil)
								// Access raw storage through the C store or pool API
				Pickups_aPickUps[i].m_pExtraObject = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), (uintptr)Pickups_aPickUps[i].m_pExtraObject - 1));
						}
#endif
	}

		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Pickups_CollectedPickUpIndex, &buf, sizeof(Pickups_CollectedPickUpIndex));
	SkipSaveBuf(&buf, 2);
		Pickups_NumMessages = 0;

	for (uint16 i = 0; i < NUMCOLLECTEDPICKUPS; i++)
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Pickups_aPickUpsCollected[i], &buf, sizeof(Pickups_aPickUpsCollected[i]));
		
VALIDATESAVEBUF(size)
}

void
Pickups_Save(uint8 *buf, uint32 *size)
{
	*size = PICKUPS_SAVE_SIZE;
	*size += sizeof(uint16) + sizeof(uint16) + sizeof(Pickups_aPickUpsCollected);

INITSAVEBUF

	for (int32 i = 0; i < NUMPICKUPS; i++) {
#ifdef COMPATIBLE_SAVES
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_vecPos, sizeof(Pickups_aPickUps[i].m_vecPos));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_fRevenue, sizeof(Pickups_aPickUps[i].m_fRevenue));
						// Access raw storage through the C store or pool API
		int32 tmp = Pickups_aPickUps[i].m_eType != PICKUP_NONE && Pickups_aPickUps[i].m_pObject != nil ? CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), Pickups_aPickUps[i].m_pObject) + 1 : 0;
						// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &tmp, sizeof(tmp));
						// Access raw storage through the C store or pool API
		tmp = Pickups_aPickUps[i].m_eType != PICKUP_NONE && Pickups_aPickUps[i].m_pExtraObject != nil ? CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), Pickups_aPickUps[i].m_pExtraObject) + 1 : 0;
						// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &tmp, sizeof(tmp));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_nQuantity, sizeof(Pickups_aPickUps[i].m_nQuantity));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_nTimer, sizeof(Pickups_aPickUps[i].m_nTimer));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_nMoneySpeed, sizeof(Pickups_aPickUps[i].m_nMoneySpeed));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_eModelIndex, sizeof(Pickups_aPickUps[i].m_eModelIndex));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_nIndex, sizeof(Pickups_aPickUps[i].m_nIndex));
				memcpy(buf, Pickups_aPickUps[i].m_sTextKey, sizeof(Pickups_aPickUps[i].m_sTextKey));
				// Transfer save data through the C buffer API with explicit sizes
		SkipSaveBuf(&buf, sizeof(Pickups_aPickUps[i].m_sTextKey));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_eType, sizeof(Pickups_aPickUps[i].m_eType));
		WriteSaveBuf(&buf, &Pickups_aPickUps[i].m_bRemoved, sizeof(Pickups_aPickUps[i].m_bRemoved));
				uint8 flags = 0;
		if (Pickups_aPickUps[i].m_bWasAmmoCollected) flags |= BIT(0);
		if (Pickups_aPickUps[i].m_bWasControlMessageShown) flags |= BIT(1);
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &flags, sizeof(flags));
		ZeroSaveBuf(&buf, 3);
		#else
				// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		CPickup *buf_pickup = (CPickup*)buf;
		*buf_pickup = Pickups_aPickUps[i];
		SkipSaveBuf(&buf, sizeof(Pickups_aPickUps[i]));
				if (buf_pickup->m_eType != PICKUP_NONE) {
			if (buf_pickup->m_pObject != nil)
								// Access raw storage through the C store or pool API
				buf_pickup->m_pObject = (CObject*)(CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), buf_pickup->m_pObject) + 1);
							if (buf_pickup->m_pExtraObject != nil)
								// Access raw storage through the C store or pool API
				buf_pickup->m_pExtraObject = (CObject*)(CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), buf_pickup->m_pExtraObject) + 1);
						}
#endif
	}

		// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &Pickups_CollectedPickUpIndex, sizeof(Pickups_CollectedPickUpIndex));
	{
		// Materialize the saved value with its original serialized type
		uint16 saveValue = (uint16)0;
		WriteSaveBuf(&buf, &saveValue, sizeof(saveValue)); // possibly was NumMessages
	}
	
	for (uint16 i = 0; i < NUMCOLLECTEDPICKUPS; i++)
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &Pickups_aPickUpsCollected[i], sizeof(Pickups_aPickUpsCollected[i]));
		
VALIDATESAVEBUF(*size)
}

void
CPacManPickup::Update()
{
}

int32 CollectGameState;
int16 ThingsToCollect;

CPacManPickup PacManPickups_aPMPickUps[NUMPACMANPICKUPS];
CVector PacManPickups_LastPickUpCoors;
int32 PacManPickups_PillsEatenInRace;
bool PacManPickups_bPMActive;

void
PacManPickups_Init()
{
}

void
PacManPickups_Update()
{
}

void
PacManPickups_GeneratePMPickUps(CVector pos, float scrambleMult, int16 count, uint8 type)
{
}

// diablo porn mission pickups
static const CVector aRacePoints1[] = {
	CVector(913.62219f, -155.13692f, 4.9699469f),
	CVector(913.92401f, -124.12943f, 4.9692569f),
	CVector(913.27899f, -93.524231f, 7.4325991f),
	CVector(912.60852f, -63.15905f, 7.4533591f),
	CVector(934.22144f, -42.049122f, 7.4511471f),
	CVector(0.0f, 0.0f, 0.0f),
};

void
PacManPickups_GeneratePMPickUpsForRace(int32 race)
{
}

void
PacManPickups_GenerateOnePMPickUp(CVector pos)
{
}

void
PacManPickups_Render()
{
}

void
PacManPickups_ClearPMPickUps()
{
}

void
PacManPickups_StartPacManRace(int32 race)
{
}

void
PacManPickups_StartPacManRecord()
{
}

uint32
PacManPickups_QueryPowerPillsEatenInRace()
{
	return 0;
}

void
PacManPickups_ResetPowerPillsEatenInRace()
{
}

void
PacManPickups_CleanUpPacManStuff()
{
}

void
PacManPickups_StartPacManScramble(CVector pos, float scrambleMult, int16 count)
{
}

uint32
PacManPickups_QueryPowerPillsCarriedByPlayer()
{
	return 0;
}

void
PacManPickups_ResetPowerPillsCarriedByPlayer()
{
}

void
CPed::CreateDeadPedMoney(void)
{
	if (!Game_nastyGame)
		return;

	int mi = GetModelIndex();

	if ((mi >= MI_COP && mi <= MI_FIREMAN) || (CharCreatedBy == MISSION_CHAR && !bMoneyHasBeenGivenByScript) || bInVehicle)
		return;

	int money = m_nPedMoney;
	if (money < 10)
		return;

	CVector pickupPos = GetPosition();
	Pickups_CreateSomeMoney(pickupPos, money);
	m_nPedMoney = 0;
}

void
CPed::CreateDeadPedWeaponPickups(void)
{
	CVector pickupPos;

	if (bInVehicle)
		return;

	for(int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {

		eWeaponType weapon = GetWeapon(i).m_eWeaponType;
		int weaponAmmo = GetWeapon(i).m_nAmmoTotal;
		if (weapon == WEAPONTYPE_UNARMED || weapon == WEAPONTYPE_DETONATOR || (weaponAmmo == 0 && !GetWeapon(i).IsTypeMelee()))
			continue;

		int quantity = Min(weaponAmmo, AmmoForWeapon_OnStreet[weapon] / 2);
		CreateDeadPedPickupCoors(&pickupPos.x, &pickupPos.y, &pickupPos.z);
		pickupPos.z += 0.3f;
		if (!Pickups_TryToMerge_WeaponType(pickupPos, weapon, PICKUP_ONCE_TIMEOUT, quantity, false)) {
			Pickups_GenerateNewOne_WeaponType(pickupPos, weapon, PICKUP_ONCE_TIMEOUT, Min(weaponAmmo, quantity));
		}
	}
	ClearWeapons();
}

void
CPed::CreateDeadPedPickupCoors(float *x, float *y, float *z)
{
	bool found = false;
	CVector pickupPos;

#define NUMBER_OF_ATTEMPTS 32
	for (int i = 0; i < NUMBER_OF_ATTEMPTS; i++) {

		pickupPos = GetPosition();
		// Delegate the offsets on the ring to the C implementation
		pickupPos.x = Pickups_DeadPedRingOffset(General_GetRandomNumber(), false) + GetPosition().x;
		pickupPos.y = Pickups_DeadPedRingOffset(General_GetRandomNumber(), true) + GetPosition().y;
		pickupPos.z = World_FindGroundZFor3DCoord(pickupPos.x, pickupPos.y, pickupPos.z, &found) + 0.5f;

		if (!found)
			continue;

		CVector pedPos = GetPosition();
		pedPos.z += 0.3f;

		CVector pedToPickup = pickupPos - pedPos;
		float distance = pedToPickup.Magnitude();

		// outer edge of pickup
		distance = (distance + 0.4f) / distance;
		CVector pickupPos2 = pedPos;
		pickupPos2 += distance * pedToPickup;

		if ((pickupPos - FindPlayerCoors()).Magnitude2D() > 2.0f || i > NUMBER_OF_ATTEMPTS / 2) {

			if (i > NUMBER_OF_ATTEMPTS / 2 || !Pickups_TestForPickupsInBubble(pickupPos, 1.3f)) {

				if (World_GetIsLineOfSightClear(pickupPos2, pedPos,
					true, i < NUMBER_OF_ATTEMPTS / 2, false, i < NUMBER_OF_ATTEMPTS / 2, false, false, false)) {

					if (i > NUMBER_OF_ATTEMPTS / 2 || !World_TestSphereAgainstWorld(pickupPos, 1.2f, nil, false, true, false, false, false, false)) {
						*x = pickupPos.x;
						*y = pickupPos.y;
						*z = pickupPos.z;
						return;
					}
				}
			}
		}
	}
	*x = GetPosition().x;
	*y = GetPosition().y;
	*z = GetPosition().z + 0.4f;
#undef NUMBER_OF_ATTEMPTS
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
