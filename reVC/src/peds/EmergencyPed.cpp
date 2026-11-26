//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "EmergencyPed.h"
#include "DMAudio.h"
#include "ModelIndices.h"
#include "Vehicle.h"
#include "Fire.h"
#include "General.h"
#include "CarCtrl.h"
#include "Accident.h"
#include "PedLogic.h"

CEmergencyPed::CEmergencyPed(uint32 type) : CPed(type)
{
	switch (type){
		case PEDTYPE_EMERGENCY:
			SetModelIndex(MI_MEDIC);
			m_pRevivedPed = nil;
			field_1360 = 0;
			break;
		case PEDTYPE_FIREMAN:
			SetModelIndex(MI_FIREMAN);
			m_pRevivedPed = nil;
			break;
		default:
			break;
	}
	m_nEmergencyPedState = EMERGENCY_PED_READY;
	m_pAttendedAccident = nil;
	m_bStartedToCPR = false;
}

bool
CEmergencyPed::InRange(CPed *victim)
{
	if (!m_pMyVehicle)
		return true;

	if ((m_pMyVehicle->GetPosition() - victim->GetPosition()).Magnitude() > 30.0f)
		return false;

	return true;
}

void
CEmergencyPed::ProcessControl(void)
{
	CPed::ProcessControl();
	if (bWasPostponed)
		return;

	if(!DyingOrDead()) {
		GetWeapon()->Update(m_audioEntityId, nil);

		if (IsPedInControl() && m_moved.Magnitude() > 0.0f)
			Avoid();

		switch (m_nPedState) {
			case PED_SEEK_POS:
				Seek();
				break;
			case PED_SEEK_ENTITY:
				if (m_pSeekTarget) {
					m_vecSeekPos = m_pSeekTarget->GetPosition();
					Seek();
				} else {
					ClearSeek();
				}
				break;
			default:
				break;
		}

		switch (m_nPedType) {
			case PEDTYPE_EMERGENCY:
				if (IsPedInControl() || m_nPedState == PED_DRIVING)
					MedicAI();
				break;
			case PEDTYPE_FIREMAN:
				if (IsPedInControl())
					FiremanAI();
				break;
			default:
				return;
		}
	}
}

// This function was buggy and incomplete in both III and VC, firemen had to be in 5.0m range of fire etc. etc.
// Copied some code from MedicAI to make it work.
void
CEmergencyPed::FiremanAI(void)
{
	static_assert(EMERGENCY_PED_READY == PED_EMERGENCY_READY && EMERGENCY_PED_DETERMINE_NEXT_STATE == PED_EMERGENCY_DETERMINE_NEXT_STATE &&
		EMERGENCY_PED_STAND_STILL == PED_EMERGENCY_STAND_STILL && EMERGENCY_PED_STOP == PED_EMERGENCY_STOP, "Mirrored emergency ped states");

	float fireDist = 0.0f;
	CFire *nearestFire = nil;
	bool attendedIsOngoing = false;

	// Look up the nearest fire in the states that need it
	switch (m_nEmergencyPedState) {
		case EMERGENCY_PED_READY:
		case EMERGENCY_PED_DETERMINE_NEXT_STATE:
			nearestFire = FireManager_FindNearestFire(&gFireManager,Fire_VectorFromNative(GetPosition()), &fireDist);
			break;
		case EMERGENCY_PED_STAND_STILL:
			attendedIsOngoing = m_pAttendedFire->m_bIsOngoing;
			nearestFire = FireManager_FindNearestFire(&gFireManager,Fire_VectorFromNative(GetPosition()), &fireDist);
			break;
		default:
			break;
	}

	// Delegate the decision to the C implementation
	PedFiremanResult step = PedLogic_FiremanDecide(m_nEmergencyPedState, nearestFire != nil, nearestFire == m_pAttendedFire, fireDist, attendedIsOngoing,
		nearestFire && (Fire_VectorToNative(nearestFire->m_vecPos) - GetPosition()).Magnitude() < 30.0f);

	// Run to the nearest fire
	if (step.chaseNearestFire) {
		SetPedState(PED_NONE);
		SetSeek(Fire_VectorToNative(nearestFire->m_vecPos), 1.0f);
		SetMoveState(PEDMOVE_RUN);
		bIsRunning = true;
		m_pAttendedFire = nearestFire;
	}

	// Stand still near the fire
	if (step.standIdle)
		SetIdle();

	if (step.sayExtinguishing)
		Say(SOUND_PED_EXTINGUISHING_FIRE);

	// Give up the fire and walk away
	if (step.stop) {
		bIsRunning = false;
		SetPedState(PED_NONE);
		SetWanderPath(General_GetRandomNumber() & 7);
		m_pAttendedFire = nil;
		SetMoveState(PEDMOVE_WALK);
	}
	m_nEmergencyPedState = (EmergencyPedState)step.newState;
}

void
CEmergencyPed::MedicAI(void)
{
	float distToEmergency;
	if (!bInVehicle && IsPedInControl()) {
		ScanForDelayedResponseThreats();
		if (m_threatFlags && (*Timer_GetTimeInMilliseconds()) > m_threatCheckTimer) {
			CheckThreatValidity();
			m_threatFlags = 0;
			m_threatCheckTimer = 0;
			if (m_threatEntity && m_threatEntity->IsPed() && ((CPed*)m_threatEntity)->IsPlayer()) {
				if (((CPed*)m_threatEntity)->GetWeapon()->IsTypeMelee()) {
					SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, m_threatEntity);
				} else {
					SetFlee(m_threatEntity, 6000);
					Say(SOUND_PED_FLEE_SPRINT);
				}
				return;
			}
		}
	}

	if (InVehicle()) {
		if (m_pMyVehicle->IsCar() && m_objective != OBJECTIVE_LEAVE_CAR) {
			if (AccidentManager_FindNearestAccident(&gAccidentManager,Events_VectorFromNative(m_pMyVehicle->GetPosition()),&distToEmergency)
				&& distToEmergency < 25.0f && m_pMyVehicle->m_vecMoveSpeed.Magnitude() < 0.01f) {

				m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
				SetObjective(OBJECTIVE_LEAVE_CAR, m_pMyVehicle);
				Say(SOUND_PED_LEAVE_VEHICLE);
			} else if (m_pMyVehicle->pDriver == this && m_nPedState == PED_DRIVING
				&& m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_NONE && !(General_GetRandomNumber() & 31)) {

				bool waitUntilMedicEntersCar = false;
				for (int i = 0; i < m_numNearPeds; ++i) {
					CPed *nearPed = m_nearPeds[i];
					if (nearPed->m_nPedType == PEDTYPE_EMERGENCY) {
						if ((nearPed->m_nPedState == PED_SEEK_CAR || nearPed->m_nPedState == PED_ENTER_CAR)
							&& nearPed->m_pMyVehicle == m_pMyVehicle) {
							waitUntilMedicEntersCar = true;
							break;
						}
					}
				}
				if (!waitUntilMedicEntersCar) {
					CarCtrl_JoinCarWithRoadSystem(m_pMyVehicle);
					m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
					m_pMyVehicle->m_bSirenOrAlarm = false;
					m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 12;
					m_pMyVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_SLOW_DOWN_FOR_CARS;
					if (m_pMyVehicle->bIsAmbulanceOnDuty) {
						m_pMyVehicle->bIsAmbulanceOnDuty = false;
						--CarCtrl_NumAmbulancesOnDuty;
					}
				}
			}
		}
	}

	CVector headPos, midPos;
	CAccident *nearestAccident;
	if (IsPedInControl()) {
		switch (m_nEmergencyPedState) {
			case EMERGENCY_PED_READY:
				nearestAccident = AccidentManager_FindNearestAccident(&gAccidentManager,Events_VectorFromNative(GetPosition()),&distToEmergency);
				field_1360 = 0;
				if (nearestAccident) {
					m_pRevivedPed = nearestAccident->m_pVictim;
					m_pRevivedPed->RegisterReference((CEntity**)&m_pRevivedPed);
					m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
					m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
					SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, CVector((headPos + midPos) * 0.5f));
					bIsRunning = true;
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
					m_pAttendedAccident = nearestAccident;
					++m_pAttendedAccident->m_nMedicsAttending;
				} else {
					// Delegate the choice of what to do without an accident to the C implementation
					CPed *vehicleDriver = m_pMyVehicle ? m_pMyVehicle->pDriver : nil;
					switch (PedLogic_MedicIdleAction(m_pMyVehicle != nil, bInVehicle, m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER, m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER,
						m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT, vehicleDriver != nil, vehicleDriver && vehicleDriver->m_nPedType == PEDTYPE_EMERGENCY,
						m_pMyVehicle && m_pMyVehicle->m_nGettingInFlags, m_nPedState == PED_WANDER_PATH)) {
					case PED_MEDICIDLE_KILL_DRIVER:
						SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, vehicleDriver);
						break;
					case PED_MEDICIDLE_ENTER_AS_PASSENGER:
						SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_pMyVehicle);
						break;
					case PED_MEDICIDLE_ENTER_AS_DRIVER:
						SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
						break;
					case PED_MEDICIDLE_WANDER:
						SetWanderPath(General_GetRandomNumber() & 7);
						break;
					default:
						break;
					}
				}
				break;
			case EMERGENCY_PED_DETERMINE_NEXT_STATE:
				nearestAccident = AccidentManager_FindNearestAccident(&gAccidentManager,Events_VectorFromNative(GetPosition()),&distToEmergency);
				if (nearestAccident) {
					if (nearestAccident != m_pAttendedAccident || m_nPedState != PED_SEEK_POS) {
						m_pRevivedPed = nearestAccident->m_pVictim;
						m_pRevivedPed->RegisterReference((CEntity**)&m_pRevivedPed);
						if (!InRange(m_pRevivedPed)) {
							m_nEmergencyPedState = EMERGENCY_PED_STOP;
							break;
						}
						m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
						m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
						SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, CVector((headPos + midPos) * 0.5f));
						bIsRunning = true;
						--m_pAttendedAccident->m_nMedicsAttending;
						++nearestAccident->m_nMedicsAttending;
						m_pAttendedAccident = nearestAccident;
					}
				} else {
					m_nEmergencyPedState = EMERGENCY_PED_STOP;
					bIsRunning = false;
				}
				// Delegate the arrival at the accident to the C implementation
				if (distToEmergency < 5.0f) {
					bool victimOnFire = m_pRevivedPed->m_pFire != nil;
					bool cprInProgress = !victimOnFire && distToEmergency < 4.5f && distToEmergency >= 1.0f && m_pAttendedAccident->m_nMedicsPerformingCPR;
					PedMedicArrival arrival = PedLogic_MedicArrival(distToEmergency, victimOnFire, cprInProgress);
					if (arrival.stopRunning)
						bIsRunning = false;
					if (arrival.moveState != PEDMOVE_NONE)
						SetMoveState((eMoveState)arrival.moveState);
					if (arrival.startCPR)
						m_nEmergencyPedState = EMERGENCY_PED_START_CPR;
				}
				break;
			case EMERGENCY_PED_START_CPR:
				if (!m_pRevivedPed || m_pRevivedPed->m_fHealth > 0.0f || m_pRevivedPed->bFadeOut) {
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				} else {
					m_pRevivedPed->m_bloodyFootprintCountOrDeathTime = (*Timer_GetTimeInMilliseconds());
					SetMoveState(PEDMOVE_STILL);
					SetPedState(PED_CPR);
					m_nLastPedState = PED_CPR;
					SetLookFlag(m_pRevivedPed, 0);
					SetLookTimer(500);
					Say(SOUND_PED_HEALING);
					if (m_pAttendedAccident->m_nMedicsPerformingCPR) {
						SetIdle();
						m_nEmergencyPedState = EMERGENCY_PED_STAND_STILL;
					} else {
						m_nEmergencyPedState = EMERGENCY_PED_FACE_TO_PATIENT;
						// Use the complete C animation manager at the original call boundary
						m_pVehicleAnim = AnimManager_BlendAnimation(GetClump(), ASSOCGRP_MEDIC, ANIM_MEDIC_CPR, 4.0f);
						bIsDucking = true;
					}
					SetLookTimer(2000);
					++m_pAttendedAccident->m_nMedicsPerformingCPR;
					m_bStartedToCPR = true;
				}
				break;
			case EMERGENCY_PED_FACE_TO_PATIENT:
				if (!m_pRevivedPed || m_pRevivedPed->m_fHealth > 0.0f)
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				else {
					m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
					m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
					midPos = (headPos + midPos) * 0.5f;
					m_fRotationDest = General_GetRadianAngleBetweenPoints(
						midPos.x, midPos.y,
						GetPosition().x, GetPosition().y);
					m_fRotationDest = General_LimitAngle(m_fRotationDest);
					m_pLookTarget = m_pRevivedPed;
					m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
					TurnBody();

					// Delegate the turn towards the patient to the C implementation
					float turnedRotation = m_fRotationCur;
					if (PedLogic_MedicFacesPatient(m_fRotationCur, m_fRotationDest, &turnedRotation))
						m_nEmergencyPedState = EMERGENCY_PED_PERFORM_CPR;
					else
						m_fRotationCur = turnedRotation;
				}
				break;
			case EMERGENCY_PED_PERFORM_CPR:
				if (!m_pRevivedPed || m_pRevivedPed->m_fHealth > 0.0f) {
					m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
					break;
				}
				m_pRevivedPed->m_pedIK.GetComponentPosition(midPos, PED_MID);
				m_pRevivedPed->m_pedIK.GetComponentPosition(headPos, PED_HEAD);
				midPos = (headPos + midPos) * 0.5f;
				m_fRotationDest = General_GetRadianAngleBetweenPoints(
					midPos.x, midPos.y,
					GetPosition().x, GetPosition().y);
				m_fRotationDest = General_LimitAngle(m_fRotationDest);
				m_pLookTarget = m_pRevivedPed;
				m_pLookTarget->RegisterReference((CEntity**)&m_pLookTarget);
				TurnBody();
				if ((*Timer_GetTimeInMilliseconds()) <= m_lookTimer) {
					SetMoveState(PEDMOVE_STILL);
					break;
				}
				m_nEmergencyPedState = EMERGENCY_PED_STOP_CPR;
				SetPedState(PED_NONE);
				SetMoveState(PEDMOVE_WALK);
				m_pVehicleAnim = nil;
				if (!m_pRevivedPed->bBodyPartJustCameOff) {
					m_pRevivedPed->m_fHealth = 100.0f;
					m_pRevivedPed->SetPedState(PED_NONE);
					m_pRevivedPed->m_nLastPedState = PED_WANDER_PATH;
					m_pRevivedPed->SetGetUp();
					m_pRevivedPed->bUsesCollision = true;
					m_pRevivedPed->SetMoveState(PEDMOVE_WALK);
					m_pRevivedPed->RestartNonPartialAnims();
					m_pRevivedPed->bIsPedDieAnimPlaying = false;
					m_pRevivedPed->bKnockedUpIntoAir = false;
					m_pRevivedPed->m_pCollidingEntity = nil;
					m_pRevivedPed->bKnockedOffBike = false;
					m_pRevivedPed->Say(SOUND_PED_ACCIDENTREACTION1);
				}
				break;
			case EMERGENCY_PED_STOP_CPR:
				m_nEmergencyPedState = EMERGENCY_PED_STOP;
				bIsDucking = true;
				break;
			case EMERGENCY_PED_STAND_STILL:
				// Delegate the decision to look for accidents again to the C implementation
				{
					bool noPed = !m_pRevivedPed;
					bool healthy = !noPed && m_pRevivedPed->m_fHealth > 0.0f;
					bool alive = noPed || healthy;
					bool noVictim = !alive && !m_pAttendedAccident->m_pVictim;
					bool noCPR = !alive && !m_pAttendedAccident->m_nMedicsPerformingCPR;
					bool unattended = !alive && AccidentManager_UnattendedAccidents(&gAccidentManager);
					if (PedLogic_MedicStandStillResumes(noPed, healthy, noVictim, noCPR, unattended))
						m_nEmergencyPedState = EMERGENCY_PED_DETERMINE_NEXT_STATE;
				}
				break;
			case EMERGENCY_PED_STOP:
				m_bStartedToCPR = false;
				SetPedState(PED_NONE);
				if (m_pAttendedAccident) {
					m_pAttendedAccident->m_pVictim = nil;
					--m_pAttendedAccident->m_nMedicsAttending;
					m_pAttendedAccident = nil;
				}
				SetWanderPath(General_GetRandomNumber() & 7);
				m_pRevivedPed = nil;
				m_nEmergencyPedState = EMERGENCY_PED_READY;
				SetMoveState(PEDMOVE_WALK);
				break;
			default: break;
		}
	}
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
