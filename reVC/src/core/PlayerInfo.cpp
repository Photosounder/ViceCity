//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "Automobile.h"
#include "Bridge.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "Cranes.h"
#include "Darkel.h"
#include "Explosion.h"
#include "Fire.h"
#include "Frontend.h"
#include "General.h"
#include "HandlingMgr.h"
#include "Messages.h"
#include "Pad.h"
#include "PathFind.h"
#include "PlayerInfo.h"
#include "PlayerPed.h"
#include "PlayerSkin.h"
#include "ProjectileInfo.h"
#include "Remote.h"
#include "Renderer.h"
#include "Replay.h"
#include "Script.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "Wanted.h"
#include "WaterLevel.h"
#include "World.h"
#include "ZoneCull.h"
#include "main.h"
#include "Bike.h"
#include "Automobile.h"
#include "GameLogic.h"


CPlayerInfo::CPlayerInfo(void)
{
    // Initialize the embedded C collision owner at its original native member-construction boundary
    ColModel_Init(&m_ColModel);
}
CPlayerInfo::~CPlayerInfo(void)
{
    // Release the embedded C collision owner at its original native member-destruction boundary
    ColModel_Destroy(&m_ColModel);
}
CPlayerInfo &CPlayerInfo::operator=(const CPlayerInfo &other)
{
    // Copy native parent fields in their original declaration order around the complete C model assignment
    m_pPed=other.m_pPed;
    m_pRemoteVehicle=other.m_pRemoteVehicle;
    ColModel_Assign(&m_ColModel,&other.m_ColModel);
    m_pVehicleEx=other.m_pVehicleEx;
    for(unsigned i=0;i<70;++i) m_aPlayerName[i]=other.m_aPlayerName[i];
    m_nMoney=other.m_nMoney;
    m_nVisibleMoney=other.m_nVisibleMoney;
    m_nCollectedPackages=other.m_nCollectedPackages;
    m_nTotalPackages=other.m_nTotalPackages;
    m_nLastBumpPlayerCarTimer=other.m_nLastBumpPlayerCarTimer;
    m_nUnusedTaxiTimer=other.m_nUnusedTaxiTimer;
    m_bUnusedTaxiThing=other.m_bUnusedTaxiThing;
    m_nNextSexFrequencyUpdateTime=other.m_nNextSexFrequencyUpdateTime;
    m_nNextSexMoneyUpdateTime=other.m_nNextSexMoneyUpdateTime;
    m_nSexFrequency=other.m_nSexFrequency;
    m_pHooker=other.m_pHooker;
    m_WBState=other.m_WBState;
    m_nWBTime=other.m_nWBTime;
    m_bInRemoteMode=other.m_bInRemoteMode;
    field_D5=other.field_D5;
    field_D6=other.field_D6;
    m_nTimeLostRemoteCar=other.m_nTimeLostRemoteCar;
    m_nTimeLastHealthLoss=other.m_nTimeLastHealthLoss;
    m_nTimeLastArmourLoss=other.m_nTimeLastArmourLoss;
    m_nTimeTankShotGun=other.m_nTimeTankShotGun;
    m_nUpsideDownCounter=other.m_nUpsideDownCounter;
    field_EC=other.field_EC;
    m_nTimeCarSpentOnTwoWheels=other.m_nTimeCarSpentOnTwoWheels;
    m_nDistanceCarTravelledOnTwoWheels=other.m_nDistanceCarTravelledOnTwoWheels;
    m_nTimeNotFullyOnGround=other.m_nTimeNotFullyOnGround;
    m_nTimeSpentOnWheelie=other.m_nTimeSpentOnWheelie;
    m_nDistanceTravelledOnWheelie=other.m_nDistanceTravelledOnWheelie;
    m_nTimeSpentOnStoppie=other.m_nTimeSpentOnStoppie;
    m_nDistanceTravelledOnStoppie=other.m_nDistanceTravelledOnStoppie;
    m_nCancelWheelStuntTimer=other.m_nCancelWheelStuntTimer;
    m_nLastTimeCarSpentOnTwoWheels=other.m_nLastTimeCarSpentOnTwoWheels;
    m_nLastDistanceCarTravelledOnTwoWheels=other.m_nLastDistanceCarTravelledOnTwoWheels;
    m_nLastTimeSpentOnWheelie=other.m_nLastTimeSpentOnWheelie;
    m_nLastDistanceTravelledOnWheelie=other.m_nLastDistanceTravelledOnWheelie;
    m_nLastTimeSpentOnStoppie=other.m_nLastTimeSpentOnStoppie;
    m_nLastDistanceTravelledOnStoppie=other.m_nLastDistanceTravelledOnStoppie;
    m_nTrafficMultiplier=other.m_nTrafficMultiplier;
    field_12A=other.field_12A;
    m_fRoadDensity=other.m_fRoadDensity;
    m_nPreviousTimeRewardedForExplosion=other.m_nPreviousTimeRewardedForExplosion;
    m_nExplosionsSinceLastReward=other.m_nExplosionsSinceLastReward;
    m_nHavocLevel=other.m_nHavocLevel;
    m_fMediaAttention=other.m_fMediaAttention;
    m_bInfiniteSprint=other.m_bInfiniteSprint;
    m_bFastReload=other.m_bFastReload;
    m_bFireproof=other.m_bFireproof;
    m_nMaxHealth=other.m_nMaxHealth;
    m_nMaxArmour=other.m_nMaxArmour;
    m_bGetOutOfJailFree=other.m_bGetOutOfJailFree;
    m_bGetOutOfHospitalFree=other.m_bGetOutOfHospitalFree;
    m_bDriveByAllowed=other.m_bDriveByAllowed;
    m_nBustedAudioStatus=other.m_nBustedAudioStatus;
    m_nCurrentBustedAudio=other.m_nCurrentBustedAudio;
#ifdef GTA_PC
    for(unsigned i=0;i<32;++i) m_aSkinName[i]=other.m_aSkinName[i];
    m_pSkinTexture=other.m_pSkinTexture;
#endif
    return *this;
}

CVector lastPlayerPos;

void
CPlayerInfo::Clear(void)
{
	m_pPed = nil;
	m_pRemoteVehicle = nil;
	if (m_pVehicleEx) {
		m_pVehicleEx->bUsingSpecialColModel = false;
		m_pVehicleEx = nil;
	}
	m_nVisibleMoney = 0;
	m_nMoney = m_nVisibleMoney;
	m_WBState = WBSTATE_PLAYING;
	m_nWBTime = 0;
	m_nTrafficMultiplier = 0;
	m_fRoadDensity = 1.0f;
	m_bInRemoteMode = false;
	field_D5 = false;
	field_D6 = false;
	m_bUnusedTaxiThing = false;
	m_nUnusedTaxiTimer = 0;
	m_nCollectedPackages = 0;
	m_nTotalPackages = 3;
	m_nTimeLastHealthLoss = 0;
	m_nTimeLastArmourLoss = 0;
	m_nNextSexFrequencyUpdateTime = 0;
	m_nNextSexMoneyUpdateTime = 0;
	m_nSexFrequency = 0;
	m_pHooker = nil;
	m_nTimeTankShotGun = 0;
	field_EC = 0;
	m_nUpsideDownCounter = 0;
	m_nTimeCarSpentOnTwoWheels = 0;
	m_nDistanceCarTravelledOnTwoWheels = 0;
	m_nTimeNotFullyOnGround = 0;
	m_nTimeSpentOnWheelie = 0;
	m_nDistanceTravelledOnWheelie = 0.0f;
	m_nTimeSpentOnStoppie = 0;
	m_nDistanceTravelledOnStoppie = 0.0f;
	m_nCancelWheelStuntTimer = 0;
	m_nLastTimeCarSpentOnTwoWheels = 0;
	m_nLastDistanceCarTravelledOnTwoWheels = 0;
	m_nLastTimeSpentOnWheelie = 0;
	m_nLastDistanceTravelledOnWheelie = 0;
	m_nLastTimeSpentOnStoppie = 0;
	m_nLastDistanceTravelledOnStoppie = 0;
	m_bInfiniteSprint = false;
	m_bFastReload = false;
	m_bFireproof = false;
	m_nMaxHealth = m_nMaxArmour = 100;
	m_bGetOutOfJailFree = false;
	m_bGetOutOfHospitalFree = false;
	m_bDriveByAllowed = true;
	m_nPreviousTimeRewardedForExplosion = 0;
	m_nExplosionsSinceLastReward = 0;
	m_nHavocLevel = 0;
	m_fMediaAttention = 0.0f;
	m_nCurrentBustedAudio = 1;
	m_nBustedAudioStatus = BUSTEDAUDIO_NONE;
}

void
CPlayerInfo::Process(void)
{
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;
#endif
	// Unused taxi feature. Gives you a dollar for every second with a passenger. Can be toggled via 0x29A opcode.
	bool startTaxiTimer = true;
	if (m_bUnusedTaxiThing && m_pPed->bInVehicle) {
		CVehicle *veh = m_pPed->m_pMyVehicle;
		if (veh->IsTaxi()
			&& veh->pDriver == m_pPed && veh->m_nNumPassengers != 0) {
			for (uint32 timePassed = (*Timer_GetTimeInMilliseconds()) - m_nUnusedTaxiTimer; timePassed >= 1000; m_nUnusedTaxiTimer += 1000) {
				timePassed -= 1000;
				++m_nMoney;
			}
			startTaxiTimer = false;
		}
	}
	if (startTaxiTimer)
		m_nUnusedTaxiTimer = (*Timer_GetTimeInMilliseconds());

	if (!m_pPed->InVehicle()) {
		m_nTimeCarSpentOnTwoWheels = 0;
		m_nTimeNotFullyOnGround = 0;
		m_nTimeSpentOnWheelie = 0;
		m_nTimeSpentOnStoppie = 0;
		m_nCancelWheelStuntTimer = 0;
	} else if (m_pPed->m_pMyVehicle->IsCar()) {
		CAutomobile *car = (CAutomobile*)m_pPed->m_pMyVehicle;

		if (car->m_nWheelsOnGround < 3)
			m_nTimeNotFullyOnGround += Timer_GetTimeStepInMilliseconds();
		else
			m_nTimeNotFullyOnGround = 0;

		if (car->m_aSuspensionSpringRatioPrev[2] == 1.f && car->m_aSuspensionSpringRatioPrev[3] == 1.f) {
			if (car->m_aSuspensionSpringRatioPrev[0] < 1.0f && car->m_aSuspensionSpringRatioPrev[1] < 1.0f && car->m_fDamageImpulse == 0.0f) {
				m_nTimeCarSpentOnTwoWheels += Timer_GetTimeStepInMilliseconds();
				m_nDistanceCarTravelledOnTwoWheels += car->m_fDistanceTravelled;
				m_nCancelWheelStuntTimer = Max(0.0f, m_nCancelWheelStuntTimer - Timer_GetTimeStepInMilliseconds() * 0.5f);

			} else {
				if (m_nTimeCarSpentOnTwoWheels != 0 && m_nCancelWheelStuntTimer < 500) {
					m_nCancelWheelStuntTimer += Timer_GetTimeStepInMilliseconds();
				} else {
					if (m_nTimeCarSpentOnTwoWheels >= 2000) {
						m_nLastTimeCarSpentOnTwoWheels = m_nTimeCarSpentOnTwoWheels;
						m_nLastDistanceCarTravelledOnTwoWheels = m_nDistanceCarTravelledOnTwoWheels;
						if (Stats_Longest2Wheel < m_nTimeCarSpentOnTwoWheels / 1000)
							Stats_Longest2Wheel = m_nTimeCarSpentOnTwoWheels / 1000;
						if (Stats_Longest2WheelDist < m_nDistanceCarTravelledOnTwoWheels)
							Stats_Longest2WheelDist = m_nDistanceCarTravelledOnTwoWheels;
					}
					m_nTimeCarSpentOnTwoWheels = 0;
					m_nDistanceCarTravelledOnTwoWheels = 0;
					m_nCancelWheelStuntTimer = 0;
				}
			}
		} else if (car->m_aSuspensionSpringRatioPrev[0] == 1.0f && car->m_aSuspensionSpringRatioPrev[1] == 1.0f) {
#ifdef FIX_BUGS
			if (car->m_aSuspensionSpringRatioPrev[2] < 1.f
#else
			if (car->m_aSuspensionSpringRatioPrev[1] < 1.f
#endif
				&& car->m_aSuspensionSpringRatioPrev[3] < 1.f && 0.0f == car->m_fDamageImpulse) {
				m_nTimeCarSpentOnTwoWheels += Timer_GetTimeStepInMilliseconds();
				m_nDistanceCarTravelledOnTwoWheels += car->m_fDistanceTravelled;
				m_nCancelWheelStuntTimer = Max(0.0f, m_nCancelWheelStuntTimer - Timer_GetTimeStepInMilliseconds() * 0.2f);

			} else if (m_nTimeCarSpentOnTwoWheels != 0 && m_nCancelWheelStuntTimer < 500) {
				m_nCancelWheelStuntTimer += Timer_GetTimeStepInMilliseconds();

			} else {
				if (m_nTimeCarSpentOnTwoWheels >= 2000) {
					m_nLastTimeCarSpentOnTwoWheels = m_nTimeCarSpentOnTwoWheels;
					m_nLastDistanceCarTravelledOnTwoWheels = m_nDistanceCarTravelledOnTwoWheels;
					if (Stats_Longest2Wheel < m_nTimeCarSpentOnTwoWheels / 1000)
						Stats_Longest2Wheel = m_nTimeCarSpentOnTwoWheels / 1000;
					if (Stats_Longest2WheelDist < m_nDistanceCarTravelledOnTwoWheels)
						Stats_Longest2WheelDist = m_nDistanceCarTravelledOnTwoWheels;
				}
				m_nTimeCarSpentOnTwoWheels = 0;
				m_nDistanceCarTravelledOnTwoWheels = 0;
				m_nCancelWheelStuntTimer = 0;
			}
		} else if (m_nTimeCarSpentOnTwoWheels != 0) {
			if (m_nTimeCarSpentOnTwoWheels >= 2000) {
				m_nLastTimeCarSpentOnTwoWheels = m_nTimeCarSpentOnTwoWheels;
				m_nLastDistanceCarTravelledOnTwoWheels = m_nDistanceCarTravelledOnTwoWheels;
				if (Stats_Longest2Wheel < m_nTimeCarSpentOnTwoWheels / 1000)
					Stats_Longest2Wheel = m_nTimeCarSpentOnTwoWheels / 1000;
				if (Stats_Longest2WheelDist < m_nDistanceCarTravelledOnTwoWheels)
					Stats_Longest2WheelDist = m_nDistanceCarTravelledOnTwoWheels;
			}
			m_nTimeCarSpentOnTwoWheels = 0;
			m_nDistanceCarTravelledOnTwoWheels = 0;
			m_nCancelWheelStuntTimer = 0;
		}
		m_nTimeSpentOnWheelie = 0;
		m_nTimeSpentOnStoppie = 0;
	} else if (m_pPed->m_pMyVehicle->IsBike()) {
		CBike *bike = (CBike*)m_pPed->m_pMyVehicle;
		if (bike->m_aSuspensionSpringRatioPrev[0] == 1.0f && bike->m_aSuspensionSpringRatioPrev[1] == 1.0f) {
			if (bike->m_aSuspensionSpringRatioPrev[2] < 1.0f
				|| (bike->m_aSuspensionSpringRatioPrev[3] < 1.0f && 0.0f == bike->m_fDamageImpulse)) {
				m_nTimeSpentOnWheelie += Timer_GetTimeStepInMilliseconds();
				m_nDistanceTravelledOnWheelie += bike->m_fDistanceTravelled;
				m_nCancelWheelStuntTimer = Max(0.0f, m_nCancelWheelStuntTimer - Timer_GetTimeStepInMilliseconds() * 0.2f);

			} else {
				if (m_nTimeSpentOnWheelie != 0 && m_nCancelWheelStuntTimer < 500) {
					m_nCancelWheelStuntTimer += Timer_GetTimeStepInMilliseconds();
				} else {
					if (m_nTimeSpentOnWheelie >= 5000) {
						m_nLastTimeSpentOnWheelie = m_nTimeSpentOnWheelie;
						m_nLastDistanceTravelledOnWheelie = m_nDistanceTravelledOnWheelie;
						if (Stats_LongestWheelie < m_nTimeSpentOnWheelie / 1000)
							Stats_LongestWheelie = m_nTimeSpentOnWheelie / 1000;
						if (Stats_LongestWheelieDist < m_nDistanceTravelledOnWheelie)
							Stats_LongestWheelieDist = m_nDistanceTravelledOnWheelie;
					}
					m_nTimeSpentOnWheelie = 0;
					m_nDistanceTravelledOnWheelie = 0;
					m_nCancelWheelStuntTimer = 0;
				}
			}
		} else if (m_nTimeSpentOnWheelie != 0) {
			if (m_nTimeSpentOnWheelie >= 5000) {
				m_nLastTimeSpentOnWheelie = m_nTimeSpentOnWheelie;
				m_nLastDistanceTravelledOnWheelie = m_nDistanceTravelledOnWheelie;
				if (Stats_LongestWheelie < m_nTimeSpentOnWheelie / 1000)
					Stats_LongestWheelie = m_nTimeSpentOnWheelie / 1000;
				if (Stats_LongestWheelieDist < m_nDistanceTravelledOnWheelie)
					Stats_LongestWheelieDist = m_nDistanceTravelledOnWheelie;
			}
			m_nTimeSpentOnWheelie = 0;
			m_nDistanceTravelledOnWheelie = 0;
			m_nCancelWheelStuntTimer = 0;

		} else if (bike->m_aSuspensionSpringRatioPrev[2] == 1.0f && bike->m_aSuspensionSpringRatioPrev[3] == 1.0f
			&& 0.0f == bike->m_fDamageImpulse) {
			m_nTimeSpentOnStoppie += Timer_GetTimeStepInMilliseconds();
			m_nDistanceTravelledOnStoppie += bike->m_fDistanceTravelled;
			m_nCancelWheelStuntTimer = Max(0.0f, m_nCancelWheelStuntTimer - Timer_GetTimeStepInMilliseconds() * 0.2f);

		} else {
			if (m_nTimeSpentOnStoppie != 0 && m_nCancelWheelStuntTimer < 500) {
				m_nCancelWheelStuntTimer += Timer_GetTimeStepInMilliseconds();
			} else {
				if (m_nTimeSpentOnStoppie >= 2000) {
					m_nLastTimeSpentOnStoppie = m_nTimeSpentOnStoppie;
					m_nLastDistanceTravelledOnStoppie = m_nDistanceTravelledOnStoppie;
					if (Stats_LongestStoppie < m_nTimeSpentOnStoppie / 1000)
						Stats_LongestStoppie = m_nTimeSpentOnStoppie / 1000;
					if (Stats_LongestStoppieDist < m_nDistanceTravelledOnStoppie)
						Stats_LongestStoppieDist = m_nDistanceTravelledOnStoppie;
				}
				m_nTimeSpentOnStoppie = 0;
				m_nDistanceTravelledOnStoppie = 0;
				m_nCancelWheelStuntTimer = 0;
			}
		}
		m_nTimeCarSpentOnTwoWheels = 0;
		m_nTimeNotFullyOnGround = 0;
	} else {
		m_nTimeCarSpentOnTwoWheels = 0;
		m_nTimeNotFullyOnGround = 0;
		m_nTimeSpentOnWheelie = 0;
		m_nTimeSpentOnStoppie = 0;
		m_nCancelWheelStuntTimer = 0;
	}

	// The effect that makes money counter does while earning/losing money
	if (m_nVisibleMoney != m_nMoney) {
		int diff = m_nMoney - m_nVisibleMoney;
		int diffAbs = Abs(diff);
		int changeBy;

		if (diffAbs > 100000)
			changeBy = 12345;
		else if (diffAbs > 10000)
			changeBy = 1234;
		else if (diffAbs > 1000)
			changeBy = 123;
		else if (diffAbs > 50)
			changeBy = 42;
		else
			changeBy = 1;

		if (diff < 0)
			m_nVisibleMoney -= changeBy;
		else
			m_nVisibleMoney += changeBy;
	}

	if (!((*Timer_GetFrameCounter()) & 15)) {
		CVector2D playerPos = m_pPed->bInVehicle ? m_pPed->m_pMyVehicle->GetPosition() : m_pPed->GetPosition();
		m_fRoadDensity = ThePaths.CalcRoadDensity(playerPos.x, playerPos.y);
	}

	m_fRoadDensity = Clamp(m_fRoadDensity, 0.5f, 1.45f);

	// Because vehicle enter/exit use same key binding.
	bool enterOrExitVeh;
	if (m_pPed->bVehExitWillBeInstant && m_pPed->bInVehicle)
		enterOrExitVeh = CPad::GetPad(0)->ExitVehicleJustDown();
	else
		enterOrExitVeh = CPad::GetPad(0)->GetExitVehicle();

	if (enterOrExitVeh && m_pPed->m_nPedState != PED_ANSWER_MOBILE && m_pPed->m_nPedState != PED_SNIPER_MODE && m_pPed->m_nPedState != PED_ROCKET_MODE) {
		if (m_pPed->bInVehicle) {
			if (!m_pRemoteVehicle) {
				CEntity *surfaceBelowVeh = m_pPed->m_pMyVehicle->m_pCurGroundEntity;
				if (!surfaceBelowVeh || !Bridge_ThisIsABridgeObjectMovingUp(surfaceBelowVeh->GetModelIndex())) {
					CVehicle *veh = m_pPed->m_pMyVehicle;
					if (!veh->IsBoat() || veh->m_nDoorLock == CARLOCK_LOCKED_PLAYER_INSIDE) {
						if (veh->GetStatus() != STATUS_WRECKED && veh->GetStatus() != STATUS_TRAIN_MOVING && veh->m_nDoorLock != CARLOCK_LOCKED_PLAYER_INSIDE) {
							bool canJumpOff = false;
							if (veh->m_vehType == VEHICLE_TYPE_BIKE) {
								canJumpOff = veh->CanPedJumpOffBike();
							} else if (veh->pDriver == m_pPed) {
								canJumpOff = veh->CanPedJumpOutCar();
							}

							if (canJumpOff || veh->m_vecMoveSpeed.Magnitude() < 0.1f) {
								if (!veh->bIsInWater)
									m_pPed->SetObjective(OBJECTIVE_LEAVE_CAR, veh);

							} else if (veh->GetStatus() != STATUS_PLAYER && veh != CGameLogic::pShortCutTaxi) {
								veh->AutoPilot.m_nTempAction = TEMPACT_WAIT;
								veh->AutoPilot.m_nTimeTempAction = (*Timer_GetTimeInMilliseconds()) + 1000;
							}
						}
					} else {
						m_pPed->SetExitBoat(veh);
						m_pPed->bTryingToReachDryLand = true;
					}
				}
			}
		} else {
			// Enter vehicle
			if (CPad::GetPad(0)->ExitVehicleJustDown()) {
				bool weAreOnBoat = false;
				float lastCloseness = 0.0f;
				CVehicle *carBelow = nil;
				CEntity *surfaceBelow = m_pPed->m_pCurrentPhysSurface;
				if (surfaceBelow && surfaceBelow->IsVehicle()) {
					carBelow = (CVehicle*)surfaceBelow;
					if (carBelow->IsBoat() && carBelow->m_modelIndex != MI_SKIMMER) {
						weAreOnBoat = true;
						m_pPed->bOnBoat = true;
						if (carBelow->GetStatus() != STATUS_WRECKED && carBelow->GetUp().z > 0.3f)
							m_pPed->SetSeekBoatPosition(carBelow);
					}
				}
				// Find closest car
				if (!weAreOnBoat) {
					float minX = m_pPed->GetPosition().x - 10.0f;
					float maxX = 10.0f + m_pPed->GetPosition().x;
					float minY = m_pPed->GetPosition().y - 10.0f;
					float maxY = 10.0f + m_pPed->GetPosition().y;

					int minXSector = CWorld::GetSectorIndexX(minX);
					if (minXSector < 0) minXSector = 0;
					int minYSector = CWorld::GetSectorIndexY(minY);
					if (minYSector < 0) minYSector = 0;
					int maxXSector = CWorld::GetSectorIndexX(maxX);
					if (maxXSector > NUMSECTORS_X - 1) maxXSector = NUMSECTORS_X - 1;
					int maxYSector = CWorld::GetSectorIndexY(maxY);
					if (maxYSector > NUMSECTORS_Y - 1) maxYSector = NUMSECTORS_Y - 1;

					CWorld::AdvanceCurrentScanCode();

					for (int curY = minYSector; curY <= maxYSector; curY++) {
						for (int curX = minXSector; curX <= maxXSector; curX++) {
							CSector *sector = CWorld::GetSector(curX, curY);
							FindClosestCarSectorList(sector->m_lists[ENTITYLIST_VEHICLES], m_pPed,
								minX, minY, maxX, maxY, &lastCloseness, &carBelow);
							FindClosestCarSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], m_pPed,
								minX, minY, maxX, maxY, &lastCloseness, &carBelow);
						}
					}
				}
				// carBelow is now closest vehicle
				if (carBelow && !weAreOnBoat) {
					if (carBelow->GetStatus() == STATUS_TRAIN_NOT_MOVING) {
						m_pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, carBelow);
					} else if (carBelow->IsBoat()) {
						if (!carBelow->pDriver) {
							m_pPed->m_vehDoor = 0;
							m_pPed->SetEnterCar(carBelow, m_pPed->m_vehDoor);
						}
					} else {
						m_pPed->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, carBelow);
					}
				}
			}
		}
	}

	if (m_bInRemoteMode) {
		uint32 timeWithoutRemoteCar = (*Timer_GetTimeInMilliseconds()) - m_nTimeLostRemoteCar;
		if (Timer_GetPreviousTimeInMilliseconds() - m_nTimeLostRemoteCar < 1000 && timeWithoutRemoteCar >= 1000 && m_WBState == WBSTATE_PLAYING && field_D6) {
			TheCamera.SetFadeColour(0, 0, 0);
			TheCamera.Fade(1.0f, FADE_OUT);
		}
		if (timeWithoutRemoteCar > 2000) {
			if (m_WBState == WBSTATE_PLAYING && field_D6) {
				TheCamera.RestoreWithJumpCut();
				TheCamera.SetFadeColour(0, 0, 0);
				TheCamera.Fade(1.0f, FADE_IN);
				TheCamera.Process();
				Timer_Stop();
				CullZones_ForceCullZoneCoors(CullZonesPosition_FromVector(TheCamera.GetPosition()));
				CRenderer::RequestObjectsInFrustum();
				CStreaming::LoadAllRequestedModels(false);
				Timer_Update();
			}
			m_bInRemoteMode = false;
			CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->bRemoveFromWorld = true;
			CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle = nil;
			if (FindPlayerVehicle()) {
				FindPlayerVehicle()->SetStatus(STATUS_PLAYER);
			}
		}
	}
	if (!((*Timer_GetFrameCounter()) & 31)) {
		CVehicle *veh = FindPlayerVehicle();
		if (veh && m_pPed->bInVehicle && veh->GetUp().z < 0.0f
			&& veh->m_vecMoveSpeed.Magnitude() < 0.05f && (veh->IsCar() || veh->IsBoat()) && !veh->bIsInWater) {

			if (veh->GetUp().z < -0.5f) {
				m_nUpsideDownCounter += 2;
			} else {
				m_nUpsideDownCounter++;
			}
		} else {
			m_nUpsideDownCounter = 0;
		}

		if (m_nUpsideDownCounter > 6 && veh->bCanBeDamaged) {
			veh->m_fHealth = 249.0f < veh->m_fHealth ? 249.0f : veh->m_fHealth;
			if (veh->IsCar()) {
				CAutomobile* car = (CAutomobile*)veh;
				Damage_SetEngineStatus(&car->Damage, 225);
				car->m_pSetOnFireEntity = nil;
			}
		}
	}
	if (FindPlayerVehicle()) {
		CVehicle *veh = FindPlayerVehicle();
		veh->m_nZoneLevel = LEVEL_IGNORE;
		for (int i = 0; i < ARRAY_SIZE(veh->pPassengers); i++) {
			if (veh->pPassengers[i])
				veh->pPassengers[i]->m_nZoneLevel = LEVEL_GENERIC;
		}
		if(veh->m_modelIndex == MI_CADDY)
			Stats_DistanceTravelledByGolfCart += veh->m_fDistanceTravelled;
		else {
			if(veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI)
				Stats_DistanceTravelledByHelicoptor += veh->m_fDistanceTravelled;
			if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE)
				Stats_DistanceTravelledByPlane += veh->m_fDistanceTravelled;
			if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR)
				Stats_DistanceTravelledByCar += veh->m_fDistanceTravelled;
			if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE)
				Stats_DistanceTravelledByBike += veh->m_fDistanceTravelled;
			if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_BOAT)
				Stats_DistanceTravelledByBoat += veh->m_fDistanceTravelled;

			if (veh->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE) {
				if (veh->m_vecMoveSpeed.Magnitude() > 0.2f) {
					Stats_FlightTime += (*Timer_GetTimeStep()) * 16.f; // what a weird choice
				}
			}
		}
	} else {
		Stats_DistanceTravelledOnFoot += FindPlayerPed()->m_fDistanceTravelled;
	}

	if (Wanted_GetWantedLevel(m_pPed->m_pWanted) && !CTheScripts::IsPlayerOnAMission()) {
		float maxDelta = 0.0f;
		static bool movedSignificantly = true;
		static bool thereIsACarPathNear = true;
		// there was one more guard without variable's itself???

		if ((*Timer_GetTimeInMilliseconds()) / 20000 != Timer_GetPreviousTimeInMilliseconds() / 20000) {
			float posChange = (lastPlayerPos - FindPlayerCoors()).Magnitude();
			movedSignificantly = posChange >= 10.0f;
			lastPlayerPos = FindPlayerCoors();
			thereIsACarPathNear = ThePaths.FindNodeClosestToCoors(FindPlayerCoors(), PATH_CAR, 60.0f, true, false, false, false) != 0;
		}
		switch (Wanted_GetWantedLevel(m_pPed->m_pWanted)) {
			case 1:
				maxDelta = 31.f;
				break;
			case 2:
				maxDelta = 62.f;
				break;
			case 3:
				maxDelta = 125.f;
				break;
			case 4:
				maxDelta = 250.f;
				break;
			case 5:
				maxDelta = 500.f;
				break;
			case 6:
				maxDelta = 1000.f;
				break;
			default:
				break;
		}
		float increaseDelta = maxDelta - m_fMediaAttention;
		float increaseAttentionBy = (*Timer_GetTimeStep()) * 0.0001f * increaseDelta;
		if (increaseAttentionBy < 0.0f
			|| movedSignificantly && thereIsACarPathNear && !CullZones_NoPolice() && !CullZones_PoliceAbandonCars() && CGame::currArea == AREA_MAIN_MAP) {
			m_fMediaAttention += increaseAttentionBy;
		}
	} else {
		m_fMediaAttention = 0.0f;
	}
	Stats_HighestChaseValue = Max(m_fMediaAttention, Stats_HighestChaseValue);
	m_nMoney = Min(999999999, m_nMoney);
	m_nVisibleMoney = Min(999999999, m_nVisibleMoney);
}

bool
CPlayerInfo::IsPlayerInRemoteMode()
{
	return m_pRemoteVehicle || m_bInRemoteMode;
}

void
CPlayerInfo::SavePlayerInfo(uint8 *buf, uint32 *size)
{
	// Interesting
	*size = sizeof(CPlayerInfo);

#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); buf += sizeof(data);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMoney);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_WBState);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nWBTime);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTrafficMultiplier);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_fRoadDensity);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTotalPackages);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bInfiniteSprint);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bFastReload);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bFireproof);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMaxHealth);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMaxArmour);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfJailFree);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfHospitalFree);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bDriveByAllowed);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_aPlayerName);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nBustedAudioStatus);
	CopyToBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nCurrentBustedAudio);
#undef CopyToBuf
}

void
CPlayerInfo::LoadPlayerInfo(uint8 *buf, uint32 size)
{
#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); buf += sizeof(data);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMoney);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_WBState);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nWBTime);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTrafficMultiplier);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_fRoadDensity);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nCollectedPackages);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nTotalPackages);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bInfiniteSprint);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bFastReload);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bFireproof);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMaxHealth);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nMaxArmour);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfJailFree);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfHospitalFree);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_bDriveByAllowed);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_aPlayerName);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nBustedAudioStatus);
	CopyFromBuf(buf, CWorld::Players[CWorld::PlayerInFocus].m_nCurrentBustedAudio)
#undef CopyFromBuf
}

void
CPlayerInfo::FindClosestCarSectorList(CPtrList& carList, CPed* ped, float unk1, float unk2, float unk3, float unk4, float* lastCloseness, CVehicle** closestCarOutput)
{
	for (CPtrNode* node = carList.first; node; node = node->next) {
		CVehicle *car = (CVehicle*)node->item;
		if(car->m_scanCode != CWorld::GetCurrentScanCode()) {
			if (!car->bUsesCollision || !car->IsVehicle())
				continue;

			car->m_scanCode = CWorld::GetCurrentScanCode();
			if (car->GetStatus() != STATUS_WRECKED && car->GetStatus() != STATUS_TRAIN_MOVING
				&& (car->GetUp().z > 0.3f || (car->IsVehicle() && ((CVehicle*)car)->m_vehType == VEHICLE_TYPE_BIKE))) {
				CVector carCentre = car->GetBoundCentre();

				if (Abs(ped->GetPosition().z - carCentre.z) < 2.0f || car->IsCar() && carCentre.z < ped->GetPosition().z && ped->GetPosition().z - 4.f < carCentre.z) {
					float dist = (ped->GetPosition() - carCentre).Magnitude2D();
					if (dist <= 10.0f && !CCranes::IsThisCarBeingCarriedByAnyCrane(car)) {
						EvaluateCarPosition(car, ped, dist, lastCloseness, closestCarOutput);
					}
				}
			}
		}
	}
}

// lastCloseness is passed to other calls of this function
void
CPlayerInfo::EvaluateCarPosition(CEntity *carToTest, CPed *player, float carBoundCentrePedDist, float *lastCloseness, CVehicle **closestCarOutput)
{
	// This dist used for determining the angle to face
	CVector2D dist(carToTest->GetPosition() - player->GetPosition());
	float neededTurn = General_GetATanOfXY(player->GetForward().x, player->GetForward().y) - General_GetATanOfXY(dist.x, dist.y);
	while (neededTurn >= PI) {
		neededTurn -= 2 * PI;
	}

	while (neededTurn < -PI) {
		neededTurn += 2 * PI;
	}

	// This dist used for evaluating cars' distances, weird...
	// Accounts inverted needed turn (or needed turn in long way) and car dist.
	float closeness = (1.0f - Abs(neededTurn) / TWOPI) * (10.0f - carBoundCentrePedDist);
	if (closeness > *lastCloseness) {
		*lastCloseness = closeness;
		*closestCarOutput = (CVehicle*)carToTest;
	}
}

const CVector &
CPlayerInfo::GetPos()
{
#ifdef FIX_BUGS
	if (!m_pPed)
		return TheCamera.GetPosition();
#endif
	if (m_pPed->InVehicle())
		return m_pPed->m_pMyVehicle->GetPosition();
	return m_pPed->GetPosition();
}

CVector
FindPlayerCoors(void)
{
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return TheCamera.GetPosition();
#endif
	CPlayerPed *ped = FindPlayerPed();

	//rouzic edit, ped can be NULL when dying
	if (ped==NULL)
	{
		CVector cv;
		return cv;
	}

	if(ped->InVehicle())
		return ped->m_pMyVehicle->GetPosition();
	else
		return ped->GetPosition();
}

const CVector &
FindPlayerSpeed(void)
{
#ifdef FIX_BUGS
	static CVector vecTmpVector(0.0f, 0.0f, 0.0f);
	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return vecTmpVector;
#endif
	CPlayerPed *ped = FindPlayerPed();
	if(ped->InVehicle())
		return ped->m_pMyVehicle->m_vecMoveSpeed;
	else
		return ped->m_vecMoveSpeed;
}

CVehicle *
FindPlayerVehicle(void)
{
	CPlayerPed *ped = FindPlayerPed();
	if(ped && ped->InVehicle()) return ped->m_pMyVehicle;
	return nil;
}

CEntity *
FindPlayerEntity(void)
{
	CPlayerPed *ped = FindPlayerPed();
	if(ped->InVehicle())
		return ped->m_pMyVehicle;
	else
		return ped;
}

CVehicle *
FindPlayerTrain(void)
{
	if(FindPlayerVehicle() && FindPlayerVehicle()->IsTrain())
		return FindPlayerVehicle();
	else
		return nil;
}

CPlayerPed *
FindPlayerPed(void)
{
	return CWorld::Players[CWorld::PlayerInFocus].m_pPed;
}

const CVector &
FindPlayerCentreOfWorld(int32 player)
{
#ifdef FIX_BUGS
	if(Replay_IsPlayingBack()) return TheCamera.GetPosition(); // rouz edit (ChatGPT)
#endif
	if(CCarCtrl::bCarsGeneratedAroundCamera) return TheCamera.GetPosition();
	if(CWorld::Players[player].m_pRemoteVehicle) return CWorld::Players[player].m_pRemoteVehicle->GetPosition();
	if(FindPlayerVehicle()) return FindPlayerVehicle()->GetPosition();
	return CWorld::Players[player].m_pPed->GetPosition();
}

const CVector &
FindPlayerCentreOfWorld_NoSniperShift(void)
{
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack()) return TheCamera.GetPosition(); // rouz edit (ChatGPT)
#endif
	if(CCarCtrl::bCarsGeneratedAroundCamera) return TheCamera.GetPosition();
	if(CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle)
		return CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->GetPosition();
	if(FindPlayerVehicle()) return FindPlayerVehicle()->GetPosition();
	return FindPlayerPed()->GetPosition();
}

float
FindPlayerHeading(void)
{
	if(CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle)
		return CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle->GetForward().Heading();
	if(FindPlayerVehicle()) return FindPlayerVehicle()->GetForward().Heading();
	return FindPlayerPed()->GetForward().Heading();
}

bool
CPlayerInfo::IsRestartingAfterDeath()
{
	return m_WBState == WBSTATE_WASTED;
}

bool
CPlayerInfo::IsRestartingAfterArrest()
{
	return m_WBState == WBSTATE_BUSTED;
}

void
CPlayerInfo::KillPlayer()
{
	if (m_WBState != WBSTATE_PLAYING) return;

	m_WBState = WBSTATE_WASTED;
	m_nWBTime = (*Timer_GetTimeInMilliseconds());
	Darkel_ResetOnPlayerDeath();
	Messages_AddBigMessage(Text_Get(&TheText,"DEAD"), 4000, 2);
	Stats_TimesDied++;
}

void
CPlayerInfo::ArrestPlayer()
{
	if (m_WBState != WBSTATE_PLAYING) return;

	m_WBState = WBSTATE_BUSTED;
	m_nWBTime = (*Timer_GetTimeInMilliseconds());
	m_nBustedAudioStatus = BUSTEDAUDIO_NONE;
	Darkel_ResetOnPlayerDeath();
	Messages_AddBigMessage(Text_Get(&TheText,"BUSTED"), 5000, 2);
	Stats_TimesArrested++;
}

void
CPlayerInfo::PlayerFailedCriticalMission()
{
	if (m_WBState != WBSTATE_PLAYING)
		return;
	m_WBState = WBSTATE_FAILED_CRITICAL_MISSION;
	m_nWBTime = (*Timer_GetTimeInMilliseconds());
	Darkel_ResetOnPlayerDeath();
}

void
CPlayerInfo::CancelPlayerEnteringCars(CVehicle *car)
{
	if (!car || car == m_pPed->m_pMyVehicle) {
		if (m_pPed->EnteringCar())
			m_pPed->QuitEnteringCar();
	}
	if (m_pPed->m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER || m_pPed->m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER)
		m_pPed->ClearObjective();
}

void
CPlayerInfo::MakePlayerSafe(bool toggle)
{
	if (toggle) {
		m_pPed->m_pWanted->m_bIgnoredByEveryone = true;
		CWorld::StopAllLawEnforcersInTheirTracks();
		CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_PLAYERINFO);
		CPad::StopPadsShaking();
		m_pPed->bBulletProof = true;
		m_pPed->bFireProof = true;
		m_pPed->bCollisionProof = true;
		m_pPed->bMeleeProof = true;
		m_pPed->bOnlyDamagedByPlayer = true;
		m_pPed->bExplosionProof = true;
		m_pPed->m_bCanBeDamaged = false;
		((CPlayerPed*)m_pPed)->ClearAdrenaline();
		CancelPlayerEnteringCars(nil);
		FireManager_ExtinguishPoint(&gFireManager,Fire_VectorFromNative(GetPos()), 4000.0f);
		// Evaluate the original position once before passing its C value
		const CVector explosionNativePosition = GetPos();
		ExplosionPosition explosionPosition = {explosionNativePosition.x,explosionNativePosition.y,explosionNativePosition.z};
		Explosion_RemoveAllExplosionsInArea(explosionPosition, 4000.0f);
		ProjectileInfo_RemoveAllProjectiles();
		CWorld::SetAllCarsCanBeDamaged(false);
		CWorld::ExtinguishAllCarFiresInArea(GetPos(), 4000.0f);
		CReplay::DisableReplays();

	} else {
		m_pPed->m_pWanted->m_bIgnoredByEveryone = false;
		CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_PLAYERINFO);
		m_pPed->bBulletProof = false;
		m_pPed->bFireProof = false;
		m_pPed->bCollisionProof = false;
		m_pPed->bMeleeProof = false;
		m_pPed->bOnlyDamagedByPlayer = false;
		m_pPed->bExplosionProof = false;
		m_pPed->m_bCanBeDamaged = true;
		CWorld::SetAllCarsCanBeDamaged(true);
		CReplay::EnableReplays();
	}
}

void
CPlayerInfo::BlowUpRCBuggy(bool actually)
{
	if (!m_pRemoteVehicle || m_pRemoteVehicle->bRemoveFromWorld)
		return;

	Remote_TakeRemoteControlledCarFromPlayer(actually); // rouz edit (ChatGPT)
	if (actually)
		m_pRemoteVehicle->BlowUpCar(FindPlayerPed());
}

#ifdef GTA_PC
void
CPlayerInfo::SetPlayerSkin(const char *skin)
{
	strncpy(m_aSkinName, skin, 32);
	LoadPlayerSkin();
}

void
CPlayerInfo::LoadPlayerSkin()
{
	DeletePlayerSkin();

	m_pSkinTexture = PlayerSkin_GetSkinTexture(m_aSkinName); // rouz edit (ChatGPT)
}

void
CPlayerInfo::DeletePlayerSkin()
{
	if (m_pSkinTexture) {
		RwTextureDestroy(m_pSkinTexture);
		m_pSkinTexture = nil;
	}
}
#endif

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
