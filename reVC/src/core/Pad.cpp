//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#define WITHDINPUT
#include "common.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)
#include "crossplatform.h"
#include "platform.h"
#ifdef XINPUT
#include <xinput.h>
#if !defined(PSAPI_VERSION) || (PSAPI_VERSION > 1)
#pragma comment( lib, "Xinput9_1_0.lib" )
#else
#pragma comment( lib, "Xinput.lib" )
#endif
#endif

#include "Pad.h"
#include "InputState_backend.h"
#include "ControllerConfig.h"
#include "Timer.h"
#include "Clock.h"
#include "Frontend.h"
#include "Camera.h"
#include "Game.h"
#include "CutsceneMgr.h"
#include "Font.h"
#include "Hud.h"
#include "Text.h"
#include "Timer.h"
#include "Record.h"
#include "World.h"
#include "Vehicle.h"
#include "Ped.h"
#include "Population.h"
#include "Record.h"
#include "Replay.h"
#include "Weather.h"
#include "Streaming.h"
#include "PathFind.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "General.h"
#include "Fluff.h"
#include "Gangs.h"
#include "platform.h"
#include "Stats.h"
#include "CarCtrl.h"
#include "TrafficLights.h"
#include "Bike.h"	// rouz edit, added for fixing bikes
#include "GameLogic.h"	// rouz edit
#include "Pools.h" // rouz edit (ChatGPT)
#include "Messages.h"	// rouz edit
extern void command_warp_player_from_car_to_coord(CPlayerPed *ped, CVector pos, CVector speed);	// rouz edit

#ifdef GTA_PS2
#include "eetypes.h"
#include "libpad.h"
#endif

CPad Pads[MAX_PADS];
#ifdef GTA_PS2
u_long128 pad_dma_buf[scePadDmaBufferMax] __attribute__((aligned(64)));
u_long128 pad2_dma_buf[scePadDmaBufferMax] __attribute__((aligned(64)));
#endif



#ifdef GTA_PS2
unsigned char act_direct[6];
unsigned char act_align[6];
#endif



static const int OldMouseControllerStateInitialized=(MouseControllerState_Init(&Input_oldMouseState),0); // rouz edit (ChatGPT)
static const int NewMouseControllerStateInitialized=(MouseControllerState_Init(&Input_newMouseState),0); // rouz edit (ChatGPT)
static const int PCTempMouseControllerStateInitialized=(MouseControllerState_Init(&Input_tempMouseState),0); // rouz edit (ChatGPT)

#ifdef DETECT_PAD_INPUT_SWITCH
#endif

_TODO("gbFastTime");


#ifdef WALLCLIMB_CHEAT
extern bool gGravityCheat;
#endif

void SpecialCarCheats()
{
	if ( !CVehicle::bCheat9 )
	{
		((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(MI_INFERNUS)))->m_wheelScale *= 1.3f; // rouz edit (ChatGPT)
		((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(MI_CHEETAH)))->m_wheelScale *= 1.3f; // rouz edit (ChatGPT)
		((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(MI_PHEONIX)))->m_wheelScale *= 1.3f; // rouz edit (ChatGPT)
		((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(MI_DELUXO)))->m_wheelScale *= 1.3f; // rouz edit (ChatGPT)
		mod_HandlingManager.GetHandlingData(HANDLING_LANDSTAL)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_PATRIOT)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_BOBCAT)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_BFINJECT)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_RANCHER)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_DESPERAD)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->Transmission.fEngineAcceleration *= 2.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_BAGGAGE)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_BAGGAGE)->Transmission.fMaxVelocity *= 2.0f;
		Transmission_InitGearRatios(&mod_HandlingManager.GetHandlingData(HANDLING_BAGGAGE)->Transmission);

		mod_HandlingManager.GetHandlingData(HANDLING_GOLFCART)->Transmission.fEngineAcceleration *= 2.0f;
		mod_HandlingManager.GetHandlingData(HANDLING_GOLFCART)->Transmission.fMaxVelocity *= 2.0f;
		Transmission_InitGearRatios(&mod_HandlingManager.GetHandlingData(HANDLING_GOLFCART)->Transmission);

		mod_HandlingManager.GetHandlingData(HANDLING_STINGER)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_STINGER)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_STINGER)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_BANSHEE)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_BANSHEE)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_BANSHEE)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_SABRETUR)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_SABRETUR)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_SABRETUR)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_COMET)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_COMET)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_COMET)->fTurnMass *= 4.0f;

		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->fCollisionDamageMultiplier *= 0.25f;
		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->fMass *= 2.5f;
		mod_HandlingManager.GetHandlingData(HANDLING_DELUXO)->fTurnMass *= 4.0f;
		CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
		CVehicle::bCheat9 = true;
		Input_playerHasCheated = true; // rouz edit (ChatGPT)
	}
}

void PickUpChicksCheat()
{
	if ( FindPlayerVehicle() && (FindPlayerVehicle()->IsCar() || FindPlayerVehicle()->IsBike()) )
	{
		CVehicle *vehicle = FindPlayerVehicle();
		if ( FindPlayerVehicle()->IsBike() )
		{
			if ( vehicle->pPassengers[0] )
				vehicle->pPassengers[0]->SetObjective(OBJECTIVE_LEAVE_CAR, vehicle);
		}
		CPed* someoneElse = (CPed*)CWorld::TestSphereAgainstWorld(vehicle->GetPosition(), 6.0f, FindPlayerPed(), false, false, true, false, false, false);
		if ( someoneElse && someoneElse->m_nPedState != PED_DRIVING )
		{
			CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
			someoneElse->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, vehicle);
		}
	}
}

void WeaponCheat1()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT2"), true);

	Streaming_RequestModel(MI_BRASS_KNUCKLES, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_BASEBALL_BAT, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_MOLOTOV, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_COLT45, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_SHOTGUN, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_TEC9, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_RUGER, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_SNIPERRIFLE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_FLAMETHROWER, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	CStreaming::LoadAllRequestedModels(false);

	FindPlayerPed()->GiveWeapon(WEAPONTYPE_BRASSKNUCKLE, 1);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_BASEBALLBAT, 1);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_MOLOTOV, 10);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_COLT45, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SHOTGUN, 50);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_TEC9, 150);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_RUGER, 120);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SNIPERRIFLE, 25);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_FLAMETHROWER, 200);

	Streaming_SetModelIsDeletable(MI_BRASS_KNUCKLES); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_BASEBALL_BAT); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_MOLOTOV); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_COLT45); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_SHOTGUN); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_TEC9); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_RUGER); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_SNIPERRIFLE); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_FLAMETHROWER); // rouz edit (ChatGPT)
#ifdef MOBILE_IMPROVEMENTS
	if (FindPlayerVehicle()) {
		FindPlayerPed()->RemoveWeaponWhenEnteringVehicle();
	}
#endif
}

void WeaponCheat2()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT2"), true);

	Streaming_RequestModel(MI_KATANA, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_BOMB, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_PYTHON, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_STUBBY_SHOTGUN, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_SILENCEDINGRAM, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_M4, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_LASERSCOPE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_ROCKETLAUNCHER, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	CStreaming::LoadAllRequestedModels(false);

#ifdef FIX_BUGS
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_KATANA, 1);
#else
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_KATANA, 0);
#endif
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_DETONATOR_GRENADE, 10);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_PYTHON, 40);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_STUBBY_SHOTGUN, 25);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SILENCED_INGRAM, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_M4, 150);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_LASERSCOPE, 21);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_ROCKETLAUNCHER, 5);

	Streaming_SetModelIsDeletable(MI_KATANA); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_GRENADE); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_BOMB); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_PYTHON); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_STUBBY_SHOTGUN); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_SILENCEDINGRAM); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_M4); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_LASERSCOPE); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_ROCKETLAUNCHER); // rouz edit (ChatGPT)
#ifdef MOBILE_IMPROVEMENTS
	if (FindPlayerVehicle()) {
		FindPlayerPed()->RemoveWeaponWhenEnteringVehicle();
	}
#endif
}

void WeaponCheat3()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT2"), true);

	Streaming_RequestModel(MI_CHAINSAW, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_GRENADE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_PYTHON, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_SPAS12_SHOTGUN, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_MP5, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_M4, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_LASERSCOPE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_MINIGUN, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_MINIGUN2, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	CStreaming::LoadAllRequestedModels(false);

#ifdef FIX_BUGS
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_CHAINSAW, 1);
#else
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_CHAINSAW, 0);
#endif
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_GRENADE, 10);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_PYTHON, 40);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_SPAS12_SHOTGUN, 30);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_MP5, 100);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_M4, 150);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_LASERSCOPE, 21);
	FindPlayerPed()->GiveWeapon(WEAPONTYPE_MINIGUN, 500);

	Streaming_SetModelIsDeletable(MI_CHAINSAW); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_GRENADE); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_PYTHON); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_SPAS12_SHOTGUN); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_MP5); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_M4); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_LASERSCOPE); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_MINIGUN); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_MINIGUN2); // rouz edit (ChatGPT)

#ifdef MOBILE_IMPROVEMENTS
	if (FindPlayerVehicle()) {
		FindPlayerPed()->RemoveWeaponWhenEnteringVehicle();
	}
#endif
}

void HealthCheat()
{
	// rouz edit
	if (FindPlayerPed()==NULL)
		return;

	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT3"), true);
	FindPlayerPed()->m_fHealth = CWorld::Players[0].m_nMaxHealth;
	if (FindPlayerVehicle()) {
		FindPlayerVehicle()->m_fHealth = 1000.0f;
		if (FindPlayerVehicle()->m_vehType == VEHICLE_TYPE_CAR) {
			Damage_SetEngineStatus(&((CAutomobile*)FindPlayerVehicle())->Damage, 0);
			for (int32 i = 0; i < 4; i++)
				Damage_SetWheelStatus(&((CAutomobile*)FindPlayerVehicle())->Damage, i, WHEEL_STATUS_OK);
		}
	}
}

// TODO(Miami): this is HELLA different on mobile, although it mostly has debug oriented things like player exiting it's current car and enters spawned one etc.
void VehicleCheat(int model)
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	Streaming_RequestModel(model, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	CStreaming::LoadAllRequestedModels(false);
	if (Streaming_modelInfo[model].m_loadState == STREAMSTATE_LOADED) { // rouz edit (ChatGPT)
		CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);

		if (!(Streaming_modelInfo[model].m_loadState & STREAMFLAGS_DONT_REMOVE)) { // rouz edit (ChatGPT)
			Streaming_SetModelIsDeletable(model); // rouz edit (ChatGPT)
			Streaming_SetModelTxdIsDeletable(model); // rouz edit (ChatGPT)
		}

		int32 node = ThePaths.FindNodeClosestToCoors(FindPlayerCoors(), PATH_CAR, 100.0f);
		if (node < 0) return;

#ifdef FIX_BUGS
//+ rouz edit (ChatGPT)
		// Allocate the cheat vehicle without invoking C++ new.
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CAutomobile* vehicle = (CAutomobile*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
		//- rouz edit (ChatGPT)
		if (vehicle)
			std::allocator<CAutomobile>().construct(vehicle, model, RANDOM_VEHICLE);
//- rouz edit (ChatGPT)
#else
//+ rouz edit (ChatGPT)
		// Allocate the cheat vehicle without invoking C++ new.
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CAutomobile* vehicle = (CAutomobile*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
		//- rouz edit (ChatGPT)
		if (vehicle)
			std::allocator<CAutomobile>().construct(vehicle, model, MISSION_VEHICLE);
//- rouz edit (ChatGPT)
#endif
		if (vehicle != nil) {
			CVector pos = ThePaths.m_pathNodes[node].GetPosition();
			pos.z += 4.0f;
			vehicle->SetPosition(pos);
			vehicle->SetOrientation(0.0f, 0.0f, DEGTORAD(200.0f));

			vehicle->SetStatus(STATUS_ABANDONED);
			vehicle->m_nDoorLock = CARLOCK_UNLOCKED;
			CWorld::Add(vehicle);
		}
	}
	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}


void BlowUpCarsCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int i = CPool_GetSize(CPools::GetVehiclePool());
	//- rouz edit (ChatGPT)
	while (i-- > 0) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		if (CVehicle *veh = ((CVehicle*)CPool_GetSlot(CPools::GetVehiclePool(), i)))
		//- rouz edit (ChatGPT)
			veh->BlowUpCar(nil);
	}
}

void ChangePlayerCheat()
{
	// I don't know wtf is going on in here...
	int modelId, anotherModelId;

	if (FindPlayerPed()->IsPedInControl() && (CBaseModelInfo*)ModelRegistry_Find("player", 0, MODELINFOSIZE-1, nil)) { // rouz edit (ChatGPT)
		CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
		CPlayerPed *ped = FindPlayerPed();
		AssocGroupId AnimGrp = ped->m_animGroup;
		do
		{
			do {
				modelId = General_GetRandomNumberInRangeInt(0, MI_PGA);
				anotherModelId = modelId+1;
			} while (!((CBaseModelInfo*)ModelRegistry_Get(anotherModelId))); // rouz edit (ChatGPT)
		} while (anotherModelId >= MI_SPECIAL01 && anotherModelId <= MI_SPECIAL04 || modelId == MI_TAXI_D);

		uint8 flags = Streaming_modelInfo[anotherModelId].m_flags; // rouz edit (ChatGPT)
		ped->DeleteRwObject();
		Streaming_RequestModel(anotherModelId, STREAMFLAGS_DEPENDENCY| STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
		CStreaming::LoadAllRequestedModels(false);
		ped->m_modelIndex = -1;
		ped->SetModelIndex(anotherModelId);
		ped->m_animGroup = AnimGrp;
		if (modelId != -1) {
			if (!(flags & STREAMFLAGS_DONT_REMOVE))
				Streaming_SetModelIsDeletable(anotherModelId); // rouz edit (ChatGPT)
		}
	}
}

void ChangePlayerModel(const char* name) {
	if (!FindPlayerVehicle()) {
		FindPlayerPed()->Undress(name);
		CStreaming::LoadAllRequestedModels(0);
		FindPlayerPed()->Dress();
	}
}

void MayhemCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	for (int i = PEDTYPE_CIVMALE; i < PEDTYPE_SPECIAL; i++)
		PedType_SetThreats(i, PED_FLAG_PLAYER1 | PED_FLAG_PLAYER2 | PED_FLAG_PLAYER3 | PED_FLAG_PLAYER4 |
			PED_FLAG_CIVMALE | PED_FLAG_CIVFEMALE | PED_FLAG_COP | PED_FLAG_GANG1 |
			PED_FLAG_GANG2 | PED_FLAG_GANG3 | PED_FLAG_GANG4 | PED_FLAG_GANG5 |
			PED_FLAG_GANG6 | PED_FLAG_GANG7 | PED_FLAG_GANG8 | PED_FLAG_GANG9 |
			PED_FLAG_EMERGENCY | PED_FLAG_PROSTITUTE | PED_FLAG_CRIMINAL | PED_FLAG_SPECIAL );
	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void EverybodyAttacksPlayerCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	for (int i = PEDTYPE_CIVMALE; i < PEDTYPE_SPECIAL; i++)
		PedType_AddThreat(i, PED_FLAG_PLAYER1);

	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void WeaponsForAllCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	Population_givePedsWeapons = !Population_givePedsWeapons; // rouz edit (ChatGPT)
	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void FastTimeCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	if ((*Timer_GetTimeScale()) < 4.0f)
		Timer_SetTimeScale((*Timer_GetTimeScale()) * 2.0f);
}

void SlowTimeCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	if ((*Timer_GetTimeScale()) > 0.25f)
		Timer_SetTimeScale((*Timer_GetTimeScale()) * 0.5f);
}

void MoneyCheat()
{
	CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 250000;
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT6"), true);
}

void ArmourCheat()
{
	// rouz edit
	if (FindPlayerPed()==NULL)
		return;

	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT4"), true);
	FindPlayerPed()->m_fArmour = CWorld::Players[0].m_nMaxArmour;
}

void WantedLevelUpCheat()
{
	// rouz edit
	if (FindPlayerPed()==NULL)
		return;

	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT5"), true);
	Wanted_CheatWantedLevel(FindPlayerPed()->m_pWanted,Min(Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) + 2, 6));
}

void WantedLevelDownCheat()
{
	// rouz edit
	if (FindPlayerPed()==NULL)
		return;

	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT5"), true);
	Wanted_CheatWantedLevel(FindPlayerPed()->m_pWanted,0);
}

void SunnyWeatherCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT7"), true);
	Weather_ForceWeatherNow(WEATHER_SUNNY);
}

void ExtraSunnyWeatherCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT7"), true);
	Weather_ForceWeatherNow(WEATHER_EXTRA_SUNNY);
}

void CloudyWeatherCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT7"), true);
	Weather_ForceWeatherNow(WEATHER_CLOUDY);
}

void RainyWeatherCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT7"), true);
	Weather_ForceWeatherNow(WEATHER_RAINY);
}

void FoggyWeatherCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT7"), true);
	Weather_ForceWeatherNow(WEATHER_FOGGY);
}

void FastWeatherCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	gbFastTime = !gbFastTime;
}

void OnlyRenderWheelsCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	CVehicle::bWheelsOnlyCheat = !CVehicle::bWheelsOnlyCheat;
	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void ChittyChittyBangBangCheat()
{
#ifdef BETTER_ALLCARSAREDODO_CHEAT
	CHud::SetHelpMessage(Text_Get(&TheText,!CVehicle::bAllDodosCheat ? "CHEAT1" : "CHEATOF"), true);
#else
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
#endif
	CVehicle::bAllDodosCheat = !CVehicle::bAllDodosCheat;
	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void StrongGripCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	CVehicle::bCheat3 = !CVehicle::bCheat3;
	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void FannyMagnetCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	CPed::bFannyMagnetCheat = !CPed::bFannyMagnetCheat;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void BlackCarsCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	gbBlackCars = true;
	gbPinkCars = false;
}

void PinkCarsCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	gbBlackCars = false;
	gbPinkCars = true;
}

void TrafficLightsCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	TrafficLights_bGreenLightsCheat = true;
}

void MadCarsCheat()
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	CCarCtrl::bMadDriversCheat = true;
}

void NoSeaBedCheat(void)
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	CWaterLevel::m_bRenderSeaBed = !CWaterLevel::m_bRenderSeaBed;
}

void RenderWaterLayersCheat(void)
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	if ( ++CWaterLevel::m_nRenderWaterLayers > 5 )
			CWaterLevel::m_nRenderWaterLayers = 0;
}

void BackToTheFuture(void)
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	CVehicle::bHoverCheat = !CVehicle::bHoverCheat;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void SuicideCheat(void) {
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	FindPlayerPed()->InflictDamage(nil, WEAPONTYPE_UNARMED, 1000.0f, PEDPIECE_TORSO, 0);
}

void DoChicksWithGunsCheat(void) {
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	Streaming_SetModelIsDeletable(Gangs_GetGangPedModel1(GANG_PLAYER)); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(Gangs_GetGangPedModel2(GANG_PLAYER)); // rouz edit (ChatGPT)
	Streaming_SetModelTxdIsDeletable(Gangs_GetGangPedModel1(GANG_PLAYER)); // rouz edit (ChatGPT)
	Streaming_SetModelTxdIsDeletable(Gangs_GetGangPedModel2(GANG_PLAYER)); // rouz edit (ChatGPT)
	Streaming_RemoveCurrentZonesModels(); // rouz edit (ChatGPT)
	Gangs_SetGangPedModels(GANG_PLAYER, MI_HFYBE, MI_WFYBE);
	Gangs_SetGangWeapons(GANG_PLAYER, WEAPONTYPE_M4, WEAPONTYPE_M4);
	Stats_CheatedCount += 1000;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

//////////////////////////////////////////////////////////////////////////

#ifdef KANGAROO_CHEAT
void KangarooCheat()
{
	wchar *string;
	CPed *playerPed = FindPlayerPed();
	int m_fMass;

	if (playerPed->m_ped_flagI80) {
		string = Text_Get(&TheText,"CHEATOF");
		m_fMass = 70.0f;
	} else {
		string = Text_Get(&TheText,"CHEAT1");
		m_fMass = 15.0f;
	}
	CHud::SetHelpMessage(string, true);
	playerPed->m_ped_flagI80 = !playerPed->m_ped_flagI80;

	playerPed->m_fMass = m_fMass;
	playerPed->m_fAirResistance = 0.4f / m_fMass;
}
#endif

#ifdef RESTORE_ALLCARSHELI_CHEAT
void AllCarsHeliCheat(void)
{
	wchar* string;
	if (bAllCarCheat) {
		string = Text_Get(&TheText,"CHEATOF");
		bAllCarCheat = false;
	}
	else {
		string = Text_Get(&TheText,"CHEAT1");
		bAllCarCheat = true;
	}
	CHud::SetHelpMessage(string, true);
}
#endif

#ifdef WALLCLIMB_CHEAT
void WallClimbingCheat(void)
{
	wchar* string;
	if (gGravityCheat) {
		string = Text_Get(&TheText,"CHEATOF");
		gGravityCheat = false;
	}
	else {
		string = Text_Get(&TheText,"CHEAT1");
		gGravityCheat = true;
	}
	CHud::SetHelpMessage(string, true);
}
#endif

void FlyingFishCheat(void)
{
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	CVehicle::bCheat8 = !CVehicle::bCheat8;
	Input_playerHasCheated = true; // rouz edit (ChatGPT)
}

void DoShowChaseStatCheat(void) {
	CHud::SetHelpMessage(Text_Get(&TheText,"CHEAT1"), true);
	Stats_ShowChaseStatOnScreen = 1;
}




#ifdef GTA_PS2_STUFF
void CPad::Initialise(void)
{
#ifdef GTA_PS2
	scePadInit(0);

	scePadPortOpen(0, 0, pad_dma_buf );
	scePadPortOpen(1, 0, pad2_dma_buf );
#endif
	for (int i = 0; i < MAX_PADS; i++)
	{
		CPad::GetPad(i)->Clear(true);
		CPad::GetPad(i)->Mode = 0;
	}

	Input_obsoleteControllerMessage	   = false; // rouz edit (ChatGPT)
	Input_oldDisplayNoControllerMessage = false; // rouz edit (ChatGPT)
	Input_displayNoControllerMessage	   = false; // rouz edit (ChatGPT)
	Input_mapPadOneToPadTwo = false; // rouz edit (ChatGPT)
	Input_debugCameraOn = false; // rouz edit (ChatGPT)
}
#endif

void CPad::Clear(bool bResetPlayerControls)
{
	ControllerState_Clear(&NewState);
	ControllerState_Clear(&OldState);

	ControllerState_Clear(&PCTempKeyState);
	ControllerState_Clear(&PCTempJoyState);
	ControllerState_Clear(&PCTempMouseState);

	Input_ClearKeyboardHistory(); // rouz edit (ChatGPT)

	MouseControllerState_Clear(&Input_newMouseState); // rouz edit (ChatGPT)
	MouseControllerState_Clear(&Input_oldMouseState); // rouz edit (ChatGPT)
	MouseControllerState_Clear(&Input_tempMouseState); // rouz edit (ChatGPT)

	Phase = 0;
	ShakeFreq = 0;
	ShakeDur = 0;

	for (int32 i = 0; i < DRUNK_STEERING_BUFFER_SIZE; i++)
		SteeringLeftRightBuffer[i] = 0;

	DrunkDrivingBufferUsed = 0;

	if ( bResetPlayerControls )
		DisablePlayerControls = PLAYERCONTROL_ENABLED;

	JustOutOfFrontend = 0;
	bApplyBrakes = false;

	for ( int32 i = 0; i < HORNHISTORY_SIZE; i++ )
		bHornHistory[i] = false;

	iCurrHornHistory = 0;

	for ( int32 i = 0; i < ARRAY_SIZE(CheatString); i++ )
		CheatString[i] = ' ';

	LastTimeTouched = (*Timer_GetTimeInMilliseconds());
	AverageWeapon = 0;
	AverageEntries = 0;
}

void CPad::UpdateMouse()
{
#if defined RW_D3D9 || defined RWLIBS
	if ( IsForegroundApp() )
	{
		if ( PSGLOBAL(mouse) == nil )
			_InputInitialiseMouse(!FrontEndMenuManager.m_bMenuActive && _InputMouseNeedsExclusive());

		DIMOUSESTATE2 state;

		if ( PSGLOBAL(mouse) != nil && SUCCEEDED(_InputGetMouseState(&state)) )
		{
			Input_ApplyDirectMouseSample(state.lX, state.lY, state.lZ, state.rgbButtons,
				FrontEndMenuManager.m_bMenuActive); // rouz edit (ChatGPT)
		}
	}
#else
	if ( IsForegroundApp() && PSGLOBAL(cursorIsInWindow) )
	{
		double xpos = 1.0f, ypos;
		glfwGetCursorPos(PSGLOBAL(window), &xpos, &ypos);
		if (xpos == 0.f)
			return;

		Input_BeginGlfwMouseSample(xpos, ypos, PSGLOBAL(lastMousePos.x), PSGLOBAL(lastMousePos.y),
			FrontEndMenuManager.m_bMenuActive); // rouz edit (ChatGPT)
		Input_tempMouseState.LMB = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_LEFT); // rouz edit (ChatGPT)
		Input_tempMouseState.RMB = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_RIGHT); // rouz edit (ChatGPT)
		Input_tempMouseState.MMB = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_MIDDLE); // rouz edit (ChatGPT)
		Input_tempMouseState.MXB1 = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_4); // rouz edit (ChatGPT)
		Input_tempMouseState.MXB2 = glfwGetMouseButton(PSGLOBAL(window), GLFW_MOUSE_BUTTON_5); // rouz edit (ChatGPT)

		Input_SetMouseWheel(PSGLOBAL(mouseWheel)); // rouz edit (ChatGPT)

		PSGLOBAL(lastMousePos.x) = xpos;
		PSGLOBAL(lastMousePos.y) = ypos;
		PSGLOBAL(mouseWheel) = 0.0f;

		Input_AdvanceMouseHistory(); // rouz edit (ChatGPT)
	}
#endif
}

void CPad::StartShake(int16 nDur, uint8 nFreq)
{
	if ( !FrontEndMenuManager.m_PrefsUseVibration )
		return;

	if ( Cutscene_IsRunning() || CGame::playingIntro ) // rouz edit (ChatGPT)
		return;

	Input_StartShake(&ShakeDur, &ShakeFreq, nDur, nFreq); // rouz edit (ChatGPT)
}

void CPad::StartShake_Distance(int16 nDur, uint8 nFreq, float fX, float fY, float fZ)
{
	if ( !FrontEndMenuManager.m_PrefsUseVibration )
		return;

	if ( Cutscene_IsRunning() || CGame::playingIntro ) // rouz edit (ChatGPT)
		return;

	float fDist = ( TheCamera.GetPosition() - CVector(fX, fY, fZ) ).Magnitude();

	if ( fDist < 70.0f )
		Input_StartShake(&ShakeDur, &ShakeFreq, nDur, nFreq); // rouz edit (ChatGPT)
}

void CPad::StartShake_Train(float fX, float fY)
{
	if ( !FrontEndMenuManager.m_PrefsUseVibration )
		return;

	if ( Cutscene_IsRunning() || CGame::playingIntro ) // rouz edit (ChatGPT)
		return;

	if (FindPlayerVehicle() != nil && FindPlayerVehicle()->IsTrain() )
		return;

	float fDist = ( TheCamera.GetPosition() - CVector(fX, fY, 0.0f) ).Magnitude2D();

	Input_StartTrainShake(&ShakeDur, &ShakeFreq, fDist); // rouz edit (ChatGPT)
}

#ifdef GTA_PS2_STUFF
void CPad::AddToCheatString(char c)
{
	for ( int32 i = ARRAY_SIZE(CheatString) - 2; i >= 0; i-- )
		CheatString[i + 1] = CheatString[i];

	CheatString[0] = c;

#define _CHEATCMP(str)	strncmp(str, CheatString, sizeof(str)-1)
	// "4414LDRULDRU"	-	R2 R2 L1 R2 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	if ( !_CHEATCMP("URDLURDL4144") )
		WeaponCheat1();

	// "4411LDRULDRU"	-	R2 R2 L1 L1 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	else if ( !_CHEATCMP("URDLURDL1144") )
		MoneyCheat();

	// "4412LDRULDRU"	-	R2 R2 L1 L2 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	else if ( !_CHEATCMP("URDLURDL2144") )
		ArmourCheat();

	// "4413LDRULDRU"	-	R2 R2 L1 R1 LEFT DOWN RIGHT UP LEFT DOWN RIGHT UP
	else if ( !_CHEATCMP("URDLURDL3144") )
		HealthCheat();

	// "4414LRLRLR"		-	R2 R2 L1 R2 LEFT RIGHT LEFT RIGHT LEFT RIGHT
	else if ( !_CHEATCMP("RLRLRL4144") )
		WantedLevelUpCheat();

	// "4414UDUDUD"		-	R2 R2 L1 R2 UP DOWN UP DOWN UP DOWN
	else if ( !_CHEATCMP("DUDUDU4144") )
		WantedLevelDownCheat();

	// "1234432T"		-	L1 L2 R1 R2 R2 R1 L2 TRIANGLE
	else if ( !_CHEATCMP("T2344321") )
		SunnyWeatherCheat();

	// "1234432S"		-	L1 L2 R1 R2 R2 R1 L2 SQUARE
	else if ( !_CHEATCMP("S2344321") )
		CloudyWeatherCheat();

	// "1234432C"		-	L1 L2 R1 R2 R2 R1 L2 CIRCLE
	else if ( !_CHEATCMP("C2344321") )
		RainyWeatherCheat();

	// "1234432X"		-	L1 L2 R1 R2 R2 R1 L2 CROSS
	else if ( !_CHEATCMP("X2344321") )
		FoggyWeatherCheat();

	// "CCCCCC321TCT"	-	CIRCLE CIRCLE CIRCLE CIRCLE CIRCLE CIRCLE R1 L2 L1 TRIANGLE CIRCLE TRIANGLE
	else if ( !_CHEATCMP("TCT123CCCCCC") )
		VehicleCheat(MI_RHINO);

	// "CCCSSSSS1TCT"	-	CIRCLE CIRCLE CIRCLE SQUARE SQUARE SQUARE SQUARE SQUARE L1 TRIANGLE CIRCLE TRIANGLE
	else if ( !_CHEATCMP("TCT1SSSSSCCC") )
		FastWeatherCheat();

	// "241324TSCT21"	-	L2 R2 L1 R1 L2 R2 TRIANGLE SQUARE CIRCLE TRIANGLE L2 L1
	else if ( !_CHEATCMP("12TCST423142") )
		BlowUpCarsCheat();

	// "RDLU12ULDR"		-	RIGHT DOWN LEFT UP L1 L2 UP LEFT DOWN RIGHT
	else if ( !_CHEATCMP("RDLU21ULDR") )
		ChangePlayerCheat();

	// "DULUX3421"		-	DOWN UP LEFT UP CROSS R1 R2 L2 L1
	else if ( !_CHEATCMP("1243XULUD") )
		MayhemCheat();

	// "DULUX3412"		-	DOWN UP LEFT UP CROSS R1 R2 L1 L2
	else if ( !_CHEATCMP("2143XULUD") )
		EverybodyAttacksPlayerCheat();

	// "43TX21UD"		-	R2 R1 TRIANGLE CROSS L2 L1 UP DOWN
	else if ( !_CHEATCMP("DU12XT34") )
		WeaponsForAllCheat();

	// "TURDS12"		-	TRIANGLE UP RIGHT DOWN SQUARE L1 L2
	else if ( !_CHEATCMP("21SDRUT") )
		FastTimeCheat();

	// "TURDS34"		-	TRIANGLE UP RIGHT DOWN SQUARE R1 R2
	else if ( !_CHEATCMP("43SDRUT") )
		SlowTimeCheat();

	// "11S4T1T"		-	L1 L1 SQUARE R2 TRIANGLE L1 TRIANGLE
	else if ( !_CHEATCMP("T1T4S11") )
		OnlyRenderWheelsCheat();

	// "R4C32D13"		-	RIGHT R2 CIRCLE R1 L2 DOWN L1 R1
	else if ( !_CHEATCMP("31D23C4R") )
		ChittyChittyBangBangCheat();

	// "3141L33T"		-	R1 L1 R2 L1 LEFT R1 R1 TRIANGLE
	else if ( !_CHEATCMP("T33L1413") )
		StrongGripCheat();

#undef _CHEATCMP
}
#endif

int Cheat_strncmp(char* sourceStr, char* origCheatStr)
{
#define ccmp(n) if((uint8)sourceStr[i] != (uint8)origCheatStr[i] - n) return 1;
	int i = 0;
	while(origCheatStr[i])
	{
		switch(i)
		{
		case 0: ccmp(3); break;
		case 1: ccmp(5); break;
		case 2: ccmp(7); break;
		case 3: ccmp(1); break;
		case 4: ccmp(13); break;
		case 5: ccmp(27); break;
		case 6: ccmp(3); break;
		case 7: ccmp(7); break;
		case 8: ccmp(1); break;
		case 9: ccmp(11); break;
		case 10: ccmp(13); break;
		case 11: ccmp(8); break;
		case 12: ccmp(7); break;
		case 13: ccmp(32); break;
		case 14: ccmp(13); break;
		case 15: ccmp(6); break;
		case 16: ccmp(28); break;
		case 17: ccmp(19); break;
		case 18: ccmp(10); break;
		case 19: ccmp(3); break;
		case 20: ccmp(3); break;
		case 21: ccmp(5); break;
		case 22: ccmp(7); break;
		case 23: ccmp(1); break;
		case 24: ccmp(13); break;
		case 25: ccmp(27); break;
		case 26: ccmp(3); break;
		case 27: ccmp(7); break;
		default: return 1;
		}
		i++;
	}
	return 0;
#undef ccmp
}

// TODO(Miami): Mobile has changed some of the cheats to include debugging things
void CPad::AddToPCCheatString(char c)
{
	// rouz edit to only have G + numbers
	if (c != '+' && c != 'G' && !(c >= '0' && c <= '9'))
		return;

	if (c != '+')	// rouz edit so that + can repeat the previous cheat
	{
		for (int32 i = ARRAY_SIZE(Input_keyboardCheatString) - 2; i >= 0; i--) // rouz edit (ChatGPT)
			Input_keyboardCheatString[i + 1] = Input_keyboardCheatString[i]; // rouz edit (ChatGPT)

		Input_keyboardCheatString[0] = c; // rouz edit (ChatGPT)
	}

#define _CHEATCMP(str) strncmp(str, Input_keyboardCheatString, sizeof(str)-1) // rouz edit (ChatGPT)

	// "THUGSTOOLS"
	if (!Cheat_strncmp(Input_keyboardCheatString, "VQVPanJ\\I_")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		WeaponCheat1();
	}
	// "PROFESSIONALTOOLS"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VQVPagDUPT`[Lf\\Xl")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		WeaponCheat2();
	}
	// "NUTTERTOOLS"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VQVPamH[U`[")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		WeaponCheat3();
	}
	// "PRECIOUSPROTECTION"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "QTPUP`WVS[`]ViPKnc")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ArmourCheat();
	}
	// "ASPIRINE"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HSPSVkVH")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		HealthCheat();
	}
	// "YOUWONTTAKEMEALIVE"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "H[PMN`PLLLa\\Uod[kl")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		WantedLevelUpCheat();
	}
	// "LEAVEMEALONE"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HSVMN`PLWLRT")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		WantedLevelDownCheat();
	}
	// "APLEASANTDAY"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "\\FKU[\\VHFW]I")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		SunnyWeatherCheat();
	}
	// "ALOVELYDAY"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "\\FKZY`YVML")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ExtraSunnyWeatherCheat();
	}
	// "ABITDRIEG"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "JJPSQoLIB")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		CloudyWeatherCheat();
	}
	// "CATSANDDOGS"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VLVEQiDZULP")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		RainyWeatherCheat();
	}
	// "CANTSEEATHING"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "JSPIa\\HLT_[IJ")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		FoggyWeatherCheat();
	}
	// "PANZER"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "UJaONk")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_RHINO);
	}
	// "LIFEISPASSINGMEBY"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "\\GLNTiLZTL][PeSOh")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		FastWeatherCheat();
	}
	// "BIGBANG"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "JSHCTdE")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		BlowUpCarsCheat();
	}
	// "STILLLIKEDRESSINGUP"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "SZNOVnVLSORSPlYReg]")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerCheat();
	}
	// "FIGHTFIGHTFIGHT"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "WMNJSoKNJQaPNiS")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		MayhemCheat();
	}
	// "NOBODYLIKESME"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HRZFXdO`EZOWU")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		EverybodyAttacksPlayerCheat();
	}
	// "OURGODGIVENRIGHTTOBEARARMS"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VRYB_\\HIP_aPNi_TaiSJGTNSbj")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		WeaponsForAllCheat();
	}
	// "ONSPEED"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "GJLQ`iR")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		FastTimeCheat();
	}
	// "BOOOOOORING"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "JSPS\\jRVPZO")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		SlowTimeCheat();
	}
	// "WHEELSAREALLINEED"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "GJLOVgOHF]N[SeRNs")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		OnlyRenderWheelsCheat();
	}
	//COMEFLYWITHME
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HROUVr\\SGPZWJ")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChittyChittyBangBangCheat();
	}
	// "GRIPISEVERYTHING"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "JSPIatULWP`QWi_M")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		StrongGripCheat();
	}
	// "CHASESTAT"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "WF[TRnDOD")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		DoShowChaseStatCheat();
	}
	// "CHICKSWITHGUNS"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VS\\HUoL^TVPQOc")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		DoChicksWithGunsCheat();
	}
	// "ICANTTAKEITANYMORE"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HWVNfiD[JPXI[t[G_\\")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		SuicideCheat();
	}
	// "GREENLIGHT"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "WMNJYiHLSR")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		TrafficLightsCheat();
	}
	// "MIAMITRAFFIC"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "FNMGNmWPNLVU")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		MadCarsCheat();
	}
	// "AHAIRDRESSERSCAR"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "UFJT_`VZF]QZPaUG")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		PinkCarsCheat();
	}
	// "IWANTITPAINTEDBLACK"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "NHHMO_H[OTNX[iaT]jS")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		BlackCarsCheat();
	}
	// "TRAVELINSTYLE"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HQ`U`iLSFaNZ[")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_BLOODRA);
	}
	// "THELASTRIDE"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HIPSanDSFSa")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_ROMERO);
	}
	// "ROCKANDROLLCAR"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "UFJMYjUKOLXKVr")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_LOVEFIST);
	}
	// "RUBBISHCAR"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "UFJI`dEIV]")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_TRASH);
	}
	// "GETTHEREQUICKLY"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "\\QRDVpTLSPU\\[eT")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_BLOODRB);
	}
	// "GETTHEREFAST"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "WXHGRmHOU_RO")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_SABRETUR);
	}
	// "BETTERTHANWALKING"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "JSPLY\\ZUBSaZLtaK^")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_CADDY);
	}
	// "GETTHEREFASTINDEED"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "GJLE[dWZBQfZLvRXa[^WHL")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_HOTRINA);
	}
	// "GETTHEREAMAZINGLYFAST"
	else if (!Cheat_strncmp(Input_keyboardCheatString, "WXHGfgJUJeNUHe_Kdg^HJ")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		VehicleCheat(MI_HOTRINB);
	}
	// LOOKLIKELANCE
	else if (!Cheat_strncmp(Input_keyboardCheatString, "HHUBY`NPMV\\WS")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igbuddy");
	}
	// IWANTBIGTITS
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VYPUTdE[OLdQ")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igcandy");
	}
	// MYSONISALAWYER
	else if (!Cheat_strncmp(Input_keyboardCheatString, "UJ`XNgDZJY\\[`m")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igken");
	}
	// ILOOKLIKEHILARY
	else if (!Cheat_strncmp(Input_keyboardCheatString, "\\WHMVcHRJWXWVlV")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("ighlary");
	}
	// ROCKANDROLLMAN
	else if (!Cheat_strncmp(Input_keyboardCheatString, "QFTMYjUKOLXKVr")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igjezz");
	}
	// ONEARMEDBANDIT
	else if (!Cheat_strncmp(Input_keyboardCheatString, "WNKON]GLN]NMUo")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igphil");
	}
	// IDONTHAVETHEMONEYSONNY
	else if (!Cheat_strncmp(Input_keyboardCheatString, "\\SUP`tHUPXRP[ecGdgXRGN")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igsonny");
	}
	// FOXYLITTLETHING
	else if (!Cheat_strncmp(Input_keyboardCheatString, "JSPIa`O[UTYa_oS")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igmerc");
	}
	// WELOVEOURDICK
	else if (!Cheat_strncmp(Input_keyboardCheatString, "NHPE_pRLWZYM^")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igdick");
	}
	// CHEATSHAVEBEENCRACKED
	else if (!Cheat_strncmp(Input_keyboardCheatString, "GJRDNmFUFPOM]aUYpTOKF")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		ChangePlayerModel("igdiaz");
	}
	// DEEPFRIEDMARSBARS
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VWHC`mDTEPVZMpRK")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		gfTommyFatness = 0.26f;
	}
	// PROGRAMMER
	else if (!Cheat_strncmp(Input_keyboardCheatString, "UJTNNmJVS[")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		gfTommyFatness = -0.3f;
	}
	// SEAWAYS
	else if (!Cheat_strncmp(Input_keyboardCheatString, "V^HXN`V")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		BackToTheFuture();
	}
	// LOADSOFLITTLETHINGS
	else if (!Cheat_strncmp(Input_keyboardCheatString, "VLUJUoHSU_VTMo`J]bV")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		SpecialCarCheats();
	}
	// HOPINGIRL
	else if (!Cheat_strncmp(Input_keyboardCheatString, "OWPH[dSVI")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		PickUpChicksCheat();
	}
	//CERTAINDEATH
	else if (!Cheat_strncmp(Input_keyboardCheatString, "KYHFQiLHU]RK")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		// Use the C smoke-trail state or algorithm at the original call point
		SmokeTrails_CigOn = !SmokeTrails_CigOn;
	}
	//AIRSHIP
	else if (!Cheat_strncmp(Input_keyboardCheatString, "SNOT_dD")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		FlyingFishCheat();
	}
	//FANNYMAGNET
	else if (!Cheat_strncmp(Input_keyboardCheatString, "WJUHNh\\UOLS")) { // rouz edit (ChatGPT)
		Input_keyboardCheatString[0] = ' '; // rouz edit (ChatGPT)
		FannyMagnetCheat();
	}

#ifdef KANGAROO_CHEAT
	// "KANGAROO"
	if (!_CHEATCMP("OORAGNAK"))
		KangarooCheat();
#endif

#ifndef MASTER
	// "PEDDEBUG"
	if (!_CHEATCMP("GUBEDDEP"))
		CPed::SwitchDebugDisplay();
#endif

#ifdef RESTORE_ALLCARSHELI_CHEAT
	// "CARSAREHELI"
	if (!_CHEATCMP("ILEHERASRAC"))
		AllCarsHeliCheat();
#endif

#ifdef WALLCLIMB_CHEAT
	// "SPIDERCAR"
	if (!_CHEATCMP("RACREDIPS"))
		WallClimbingCheat();
#endif

#if !defined(PC_WATER) && defined(WATER_CHEATS)
	// SEABEDCHEAT
	if (!_CHEATCMP("TAEHCDEBAESON"))
		NoSeaBedCheat();

	// WATERLAYERSCHEAT
	if (!_CHEATCMP("TAEHCSREYALRETAW"))
		RenderWaterLayersCheat();
#endif

	wchar_t msg[64];

	// rouz SEAWAYS
	// "G11"
	if (!_CHEATCMP("11G"))
	{
		CVehicle::bHoverCheat = !CVehicle::bHoverCheat;
		swprintf(msg, L"Seaways %s", CVehicle::bHoverCheat ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz KANGAROO
	// "G12"
//+ rouz edit (ChatGPT)
#ifdef KANGAROO_CHEAT
	// Keep the optional custom cheat consistent with its guarded implementation
	if (!_CHEATCMP("21G"))
	{
		KangarooCheat();
		swprintf(msg, L"Kangaroo");
		CHud::SetHelpMessage((wchar *) msg, true);
	}
#endif
//- rouz edit (ChatGPT)

	// rouz ?
	// "G13"
	if (!_CHEATCMP("31G"))
	{
		//VehicleCheat(MI_SABRETUR);
		//ChangePlayerCheat();
		//MayhemCheat();
		WeaponsForAllCheat();
		EverybodyAttacksPlayerCheat();
		MadCarsCheat();
		TrafficLightsCheat();
		swprintf(msg, L"???");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz GREENLIGHT
	// "G14"
	if (!_CHEATCMP("41G"))
	{
		TrafficLights_bGreenLightsCheat ^= 1;
		if (TrafficLights_bGreenLightsCheat)
			swprintf(msg, L"Traffic lights are always green");
		else
			swprintf(msg, L"Traffic lights back to normal");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz 
	// "G15"
	if (!_CHEATCMP("51G"))
	{
		rouz.peds_attack_cops ^= 1;
		if (rouz.peds_attack_cops)
			swprintf(msg, L"Peds attack cops and paramedics");
		else
			swprintf(msg, L"Peds don't attack cops anymore");
		CHud::SetHelpMessage((wchar *) msg, true);

		for (int i = PEDTYPE_CIVMALE; i < PEDTYPE_SPECIAL; i++)
			if (rouz.peds_attack_cops)
				PedType_AddThreat(i, PED_FLAG_COP | PED_FLAG_EMERGENCY);
			else
				PedType_RemoveThreat(i, PED_FLAG_COP | PED_FLAG_EMERGENCY);

		for (int i = PEDSTAT_GANG1; i <= PEDSTAT_COWARD; i++)
			if (rouz.peds_attack_cops)
			{
				PedStats_records[i]->m_temper = 100;
				PedStats_records[i]->m_fear = 0;
				PedStats_records[i]->m_lawfulness = 0;
				PedStats_records[i]->m_flags &= ~STAT_GUN_PANIC;
			}

		Population_givePedsWeapons = rouz.peds_attack_cops; // rouz edit (ChatGPT)
	}


	// rouz remote bomb cheat
	// "G21"
	if (!_CHEATCMP("12G"))
	{
		rouz.bomb_veh = FindPlayerVehicle();
		if (rouz.bomb_veh)
		{
			rouz.remote_bomb = 2;
			CHud::SetHelpMessage((wchar *) L"Remote bomb armed", true);
		}
	}

	// rouz remote control cheat
	// "G22"
	if (!_CHEATCMP("22G"))
	{
		rouz.rc_veh = FindPlayerVehicle();

		if (rouz.rc_veh == NULL && FindPlayerPed()->m_pCurrentPhysSurface)
			if (FindPlayerPed()->m_pCurrentPhysSurface->IsVehicle())
				rouz.rc_veh = (CVehicle *) FindPlayerPed()->m_pCurrentPhysSurface;

		if (rouz.rc_veh)
		{
			rouz.remote_control = 2;
			CHud::SetHelpMessage((wchar *) L"Remote control enabled", true);
		}
	}

	// rouz set car on fire cheat
	// "G23"
	if (!_CHEATCMP("32G"))
	{
		// Based on the G44 code for fixing a vehicle
		// Fix veh we're in or the remote controlled veh
		CVehicle *veh = FindPlayerVehicle();
		if(veh==NULL && rouz.rc_veh)
			veh = (CVehicle *) rouz.rc_veh;

		if (veh)
		{
			if (veh->m_fHealth > 250.f)
				CHud::SetHelpMessage((wchar *) L"Vehicle set on fire", true);

			// Restore health and fix
			float prev_health = veh->m_fHealth;
			veh->m_fHealth = 249.0f;
			if (veh->IsCar())
			{
				Damage_SetEngineStatus(&((CAutomobile *)veh)->Damage, 0);
				if (prev_health <= 0.f && ((CAutomobile *)veh)->m_rwObject)
					((CAutomobile *)veh)->Fix();
			}
			else if (veh->IsBike() && prev_health <= 0.f)
			{
				((CBike *)veh)->Fix();
			}

			// Resurrect rc vehicle
			if (veh != FindPlayerVehicle())
				veh->SetStatus(STATUS_ABANDONED);
			veh->bRenderScorched = false;
		}
	}

	// rouz set status physics
	// "G30"
	if (!_CHEATCMP("03G"))
	{
		rouz.status_physics ^= 1;
		swprintf(msg, L"Physics for all %s", rouz.status_physics & 1 ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz screwy gravity cheat
	// "G31"
	if (!_CHEATCMP("13G"))
	{
		rouz.screwy_gravity ^= 1;
		swprintf(msg, L"Screwy gravity %s", rouz.screwy_gravity & 1 ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz antigravity repelling
	// "G32"
	if (!_CHEATCMP("23G"))
	{
		rouz.screwy_gravity ^= 2;
		swprintf(msg, L"%s", rouz.screwy_gravity & 2 ? L"OUTTA MY WAY VICE FUCKING SHITS" : L"Antigravity repelling off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz flight
	// "G33"
	if (!_CHEATCMP("33G"))
	{
		rouz.flight ^= 1;
		swprintf(msg, L"Flight %s", rouz.flight ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz Earth antigravity
	// "G34"
	if (!_CHEATCMP("43G"))
	{
		rouz.screwy_gravity ^= 4;
		swprintf(msg, L"%s", rouz.screwy_gravity & 4 ? L"Falling upwards" : L"Earth antigravity off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz Tony Hawk mode
	// "G35"
	if (!_CHEATCMP("53G"))
	{
		rouz.tonyhawk ^= 1;
		swprintf(msg, L"Tony Hawk mode %s", rouz.tonyhawk ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz slippery mode
	// "G36"
	if (!_CHEATCMP("63G"))
	{
		rouz.slippery ^= 1;
		swprintf(msg, L"Slippery tires %s", rouz.slippery ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz flip car
	// "G41"
	if (!_CHEATCMP("14G"))
	{
		CVehicle *veh = FindPlayerVehicle();
		if (veh==NULL && rouz.rc_veh)
			veh = (CVehicle *) rouz.rc_veh;

		if (veh)
		{
			veh->SetHeading(veh->GetForward().Heading());
			veh->m_vecTurnSpeed *= 0.f;
			CHud::SetHelpMessage((wchar *) L"Flip vehicle", true);
		}
	}

	// rouz become passenger
	// "G42"
	if (!_CHEATCMP("24G"))
	{
		CVehicle *veh = FindPlayerVehicle();
		if (veh==NULL && rouz.rc_veh)
			veh = (CVehicle *) rouz.rc_veh;

		if (veh==NULL && FindPlayerPed()->m_pCurrentPhysSurface)
			if (FindPlayerPed()->m_pCurrentPhysSurface->IsVehicle())
				veh = (CVehicle *) FindPlayerPed()->m_pCurrentPhysSurface;

		if (veh)
		{
			//veh->AddPassenger(FindPlayerPed(), 0);
			FindPlayerPed()->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, veh);
			CHud::SetHelpMessage((wchar *) L"Become passenger", true);
		}
	}

	// rouz glue to veg
	// "G43"
	if (!_CHEATCMP("34G"))
	{
		//FindPlayerPed()->SetObjective(OBJECTIVE_SUN_BATHE);
		//FindPlayerPed()->SetWaitState(WAITSTATE_SUN_BATHE_IDLE, nil);

		if (FindPlayerPed()->m_pCurrentPhysSurface)
		{
			if (rouz.glue_veh == FindPlayerPed()->m_pCurrentPhysSurface)
			{
				rouz.glue_veh = NULL;
				CHud::SetHelpMessage((wchar *) L"Unglued to veh", true);
			}
			else if (FindPlayerPed()->m_pCurrentPhysSurface->IsVehicle())
			{
				rouz.glue_veh = (CVehicle *) FindPlayerPed()->m_pCurrentPhysSurface;
				CHud::SetHelpMessage((wchar *) L"Glued to veh", true);
			}
		}
		else
		{
			rouz.glue_veh = NULL;
			CHud::SetHelpMessage((wchar *) L"Unglued to veh", true);
		}
	}

	// rouz fix/resurrection
	// "G44"
	if (!_CHEATCMP("44G"))
	{
		rouz.health_regen = 0;	// health_regen defeats resurrection
		rouz.prev_player_health = CWorld::Players[0].m_nMaxHealth;
		rouz.base_player_health = CWorld::Players[0].m_nMaxHealth;

		// Resurrect player (doesn't work if we die while driving)
		CPlayerPed *pPlayerPed = FindPlayerPed();

		if (pPlayerPed->m_fHealth == 0.f)
		{
			HealthCheat();
			pPlayerPed->m_fHealth = 300.f;
			pPlayerPed->bIsVisible = true;
			pPlayerPed->m_bloodyFootprintCountOrDeathTime = 0;
			pPlayerPed->bDoBloodyFootprints = false;
			pPlayerPed->m_nDrunkenness = 0;
			pPlayerPed->m_nFadeDrunkenness = 0;
			//CMBlur::ClearDrunkBlur();
			pPlayerPed->m_nDrunkCountdown = 0;
			pPlayerPed->ClearAdrenaline();
			pPlayerPed->m_fCurrentStamina = pPlayerPed->m_fMaxStamina;
			//if (pPlayerPed->m_pFire)
			//	Fire_Extinguish(pPlayerPed->m_pFire);
			pPlayerPed->bInVehicle = false;
			pPlayerPed->m_pMyVehicle = nil;
			pPlayerPed->m_pVehicleAnim = nil;
			//pPlayerPed->m_pWanted->Reset();
			pPlayerPed->bCancelEnteringCar = false;
			pPlayerPed->RestartNonPartialAnims();
			pPlayerPed->GetPlayerInfoForThisPlayerPed()->MakePlayerSafe(false);
			pPlayerPed->bRemoveFromWorld = false;
			pPlayerPed->ClearWeaponTarget();
			pPlayerPed->SetInitialState();
			CCarCtrl::ClearInterestingVehicleList();
			//pPlayerPed->Teleport(pos + CVector(0.0f, 0.0f, 1.0f));
			pPlayerPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
			//pPlayerPed->m_fRotationCur = DEGTORAD(angle);
			pPlayerPed->m_fRotationDest = pPlayerPed->m_fRotationCur;
			pPlayerPed->SetHeading(pPlayerPed->m_fRotationCur);
			//CTheScripts::ClearSpaceForMissionEntity(pos, pPlayerPed);
			//CWorld::ClearExcitingStuffFromArea(pos, 4000.0f, true);
			pPlayerPed->RestoreHeadingRate();
			CGame::currArea = AREA_MAIN_MAP;
			CStreaming::RemoveBuildingsNotInArea(AREA_MAIN_MAP);
			TheCamera.SetCameraDirectlyInFrontForFollowPed_CamOnAString();
			TheCamera.Restore();
			//References_RemoveReferencesToPlayer();
			//CGarages::PlayerArrestedOrDied();
			Stats_CheckPointReachedUnsuccessfully();
			CWorld::Remove(pPlayerPed);
			CWorld::Add(pPlayerPed);
			//CHud::ResetWastedText();
			//CStreaming::StreamZoneModels(pos);

			pPlayerPed->ResolveReferences();	// seems necessary to stop getting busted
			Messages_ClearMessages();
			CHud::GetRidOfAllHudMessages();

			//CGameLogic::RestorePlayerStuffDuringResurrection(FindPlayerPed(), FindPlayerPed()->GetPosition(), 0.f);
			if (pPlayerPed->GetPlayerInfoForThisPlayerPed())
			{
				pPlayerPed->GetPlayerInfoForThisPlayerPed()->m_WBState = WBSTATE_PLAYING;
				pPlayerPed->GetPlayerInfoForThisPlayerPed()->m_nWBTime = 0;
			}
		}
		
		// Health and armour
		HealthCheat();
		//ArmourCheat();

		// Fix veh we're in or the remote controlled veh
		CVehicle *veh = FindPlayerVehicle();
		if(veh==NULL && rouz.rc_veh)
			veh = (CVehicle *) rouz.rc_veh;

		if (veh)
		{
			// Restore health and fix
			veh->m_fHealth = 1000.0f;
			if(veh->IsCar())
			{
				Damage_SetEngineStatus(&((CAutomobile *)veh)->Damage, 0);
				if (((CAutomobile *)veh)->m_rwObject)
					((CAutomobile *)veh)->Fix();
			}
			else if(veh->IsBike())
			{
				((CBike *)veh)->Fix();
			}

			// Resurrect rc vehicle
			if (veh != FindPlayerVehicle())
				veh->SetStatus(STATUS_ABANDONED);
			veh->bRenderScorched = false;
		}

		CHud::SetHelpMessage((wchar *) L"Fix everything", true);
	}

	// rouz teleport player on rc veh
	// "G45"
	if (!_CHEATCMP("54G"))
	{
		CVehicle *veh = FindPlayerVehicle();
		if(veh==NULL && rouz.rc_veh)
			veh = (CVehicle *) rouz.rc_veh;

		if (veh && FindPlayerPed())
		{
			veh->m_nDoorLock = CARLOCK_UNLOCKED;
			command_warp_player_from_car_to_coord(FindPlayerPed(), veh->GetPosition() + CVector(0.0f, 0.0f, 2.0f), veh->m_vecMoveSpeed + CVector(0.0f, 0.0f, -0.1f));
			CHud::SetHelpMessage((wchar *) L"Teleport on vehicle", true);
		}
	}

	// rouz teleport
	float teleport_height = 0.f;
	// "G46" up 40 metres
	if (!_CHEATCMP("64G"))
		teleport_height = 40.f;

	// "G47" down 5 metres
	if(!_CHEATCMP("74G"))
		teleport_height = -6.f;
	
	if (teleport_height)
	{
		CPed *ped = FindPlayerPed();
		CVehicle *veh = FindPlayerVehicle();
		if(veh == NULL && rouz.rc_veh) veh = (CVehicle *)rouz.rc_veh;

		if(veh && ped)
			veh->SetPosition(veh->GetPosition() + CVector(0.0f, 0.0f, teleport_height));

		if(ped)
			ped->Teleport(ped->GetPosition() + CVector(0.0f, 0.0f, teleport_height));

		if(veh || ped) {
			swprintf(msg, L"Teleport %s %g metres", teleport_height > 0.f ? L"up" : L"down", fabs(teleport_height));
			CHud::SetHelpMessage((wchar *) msg, true);
		}
	}

	// Regenerative health
	// "G48"
	if (!_CHEATCMP("84G"))
	{
		rouz.health_regen ^= 1;
		swprintf(msg, L"Health regen %s", rouz.health_regen ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz set wanted level
	// "G51_"
	if (strncmp("15G", &Input_keyboardCheatString[1], 3)==0 && Input_keyboardCheatString[0] >= '0' && Input_keyboardCheatString[0] <= '6') // rouz edit (ChatGPT)
		Wanted_CheatWantedLevel(FindPlayerPed()->m_pWanted,Input_keyboardCheatString[0]-'0'); // rouz edit (ChatGPT)

	// rouz set max wanted level
	// "G52_"
	if (strncmp("25G", &Input_keyboardCheatString[1], 3)==0 && Input_keyboardCheatString[0] >= '0' && Input_keyboardCheatString[0] <= '6') // rouz edit (ChatGPT)
	{
		((void)(FindPlayerPed()->m_pWanted),Wanted_SetMaximumWantedLevel(Input_keyboardCheatString[0]-'0')); // rouz edit (ChatGPT)
		Wanted_UpdateWantedLevel(FindPlayerPed()->m_pWanted);
		swprintf(msg, L"Max wanted level %d", Input_keyboardCheatString[0]-'0'); // rouz edit (ChatGPT)
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz no ambulance
	// "G53"
	if (!_CHEATCMP("35G"))
	{
		rouz.no_ambulance ^= 1;
		swprintf(msg, L"Ambulances %s spawn", rouz.no_ambulance ? L"cannot" : L"may");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz remove weapons
	// "G611"
	if (!_CHEATCMP("116G"))
	{
		FindPlayerPed()->ClearWeapons();
		CHud::SetHelpMessage((wchar *) L"Weapons removed", true);
	}

	// rouz remove armour
	// "G612"
	if (!_CHEATCMP("216G"))
	{
		FindPlayerPed()->m_fArmour = 0.f;
		CHud::SetHelpMessage((wchar *) L"Armour removed", true);
	}

	// rouz traffic volume toggle
	// "G01"
	if (!_CHEATCMP("10G"))
	{
		rouz.unlock_traffic_gen ^= 1;
		swprintf(msg, L"Traffic generation is now %s", rouz.unlock_traffic_gen & 1 ? L"unlimited" : L"limited");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz clean bike backflips
	// "G02"
	if (!_CHEATCMP("20G"))
	{
		rouz.clean_bike_backflips ^= 1;
		swprintf(msg, L"Clean bike backflips %s", rouz.clean_bike_backflips & 1 ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz can't fall of bike
	// "G03"
	if (!_CHEATCMP("30G"))
	{
		rouz.cant_fall_off_bike ^= 1;
		swprintf(msg, L"Can't fall off bike %s", rouz.cant_fall_off_bike & 1 ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// rouz lock doors
	// "G04"
	if (!_CHEATCMP("40G"))
	{
		CPed *ped = FindPlayerPed();
		CVehicle *veh = FindPlayerVehicle();
		if (veh==NULL && rouz.rc_veh)
			veh = (CVehicle *) rouz.rc_veh;

		if (veh)
		{
			if (veh->m_nDoorLock == CARLOCK_UNLOCKED)
			{
				veh->m_nDoorLock = (ped && ped->InVehicle()) ? CARLOCK_LOCKED_PLAYER_INSIDE : CARLOCK_LOCKED;
				swprintf(msg, L"Doors locked");
			}
			else
			{
				veh->m_nDoorLock = CARLOCK_UNLOCKED;
				swprintf(msg, L"Doors unlocked");
			}
			CHud::SetHelpMessage((wchar *) msg, true);
		}
	}

	// rouz won't despawn
	// "G05"
	if (!_CHEATCMP("50G"))
	{
		CVehicle *veh = FindPlayerVehicle();
		if(veh==NULL && rouz.rc_veh)
			veh = (CVehicle *) rouz.rc_veh;

		if (veh)
		{
			veh->bIsLocked ^= 1;
			rouz.veh_locked_count += (int) veh->bIsLocked * 2 - 1;
			swprintf(msg, veh->bIsLocked ? L"Veh won't despawn" : L"Veh can despawn");
			CHud::SetHelpMessage((wchar *) msg, true);
		}
	}

	// rouz traffic volume toggle
	// "G06"
	if (!_CHEATCMP("60G"))
	{
		rouz.ghost_town ^= 1;
		swprintf(msg, L"Ghost town mode is now %s", rouz.ghost_town & 1 ? L"on" : L"off");
		CHud::SetHelpMessage((wchar *) msg, true);
	}

	// Set weather
	// "G07X"
	if (!_CHEATCMP("070G")) { Weather_ReleaseWeather();				CHud::SetHelpMessage((wchar *)L"Normal weather", true); }
	if (!_CHEATCMP("170G")) { Weather_ForceWeatherNow(WEATHER_SUNNY);		CHud::SetHelpMessage((wchar *)L"Sunny weather", true); }
	if (!_CHEATCMP("270G")) { Weather_ForceWeatherNow(WEATHER_EXTRA_SUNNY);	CHud::SetHelpMessage((wchar *)L"Extra sunny weather", true); }
	if (!_CHEATCMP("370G")) { Weather_ForceWeatherNow(WEATHER_HURRICANE);		CHud::SetHelpMessage((wchar *)L"Hurricane weather", true); }

#undef _CHEATCMP
}

#ifdef XINPUT
int CPad::XInputJoy1 = 0;
int CPad::XInputJoy2 = 1;
void CPad::AffectFromXinput(uint32 pad)
{
	pad = pad == 0 ? XInputJoy1 : XInputJoy2;
	if (pad == -1) // LoadINIControllerSettings can set it to -1
		return;

	XINPUT_STATE xstate;
	memset(&xstate, 0, sizeof(XINPUT_STATE));
	if (XInputGetState(pad, &xstate) == ERROR_SUCCESS)
	{
		PCTempJoyState.Circle = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 255 : 0;
		PCTempJoyState.Cross = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 255 : 0;
		PCTempJoyState.Square = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 255 : 0;
		PCTempJoyState.Triangle = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 255 : 0;
		PCTempJoyState.DPadDown = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 255 : 0;
		PCTempJoyState.DPadLeft = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 255 : 0;
		PCTempJoyState.DPadRight = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 255 : 0;
		PCTempJoyState.DPadUp = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 255 : 0;
		PCTempJoyState.LeftShock = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 255 : 0;
		PCTempJoyState.LeftShoulder1 = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 255 : 0;
		PCTempJoyState.LeftShoulder2 = xstate.Gamepad.bLeftTrigger;
		PCTempJoyState.RightShock = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 255 : 0;
		PCTempJoyState.RightShoulder1 = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 255 : 0;
		PCTempJoyState.RightShoulder2 = xstate.Gamepad.bRightTrigger;

		PCTempJoyState.Select = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 255 : 0;
#ifdef REGISTER_START_BUTTON
		PCTempJoyState.Start = (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 255 : 0;
#endif
		float lx = (float)xstate.Gamepad.sThumbLX / (float)0x7FFF;
		float ly = (float)xstate.Gamepad.sThumbLY / (float)0x7FFF;
		float rx = (float)xstate.Gamepad.sThumbRX / (float)0x7FFF;
		float ry = (float)xstate.Gamepad.sThumbRY / (float)0x7FFF;

		if (Abs(lx) > 0.3f || Abs(ly) > 0.3f) {
			PCTempJoyState.LeftStickX = (int32)(lx * 128.0f);
			PCTempJoyState.LeftStickY = (int32)(-ly * 128.0f);
		}

		if (Abs(rx) > 0.3f || Abs(ry) > 0.3f) {
			PCTempJoyState.RightStickX = (int32)(rx * 128.0f);
			PCTempJoyState.RightStickY = (int32)(-ry * 128.0f);
		}

		XINPUT_VIBRATION VibrationState;

		memset(&VibrationState, 0, sizeof(XINPUT_VIBRATION));

		uint16 iLeftMotor = (uint16)((float)ShakeFreq / 255.0f * (float)0xffff);
		uint16 iRightMotor = (uint16)((float)ShakeFreq / 255.0f * (float)0xffff);

		if (ShakeDur < Timer_GetTimeStepInMilliseconds())
			ShakeDur = 0;
		else
			ShakeDur -= Timer_GetTimeStepInMilliseconds();
		if (ShakeDur == 0) ShakeFreq = 0;

		VibrationState.wLeftMotorSpeed = iLeftMotor;
		VibrationState.wRightMotorSpeed = iRightMotor;

		XInputSetState(pad, &VibrationState);
	}
}
#endif

void CPad::UpdatePads(void)
{
	bool bUpdate = true;

	GetPad(0)->UpdateMouse();
#ifdef XINPUT
	GetPad(0)->AffectFromXinput(Input_mapPadOneToPadTwo ? 1 : 0); // rouz edit (ChatGPT)
	GetPad(1)->AffectFromXinput(Input_mapPadOneToPadTwo ? 0 : 1); // rouz edit (ChatGPT)
#else
	CapturePad(0);
#endif

	// Improve keyboard input latency part 1
#ifdef FIX_BUGS
	Input_AdvanceKeyboardHistory(); // rouz edit (ChatGPT)
#endif

#ifdef DETECT_PAD_INPUT_SWITCH
	if (ControllerState_CheckForInput(&GetPad(0)->PCTempJoyState))
		Input_affectedByController = true; // rouz edit (ChatGPT)
	else {
#endif
		Input_ClearSimButtonPressCheckers(ControlsManager.m_aSimCheckers); // rouz edit (ChatGPT)
		Input_AffectPadFromKeyboard(ControlsManager.m_aSettings); // rouz edit (ChatGPT)
		Input_AffectPadFromMouse(ControlsManager.m_aSettings); // rouz edit (ChatGPT)

#ifdef DETECT_PAD_INPUT_SWITCH
	}
	if (Input_affectedByController && (ControllerState_CheckForInput(&GetPad(0)->PCTempKeyState) || ControllerState_CheckForInput(&GetPad(0)->PCTempMouseState))) // rouz edit (ChatGPT)
		Input_affectedByController = false; // rouz edit (ChatGPT)
#endif

	if ( Replay_IsPlayingBackFromFile() && !FrontEndMenuManager.m_bMenuActive ) // rouz edit (ChatGPT)
		bUpdate = false;

	if ( bUpdate )
		GetPad(0)->Update(0);

#ifndef MASTER
	GetPad(1)->Update(1);
#else
	ControllerState_Clear(&GetPad(1)->NewState);
	ControllerState_Clear(&GetPad(1)->OldState);
#endif

	// Improve keyboard input latency part 2
#ifndef FIX_BUGS
	Input_AdvanceKeyboardHistory(); // rouz edit (ChatGPT)
#endif
}

void CPad::ProcessPCSpecificStuff(void)
{
	;
}

void CPad::Update(int16 pad)
{
	OldState = NewState;

#ifdef GTA_PS2
	Input_obsoleteControllerMessage = false; // rouz edit (ChatGPT)

	//int iPressureBtn;
	int id;
	int ext_id=0;
	int state;
	int rterm_id = 0;
	unsigned short paddata, tpad;
	unsigned char rdata[32];

	state = scePadGetState(pad, 0);

	switch(Phase)
	{
	case 0:
		if (state != scePadStateStable && state != scePadStateFindCTP1)
			break;
		id = scePadInfoMode(pad, 0, InfoModeCurID, 0);
		if (id==0) break;

		ext_id = scePadInfoMode(pad, 0, InfoModeCurExID, 0);
		if (ext_id>0) id = ext_id;

		switch(id)
		{
		case 4: // Digital controller
			Phase = 40; // Try for analog(dualshock)
			break;
		case 7: // Dualshock2 controller
			Phase = 50;
			break;
		default:
			Phase = 99;
			break;
		}
		break;

		// Analog Controller (old dualshock)
	case 40: // Analog Contoller check valid (otherwise fail phase)
		if (scePadInfoMode(pad, 0, InfoModeIdTable, -1)==0)
		{
			Phase = 99;
			break;
		}
		Phase++;

	case 41: // Analog controller: Request Lock analog mode (asynchronous)
		if (scePadSetMainMode(pad, 0, 1, 3)==1) Phase++;
		break;

	case 42: // Analog controller: Check state of previous request
		if (scePadGetReqState(pad, 0)==scePadReqStateFaild)
		{
			Phase--;
		}

		if (scePadGetReqState(pad, 0)==scePadReqStateComplete)
		{
			// Lock mode complete
			Phase=0; // Accept normal dualshock
		}
		break;

		// DualShock 2 Controller
	case 50: // Analog Contoller check valid (otherwise fail phase)
		if (scePadInfoMode(pad, 0, InfoModeIdTable, -1)==0)
		{
			Phase = 99;
			break;
		}
		Phase++;

	case 51: // Analog controller: Request Lock analog mode (asynchronous)
		if (scePadSetMainMode(pad, 0, 1, 3)==1) Phase++;
		break;

	case 52: // Analog controller: Check state of previous request
		if (scePadGetReqState(pad, 0)==scePadReqStateFaild)
		{
			Phase--;
		}

		if (scePadGetReqState(pad, 0)==scePadReqStateComplete)
		{
			// Lock mode complete
			Phase=0; // Accept normal dualshock
		}
		break;

	case 70: // DualShock 2 check pressure sensitive possible
		if (scePadInfoPressMode(pad, 0)==1)
		{
			Phase = 76;
			break;
		}
		Phase = 99;
		break;

	case 76: // DualShock2 enable pressure sensitive mode (asynchronous function)
		if (scePadEnterPressMode(pad, 0)==1) Phase++;
		break;

	case 77: // Dualshock2 check status of request pressure sensitive mode
		if (scePadGetReqState(pad, 0)==scePadReqStateFaild) Phase--;
		if (scePadGetReqState(pad, 0)==scePadReqStateComplete)
		{
			Phase=80;
		}
		break;

		// DualShock 2 Controller
	case 80: // Set motors
		if (scePadInfoAct(pad, 0, -1, 0)==0)
		{
			Phase = 99;
		}

		act_align[0] = 0; // Offset 0 for motor0
		act_align[1] = 1; // Offset 1 for motor1

		act_align[2] = 0xff;
		act_align[3] = 0xff;
		act_align[4] = 0xff;
		act_align[5] = 0xff;

		// Asynchronous function
		if (scePadSetActAlign(pad, 0, act_align)==0) break;
		Phase++;
		break;


	case 81:
		if ( scePadGetState(pad, 0) != scePadStateExecCmd )
		{
			Phase = 99;
		}

		break;

	default:
		if ( state == scePadStateError ) break;

		if ( state == scePadStateStable || state == scePadStateFindCTP1 )
		{
			if ( ShakeDur )
			{
				ShakeDur = Max(ShakeDur - (int32)Timer_GetTimeStepInMilliseconds(), 0);

				if ( ShakeDur == 0 )
				{
					act_direct[0] = 0;
					act_direct[1] = 0;
					scePadSetActDirect(pad, 0, act_direct);
				}
				else
				{
					act_direct[0] = 0;
					act_direct[1] = (unsigned char) ShakeFreq;
					scePadSetActDirect(pad, 0, act_direct);
				}
			}

			if (scePadRead( pad, 0, rdata )==0)
			{
				ControllerState_Clear(&NewState);
				break;
			}

			if ((rdata[0] == 0))
			{
				paddata = (unsigned short) ( 0xffff ^ ((rdata[2]<<8)|rdata[3]) );
				rterm_id = (rdata[1]);

				if ( (rterm_id>>4) == 7 ) // DUALSHOCK
				{
					if (!RecordGame_IsPlayingBack() && !RecordChase_ShouldThisPadBeLeftAlone(pad)) // rouz edit (ChatGPT)
					{
						tpad = paddata;

						NewState.DPadUp			= ( tpad & SCE_PADLup )	   ? 255 : 0;
						NewState.DPadDown		= ( tpad & SCE_PADLdown )  ? 255 : 0;
						NewState.DPadLeft		= ( tpad & SCE_PADLleft )  ? 255 : 0;
						NewState.DPadRight		= ( tpad & SCE_PADLright ) ? 255 : 0;
						NewState.Triangle		= ( tpad & SCE_PADRup )	   ? 255 : 0;
						NewState.Cross			= ( tpad & SCE_PADRdown )  ? 255 : 0;
						NewState.Square			= ( tpad & SCE_PADRleft )  ? 255 : 0;
						NewState.Circle			= ( tpad & SCE_PADRright ) ? 255 : 0;
						NewState.Start			= ( tpad & SCE_PADstart )  ? 255 : 0;
						NewState.Select			= ( tpad & SCE_PADselect ) ? 255 : 0;
						NewState.LeftShoulder1	= ( tpad & SCE_PADL1 )	   ? 255 : 0;
						NewState.LeftShoulder2	= ( tpad & SCE_PADL2 )	   ? 255 : 0;
						NewState.RightShoulder1 = ( tpad & SCE_PADR1 )	   ? 255 : 0;
						NewState.RightShoulder2 = ( tpad & SCE_PADR2 )	   ? 255 : 0;
						NewState.LeftShock		= ( tpad & SCE_PADi )	   ? 255 : 0;
						NewState.RightShock		= ( tpad & SCE_PADj )	   ? 255 : 0;
						NewState.RightStickX	= (short)rdata[4];
						NewState.RightStickY	= (short)rdata[5];
						NewState.LeftStickX		= (short)rdata[6];
						NewState.LeftStickY		= (short)rdata[7];

						#define CLAMP_AXIS(x) (((x) < 43 && (x) >= -42) ? 0 : (((x) > 0) ? (Max((x)-42, 0)*127/85) : Min((x)+42, 0)*127/85))
						#define FIX_AXIS(x) CLAMP_AXIS((x)-128)

						NewState.RightStickX = FIX_AXIS(NewState.RightStickX);
						NewState.RightStickY = FIX_AXIS(NewState.RightStickY);
						NewState.LeftStickX	 = FIX_AXIS(NewState.LeftStickX);
						NewState.LeftStickY	 = FIX_AXIS(NewState.LeftStickY);

						#undef FIX_AXIS
						#undef CLAMP_AXIS
					}
				}
				else if ( (rterm_id>>4) == 4 ) // Controller (digital)
				{
					if ( pad == 0 )
						Input_obsoleteControllerMessage = true; // rouz edit (ChatGPT)
					ControllerState_Clear(&NewState);
				}

				if ( NewState.IsAnyButtonPressed() )
					LastTimeTouched = (*Timer_GetTimeInMilliseconds());

				break;
			}

			if ( ++iCurrHornHistory >= HORNHISTORY_SIZE )
				iCurrHornHistory = 0;

			bHornHistory[iCurrHornHistory] = GetHorn();
			ControllerState_Clear(&NewState);
			return;
		}
		break;
	}

	if ( pad == 0 )
	{
		Input_oldDisplayNoControllerMessage = Input_displayNoControllerMessage; // rouz edit (ChatGPT)
		if ( state == scePadStateDiscon )
		{
			Input_displayNoControllerMessage = true; // rouz edit (ChatGPT)
			Phase = 0;
		}
		else
			Input_displayNoControllerMessage = false; // rouz edit (ChatGPT)
	}

	if ( ++iCurrHornHistory >= HORNHISTORY_SIZE )
		iCurrHornHistory = 0;

	bHornHistory[iCurrHornHistory] = GetHorn();

	if ( !Input_displayNoControllerMessage ) // rouz edit (ChatGPT)
		CGame::bDemoMode = false;
#endif

#if (defined GTA_PS2 || defined FIX_BUGS)
	if (!RecordGame_IsPlayingBack() && !RecordChase_ShouldThisPadBeLeftAlone(pad)) // rouz edit (ChatGPT)
#endif
	{
		NewState = Input_ReconcileControllers(&PCTempKeyState, &PCTempJoyState); // rouz edit (ChatGPT)
		NewState = Input_ReconcileControllers(&PCTempMouseState, &NewState); // rouz edit (ChatGPT)
	}

	ControllerState_Clear(&PCTempJoyState);
	ControllerState_Clear(&PCTempKeyState);
	ControllerState_Clear(&PCTempMouseState);

	ProcessPCSpecificStuff();

	if (ControllerState_CheckForInput(&NewState))
		LastTimeTouched = (*Timer_GetTimeInMilliseconds());
	
	if ( ++iCurrHornHistory >= HORNHISTORY_SIZE )
		iCurrHornHistory = 0;

	bHornHistory[iCurrHornHistory] = GetHorn();

	for (int32 i = DRUNK_STEERING_BUFFER_SIZE - 2; i >= 0; i--) {
		SteeringLeftRightBuffer[i + 1] = SteeringLeftRightBuffer[i];
	}

	if ( !Input_displayNoControllerMessage ) // rouz edit (ChatGPT)
		CGame::bDemoMode = false;

	if ( JustOutOfFrontend != 0 )
		--JustOutOfFrontend;
}

void CPad::DoCheats(void)
{
#ifdef DETECT_PAD_INPUT_SWITCH
	if (Input_affectedByController) // rouz edit (ChatGPT)
#endif
		GetPad(0)->DoCheats(0);
}

void CPad::DoCheats(int16 unk)
{
#ifdef GTA_PS2_STUFF
	if ( GetTriangleJustDown() )
		AddToCheatString('T');

	if ( GetCircleJustDown() )
		AddToCheatString('C');

	if ( GetCrossJustDown() )
		AddToCheatString('X');

	if ( GetSquareJustDown() )
		AddToCheatString('S');

	if ( GetDPadUpJustDown() )
		AddToCheatString('U');

	if ( GetDPadDownJustDown() )
		AddToCheatString('D');

	if ( GetDPadLeftJustDown() )
		AddToCheatString('L');

	if ( GetDPadRightJustDown() )
		AddToCheatString('R');

	if ( GetLeftShoulder1JustDown() )
		AddToCheatString('1');

	if ( GetLeftShoulder2JustDown() )
		AddToCheatString('2');

	if ( GetRightShoulder1JustDown() )
		AddToCheatString('3');

	if ( GetRightShoulder2JustDown() )
		AddToCheatString('4');
#endif
}

void CPad::StopPadsShaking(void)
{
	GetPad(0)->StopShaking(0);
}

void CPad::StopShaking(int16 pad)
{
#ifdef GTA_PS2_STUFF
	ShakeFreq = 0;
	ShakeDur = 0;

#ifdef GTA_PS2
	if ( Phase == 99 )
	{
		act_direct[0] = 0;
		act_direct[1] = 0;
		scePadSetActDirect(pad, 0, act_direct);
	}
#endif

#endif
}

CPad *CPad::GetPad(int32 pad)
{
	return &Pads[pad];
}
#ifdef DETECT_PAD_INPUT_SWITCH
#define CURMODE (Input_affectedByController ? Mode : 0) // rouz edit (ChatGPT)
#else
#define CURMODE (Mode)
#endif

int16 CPad::GetSteeringLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	int16 value;
	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

			if ( Abs(axis) > Abs(dpad) )
				value = axis;
			else
				value = dpad;

			SteeringLeftRightBuffer[0] = value;
			value = SteeringLeftRightBuffer[DrunkDrivingBufferUsed];

			break;
		}

		case 1:
		case 3:
		{
			SteeringLeftRightBuffer[0] = NewState.LeftStickX;
			value = SteeringLeftRightBuffer[DrunkDrivingBufferUsed];
			break;
		}
		default:
		{
			value = 0;
			break;
		}
	}

	return value;
}

int16 CPad::GetSteeringUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}

	return 0;
}

int16 CPad::GetCarGunUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return NewState.RightStickY;

			break;
		}

		case 3:
		{
			return (NewState.DPadUp - NewState.DPadDown) / 2;

			break;
		}
	}

	return 0;
}

int16 CPad::GetCarGunLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return NewState.RightStickX;

			break;
		}

		case 3:
		{
			return (NewState.DPadRight - NewState.DPadLeft) / 2;

			break;
		}
	}

	return 0;
}

int16 CPad::GetPedWalkLeftRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickX;

			break;
		}
	}

	return 0;
}

int16 CPad::GetPedWalkUpDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}

	return 0;
}

int16 CPad::GetAnalogueUpDown(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickY;
			int16 dpad = (NewState.DPadDown - NewState.DPadUp) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickY;

			break;
		}
	}

	return 0;
}

int16 CPad::GetAnalogueLeftRight(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			int16 axis = NewState.LeftStickX;
			int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

			if ( Abs(axis) > Abs(dpad) )
				return axis;
			else
				return dpad;

			break;
		}

		case 1:
		case 3:
		{
			return NewState.LeftStickX;

			break;
		}
	}

	return 0;
}

bool CPad::GetLookLeft(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder2 && !NewState.RightShoulder2);
}

bool CPad::GetLookRight(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.RightShoulder2 && !NewState.LeftShoulder2);
}


bool CPad::GetLookBehindForCar(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.RightShoulder2 && NewState.LeftShoulder2);
}

bool CPad::GetLookBehindForPed(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!NewState.RightShock;
}

bool CPad::GetHorn(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		{
			return !!NewState.LeftShock;

			break;
		}

		case 1:
		{
			return !!NewState.LeftShoulder1;

			break;
		}

		case 2:
		{
			return !!NewState.RightShoulder1;

			break;
		}

		case 3:
		{
			return !!NewState.LeftShock;

			break;
		}
	}

	return false;
}

bool CPad::HornJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}

		case 1:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}

		case 2:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}

		case 3:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}
	}

	return false;
}

bool CPad::GetCarGunFired(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!NewState.Circle;

			break;
		}

		case 3:
		{
			return !!NewState.RightShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::CarGunJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}

		case 3:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}
	}

	return false;
}

int16 CPad::GetHandBrake(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return NewState.RightShoulder1;

			break;
		}

		case 2:
		{
			return NewState.Triangle;

			break;
		}

		case 3:
		{
			return NewState.LeftShoulder1;

			break;
		}
	}

	return 0;
}

int16 CPad::GetBrake(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			return NewState.Square;

			break;
		}

		case 1:
		{
			return NewState.Square;

			break;
		}

		case 3:
		{
			int16 axis = 2 * NewState.RightStickY;

			if ( axis < 0 )
				return 0;
			else
				return axis;

			break;
		}
	}

	return 0;
}

bool CPad::GetExitVehicle(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;


	if ( JustOutOfFrontend != 0 )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Triangle;

			break;
		}

		case 2:
		{
			return !!NewState.LeftShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::ExitVehicleJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	if ( JustOutOfFrontend != 0 )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!(NewState.Triangle && !OldState.Triangle);

			break;
		}

		case 2:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}
	}

	return false;
}

int32 CPad::GetWeapon(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return NewState.Circle;

			break;
		}

		case 2:
		{
			return NewState.Cross;

			break;
		}

		case 3:
		{
			return NewState.RightShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::WeaponJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}

		case 2:
		{
			return !!(NewState.Cross && !OldState.Cross);

			break;
		}

		case 3:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}
	}

	return false;
}

int16 CPad::GetAccelerate(void)
{
	if ( ArePlayerControlsDisabled() )
		return 0;

	switch (CURMODE)
	{
		case 0:
		case 2:
		{
			return NewState.Cross;

			break;
		}

		case 1:
		{
			return NewState.Cross;

			break;
		}

		case 3:
		{
			int16 axis = -2 * NewState.RightStickY;

			if ( axis < 0 )
				return 0;
			else
				return axis;

			break;
		}
	}

	return 0;
}

bool CPad::CycleCameraModeJustDown(void)
{
	bool result;
	switch (CURMODE)
	{
		case 0:
		case 2:
		case 3:
		{
			result = !!(NewState.Select && !OldState.Select);

			break;
		}

		case 1:
		{
			result = !!(NewState.DPadUp && !OldState.DPadUp);

			break;
		}
		default:
		{
			result = false;
			break;
		}
	}

	if (!result)
	{
		switch (CURMODE)
		{
			case 1:
			{
				result = !!(NewState.DPadDown && !OldState.DPadDown);
				break;
			}
			default:
			{
				result = false;
				break;
			}
		}
	}

	return result;
}

bool CPad::CycleCameraModeUpJustDown(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		case 3:
		{
			return !!(NewState.Select && !OldState.Select);

			break;
		}

		case 1:
		{
			return !!(NewState.DPadUp && !OldState.DPadUp);

			break;
		}
	}

	return false;
}

bool CPad::CycleCameraModeDownJustDown(void)
{
	switch (CURMODE)
	{
		case 0:
		case 2:
		case 3:
		{
			return false;

			break;
		}

		case 1:
		{
			return !!(NewState.DPadDown && !OldState.DPadDown);

			break;
		}
	}

	return false;
}

bool CPad::ChangeStationJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}

		case 1:
		{
			return !!(NewState.Select && !OldState.Select);

			break;
		}

		case 2:
		{
			return !!(NewState.LeftShock && !OldState.LeftShock);

			break;
		}

		case 3:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}
	}

	return false;
}

bool CPad::CycleWeaponLeftJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2);
}

bool CPad::CycleWeaponRightJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.RightShoulder2 && !OldState.RightShoulder2);
}

bool CPad::GetTarget(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!NewState.RightShoulder1;

			break;
		}

		case 3:
		{
			return !!NewState.LeftShoulder1;

			break;
		}
	}

	return false;
}

bool CPad::TargetJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 2:
		{
			return !!(NewState.RightShoulder1 && !OldState.RightShoulder1);

			break;
		}

		case 3:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}
	}

	return false;
}

bool CPad::CollectPickupJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1);

			break;
		}
		case 2:
		{
			return !!(NewState.Triangle && !OldState.Triangle);

			break;
		}

		case 3:
		{
			return !!(NewState.Circle && !OldState.Circle);

			break;
		}
	}

	return false;
}

bool CPad::DuckJustDown(void)
{
	if (ArePlayerControlsDisabled())
		return false;

	return !!(NewState.LeftShock && !OldState.LeftShock);
}

bool CPad::JumpJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.Square && !OldState.Square);
}

bool CPad::GetSprint(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Cross;

			break;
		}

		case 2:
		{
			return !!NewState.Circle;

			break;
		}
	}

	return false;
}

bool CPad::ShiftTargetLeftJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder2 && !OldState.LeftShoulder2);
}

bool CPad::ShiftTargetRightJustDown(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	return !!(NewState.LeftShoulder1 && !OldState.LeftShoulder1) || !!(NewState.RightShoulder2 && !OldState.RightShoulder2);
}

bool CPad::GetAnaloguePadUp(void)
{
	static int16 oldfStickY = 0;

	int16 leftStickY = CPad::GetPad(0)->GetLeftStickY();

	if ( leftStickY < -15 && oldfStickY >= -5 )
	{
		oldfStickY = leftStickY;
		return true;
	}
	else
	{
		oldfStickY = leftStickY;
		return false;
	}
}

bool CPad::GetAnaloguePadDown(void)
{
	static int16 oldfStickY = 0;

	int16 leftStickY = CPad::GetPad(0)->GetLeftStickY();

	if ( leftStickY > 15 && oldfStickY <= 5 )
	{
		oldfStickY = leftStickY;
		return true;
	}
	else
	{
		oldfStickY = leftStickY;
		return false;
	}
}

bool CPad::GetAnaloguePadLeft(void)
{
	static int16 oldfStickX = 0;

	int16 leftStickX = CPad::GetPad(0)->GetLeftStickX();

	if ( leftStickX < -15 && oldfStickX >= -5 )
	{
		oldfStickX = leftStickX;
		return true;
	}
	else
	{
		oldfStickX = leftStickX;
		return false;
	}
}

bool CPad::GetAnaloguePadRight(void)
{
	static int16 oldfStickX = 0;

	int16 leftStickX = CPad::GetPad(0)->GetLeftStickX();

	if ( leftStickX > 15 && oldfStickX <= 5 )
	{
		oldfStickX = leftStickX;
		return true;
	}
	else
	{
		oldfStickX = leftStickX;
		return false;
	}
}

bool CPad::GetAnaloguePadLeftJustUp(void)
{
	static int16 oldfStickX = 0;

	int16 X = GetPad(0)->GetPedWalkLeftRight();

	if ( X == 0 && oldfStickX < 0 )
	{
		oldfStickX = 0;

		return true;
	}
	else
	{
		oldfStickX = X;

		return false;
	}
}

bool CPad::GetAnaloguePadRightJustUp(void)
{
	static int16 oldfStickX = 0;

	int16 X = GetPad(0)->GetPedWalkLeftRight();

	if ( X == 0 && oldfStickX > 0 )
	{
		oldfStickX = 0;

		return true;
	}
	else
	{
		oldfStickX = X;

		return false;
	}
}

bool CPad::ForceCameraBehindPlayer(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		{
			return !!NewState.LeftShoulder1;

			break;
		}

		case 2:
		{
			return !!NewState.Triangle;

			break;
		}

		case 3:
		{
			return !!NewState.Circle;

			break;
		}
	}

	return false;
}

bool CPad::SniperZoomIn(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Square;

			break;
		}

		case 2:
		{
			return !!NewState.Triangle;

			break;
		}
	}

	return false;
}

bool CPad::SniperZoomOut(void)
{
	if ( ArePlayerControlsDisabled() )
		return false;

	switch (CURMODE)
	{
		case 0:
		case 1:
		case 3:
		{
			return !!NewState.Cross;

			break;
		}

		case 2:
		{
			return !!NewState.Square;

			break;
		}
	}

	return false;
}

#undef CURMODE

int16 CPad::SniperModeLookLeftRight(void)
{
	int16 axis = NewState.LeftStickX;
	int16 dpad = (NewState.DPadRight - NewState.DPadLeft) / 2;

	if ( Abs(axis) > Abs(dpad) ) {
		if ( Abs(axis) > 35.0f ) {
		  return (axis > 0.f ? axis - 35.f : axis + 35.f) * (128.f / (128 - 35));
		} else {
		  return 0;
		}
	} else
		return dpad;
}

int16 CPad::SniperModeLookUpDown(void)
{
	int16 axis = NewState.LeftStickY;
	int16 dpad;

#ifdef FIX_BUGS
	axis = -axis;
#endif
	if (Input_invertPadLook) { // rouz edit (ChatGPT)
		axis = -axis;
		dpad = (NewState.DPadDown - NewState.DPadUp) / 2;
	} else {
		dpad = (NewState.DPadUp - NewState.DPadDown) / 2;
	}

	if ( Abs(axis) > Abs(dpad) ) {
	    if ( Abs(axis) > 35.0f ) {
	      return (axis > 0.f ? axis - 35.f : axis + 35.f) * (128.f / (128 - 35));
	    } else {
	      return 0;
	    }
	} else
		return dpad;
}

int16 CPad::LookAroundLeftRight(void)
{
	float axis = GetPad(0)->NewState.RightStickX;

	if ( Abs(axis) > 85 && !GetLookBehindForPed() )
		return (int16) ( (axis + ( ( axis > 0 ) ? -85 : 85) )
							* (127.0f / 32.0f) ); // 3.96875f

	else if ( TheCamera.Cams[0].Using3rdPersonMouseCam() && Abs(axis) > 10 )
		return (int16) ( (axis + ( ( axis > 0 ) ? -10 : 10) )
							* (127.0f / 64.0f) ); // 1.984375f

	return 0;
}

int16 CPad::LookAroundUpDown(void)
{
	int16 axis = GetPad(0)->NewState.RightStickY;
#ifdef FIX_BUGS
	axis = -axis;
#endif
	if (Input_invertPadLook) // rouz edit (ChatGPT)
		axis = -axis;

	if ( Abs(axis) > 85 && !GetLookBehindForPed() )
		return (int16) ( (axis + ( ( axis > 0 ) ? -85 : 85) )
							* (127.0f / 32.0f) ); // 3.96875f

	else if ( TheCamera.Cams[0].Using3rdPersonMouseCam() && Abs(axis) > 40 )
		return (int16) ( (axis + ( ( axis > 0 ) ? -40 : 40) )
							* (127.0f / 64.0f) ); // 1.984375f

	return 0;
}

void CPad::ResetAverageWeapon(void)
{
	AverageWeapon = GetWeapon();
	AverageEntries = 1;
}

void CPad::PrintErrorMessage(void)
{
	if (TheCamera.m_WideScreenOn)
		return;

	if ( Input_displayNoControllerMessage && !CGame::playingIntro && !FrontEndMenuManager.m_bMenuActive ) // rouz edit (ChatGPT)
	{
		([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(Rect_Make(SCREEN_STRETCH_X(20.0f), SCREEN_SCALE_FROM_BOTTOM(130.0f), SCREEN_STRETCH_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(140.0f)), CRGBA(50, 50, 50, 210));
#ifdef FIX_BUGS
		CFont::SetScale(SCREEN_SCALE_X(0.85f), SCREEN_SCALE_Y(1.0f));
#else
		CFont::SetScale(0.85f, 1.0f);
#endif
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
#ifdef FIX_BUGS
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 20));
#else
		CFont::SetCentreSize(SCREEN_WIDTH - 20);
#endif
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetColor(CRGBA(255, 255, 200, 200));
		CFont::SetFontStyle(FONT_STANDARD);
		CFont::PrintString
		(
			SCREEN_WIDTH  / 2,
			SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(40.0f),
			Text_Get(&TheText,"NOCONT") // Please reconnect an analog controller (DUALSHOCK@) or analog controller (DUALSHOCK@2). to controller port 1 to continue
		);
	}
	else if ( Input_obsoleteControllerMessage ) // rouz edit (ChatGPT)
	{
		([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(Rect_Make(SCREEN_STRETCH_X(20.0f), SCREEN_SCALE_FROM_BOTTOM(130.0f), SCREEN_STRETCH_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(140.0f)), CRGBA(50, 50, 50, 210));
#ifdef FIX_BUGS
		CFont::SetScale(SCREEN_SCALE_X(0.85f), SCREEN_SCALE_Y(1.0f));
#else
		CFont::SetScale(0.85f, 1.0f);
#endif
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
#ifdef FIX_BUGS
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 20));
#else
		CFont::SetCentreSize(SCREEN_WIDTH - 20);
#endif
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetColor(CRGBA(255, 255, 200, 200));
		CFont::SetFontStyle(FONT_STANDARD);
		CFont::PrintString
		(
			SCREEN_WIDTH  / 2,
			SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(40.0f),
			Text_Get(&TheText,"WRCONT") // The controller connected to controller port 1 is an unsupported controller. Grand Theft Auto III requires an analog controller (DUALSHOCK@) or analog controller (DUALSHOCK@2).
		);
	}

}

void LittleTest(void)
{
	static int32 Cunt = 0;

	Cunt++; // ???
}

void CPad::ResetCheats(void)
{
	Weather_ReleaseWeather();

	gbFastTime = false;
	Population_givePedsWeapons = false; // rouz edit (ChatGPT)
	Timer_SetTimeScale(1.0f);

	CVehicle::bWheelsOnlyCheat = false;
	CVehicle::bAllDodosCheat = false;
	CVehicle::bCheat3 = false;
	CVehicle::bCheat4 = false;
	CVehicle::bCheat5 = false;
	CVehicle::bAllTaxisHaveNitro = false;
	CVehicle::bHoverCheat = false;
	CVehicle::bCheat8 = false;
	CVehicle::bCheat9 = false;
	CVehicle::bCheat10 = false;
#ifdef RESTORE_ALLCARSHELI_CHEAT
	bAllCarCheat = false;
#endif
	gbBlackCars = false;
	gbPinkCars = false;

	CCarCtrl::bMadDriversCheat = false;
	TrafficLights_bGreenLightsCheat = false;
	Stats_ShowChaseStatOnScreen = 0;
	CPed::bNastyLimbsCheat = false;
	CPed::bFannyMagnetCheat = false;
	CPed::bPedCheat3 = false;

}

char *CPad::EditString(char *pStr, int32 nSize)
{
	int32 pos = (int32)strlen(pStr);

	// letters
	for ( int32 i = 0; i < ('Z' - 'A' + 1); i++ )
	{
		if ( Input_GetCharJustDown(i + 'A') && pos < nSize - 1 ) // rouz edit (ChatGPT)
		{
			pStr[pos++] = i + 'A';
			pStr[pos] = '\0';
		}

		if ( Input_GetCharJustDown(i + 'a') && pos < nSize - 1 ) // rouz edit (ChatGPT)
		{
			pStr[pos++] = i + 'a';
			pStr[pos] = '\0';
		}
	}

	// numbers
	for ( int32 i = 0; i < ('9' - '0' + 1); i++ )
	{
		if ( Input_GetCharJustDown(i + '0') && pos < nSize - 1 ) // rouz edit (ChatGPT)
		{
			pStr[pos++] = i + '0';
			pStr[pos] = '\0';
		}
	}

	// space
	if ( Input_GetCharJustDown(' ') && pos < nSize - 1 ) // rouz edit (ChatGPT)
	{
		pStr[pos++] = ' ';
		pStr[pos] = '\0';
	}


	// del
	if ( Input_GetDeleteJustDown() || Input_GetBackspaceJustDown() ) // rouz edit (ChatGPT)
	{
		if ( pos > 0 )
			pStr[pos - 1] = '\0';
	}

	// extenter/up/down
	if ( Input_GetReturnJustDown() || Input_GetUpJustDown() || Input_GetDownJustDown() ) // rouz edit (ChatGPT)
		return nil;

	return pStr;
}

int32 *CPad::EditCodesForControls(int32 *pRsKeys, int32 nSize)
{
	*pRsKeys = rsNULL;

	for ( int32 i = 0; i < 255; i++ )
	{
		if ( Input_GetCharJustDown(i) ) // rouz edit (ChatGPT)
			*pRsKeys = i;
	}

	for ( int32 i = 0; i < 12; i++ )
	{
		if ( Input_GetFJustDown(i) ) // rouz edit (ChatGPT)
			*pRsKeys = i + rsF1;
	}

	if ( Input_GetEscapeJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsESC;

	if ( Input_GetInsertJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsINS;

	if ( Input_GetDeleteJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsDEL;

	if ( Input_GetHomeJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsHOME;

	if ( Input_GetEndJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsEND;

	if ( Input_GetPageUpJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPGUP;

	if ( Input_GetPageDownJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPGDN;

	if ( Input_GetUpJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsUP;

	if ( Input_GetDownJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsDOWN;

	if ( Input_GetLeftJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsLEFT;

	if ( Input_GetRightJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsRIGHT;

	if ( Input_GetScrollLockJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsSCROLL;

	if ( Input_GetPauseJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPAUSE;

	if ( Input_GetNumLockJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsNUMLOCK;

	if ( Input_GetDivideJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsDIVIDE;

	if ( Input_GetTimesJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsTIMES;

	if ( Input_GetMinusJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsMINUS;

	if ( Input_GetPlusJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPLUS;

	if ( Input_GetPadEnterJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADENTER;

	if ( Input_GetPadDelJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADDEL;

	if ( Input_GetPad1JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADEND;

	if ( Input_GetPad2JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADDOWN;

	if ( Input_GetPad3JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADPGDN;

	if ( Input_GetPad4JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADLEFT;

	if ( Input_GetPad5JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPAD5;

	if ( Input_GetPad6JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADRIGHT;

	if ( Input_GetPad7JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADHOME;

	if ( Input_GetPad8JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADUP;

	if ( Input_GetPad9JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADPGUP;

	if ( Input_GetPad0JustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsPADINS;

	if ( Input_GetBackspaceJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsBACKSP;

	if ( Input_GetTabJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsTAB;

	if ( Input_GetCapsLockJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsCAPSLK;

	if ( Input_GetReturnJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsENTER;

	if ( Input_GetLeftShiftJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsLSHIFT;

	if ( Input_GetShiftJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsSHIFT;

	if ( Input_GetRightShiftJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsRSHIFT;

	if ( Input_GetLeftCtrlJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsLCTRL;

	if ( Input_GetRightCtrlJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsRCTRL;

	if ( Input_GetLeftAltJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsLALT;

	if ( Input_GetRightAltJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsRALT;

	if ( Input_GetLeftWinJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsLWIN;

	if ( Input_GetRightWinJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsRWIN;

	if ( Input_GetAppsJustDown() ) // rouz edit (ChatGPT)
		*pRsKeys = rsAPPS;

	return pRsKeys;
}

void CPad::FixPadsAfterSave(void)
{
	UpdatePads();
	if ( Input_obsoleteControllerMessage ) // rouz edit (ChatGPT)
	{
		Input_obsoleteControllerMessage = false; // rouz edit (ChatGPT)
		GetPad(0)->Phase = 0;
	}
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#if defined RW_D3D9 || defined RWLIBS
bool InputState_BackendHasMouse(void)
{
    // Borrow the current platform mouse availability
    return PSGLOBAL(mouse) != nil;
}
bool InputState_BackendMenuActive(void)
{
    // Borrow the current frontend flag without changing it
    return FrontEndMenuManager.m_bMenuActive;
}
bool InputState_BackendNeedsExclusive(void)
{
    // Retain the platform's original exclusivity query
    return _InputMouseNeedsExclusive();
}
void InputState_BackendInitialiseMouse(bool exclusive)
{
    // Pass the selected exclusivity directly to the existing device initializer
    _InputInitialiseMouse(exclusive);
}
void InputState_BackendCapabilities(uint32_t *buttons,uint32_t *axes)
{
    // Query the original device-capability structure without changing failure handling
    DIDEVCAPS devCaps;
    devCaps.dwSize=sizeof(DIDEVCAPS);
    PSGLOBAL(mouse)->GetCapabilities(&devCaps);
    *buttons=devCaps.dwButtons;
    *axes=devCaps.dwAxes;
}
#else
double InputState_BackendCursorX(void)
{
    // Retain the original cursor default if the platform query leaves it untouched
    double xpos=1.0f,ypos;
    glfwGetCursorPos(PSGLOBAL(window),&xpos,&ypos);
    return xpos;
}
#endif
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
