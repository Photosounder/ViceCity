//+ rouz edit (ChatGPT)
#include "common.h"

#include "CarAI.h"
#include "CarAILogic.h"

#include "Accident.h"
#include "AutoPilot.h"
#include "CarCtrl.h"
#include "General.h"
#include "HandlingMgr.h"
#include "ModelIndices.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "DMAudio.h"
#include "Fire.h"
#include "Pools.h"
#include "Population.h"
#include "Timer.h"
#include "TrafficLights.h"
#include "Vehicle.h"
#include "World.h"
#include "ZoneCull.h"

#define DISTANCE_TO_SWITCH_DISTANCE_GOTO 20.0f

float CarAI_FindSwitchDistanceClose(CVehicle* pVehicle)
{
	return pVehicle->AutoPilot.m_nSwitchDistance;
}

float CarAI_FindSwitchDistanceFarNormalVehicle(CVehicle* pVehicle)
{
	return CarAI_FindSwitchDistanceClose(pVehicle) + 5.0f;
}

float CarAI_FindSwitchDistanceFar(CVehicle* pVehicle)
{
	// Delegate to the C implementation
	return CarAILogic_FindSwitchDistanceFar(pVehicle->AutoPilot.m_nSwitchDistance, pVehicle->bIsLawEnforcer);
}

void CarAI_BackToCruisingIfNoWantedLevel(CVehicle* pVehicle)
{
	if (FindPlayerPed()->m_pWanted->m_bIgnoredByEveryone || pVehicle->bIsLawEnforcer &&
		(Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) == 0 || FindPlayerPed()->m_pWanted->m_bIgnoredByCops || CullZones_NoPolice())) {
		CarCtrl_JoinCarWithRoadSystem(pVehicle);
		pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
		pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
		pVehicle->m_bSirenOrAlarm = false;
		if (CullZones_NoPolice())
			pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
	}
}

void CarAI_UpdateCarAI(CVehicle* pVehicle)
{
	if (pVehicle->bIsLawEnforcer){
		if (pVehicle->AutoPilot.m_nCarMission == MISSION_BLOCKCAR_FARAWAY ||
			pVehicle->AutoPilot.m_nCarMission == MISSION_RAMPLAYER_FARAWAY ||
			pVehicle->AutoPilot.m_nCarMission == MISSION_BLOCKPLAYER_CLOSE ||
			pVehicle->AutoPilot.m_nCarMission == MISSION_RAMPLAYER_CLOSE)
			pVehicle->AutoPilot.m_nCruiseSpeed = CarAI_FindPoliceCarSpeedForWantedLevel(pVehicle);
	}
	switch (pVehicle->GetStatus()){
	case STATUS_PLAYER:
	case STATUS_PLAYER_PLAYBACKFROMBUFFER:
	case STATUS_TRAIN_MOVING:
	case STATUS_TRAIN_NOT_MOVING:
	case STATUS_HELI:
	case STATUS_PLANE:
	case STATUS_PLAYER_REMOTE:
	case STATUS_PLAYER_DISABLED:
		break;
	case STATUS_SIMPLE:
	case STATUS_PHYSICS:
		switch (pVehicle->AutoPilot.m_nCarMission) {
		case MISSION_RAMPLAYER_FARAWAY:
			if (CarAI_FindSwitchDistanceClose(pVehicle) > (FindPlayerCoors() - pVehicle->GetPosition()).Magnitude2D() ||
				pVehicle->AutoPilot.m_bIgnorePathfinding) {
				pVehicle->AutoPilot.m_nCarMission = MISSION_RAMPLAYER_CLOSE;
				if (pVehicle->UsesSiren())
					pVehicle->m_bSirenOrAlarm = true;
			}
			CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
			break;
		case MISSION_RAMPLAYER_CLOSE:
			if (CarAI_FindSwitchDistanceFar(pVehicle) >= (FindPlayerCoors() - pVehicle->GetPosition()).Magnitude2D() ||
				pVehicle->AutoPilot.m_bIgnorePathfinding) {
				if (FindPlayerVehicle()) {
					if (pVehicle->GetHasCollidedWith(FindPlayerVehicle())) {
						if (pVehicle->AutoPilot.m_nTempAction != TEMPACT_TURNLEFT && pVehicle->AutoPilot.m_nTempAction != TEMPACT_TURNRIGHT) {
							if (FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.05f) {
								pVehicle->AutoPilot.m_nTempAction = TEMPACT_REVERSE;
								pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 800;
							}
							else {
								pVehicle->AutoPilot.m_nTempAction = TEMPACT_REVERSE;
								pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 50;
							}
						}
					}
				}
				if (FindPlayerVehicle() && FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.05f)
#ifdef FIX_BUGS
					pVehicle->m_nTimeBlocked += Timer_GetTimeStepInMilliseconds();
#else
					pVehicle->m_nTimeBlocked += 1000.0f / 60.0f * (*Timer_GetTimeStep());
#endif
				else
					pVehicle->m_nTimeBlocked = 0;
				if (!FindPlayerVehicle() || FindPlayerVehicle()->IsUpsideDown() ||
					FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.05f && pVehicle->m_nTimeBlocked > TIME_COPS_WAIT_TO_EXIT_AFTER_STOPPING) {
					if (pVehicle->bIsLawEnforcer &&
						(pVehicle->GetModelIndex() != MI_RHINO || pVehicle->m_randomSeed > 10000) &&
						(FindPlayerCoors() - pVehicle->GetPosition()).Magnitude2D() < 10.0f) {
						CarAI_TellOccupantsToLeaveCar(pVehicle);
						pVehicle->AutoPilot.m_nCruiseSpeed = 0;
						pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
						if (Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) <= 1)
							pVehicle->m_bSirenOrAlarm = false;
					}
				}
			}
			else if (!CarCtrl_JoinCarWithRoadSystemGotoCoors(pVehicle, FindPlayerCoors(), true)){
				pVehicle->AutoPilot.m_nCarMission = MISSION_RAMPLAYER_FARAWAY;
				pVehicle->m_bSirenOrAlarm = false;
				pVehicle->m_nCarHornTimer = 0;
			}
			if (pVehicle->bIsLawEnforcer)
				CarAI_MellowOutChaseSpeed(pVehicle);
			CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
			break;
		case MISSION_BLOCKPLAYER_FARAWAY:
			if (CarAI_FindSwitchDistanceClose(pVehicle) > (FindPlayerCoors() - pVehicle->GetPosition()).Magnitude2D() ||
				pVehicle->AutoPilot.m_bIgnorePathfinding) {
				pVehicle->AutoPilot.m_nCarMission = MISSION_BLOCKPLAYER_CLOSE;
				if (pVehicle->UsesSiren())
					pVehicle->m_bSirenOrAlarm = true;
			}
			CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
			break;
		case MISSION_BLOCKPLAYER_CLOSE:
			if (CarAI_FindSwitchDistanceFar(pVehicle) >= (FindPlayerCoors() - pVehicle->GetPosition()).Magnitude2D() ||
				pVehicle->AutoPilot.m_bIgnorePathfinding) {
				if (FindPlayerVehicle() && FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.04f)
#ifdef FIX_BUGS
					pVehicle->m_nTimeBlocked += Timer_GetTimeStepInMilliseconds();
#else
					pVehicle->m_nTimeBlocked += 1000.0f / 60.0f * (*Timer_GetTimeStep());
#endif
				else
					pVehicle->m_nTimeBlocked = 0;
				if (!FindPlayerVehicle() || FindPlayerVehicle()->IsUpsideDown() ||
					FindPlayerVehicle()->GetMoveSpeed().Magnitude() < 0.04f && pVehicle->m_nTimeBlocked > TIME_COPS_WAIT_TO_EXIT_AFTER_STOPPING) {
					if (pVehicle->bIsLawEnforcer &&
						(pVehicle->GetModelIndex() != MI_RHINO || pVehicle->m_randomSeed > 10000) &&
						(FindPlayerCoors() - pVehicle->GetPosition()).Magnitude2D() < 10.0f) {
						CarAI_TellOccupantsToLeaveCar(pVehicle);
						pVehicle->AutoPilot.m_nCruiseSpeed = 0;
						pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
						if (Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) <= 1)
							pVehicle->m_bSirenOrAlarm = false;
					}
				}
			}else if (!CarCtrl_JoinCarWithRoadSystemGotoCoors(pVehicle, FindPlayerCoors(), true)) {
				pVehicle->AutoPilot.m_nCarMission = MISSION_BLOCKPLAYER_FARAWAY;
				pVehicle->m_bSirenOrAlarm = false;
				pVehicle->m_nCarHornTimer = 0;
			}
			if (pVehicle->bIsLawEnforcer)
				CarAI_MellowOutChaseSpeed(pVehicle);
			CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
			break;
		case MISSION_GOTOCOORDS:
			if ((AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors) - pVehicle->GetPosition()).Magnitude2D() < CarAI_FindSwitchDistanceClose(pVehicle) ||
			  pVehicle->AutoPilot.m_bIgnorePathfinding)
				pVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
			break;
		case MISSION_GOTOCOORDS_STRAIGHT:
		{
			float distance = (AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors) - pVehicle->GetPosition()).Magnitude2D();
			if ((pVehicle->bIsAmbulanceOnDuty || pVehicle->bIsFireTruckOnDuty) && distance < 20.0f)
				pVehicle->AutoPilot.m_nCarMission = MISSION_EMERGENCYVEHICLE_STOP;
			if (distance < 3.0f){
				pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
				pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
				if (pVehicle->bParking) {
					CarAI_TellOccupantsToLeaveCar(pVehicle);
					pVehicle->bParking = false;
				}
			}
			else if (distance > CarAI_FindSwitchDistanceFarNormalVehicle(pVehicle) && !pVehicle->AutoPilot.m_bIgnorePathfinding && ((*Timer_GetFrameCounter()) & 7) == 0){
				pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
				pVehicle->AutoPilot.m_nCarMission = (CarCtrl_JoinCarWithRoadSystemGotoCoors(pVehicle, AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors), true)) ?
					MISSION_GOTOCOORDS_STRAIGHT : MISSION_GOTOCOORDS;
			}
			break;
		}
		case MISSION_EMERGENCYVEHICLE_STOP:
			if (pVehicle->GetMoveSpeed().Magnitude2D() < 0.01f){
				if (pVehicle->bIsAmbulanceOnDuty){
					float distance = 30.0f;
					if (AccidentManager_FindNearestAccident(&gAccidentManager,Events_VectorFromNative(AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors)),&distance)){
						CarAI_TellOccupantsToLeaveCar(pVehicle);
						pVehicle->AutoPilot.m_nCarMission = MISSION_STOP_FOREVER;
					}else{
						CarCtrl_JoinCarWithRoadSystem(pVehicle);
						pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
						pVehicle->m_bSirenOrAlarm = false;
						pVehicle->AutoPilot.m_nCruiseSpeed = 17;
						if (pVehicle->bIsAmbulanceOnDuty){
							pVehicle->bIsAmbulanceOnDuty = false;
							--CarCtrl_NumAmbulancesOnDuty;
						}
					}
				}
				if (pVehicle->bIsFireTruckOnDuty) {
					float distance = 30.0f;
					if (FireManager_FindNearestFire(&gFireManager,Fire_VectorFromNative(AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors)), &distance)) {
						CarAI_TellOccupantsToLeaveCar(pVehicle);
						pVehicle->AutoPilot.m_nCarMission = MISSION_STOP_FOREVER;
					}
					else {
						CarCtrl_JoinCarWithRoadSystem(pVehicle);
						pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
						pVehicle->m_bSirenOrAlarm = false;
						pVehicle->AutoPilot.m_nCruiseSpeed = 17;
						if (pVehicle->bIsFireTruckOnDuty) {
							pVehicle->bIsFireTruckOnDuty = false;
							--CarCtrl_NumFiretrucksOnDuty;
						}
					}
				}
			}
			break;
		case MISSION_GOTOCOORDS_ACCURATE:
			if ((AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors) - pVehicle->GetPosition()).Magnitude2D() < CarAI_FindSwitchDistanceClose(pVehicle) ||
				pVehicle->AutoPilot.m_bIgnorePathfinding)
				pVehicle->AutoPilot.m_nCarMission = MISSION_GOTO_COORDS_STRAIGHT_ACCURATE;
			break;
		case MISSION_GOTO_COORDS_STRAIGHT_ACCURATE:
		{
			float distance = (AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors) - pVehicle->GetPosition()).Magnitude2D();
			if (distance < 1.0f) {
				pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
				pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
				if (pVehicle->bParking) {
					CarAI_TellOccupantsToLeaveCar(pVehicle);
					pVehicle->bParking = false;
				}
			}
			else if (distance > CarAI_FindSwitchDistanceFarNormalVehicle(pVehicle) && !pVehicle->AutoPilot.m_bIgnorePathfinding && ((*Timer_GetFrameCounter()) & 7) == 0) {
				pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
				pVehicle->AutoPilot.m_nCarMission = (CarCtrl_JoinCarWithRoadSystemGotoCoors(pVehicle, AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors), true)) ?
					MISSION_GOTO_COORDS_STRAIGHT_ACCURATE : MISSION_GOTOCOORDS_ACCURATE;
			}
			break;
		}
		case MISSION_RAMCAR_FARAWAY:
			if (pVehicle->AutoPilot.m_pTargetCar){
				if ((pVehicle->GetPosition() - pVehicle->AutoPilot.m_pTargetCar->GetPosition()).Magnitude2D() < CarAI_FindSwitchDistanceClose(pVehicle) ||
				  pVehicle->AutoPilot.m_bIgnorePathfinding)
					pVehicle->AutoPilot.m_nCarMission = MISSION_RAMCAR_CLOSE;
			}else{
				pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
			}
			break;
		case MISSION_RAMCAR_CLOSE:
			if (pVehicle->AutoPilot.m_pTargetCar){
#ifdef FIX_BUGS // btw fixed in SA
				if (FindPlayerVehicle() == pVehicle->AutoPilot.m_pTargetCar)
#endif
					CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
				if ((pVehicle->AutoPilot.m_pTargetCar->GetPosition() - pVehicle->GetPosition()).Magnitude2D() <= CarAI_FindSwitchDistanceFar(pVehicle) ||
				  pVehicle->AutoPilot.m_bIgnorePathfinding){
					if (pVehicle->GetHasCollidedWith(pVehicle->AutoPilot.m_pTargetCar)){
						if (pVehicle->GetMoveSpeed().Magnitude() < 0.04f){
							pVehicle->AutoPilot.m_nTempAction = TEMPACT_REVERSE;
							pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 800;
						}
					}
				}else{
					pVehicle->AutoPilot.m_nCarMission = MISSION_RAMCAR_FARAWAY;
					CarCtrl_JoinCarWithRoadSystem(pVehicle);
				}
			}else{
				pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
			}
			break;
		case MISSION_BLOCKCAR_FARAWAY:
			if (pVehicle->AutoPilot.m_pTargetCar){
				if ((pVehicle->AutoPilot.m_pTargetCar->GetPosition() - pVehicle->GetPosition()).Magnitude2D() < CarAI_FindSwitchDistanceClose(pVehicle) ||
				  pVehicle->AutoPilot.m_bIgnorePathfinding){
					pVehicle->AutoPilot.m_nCarMission = MISSION_BLOCKCAR_CLOSE;
					if (pVehicle->UsesSiren())
						pVehicle->m_bSirenOrAlarm = true;
				}
			}else{
				pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
			}
			break;
		case MISSION_BLOCKCAR_CLOSE:
			if (pVehicle->AutoPilot.m_pTargetCar){
				if ((pVehicle->AutoPilot.m_pTargetCar->GetPosition() - pVehicle->GetPosition()).Magnitude2D() > CarAI_FindSwitchDistanceFar(pVehicle) &&
				  !pVehicle->AutoPilot.m_bIgnorePathfinding){
					pVehicle->AutoPilot.m_nCarMission = MISSION_BLOCKCAR_FARAWAY;
					pVehicle->m_bSirenOrAlarm = false;
					pVehicle->m_nCarHornTimer = 0;
					CarCtrl_JoinCarWithRoadSystem(pVehicle);
				}
			}else{
				pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
			}
			break;
		case MISSION_ATTACKPLAYER:
			if (pVehicle->bIsLawEnforcer)
				CarAI_MellowOutChaseSpeedBoat(pVehicle);
			CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
			break;
		case MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1:
			if (((CVector2D)(AutoPilot_ToVector(pVehicle->AutoPilot.m_vecDestinationCoors)) - pVehicle->GetPosition()).Magnitude() < 1.5f)
				pVehicle->AutoPilot.m_nCarMission = MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_2;
			CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
			break;
		case MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_2:
		{
			float distance = ((CVector2D)FindPlayerCoors() - pVehicle->GetPosition()).Magnitude();
			if (distance < 13.0f) {
				CarAI_TellOccupantsToLeaveCar(pVehicle);
				pVehicle->AutoPilot.m_nCruiseSpeed = 0;
				pVehicle->AutoPilot.m_nCarMission = MISSION_STOP_FOREVER;
			}
			if (distance > 70.0f || FindPlayerPed()->m_pWanted->m_bIgnoredByEveryone ||
				(Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) == 0 || FindPlayerPed()->m_pWanted->m_bIgnoredByCops || CullZones_NoPolice())) {
				CarAI_TellOccupantsToLeaveCar(pVehicle);
				pVehicle->AutoPilot.m_nCruiseSpeed = 0;
				pVehicle->AutoPilot.m_nCarMission = MISSION_STOP_FOREVER;
			}
			break;
		}
		case MISSION_BLOCKPLAYER_FORWARDANDBACK:
		{
			CVector2D diff = (CVector2D)FindPlayerCoors() - pVehicle->GetPosition();
			float distance = General_MaxFloat(0.001f, diff.Magnitude());
			if (!FindPlayerVehicle() || DotProduct2D(CVector2D(diff.x / distance, diff.y / distance), FindPlayerSpeed()) > 0.05f)
				pVehicle->AutoPilot.m_nCarMission = MISSION_BLOCKPLAYER_CLOSE;
			CarAI_BackToCruisingIfNoWantedLevel(pVehicle);
			break;
		}
		default:
			if (pVehicle->bIsLawEnforcer && Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) > 0 && !CullZones_NoPolice()){
				if (ABS(FindPlayerCoors().x - pVehicle->GetPosition().x) > 10.0f ||
				  ABS(FindPlayerCoors().y - pVehicle->GetPosition().y) > 10.0f){
					pVehicle->AutoPilot.m_nCruiseSpeed = CarAI_FindPoliceCarSpeedForWantedLevel(pVehicle);
					pVehicle->SetStatus(STATUS_PHYSICS);
					pVehicle->AutoPilot.m_nCarMission = 
						pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BOAT ? CarAI_FindPoliceBoatMissionForWantedLevel() : CarAI_FindPoliceCarMissionForWantedLevel();
					pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
					pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
				}else if (pVehicle->AutoPilot.m_nCarMission == MISSION_CRUISE){
					pVehicle->SetStatus(STATUS_PHYSICS);
					CarAI_TellOccupantsToLeaveCar(pVehicle);
					pVehicle->AutoPilot.m_nCruiseSpeed = 0;
					pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
					if (Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) <= 1)
						pVehicle->m_bSirenOrAlarm = false;
				}
			}
			break;
		}
		break;
	case STATUS_ABANDONED:
	case STATUS_WRECKED:
		pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
		pVehicle->AutoPilot.m_nCruiseSpeed = 0;
		break;
	}
	if (pVehicle->bIsLawEnforcer && Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) >= 1 && CullZones_PoliceAbandonCars()) {
		CarAI_TellOccupantsToLeaveCar(pVehicle);
		pVehicle->AutoPilot.m_nCruiseSpeed = 0;
		pVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
	}
	float flatSpeed = pVehicle->GetMoveSpeed().MagnitudeSqr2D();
	if (flatSpeed > SQR(0.018f)){
		pVehicle->AutoPilot.m_nTimeToStartMission = (*Timer_GetTimeInMilliseconds());
		pVehicle->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
	}
	if (pVehicle->GetStatus() == STATUS_PHYSICS && pVehicle->AutoPilot.m_nTempAction == TEMPACT_NONE){
		if (pVehicle->AutoPilot.m_nCarMission != MISSION_NONE){
			if (pVehicle->AutoPilot.m_nCarMission != MISSION_STOP_FOREVER &&
				pVehicle->AutoPilot.m_nCarMission != MISSION_BLOCKPLAYER_HANDBRAKESTOP &&
			  pVehicle->AutoPilot.m_nCruiseSpeed != 0 &&
			  (pVehicle->VehicleCreatedBy != RANDOM_VEHICLE || pVehicle->AutoPilot.m_nCarMission != MISSION_CRUISE)){
				if (pVehicle->AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_STOP_FOR_CARS
					&& pVehicle->AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_STOP_FOR_CARS_IGNORE_LIGHTS ||
					pVehicle->VehicleCreatedBy == MISSION_VEHICLE
					) {
					if ((*Timer_GetTimeInMilliseconds()) - pVehicle->m_nLastTimeCollided > 500)
						pVehicle->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
					if (flatSpeed < SQR(0.018f) && (*Timer_GetTimeInMilliseconds()) - pVehicle->AutoPilot.m_nAntiReverseTimer > 2000){
						pVehicle->AutoPilot.m_nTempAction = TEMPACT_REVERSE;
						if (pVehicle->AutoPilot.m_nCarMission != MISSION_NONE &&
						  pVehicle->AutoPilot.m_nCarMission != MISSION_CRUISE || pVehicle->VehicleCreatedBy == MISSION_VEHICLE)
							pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 1500;
						else
							pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 750;
						pVehicle->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
						if (pVehicle->VehicleCreatedBy == RANDOM_VEHICLE)
							pVehicle->AutoPilot.m_nDrivingStyle = General_MaxInt(DRIVINGSTYLE_AVOID_CARS, pVehicle->AutoPilot.m_nDrivingStyle);
						pVehicle->PlayCarHorn();
					}
				}
			}
		}
	}
	if ((pVehicle->m_randomSeed & 7) == 0){
		if ((*Timer_GetTimeInMilliseconds()) - pVehicle->AutoPilot.m_nTimeToStartMission > 30000 &&
		  Timer_GetPreviousTimeInMilliseconds() - pVehicle->AutoPilot.m_nTimeToStartMission <= 30000 &&
		  pVehicle->AutoPilot.m_nCarMission == MISSION_CRUISE &&
		  !TrafficLights_ShouldCarStopForBridge(pVehicle)){
			pVehicle->SetStatus(STATUS_PHYSICS);
			CarCtrl_SwitchVehicleToRealPhysics(pVehicle);
			pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
			pVehicle->AutoPilot.m_nTempAction = TEMPACT_REVERSE;
			pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 400;
		}
	}
	if (pVehicle->bIsLawEnforcer) {
		if (pVehicle->AutoPilot.m_nCarMission == MISSION_RAMPLAYER_FARAWAY ||
			pVehicle->AutoPilot.m_nCarMission == MISSION_RAMPLAYER_CLOSE) {
			if (FindPlayerVehicle() && FindPlayerVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE)
				pVehicle->AutoPilot.m_nCarMission = MISSION_BLOCKPLAYER_FARAWAY;
		}
	}
	if (pVehicle->GetUp().z < -0.7f){ // rouz edit (ChatGPT)
		pVehicle->AutoPilot.m_nTempAction = TEMPACT_WAIT;
		pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 1000;
	}
	if (pVehicle->AutoPilot.m_nTempAction == TEMPACT_NONE){
		switch (pVehicle->AutoPilot.m_nCarMission){
		case MISSION_RAMPLAYER_FARAWAY:
		case MISSION_RAMPLAYER_CLOSE:
		case MISSION_BLOCKPLAYER_FARAWAY:
		case MISSION_BLOCKPLAYER_CLOSE:
			if (FindPlayerVehicle() && FindPlayerSpeed().Magnitude() > pVehicle->GetMoveSpeed().Magnitude()){
				if (FindPlayerSpeed().Magnitude() > 0.1f){
					if (DotProduct2D(FindPlayerVehicle()->GetForward(), pVehicle->GetForward()) > 0.0f){
						CVector2D dist = pVehicle->GetPosition() - FindPlayerCoors();
						CVector2D speed = FindPlayerSpeed();
						if (0.5f * dist.Magnitude() * speed.Magnitude() < DotProduct2D(dist, speed)){
							if ((FindPlayerCoors() - pVehicle->GetPosition()).Magnitude() > 12.0f){
								pVehicle->AutoPilot.m_nTempAction = TEMPACT_WAIT;
								pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 500;
							}
						}
					}
				}
			}
			break;
		default: break;
		}
	}
	if (pVehicle->pDriver && pVehicle->pDriver->m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS){
		if ((pVehicle->GetPosition() - FindPlayerCoors()).Magnitude() < 15.0f){
			if (!FindPlayerVehicle() || pVehicle->GetHasCollidedWith(FindPlayerVehicle())){
				pVehicle->AutoPilot.m_nTempAction = TEMPACT_WAIT;
				pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 3000;
			}
		}
	}
	if (pVehicle->m_bSirenOrAlarm){
		if ((uint8)(pVehicle->m_randomSeed ^ General_GetRandomNumber()) == 0xAD)
			pVehicle->m_nCarHornTimer = 45;
	}
	float target = 1.0f;
	if (pVehicle->AutoPilot.m_nCarMission == MISSION_CRUISE)
		target = CarCtrl_FindSpeedMultiplierWithSpeedFromNodes(pVehicle->AutoPilot.m_nCruiseSpeedMultiplierType);
	float change = (*Timer_GetTimeStep()) * 0.01f;
	if (General_AbsDiffBelow(pVehicle->AutoPilot.m_fCruiseSpeedMultiplier, target, change))
		pVehicle->AutoPilot.m_fCruiseSpeedMultiplier = target;
	else if (pVehicle->AutoPilot.m_fCruiseSpeedMultiplier > target)
		pVehicle->AutoPilot.m_fCruiseSpeedMultiplier -= change;
	else
		pVehicle->AutoPilot.m_fCruiseSpeedMultiplier += change;

	if (pVehicle->bIsLawEnforcer && Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) > 0) {
		if (!FindPlayerVehicle() ||
			FindPlayerVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR ||
			FindPlayerVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE) {
			if (pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BOAT) {
				pVehicle->AutoPilot.m_nTempAction = TEMPACT_WAIT;
				pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 1000;
			}
		}
		else if (FindPlayerVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_BOAT) {
			if (pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR ||
				pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE) {
				pVehicle->AutoPilot.m_nTempAction = TEMPACT_WAIT;
				pVehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 1000;
			}
		}
	}
}

void CarAI_CarHasReasonToStop(CVehicle* pVehicle)
{
	pVehicle->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
}

float CarAI_GetCarToGoToCoors(CVehicle* pVehicle, CVector* pTarget)
{
	if (pVehicle->AutoPilot.m_nCarMission != MISSION_GOTOCOORDS && pVehicle->AutoPilot.m_nCarMission != MISSION_GOTOCOORDS_STRAIGHT){
		pVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
		pVehicle->AutoPilot.m_nTempAction = TEMPACT_NONE;
		pVehicle->AutoPilot.m_nCruiseSpeed = 20;
		pVehicle->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
		pVehicle->SetStatus(STATUS_PHYSICS);
		pVehicle->AutoPilot.m_nCarMission = (CarCtrl_JoinCarWithRoadSystemGotoCoors(pVehicle, *pTarget, false)) ?
			MISSION_GOTOCOORDS_STRAIGHT : MISSION_GOTOCOORDS;
	}else if (General_AbsDiffAbove(pTarget->x, pVehicle->AutoPilot.m_vecDestinationCoors.x, 2.0f) ||
	  General_AbsDiffAbove(pTarget->y, pVehicle->AutoPilot.m_vecDestinationCoors.y, 2.0f)){
		pVehicle->AutoPilot.m_vecDestinationCoors = AutoPilot_FromVector(*pTarget);
	}
	return (pVehicle->GetPosition() - *pTarget).Magnitude2D();
}

float CarAI_GetCarToParkAtCoors(CVehicle* pVehicle, CVector* pTarget)
{
	CarAI_GetCarToGoToCoors(pVehicle, pTarget);
	pVehicle->bParking = true;
	pVehicle->AutoPilot.m_nCruiseSpeed = 10;
	return (pVehicle->GetPosition() - *pTarget).Magnitude2D();
}

void CarAI_AddPoliceCarOccupants(CVehicle* pVehicle)
{
	if (pVehicle->bOccupantsHaveBeenGenerated)
		return;
	pVehicle->bOccupantsHaveBeenGenerated = true;
	switch (pVehicle->GetModelIndex()){
	case MI_FBIRANCH:
	case MI_ENFORCER:
		pVehicle->SetUpDriver();
		for (int i = 0; i < 3; i++)
			pVehicle->SetupPassenger(i);
		return;
	case MI_POLICE:
	case MI_RHINO:
	case MI_BARRACKS:
		pVehicle->SetUpDriver();
		if (Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) > 1)
			pVehicle->SetupPassenger(0);
		return;
	case MI_PREDATOR:
		pVehicle->SetUpDriver();
		return;
	case MI_VICECHEE:
	{
		pVehicle->SetUpDriver()->bMiamiViceCop = true;
		pVehicle->SetupPassenger(0)->bMiamiViceCop = true;
		Population_numMiamiViceCops += 2; // rouz edit (ChatGPT)
		CarCtrl_MiamiViceCycle = (CarCtrl_MiamiViceCycle + 1) % 4;
		CarCtrl_LastTimeMiamiViceGenerated = (*Timer_GetTimeInMilliseconds());
		return;
	}
	default:
		return;
	}
}

void CarAI_AddAmbulanceOccupants(CVehicle* pVehicle)
{
	pVehicle->SetUpDriver();
	pVehicle->SetupPassenger(1);
}

void CarAI_AddFiretruckOccupants(CVehicle* pVehicle)
{
	pVehicle->SetUpDriver();
	pVehicle->SetupPassenger(0);
}

void CarAI_TellOccupantsToLeaveCar(CVehicle* pVehicle)
{
	if (pVehicle->pDriver){
		pVehicle->pDriver->SetObjective(OBJECTIVE_LEAVE_CAR, pVehicle);
		switch (pVehicle->GetModelIndex()) {
		case MI_FIRETRUCK:
		case MI_FBICAR:
		case MI_ENFORCER:
		case MI_BARRACKS:
		case MI_RHINO:
		case MI_POLICE:
			break;
		case MI_AMBULAN:
			pVehicle->pDriver->Say(SOUND_PED_LEAVE_VEHICLE);
			break;
		}
	}
	int timer = 100;
	for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++){
		if (pVehicle->pPassengers[i]) {
			pVehicle->pPassengers[i]->m_leaveCarTimer = timer;
			pVehicle->pPassengers[i]->SetObjective(OBJECTIVE_LEAVE_CAR, pVehicle);
			timer += General_GetRandomNumberInRangeInt(200, 400);
		}
	}
}

void CarAI_TellOccupantsToFleeCar(CVehicle* pVehicle)
{
	if (pVehicle->pDriver && !pVehicle->pDriver->IsPlayer()) {
		pVehicle->pDriver->SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
		switch (pVehicle->GetModelIndex()) {
		case MI_FIRETRUCK:
		case MI_FBIRANCH:
		case MI_ENFORCER:
		case MI_BARRACKS:
		case MI_RHINO:
		case MI_POLICE:
			break;
		case MI_AMBULAN:
			pVehicle->pDriver->Say(SOUND_PED_LEAVE_VEHICLE);
			break;
		}
	}
	int timer = 100;
	for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
		if (pVehicle->pPassengers[i]) {
			pVehicle->pPassengers[i]->m_leaveCarTimer = timer;
			pVehicle->pPassengers[i]->SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
			timer += General_GetRandomNumberInRangeInt(200, 400);
		}
	}
}

void CarAI_TellCarToRamOtherCar(CVehicle* pVehicle, CVehicle* pTarget)
{
	pVehicle->AutoPilot.m_pTargetCar = pTarget;
	pTarget->RegisterReference((CEntity**)&pVehicle->AutoPilot.m_pTargetCar);
	pVehicle->AutoPilot.m_nCarMission = MISSION_RAMCAR_FARAWAY;
	pVehicle->bEngineOn = true;
	pVehicle->AutoPilot.m_nCruiseSpeed = General_MaxInt(6, pVehicle->AutoPilot.m_nCruiseSpeed);
}

void CarAI_TellCarToBlockOtherCar(CVehicle* pVehicle, CVehicle* pTarget)
{
	pVehicle->AutoPilot.m_pTargetCar = pTarget;
	pTarget->RegisterReference((CEntity**)&pVehicle->AutoPilot.m_pTargetCar);
	pVehicle->AutoPilot.m_nCarMission = MISSION_BLOCKCAR_FARAWAY;
	pVehicle->bEngineOn = true;
	pVehicle->AutoPilot.m_nCruiseSpeed = General_MaxInt(6, pVehicle->AutoPilot.m_nCruiseSpeed);
}

uint8 CarAI_FindPoliceCarMissionForWantedLevel()
{
	// Delegate to the C implementation
	return CarAILogic_FindPoliceCarMissionForWantedLevel(Wanted_GetWantedLevel(World_Players[World_PlayerInFocus].m_pPed->m_pWanted), General_GetRandomNumber);
}

uint8 CarAI_FindPoliceBoatMissionForWantedLevel()
{
	// Delegate to the C implementation
	return CarAILogic_FindPoliceBoatMissionForWantedLevel(Wanted_GetWantedLevel(World_Players[World_PlayerInFocus].m_pPed->m_pWanted));
}

int32 CarAI_FindPoliceCarSpeedForWantedLevel(CVehicle* pVehicle)
{
	// Delegate to the C implementation
	return CarAILogic_FindPoliceCarSpeedForWantedLevel(Wanted_GetWantedLevel(World_Players[World_PlayerInFocus].m_pPed->m_pWanted), pVehicle->pHandling->Transmission.fMaxVelocity, General_GetRandomNumberInRangeInt);
}

void CarAI_MellowOutChaseSpeed(CVehicle* pVehicle)
{
	// Delegate to the C implementation
	pVehicle->AutoPilot.m_nCruiseSpeed = CarAILogic_MellowOutChaseSpeed(Wanted_GetWantedLevel(World_Players[World_PlayerInFocus].m_pPed->m_pWanted), pVehicle->AutoPilot.m_nCruiseSpeed,
		(pVehicle->GetPosition() - FindPlayerCoors()).Magnitude(), FindPlayerVehicle() != nil, FindPlayerPed()->GetMoveSpeed().Magnitude());
}

void CarAI_MellowOutChaseSpeedBoat(CVehicle* pVehicle)
{
	// Delegate to the C implementation
	pVehicle->AutoPilot.m_nCruiseSpeed = CarAILogic_MellowOutChaseSpeedBoat(Wanted_GetWantedLevel(World_Players[World_PlayerInFocus].m_pPed->m_pWanted), pVehicle->AutoPilot.m_nCruiseSpeed);
}

void CarAI_MakeWayForCarWithSiren(CVehicle *pVehicle)
{
	float flatSpeed = pVehicle->GetMoveSpeed().Magnitude2D();
	if (flatSpeed < 0.1f)
		return;
	CVector2D forward = pVehicle->GetMoveSpeed() / flatSpeed;
	float projection = flatSpeed * 45 + 20;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int i = CPool_GetSize(Pools_GetVehiclePool());
	//- rouz edit (ChatGPT)
	while (--i >= 0) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CVehicle* vehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
		//- rouz edit (ChatGPT)
		if (!vehicle)
			continue;
		if (!vehicle->IsCar() && !vehicle->IsBike())
			continue;
		if (vehicle->GetStatus() != STATUS_SIMPLE && vehicle->GetStatus() != STATUS_PHYSICS)
			continue;
		if (vehicle->VehicleCreatedBy != RANDOM_VEHICLE)
			continue;
		if (vehicle->bIsLawEnforcer || vehicle->bIsAmbulanceOnDuty || vehicle->bIsFireTruckOnDuty)
			continue;
		if (vehicle == pVehicle)
			continue;
		if (vehicle->AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_AVOID_CARS)
			return;
		if (General_AbsDiffAtLeast(pVehicle->GetPosition().z, vehicle->GetPosition().z, 5.0f))
			continue;
		CVector2D distance = vehicle->GetPosition() - pVehicle->GetPosition();
		if (distance.Magnitude() >= projection)
			continue;
		if (vehicle->GetMoveSpeed().Magnitude2D() <= 0.05f)
			continue;
		float correlation = DotProduct2D(forward, distance) / distance.Magnitude();
		if (correlation <= 0.0f)
			continue;
		if (correlation > 0.8f && DotProduct2D(forward, vehicle->GetForward()) > 0.7f){
			if (vehicle->AutoPilot.m_nTempAction != TEMPACT_SWERVELEFT && vehicle->AutoPilot.m_nTempAction != TEMPACT_SWERVERIGHT){
				vehicle->AutoPilot.m_nTempAction = (distance.x * forward.y - distance.y * forward.x > 0.0f) ?
					TEMPACT_SWERVELEFT : TEMPACT_SWERVERIGHT;
				vehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 2000;
			}
			vehicle->SetStatus(STATUS_PHYSICS);
		}else{
			if (DotProduct2D(vehicle->GetMoveSpeed(), distance) < 0.0f && vehicle->AutoPilot.m_nTempAction != TEMPACT_WAIT){
				vehicle->AutoPilot.m_nTempAction = TEMPACT_WAIT;
				vehicle->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 2000;
			}
		}
	}
}

//- rouz edit (ChatGPT)
