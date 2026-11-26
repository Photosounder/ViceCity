//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "GameLogic.h"
#include "Clock.h"
#include "Stats.h"
#include "Pickups.h"
#include "Timer.h"
#include "Streaming.h"
#include "CutsceneMgr.h"
#include "World.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "Camera.h"
#include "Messages.h"
#include "CarCtrl.h"
#include "Restart.h"
#include "Pad.h"
#include "References.h"
#include "Fire.h"
#include "Script.h"
#include "Garages.h"
#include "Population.h"
#include "General.h"
#include "DMAudio.h"
#include "Radar.h"
#include "Pools.h"
#include "Hud.h"
#include "Particle.h"
#include "ColStore.h"
#include "Automobile.h"
#include "MBlur.h"
#include "screendroplets.h"
#include "SaveBuf.h"
#include "GameLogicMath.h"

uint8 GameLogic_ActivePlayers;
uint8 GameLogic_ShortCutState;
CAutomobile* GameLogic_pShortCutTaxi;
uint32 GameLogic_NumAfterDeathStartPoints;
CVector GameLogic_ShortCutStart;
float GameLogic_ShortCutStartOrientation;
CVector GameLogic_ShortCutDestination;
float GameLogic_ShortCutDestinationOrientation;
uint32 GameLogic_ShortCutTimer;
CVector GameLogic_AfterDeathStartPoints[NUM_SHORTCUT_START_POINTS];
float GameLogic_AfterDeathStartPointOrientation[NUM_SHORTCUT_START_POINTS];
CVector GameLogic_ShortCutDropOffForMission;
float GameLogic_ShortCutDropOffOrientationForMission;
bool GameLogic_MissionDropOffReadyToBeUsed;

#define SHORTCUT_TAXI_COST (9)
#define TOTAL_BUSTED_AUDIO (28)

void
GameLogic_InitAtStartOfGame()
{
	GameLogic_ActivePlayers = 1;
	GameLogic_ShortCutState = SHORTCUT_NONE;
	GameLogic_pShortCutTaxi = nil;
	GameLogic_NumAfterDeathStartPoints = 0;
}

void
GameLogic_PassTime(uint32 time)
{
	int32 minutes, hours, days;

	// Do the clock arithmetic in C
	GameLogicMath_PassTime(time, Clock_GetMinutes(), Clock_GetHours(), Stats_DaysPassed, &minutes, &hours, &days);
	Stats_DaysPassed = days;

	Clock_SetGameClock(hours, minutes);
	Pickups_PassTime(time * 1000);
}

void 
GameLogic_SortOutStreamingAndMemory(const CVector &pos)
{
	Timer_Stop();
	Streaming_FlushRequestList(); // rouz edit (ChatGPT)
	Streaming_DeleteRwObjectsAfterDeath(pos);
	Streaming_RemoveUnusedModelsInLoadedList();
	Game_DrasticTidyUpMemory(true);
	World_Players[World_PlayerInFocus].m_pPed->Undress("player");
	Streaming_LoadSceneCollision(pos);
	Streaming_LoadScene(pos);
	World_Players[World_PlayerInFocus].m_pPed->Dress();
	Timer_Update();
}

void
GameLogic_Update()
{
	CVector vecRestartPos;
	float fRestartFloat;

#ifdef MISSION_REPLAY
	// what a place to check!
	if (gbTryingPorn4Again) {
		CRunningScript* pScript = TheScripts_pActiveScripts;
		if (pScript && !General_faststricmp(pScript->m_abScriptName, "porno4"))
			gbTryingPorn4Again = false;
	}
#endif

	if (Cutscene_IsProcessing()) return; // rouz edit (ChatGPT)

	GameLogic_UpdateShortCut();
	CPlayerInfo &pPlayerInfo = World_Players[World_PlayerInFocus];

	switch (pPlayerInfo.m_WBState) {
	case WBSTATE_PLAYING:
		if (pPlayerInfo.m_pPed->m_nPedState == PED_DEAD) {
			pPlayerInfo.m_pPed->ClearAdrenaline();
			pPlayerInfo.KillPlayer();
		}
		if (pPlayerInfo.m_pPed->m_nPedState == PED_ARRESTED) {
			pPlayerInfo.m_pPed->ClearAdrenaline();
			pPlayerInfo.ArrestPlayer();
		}
		break;
	case WBSTATE_WASTED:
#ifdef MISSION_REPLAY
		if (((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime > AddExtraDeathDelay() + 0x800) && (Timer_GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= AddExtraDeathDelay() + 0x800)) {
#else
		if (((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime > 0x800) && (Timer_GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
#endif
			TheCamera.SetFadeColour(200, 200, 200);
			TheCamera.Fade(2.0f, FADE_OUT);
		}

#ifdef MISSION_REPLAY
		if ((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime >= AddExtraDeathDelay() + 0x1000) {
#else
		if ((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime >= 0x1000) {
#endif
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			if (pPlayerInfo.m_bGetOutOfHospitalFree) {
				pPlayerInfo.m_bGetOutOfHospitalFree = false;
			} else {
				pPlayerInfo.m_nMoney = General_MaxInt(0, pPlayerInfo.m_nMoney - 100);
				pPlayerInfo.m_pPed->ClearWeapons();
			}

			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->GetStatus() != STATUS_WRECKED)
							pVehicle->SetStatus(STATUS_ABANDONED);
					} else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			Events_Initialise();
#ifdef SCREEN_DROPLETS
			ScreenDroplets::Initialise();
#endif
			Messages_ClearMessages();
			CarCtrl_ClearInterestingVehicleList();
			World_ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, true);
			Restart_FindClosestHospitalRestartPoint(&pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat); // rouz edit (ChatGPT)
			Restart_OverrideHospitalLevel = LEVEL_GENERIC; // rouz edit (ChatGPT)
			Restart_OverridePoliceStationLevel = LEVEL_GENERIC; // rouz edit (ChatGPT)
			GameLogic_PassTime(720);
			GameLogic_RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			GameLogic_AfterDeathArrestSetUpShortCutTaxi();
			GameLogic_SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MOTION_BLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			References_RemoveReferencesToPlayer();
			Population_countDownToPedsAtStart = 10; // rouz edit (ChatGPT)
			CarCtrl_CountDownToCarsAtStart = 10;
			CPad::GetPad(World_PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			if (Restart_bFadeInAfterNextDeath) {  // rouz edit (ChatGPT)
				TheCamera.SetFadeColour(200, 200, 200);
				TheCamera.Fade(4.0f, FADE_IN);
			} else
				Restart_bFadeInAfterNextDeath = true; // rouz edit (ChatGPT)
		}
		break;
	case WBSTATE_BUSTED:
#ifdef MISSION_REPLAY
		if (((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime > AddExtraDeathDelay() + 0x800) && (Timer_GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= AddExtraDeathDelay() + 0x800)) {
#else
		if (((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime > 0x800) && (Timer_GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
#endif
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.0f, FADE_OUT);
		}


		if (!TheScripts_IsPlayerOnAMission() && pPlayerInfo.m_nBustedAudioStatus == BUSTEDAUDIO_NONE) {
			if (General_GetRandomNumberInRangeInt(0, 4) == 0)
				pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_DONE;
			else {
				pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_LOADING;
				char name[12];
				sprintf(name, pPlayerInfo.m_nCurrentBustedAudio >= 10 ? "bust_%d" : "bust_0%d", pPlayerInfo.m_nCurrentBustedAudio);
				DMAudio_ClearMissionAudio(0);
				DMAudio_PreloadMissionAudio(0, name);
				pPlayerInfo.m_nCurrentBustedAudio = pPlayerInfo.m_nCurrentBustedAudio % TOTAL_BUSTED_AUDIO + 1;
			}
		}
		if ((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime > 4000 &&
			pPlayerInfo.m_nBustedAudioStatus == BUSTEDAUDIO_LOADING &&
			DMAudio_GetMissionAudioLoadingStatus(0) == 1) {
			DMAudio_PlayLoadedMissionAudio(0);
			pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_DONE;
		}
		
#ifdef MISSION_REPLAY
		if ((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime >= AddExtraDeathDelay() + 0x1000) {
#else
		if ((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime >= 0x1000) {
#endif
#ifdef FIX_BUGS
			pPlayerInfo.m_nBustedAudioStatus = BUSTEDAUDIO_NONE;
#endif
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			int takeMoney;

			switch (Wanted_GetWantedLevel(pPlayerInfo.m_pPed->m_pWanted)) {
			case 0:
			case 1:
				takeMoney = 100;
				break;
			case 2:
				takeMoney = 200;
				break;
			case 3:
				takeMoney = 400;
				break;
			case 4:
				takeMoney = 600;
				break;
			case 5:
				takeMoney = 900;
				break;
			case 6:
				takeMoney = 1500;
				break;
			}
			if (pPlayerInfo.m_bGetOutOfJailFree) {
				pPlayerInfo.m_bGetOutOfJailFree = false;
			} else {
				pPlayerInfo.m_nMoney = General_MaxInt(0, pPlayerInfo.m_nMoney - takeMoney);
				pPlayerInfo.m_pPed->ClearWeapons();
			}

			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->GetStatus() != STATUS_WRECKED)
							pVehicle->SetStatus(STATUS_ABANDONED);
					}
					else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			Events_Initialise();
#ifdef SCREEN_DROPLETS
			ScreenDroplets::Initialise();
#endif
			Messages_ClearMessages();
			CarCtrl_ClearInterestingVehicleList();
			World_ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, true);
			Restart_FindClosestPoliceRestartPoint(&pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat); // rouz edit (ChatGPT)
			Restart_OverrideHospitalLevel = LEVEL_GENERIC; // rouz edit (ChatGPT)
			Restart_OverridePoliceStationLevel = LEVEL_GENERIC; // rouz edit (ChatGPT)
			GameLogic_PassTime(720);
			GameLogic_RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			GameLogic_AfterDeathArrestSetUpShortCutTaxi();
			pPlayerInfo.m_pPed->ClearWeapons();
			GameLogic_SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MOTION_BLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			References_RemoveReferencesToPlayer();
			Population_countDownToPedsAtStart = 10; // rouz edit (ChatGPT)
			CarCtrl_CountDownToCarsAtStart = 10;
			CPad::GetPad(World_PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			if (Restart_bFadeInAfterNextArrest) { // rouz edit (ChatGPT)
				TheCamera.SetFadeColour(0, 0, 0);
				TheCamera.Fade(4.0f, FADE_IN);
			} else
				Restart_bFadeInAfterNextArrest = true; // rouz edit (ChatGPT)
		}
		break;
	case WBSTATE_FAILED_CRITICAL_MISSION:
#ifdef MISSION_REPLAY
		if (((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime > AddExtraDeathDelay() + 0x800) && (Timer_GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= AddExtraDeathDelay() + 0x800)) {
#else
		if (((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime > 0x800) && (Timer_GetPreviousTimeInMilliseconds() - pPlayerInfo.m_nWBTime <= 0x800)) {
#endif
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.0f, FADE_OUT);
		}
#ifdef MISSION_REPLAY
		if ((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime >= AddExtraDeathDelay() + 0x1000) {
#else
		if ((*Timer_GetTimeInMilliseconds()) - pPlayerInfo.m_nWBTime >= 0x1000) {
#endif
			pPlayerInfo.m_WBState = WBSTATE_PLAYING;
			if (pPlayerInfo.m_pPed->bInVehicle) {
				CVehicle *pVehicle = pPlayerInfo.m_pPed->m_pMyVehicle;
				if (pVehicle != nil) {
					if (pVehicle->pDriver == pPlayerInfo.m_pPed) {
						pVehicle->pDriver = nil;
						if (pVehicle->GetStatus() != STATUS_WRECKED)
							pVehicle->SetStatus(STATUS_ABANDONED);
					} else
						pVehicle->RemovePassenger(pPlayerInfo.m_pPed);
				}
			}
			Events_Initialise();
#ifdef SCREEN_DROPLETS
			ScreenDroplets::Initialise();
#endif
			Messages_ClearMessages();
			CarCtrl_ClearInterestingVehicleList();
			World_ClearExcitingStuffFromArea(pPlayerInfo.GetPos(), 4000.0f, true);
			Restart_FindClosestPoliceRestartPoint(&pPlayerInfo.GetPos(), &vecRestartPos, &fRestartFloat); // rouz edit (ChatGPT)
			Restart_OverridePoliceStationLevel = LEVEL_GENERIC; // rouz edit (ChatGPT)
			Restart_OverrideHospitalLevel = LEVEL_GENERIC; // rouz edit (ChatGPT)
			GameLogic_RestorePlayerStuffDuringResurrection(pPlayerInfo.m_pPed, vecRestartPos, fRestartFloat);
			GameLogic_SortOutStreamingAndMemory(pPlayerInfo.GetPos());
			TheCamera.m_fCamShakeForce = 0.0f;
			TheCamera.SetMotionBlur(0, 0, 0, 0, MOTION_BLUR_NONE);
			CPad::GetPad(0)->StopShaking(0);
			References_RemoveReferencesToPlayer();
			Population_countDownToPedsAtStart = 10; // rouz edit (ChatGPT)
			CarCtrl_CountDownToCarsAtStart = 10;
			CPad::GetPad(World_PlayerInFocus)->DisablePlayerControls = PLAYERCONTROL_ENABLED;
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(4.0f, FADE_IN);
		}
		break;
	case 4:
		return;
	}
}

void
GameLogic_RestorePlayerStuffDuringResurrection(CPlayerPed *pPlayerPed, CVector pos, float angle)
{
	GameLogic_ClearShortCut();
	CPlayerInfo* pPlayerInfo = pPlayerPed->GetPlayerInfoForThisPlayerPed();
	pPlayerPed->m_fHealth = pPlayerInfo->m_nMaxHealth;
	pPlayerPed->m_fArmour = 0.0f;
	pPlayerPed->bIsVisible = true;
	pPlayerPed->m_bloodyFootprintCountOrDeathTime = 0;
	pPlayerPed->bDoBloodyFootprints = false;
	pPlayerPed->m_nDrunkenness = 0;
	pPlayerPed->m_nFadeDrunkenness = 0;
	// Use the complete C motion-blur owner and its live shared state
	MBlur_ClearDrunkBlur();
	pPlayerPed->m_nDrunkCountdown = 0;
	pPlayerPed->ClearAdrenaline();
	pPlayerPed->m_fCurrentStamina = pPlayerPed->m_fMaxStamina;
	if (pPlayerPed->m_pFire)
		Fire_Extinguish(pPlayerPed->m_pFire);
	pPlayerPed->bInVehicle = false;
	pPlayerPed->m_pMyVehicle = nil;
	pPlayerPed->m_pVehicleAnim = nil;
	Wanted_Reset(pPlayerPed->m_pWanted);
	pPlayerPed->bCancelEnteringCar = false;
	pPlayerPed->RestartNonPartialAnims();
	pPlayerInfo->MakePlayerSafe(false);
	pPlayerPed->bRemoveFromWorld = false;
	pPlayerPed->ClearWeaponTarget();
	pPlayerPed->SetInitialState();
	CarCtrl_ClearInterestingVehicleList();
	pPlayerPed->Teleport(pos + CVector(0.0f, 0.0f, 1.0f));
	pPlayerPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
	pPlayerPed->m_fRotationCur = DEGTORAD(angle);
	pPlayerPed->m_fRotationDest = pPlayerPed->m_fRotationCur;
	pPlayerPed->SetHeading(pPlayerPed->m_fRotationCur);
	TheScripts_ClearSpaceForMissionEntity(pos, pPlayerPed);
	World_ClearExcitingStuffFromArea(pos, 4000.0f, true);
	pPlayerPed->RestoreHeadingRate();
	Game_currArea = AREA_MAIN_MAP;
	Streaming_RemoveBuildingsNotInArea(AREA_MAIN_MAP);
	TheCamera.SetCameraDirectlyInFrontForFollowPed_CamOnAString();
	TheCamera.Restore();
	References_RemoveReferencesToPlayer();
	Garages_PlayerArrestedOrDied();
	Stats_CheckPointReachedUnsuccessfully();
	World_Remove(pPlayerPed);
	World_Add(pPlayerPed);
	Hud_ResetWastedText();
	Streaming_StreamZoneModels(pos);
	clearWaterDrop = true;
}

void
GameLogic_ClearShortCut()
{
	if (GameLogic_pShortCutTaxi) {
		if (GameLogic_pShortCutTaxi->VehicleCreatedBy == MISSION_VEHICLE) {
			GameLogic_pShortCutTaxi->VehicleCreatedBy = RANDOM_VEHICLE;
			--CarCtrl_NumMissionCars;
			++CarCtrl_NumRandomCars;
		}
				// Access raw storage through the C store or pool API
		Radar_ClearBlipForEntity(BLIP_CAR, CPool_GetIndex(Pools_GetVehiclePool(), GameLogic_pShortCutTaxi));
				GameLogic_pShortCutTaxi = nil;
	}
	CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_SHORTCUT_TAXI);
}

void
GameLogic_SetUpShortCut(CVector vStartPos, float fStartAngle, CVector vEndPos, float fEndAngle)
{
	GameLogic_ClearShortCut();
	GameLogic_ShortCutState = SHORTCUT_INIT;
	GameLogic_ShortCutStart = vStartPos;
	GameLogic_ShortCutStartOrientation = fStartAngle;
	GameLogic_ShortCutDestination = vEndPos;
	GameLogic_ShortCutDestinationOrientation = fEndAngle;
	Streaming_RequestModel(MI_KAUFMAN, 0); // rouz edit (ChatGPT)
}

void
GameLogic_AbandonShortCutIfTaxiHasBeenMessedWith()
{
	if (!GameLogic_pShortCutTaxi)
		return;
	// Decide in C, the driver tests are only valid with a driver
	CPed *driver = GameLogic_pShortCutTaxi->pDriver;
	if (GameLogic_TaxiHasBeenMessedWith(driver != nil, driver && driver->DyingOrDead(),
		driver && driver->GetPedState() == PED_DRAG_FROM_CAR, driver && driver->GetPedState() == PED_ON_FIRE,
		driver && driver->m_objective == OBJECTIVE_LEAVE_CAR_AND_DIE, GameLogic_pShortCutTaxi->m_fHealth, GameLogic_pShortCutTaxi->bRenderScorched))
		GameLogic_ClearShortCut();
}

void
GameLogic_AbandonShortCutIfPlayerMilesAway()
{
	if (!GameLogic_pShortCutTaxi)
		return;
	if ((FindPlayerCoors() - GameLogic_pShortCutTaxi->GetPosition()).Magnitude() > 120.0f)
		GameLogic_ClearShortCut();
}

void
GameLogic_UpdateShortCut()
{
	switch (GameLogic_ShortCutState) {
	case SHORTCUT_INIT:
		if (!Streaming_HasModelLoaded(MI_KAUFMAN)) { // rouz edit (ChatGPT)
			Streaming_RequestModel(MI_KAUFMAN, 0); // rouz edit (ChatGPT)
			return;
		}
		// Allocate the shortcut taxi without invoking C++ new.
				// Access raw storage through the C store or pool API
		GameLogic_pShortCutTaxi = (CAutomobile*)((CVehicle*)CPool_New(Pools_GetVehiclePool()));
				if (GameLogic_pShortCutTaxi)
			std::allocator<CAutomobile>().construct(GameLogic_pShortCutTaxi, MI_KAUFMAN, RANDOM_VEHICLE);
		if (!GameLogic_pShortCutTaxi)
			return;
		GameLogic_pShortCutTaxi->SetPosition(GameLogic_ShortCutStart);
		GameLogic_pShortCutTaxi->SetHeading(DEGTORAD(GameLogic_ShortCutStartOrientation));
		GameLogic_pShortCutTaxi->PlaceOnRoadProperly();
		GameLogic_pShortCutTaxi->SetStatus(STATUS_PHYSICS);
		GameLogic_pShortCutTaxi->AutoPilot.m_nCarMission = MISSION_STOP_FOREVER;
		GameLogic_pShortCutTaxi->AutoPilot.m_nCruiseSpeed = 0;
		GameLogic_pShortCutTaxi->SetUpDriver();
		GameLogic_pShortCutTaxi->VehicleCreatedBy = MISSION_VEHICLE;
		++CarCtrl_NumMissionCars;
		--CarCtrl_NumRandomCars;
		TheScripts_ClearSpaceForMissionEntity(GameLogic_ShortCutStart, GameLogic_pShortCutTaxi);
		World_Add(GameLogic_pShortCutTaxi);
				// Access raw storage through the C store or pool API
		Radar_SetEntityBlip(BLIP_CAR, CPool_GetIndex(Pools_GetVehiclePool(), GameLogic_pShortCutTaxi), 0, BLIP_DISPLAY_MARKER_ONLY);
				GameLogic_ShortCutState = SHORTCUT_IDLE;
		break;
	case SHORTCUT_IDLE:
		if (FindPlayerPed()->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER && FindPlayerPed()->m_carInObjective == GameLogic_pShortCutTaxi) {
			CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_SHORTCUT_TAXI);
			FindPlayerPed()->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, GameLogic_pShortCutTaxi);
			GameLogic_ShortCutState = SHORTCUT_GETTING_IN;
		}
		GameLogic_AbandonShortCutIfTaxiHasBeenMessedWith();
		GameLogic_AbandonShortCutIfPlayerMilesAway();
		break;
	case SHORTCUT_GETTING_IN:
		if (GameLogic_pShortCutTaxi->pPassengers[0] == FindPlayerPed() ||
			GameLogic_pShortCutTaxi->pPassengers[1] == FindPlayerPed() ||
			GameLogic_pShortCutTaxi->pPassengers[2] == FindPlayerPed()) {
			GameLogic_pShortCutTaxi->AutoPilot.m_nTempAction = TEMPACT_GOFORWARD;
			GameLogic_pShortCutTaxi->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 2500;
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(2.5f, FADE_OUT);
			GameLogic_ShortCutState = SHORTCUT_TRANSITION;
			GameLogic_ShortCutTimer = (*Timer_GetTimeInMilliseconds()) + 3000;
			Messages_AddBigMessage(Text_Get(&TheText,"TAXI"), 4500, 1);
		}
		GameLogic_AbandonShortCutIfTaxiHasBeenMessedWith();
		break;
	case SHORTCUT_TRANSITION:
		if ((*Timer_GetTimeInMilliseconds()) > GameLogic_ShortCutTimer) {
			Timer_Suspend();
			ColStore_RequestCollision(GameLogic_ShortCutDestination);
			Streaming_LoadSceneCollision(GameLogic_ShortCutDestination);
			Streaming_LoadScene(GameLogic_ShortCutDestination);
			TheScripts_ClearSpaceForMissionEntity(GameLogic_ShortCutDestination, GameLogic_pShortCutTaxi);
			GameLogic_pShortCutTaxi->Teleport(GameLogic_ShortCutDestination);
			GameLogic_pShortCutTaxi->SetHeading(DEGTORAD(GameLogic_ShortCutDestinationOrientation));
			GameLogic_pShortCutTaxi->PlaceOnRoadProperly();
			GameLogic_pShortCutTaxi->SetMoveSpeed(GameLogic_pShortCutTaxi->GetForward() * 0.4f);
			GameLogic_ShortCutTimer = (*Timer_GetTimeInMilliseconds()) + 1500;
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(1.0f, FADE_IN);
			GameLogic_ShortCutState = SHORTCUT_ARRIVING;
			Timer_Resume();
		}
		break;
	case SHORTCUT_ARRIVING:
		if ((*Timer_GetTimeInMilliseconds()) > GameLogic_ShortCutTimer) {
			World_Players[World_PlayerInFocus].m_nMoney = General_MaxInt(0, World_Players[World_PlayerInFocus].m_nMoney - SHORTCUT_TAXI_COST);
			FindPlayerPed()->SetObjective(OBJECTIVE_LEAVE_CAR, GameLogic_pShortCutTaxi);
			FindPlayerPed()->m_carInObjective = GameLogic_pShortCutTaxi;
			GameLogic_ShortCutState = SHORTCUT_GETTING_OUT;
		}
		GameLogic_AbandonShortCutIfTaxiHasBeenMessedWith();
		break;
	case SHORTCUT_GETTING_OUT:
		if (GameLogic_pShortCutTaxi->pPassengers[0] != FindPlayerPed() &&
			GameLogic_pShortCutTaxi->pPassengers[1] != FindPlayerPed() &&
			GameLogic_pShortCutTaxi->pPassengers[2] != FindPlayerPed()) {
			CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_SHORTCUT_TAXI);
			GameLogic_pShortCutTaxi->AutoPilot.m_nCarMission = MISSION_CRUISE;
			GameLogic_pShortCutTaxi->AutoPilot.m_nCruiseSpeed = 18;
			CarCtrl_JoinCarWithRoadSystem(GameLogic_pShortCutTaxi);
			GameLogic_pShortCutTaxi->VehicleCreatedBy = RANDOM_VEHICLE;
			++CarCtrl_NumRandomCars;
			--CarCtrl_NumMissionCars;
						// Access raw storage through the C store or pool API
			Radar_ClearBlipForEntity(BLIP_CAR, CPool_GetIndex(Pools_GetVehiclePool(), GameLogic_pShortCutTaxi));
						GameLogic_ShortCutState = SHORTCUT_NONE;
			GameLogic_pShortCutTaxi = nil;
		}
		GameLogic_AbandonShortCutIfTaxiHasBeenMessedWith();
		break;
	}
}

void
GameLogic_AddShortCutPointAfterDeath(CVector point, float angle)
{
	if (GameLogic_NumAfterDeathStartPoints >= NUM_SHORTCUT_START_POINTS)
		return;
	GameLogic_AfterDeathStartPoints[GameLogic_NumAfterDeathStartPoints] = point;
	GameLogic_AfterDeathStartPointOrientation[GameLogic_NumAfterDeathStartPoints] = angle;
	GameLogic_NumAfterDeathStartPoints++;
}

void
GameLogic_AddShortCutDropOffPointForMission(CVector point, float angle)
{
	GameLogic_ShortCutDropOffForMission = point;
	GameLogic_ShortCutDropOffOrientationForMission = angle;
	GameLogic_MissionDropOffReadyToBeUsed = true;
}

void
GameLogic_RemoveShortCutDropOffPointForMission()
{
	GameLogic_MissionDropOffReadyToBeUsed = false;
}

void
GameLogic_AfterDeathArrestSetUpShortCutTaxi()
{
	if (!GameLogic_MissionDropOffReadyToBeUsed)
		return;
	// Find the nearest start point in C
	float fDistanceToPoint;
	CVector playerCoors = FindPlayerCoors();
	int nClosestPoint = GameLogic_FindClosestPoint((const float*)GameLogic_AfterDeathStartPoints, NUM_SHORTCUT_START_POINTS,
		playerCoors.x, playerCoors.y, playerCoors.z, &fDistanceToPoint);
	if (fDistanceToPoint < 100.0f)
		GameLogic_SetUpShortCut(GameLogic_AfterDeathStartPoints[nClosestPoint],
			GameLogic_AfterDeathStartPointOrientation[nClosestPoint],
			GameLogic_ShortCutDropOffForMission,
			GameLogic_ShortCutDropOffOrientationForMission);
	GameLogic_MissionDropOffReadyToBeUsed = false;
}

void
GameLogic_Save(uint8* buf, uint32* size)
{
INITSAVEBUF
		// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &GameLogic_NumAfterDeathStartPoints, sizeof(GameLogic_NumAfterDeathStartPoints));
		*size += sizeof(GameLogic_NumAfterDeathStartPoints);
	for (int i = 0; i < NUM_SHORTCUT_START_POINTS; i++) {
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &GameLogic_AfterDeathStartPoints[i].x, sizeof(GameLogic_AfterDeathStartPoints[i].x));
				*size += sizeof(GameLogic_AfterDeathStartPoints[i].x);
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &GameLogic_AfterDeathStartPoints[i].y, sizeof(GameLogic_AfterDeathStartPoints[i].y));
				*size += sizeof(GameLogic_AfterDeathStartPoints[i].y);
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &GameLogic_AfterDeathStartPoints[i].z, sizeof(GameLogic_AfterDeathStartPoints[i].z));
				*size += sizeof(GameLogic_AfterDeathStartPoints[i].z);
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &GameLogic_AfterDeathStartPointOrientation[i], sizeof(GameLogic_AfterDeathStartPointOrientation[i]));
				*size += sizeof(GameLogic_AfterDeathStartPointOrientation[i]);
	}
VALIDATESAVEBUF(*size)
}

void
GameLogic_Load(uint8* buf, uint32 size)
{
INITSAVEBUF
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&GameLogic_NumAfterDeathStartPoints, &buf, sizeof(GameLogic_NumAfterDeathStartPoints));
		for (int i = 0; i < NUM_SHORTCUT_START_POINTS; i++) {
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&GameLogic_AfterDeathStartPoints[i].x, &buf, sizeof(GameLogic_AfterDeathStartPoints[i].x));
		ReadSaveBuf(&GameLogic_AfterDeathStartPoints[i].y, &buf, sizeof(GameLogic_AfterDeathStartPoints[i].y));
		ReadSaveBuf(&GameLogic_AfterDeathStartPoints[i].z, &buf, sizeof(GameLogic_AfterDeathStartPoints[i].z));
		ReadSaveBuf(&GameLogic_AfterDeathStartPointOrientation[i], &buf, sizeof(GameLogic_AfterDeathStartPointOrientation[i]));
			}
VALIDATESAVEBUF(size)
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
