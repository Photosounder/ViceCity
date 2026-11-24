//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"
#include "CollisionVector_native.h"
#include "main.h"

#include "General.h"
#include "Timer.h"
#include "Pad.h"
#include "Vehicle.h"
#include "VehicleRules.h"
#include "Bike.h"
#include "Automobile.h"
#include "Pools.h"
#include "HandlingMgr.h"
#include "CarCtrl.h"
#include "Population.h"
#include "ModelIndices.h"
#include "WorldLogic.h"
#include "World.h"
#include "Lights.h"
#include "PointLights.h"
#include "Renderer.h"
#include "VisibilityPlugins.h"
#include "DMAudio.h"
#include "Radar.h"
#include "Fire.h"
#include "Darkel.h"
#include "Streaming.h"
#include "Camera.h"
#include "Stats.h"
#include "Garages.h"
#include "Wanted.h"
#include "SurfaceTable.h"
#include "Particle.h"
#include "WaterLevel.h"
#include "Timecycle.h"
#include "Weather.h"
#include "Coronas.h"
#include "SaveBuf.h"

bool CVehicle::bWheelsOnlyCheat;
bool CVehicle::bAllDodosCheat;
bool CVehicle::bCheat3;
bool CVehicle::bCheat4;
bool CVehicle::bCheat5;
bool CVehicle::bCheat8;
bool CVehicle::bCheat9;
bool CVehicle::bCheat10;
bool CVehicle::bHoverCheat;
bool CVehicle::bAllTaxisHaveNitro;
bool CVehicle::m_bDisableMouseSteering = true;
bool CVehicle::bDisableRemoteDetonation;
bool CVehicle::bDisableRemoteDetonationOnContact;
#ifndef MASTER
bool CVehicle::m_bDisplayHandlingInfo;
#endif

#ifdef FIX_BUGS
// I think they meant that
#define DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE (MYRAND_MAX * 35 / 100)
#define DAMAGE_FLEE_ON_FOOT_PROBABILITY_VALUE (MYRAND_MAX * 70 / 100)
#else
#define DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE (35000)
#define DAMAGE_FLEE_ON_FOOT_PROBABILITY_VALUE (70000)
#endif
#define DAMAGE_HEALTH_TO_FLEE_ALWAYS (200.0f)
#define DAMAGE_HEALTH_TO_CATCH_FIRE (250.0f)


CVehicle::CVehicle(uint8 CreatedBy)
{
    // Initialize the plain autopilot before any original vehicle constructor statement
    AutoPilot_Init(&AutoPilot);
	int i;

	m_nCurrentGear = 1;
	m_fChangeGearTime = 0.0f;
	m_fSteerInput = 0.0f;
	m_type = ENTITY_TYPE_VEHICLE;
	VehicleCreatedBy = CreatedBy;
	m_nRouteSeed = 0;
	bIsLocked = false;
	bIsLawEnforcer = false;
	bIsAmbulanceOnDuty = false;
	bIsFireTruckOnDuty = false;
#ifdef FIX_BUGS
	bIsHandbrakeOn = false;
#endif
	CCarCtrl::UpdateCarCount(this, false);
	m_fHealth = 1000.0f;
	bEngineOn = true;
	bFreebies = true;
	pDriver = nil;
	m_nNumPassengers = 0;
	m_nNumGettingIn = 0;
	m_nGettingInFlags = 0;
	m_nGettingOutFlags = 0;
	m_nNumMaxPassengers = ARRAY_SIZE(pPassengers);
	for(i = 0; i < m_nNumMaxPassengers; i++)
		pPassengers[i] = nil;
	m_nBombTimer = 0;
	m_pBlowUpEntity = nil;
	m_nPacManPickupsCarried = 0;
	bComedyControls = false;
	bCraneMessageDone = false;
	bExtendedRange = false;
	bTakeLessDamage = false;
	bIsDamaged = false;
	bFadeOut = false;
	bIsBeingCarJacked = false;
	m_nTimeOfDeath = 0;
	m_pCarFire = nil;
	bHasBeenOwnedByPlayer = false;
	bCreateRoadBlockPeds = false;
	bCanBeDamaged = true;
	bUsingSpecialColModel = false;
	bOccupantsHaveBeenGenerated = false;
	bGunSwitchedOff = false;
	m_nGunFiringTime = 0;
	m_nTimeBlocked = 0;
	bLightsOn = false;
	bVehicleColProcessed = false;
	m_numPedsUseItAsCover = 0;
	bIsCarParkVehicle = false;
	bHasAlreadyBeenRecorded = false;
	m_bSirenOrAlarm = false;
	m_nCarHornTimer = 0;
	m_nCarHornPattern = 0;
	m_nCarHornDelay = 0;
	bPartOfConvoy = false;
	bHeliMinimumTilt = false;
	bAudioChangingGear = false;
	bIsDrowning = false;
	bTyresDontBurst = false;
	bCreatedAsPoliceVehicle = false;
	bRestingOnPhysical = false;
	bParking = false;
	bCanPark = General_GetRandomNumberInRangeFloat(0.0f, 1.0f) < 0.0f;	// never true. probably doesn't work very well
	bIsVan = false;
	bIsBus = false;
	bIsBig = false;
	bLowVehicle = false;

	m_bombType = CARBOMB_NONE;
	bDriverLastFrame = false;
	m_pBombRigger = nil;

	m_nSetPieceExtendedRangeTime = 0;
	m_nAlarmState = 0;
	m_nDoorLock = CARLOCK_UNLOCKED;
	m_nLastWeaponDamage = -1;
	m_pLastDamageEntity = nil;
	m_fMapObjectHeightAhead = m_fMapObjectHeightBehind = 0.0f;
	m_audioEntityId = DMAudio_CreateEntity(AUDIOTYPE_PHYSICAL, this);
	if(m_audioEntityId >= 0)
		DMAudio_SetEntityStatus(m_audioEntityId, TRUE);
	//m_nRadioStation = CGeneral::GetRandomNumber() % NUM_RADIOS;
	switch(GetModelIndex()){
	case MI_HUNTER:
	case MI_ANGEL:
	case MI_FREEWAY:
		m_nRadioStation = V_ROCK;
		break;
	case MI_RCBARON:
	case MI_RCBANDIT:
	case MI_RCRAIDER:
	case MI_RCGOBLIN:
	case MI_TOPFUN:
	case MI_CADDY:
	case MI_BAGGAGE:
		m_nRadioStation = RADIO_OFF;
		break;
	default:
		m_nRadioStation = General_GetRandomNumber() % NUM_RADIOS;
		break;
	}
	m_pCurGroundEntity = nil;
	m_bRainAudioCounter = 0;
	m_bRainSamplesCounter = 0;
	m_comedyControlState = 0;
	m_aCollPolys[0].valid = false;
	m_aCollPolys[1].valid = false;
	AutoPilot.m_nCarMission = MISSION_NONE;
	AutoPilot.m_nTempAction = TEMPACT_NONE;
	AutoPilot.m_nTimeToStartMission = (*Timer_GetTimeInMilliseconds());
	AutoPilot.m_bStayInCurrentLevel = false;
	AutoPilot.m_bIgnorePathfinding = false;
	AutoPilot.m_nSwitchDistance = 20;
}

CVehicle::~CVehicle()
{
	m_nAlarmState = 0;
	if (m_audioEntityId >= 0){
		DMAudio_DestroyEntity(m_audioEntityId);
		m_audioEntityId = -5;
	}
		// Access raw storage through the C store or pool API
	CRadar::ClearBlipForEntity(BLIP_CAR, CPool_GetIndex(CPools::GetVehiclePool(), this));
		if (pDriver)
		pDriver->FlagToDestroyWhenNextProcessed();
	for (int i = 0; i < m_nNumMaxPassengers; i++){
		if (pPassengers[i])
			pPassengers[i]->FlagToDestroyWhenNextProcessed();
	}
	if (m_pCarFire)
		Fire_Extinguish(m_pCarFire);
	CCarCtrl::UpdateCarCount(this, true);
	if (bIsAmbulanceOnDuty){
		CCarCtrl::NumAmbulancesOnDuty--;
		bIsAmbulanceOnDuty = false;
	}
	if (bIsFireTruckOnDuty){
		CCarCtrl::NumFiretrucksOnDuty--;
		bIsFireTruckOnDuty = false;
	}
}

void
CVehicle::SetModelIndex(uint32 id)
{
	CEntity::SetModelIndex(id);
	m_aExtras[0] = CVehicleModelInfo::ms_compsUsed[0];
	m_aExtras[1] = CVehicleModelInfo::ms_compsUsed[1];
	m_nNumMaxPassengers = CVehicleModelInfo::GetMaximumNumberOfPassengersFromNumberOfDoors(id);
}

bool
CVehicle::SetupLighting(void)
{
	// Use the complete C lighting owner and its borrowed SDK handles
	Lights_ActivateDirectional();
	// Use the complete C lighting owner and its borrowed SDK handles
	Lights_SetAmbientColoursForPedsCarsAndObjects();

	if(bRenderScorched){
		// Use the complete C lighting owner and its borrowed SDK handles
		Lights_WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
	}else{
		CVector coors = GetPosition();
		// Enter the C point-light algorithm with the original owner expressions
		float lighting = PointLights_GenerateLightsAffectingObjectLegacy(&coors);
		if(lighting != 1.0f){
			// Use the complete C lighting owner and its borrowed SDK handles
			Lights_SetAmbientAndDirectionalColours(lighting);
			return true;
		}
	}

	return false;
}

void
CVehicle::RemoveLighting(bool reset)
{
	CRenderer::RemoveVehiclePedLights(this, reset);
}

bool
CVehicle::IsClearToDriveAway(void)
{
	CColPoint point;
	float length = CollisionVector_ToNative(Box_GetSize(&GetColModel()->boundingBox)).y;
	CEntity *ent = nil;
	CVector front = GetForward() * (length*0.5f + 3.0f);
	return !CWorld::ProcessLineOfSight(GetPosition() + front, GetPosition(),
			point, ent, true, true, false, false, false, true, true) ||
		ent == this;
}

float
CVehicle::GetHeightAboveRoad(void)
{
	return -1.0f * GetColModel()->boundingBox.min.z;
}

void
CVehicle::FlyingControl(eFlightModel flightModel)
{
	if(pFlyingHandling == nil)
		return;

	switch(flightModel){
	case FLIGHT_MODEL_DODO:
	{
		// This seems pretty magic

		// Move Left/Right
		float moveSpeed = m_vecMoveSpeed.Magnitude();
		float sideSpeed = DotProduct(m_vecMoveSpeed, GetRight());
		// Delegate the forces of the Dodo to the C implementation
		float sideImpulse = VehicleLogic_DodoImpulse(sideSpeed, moveSpeed);
		float fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward());
		float magic = VehicleLogic_DodoMagic(m_vecMoveSpeed.MagnitudeSqr(), fwdSpeed);
		float turnImpulse = VehicleLogic_DodoTurnForce(sideImpulse, 0.003f, m_fSteerAngle, 0.001f, magic, m_fTurnMass, (*Timer_GetTimeStep()));
		ApplyTurnForce(turnImpulse*GetRight(), -4.0f*GetForward());

		float impulse = VehicleLogic_DodoSideForce(sideImpulse, magic, m_fMass, (*Timer_GetTimeStep()));
		ApplyMoveForce(impulse*GetRight());
		ApplyTurnForce(impulse*GetRight(), 2.0f*GetUp());


		// Move Up/Down
		moveSpeed = m_vecMoveSpeed.Magnitude();
		float upSpeed = DotProduct(m_vecMoveSpeed, GetUp());
		float upImpulse = VehicleLogic_DodoImpulse(upSpeed, moveSpeed);
		turnImpulse = VehicleLogic_DodoTurnForce(upImpulse, 0.002f, -CPad::GetPad(0)->GetSteeringUpDown()/128.0f, 0.001f, magic, m_fTurnMass, (*Timer_GetTimeStep()));
		ApplyTurnForce(turnImpulse*GetUp(), -4.0f*GetForward());

		impulse = VehicleLogic_DodoLiftForce(upImpulse, magic, m_fMass, (*Timer_GetTimeStep()), GRAVITY, GetPosition().z);
		CVector com = Multiply3x3(GetMatrix(), m_vecCentreOfMass);
		ApplyMoveForce(impulse*GetUp());
		ApplyTurnForce(impulse*GetUp(), 2.0f*GetUp() + com);


		// Delegate the resistance per time step to the C implementation
		m_vecTurnSpeed.y *= VehicleLogic_ResistancePerStep(0.9f, (*Timer_GetTimeStep()));


		// Delegate the limits of the speeds to the C implementation
		m_vecMoveSpeed *= VehicleLogic_SpeedLimitScale(m_vecMoveSpeed.MagnitudeSqr(), 1.5f);
		m_vecTurnSpeed *= VehicleLogic_SpeedLimitScale(m_vecTurnSpeed.MagnitudeSqr(), 0.2f);
		break;
	}

	case FLIGHT_MODEL_RCPLANE:
	case FLIGHT_MODEL_SEAPLANE:
	case FLIGHT_MODEL_PLANE_UNUSED:
	case FLIGHT_MODEL_PLANE:
	{
		float fSteerLR = CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
		float fSteerUD = CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
		float fGunUD = Abs(CPad::GetPad(0)->GetCarGunUpDown());
#ifdef FREE_CAM
		if(!CCamera::bFreeCam || (CCamera::bFreeCam && !Input_affectedByController)) // rouz edit (ChatGPT)
#endif
		if(fGunUD > 1.0f)
			fSteerUD = -CPad::GetPad(0)->GetCarGunUpDown() / 128.0f;

		// Delegate the factor that stretches the input of the stick to the C implementation
		float fSteerMult = VehicleLogic_PlaneSteerMultiplier(fSteerUD, fSteerLR);

		fSteerLR *= fSteerMult;
		fSteerUD *= -fSteerMult;

		// thrust
		float fThrust = pFlyingHandling->fThrust;
		float fThrustFallOff = pFlyingHandling->fThrustFallOff;
		float fThrustFallOffBack = pFlyingHandling->fThrustFallOff * 8.0f;
#ifdef BETTER_ALLCARSAREDODO_CHEAT
		if (bAllDodosCheat && !IsRealPlane()) {
			fThrust = pHandling->Transmission.fEngineAcceleration
				* (pHandling->Transmission.nDriveType == '4' ? 4.0f : 2.0f);
			fThrust = 5.0f * General_MaxFloat(fThrust, pFlyingHandling->fThrust); //tweak: (cars engines too weak to thrust car on air)
			fThrustFallOff = General_MinFloat(0.7f / pHandling->Transmission.fMaxVelocity, fThrustFallOff); //tweak: (use 0.7 instead of 1.0 to make cars 30% faster)
			fThrustFallOffBack = -1.0f / pHandling->Transmission.fMaxReverseVelocity;
		}
#endif
		float fForwSpeed = DotProduct(GetMoveSpeed(), GetForward());
		CVector vecTail = GetColModel()->boundingBox.min.y * GetForward();
		float fPedalState = (CPad::GetPad(0)->GetAccelerate() - CPad::GetPad(0)->GetBrake()) / 255.0f;
		// Delegate the acceleration of the engine to the C implementation
		float fThrustAccel = VehicleLogic_PlaneThrustAccel(fPedalState, fForwSpeed, fThrust, fThrustFallOff, fThrustFallOffBack,
			flightModel == FLIGHT_MODEL_PLANE_UNUSED, flightModel == FLIGHT_MODEL_PLANE);
		ApplyMoveForce(fThrustAccel * GetForward() * m_fMass * (*Timer_GetTimeStep()));

		// left/right
		float fSideSpeed = -DotProduct(GetMoveSpeed(), GetRight());
		// Delegate the sideways acceleration to the C implementation
		float fSideSlipAccel = VehicleLogic_SideSlipAccel(pFlyingHandling->fSideSlip, fSideSpeed);
		ApplyMoveForce(m_fMass * GetRight() * fSideSlipAccel * (*Timer_GetTimeStep()));

		float fYaw = -DotProduct(GetSpeed(vecTail), GetRight());
		// Delegate the acceleration around the yaw axis to the C implementation
		float fYawAccel = VehicleLogic_PlaneAxisAccel(pFlyingHandling->fYawStab, fYaw, pFlyingHandling->fYaw, fSteerLR, fForwSpeed);
		ApplyTurnForce(fYawAccel * GetRight() * m_fTurnMass * (*Timer_GetTimeStep()), vecTail);

		float fDirectionMultiplier = 0.0f;
		if (flightModel == FLIGHT_MODEL_RCPLANE) {
			fDirectionMultiplier = CPad::GetPad(0)->GetLookRight();
			if (CPad::GetPad(0)->GetLookLeft())
				fDirectionMultiplier = -1;
		}
		// Delegate the acceleration around the roll axis to the C implementation
		float fRollAccel = VehicleLogic_PlaneRollAccel(flightModel == FLIGHT_MODEL_RCPLANE, fDirectionMultiplier, fSteerLR, pFlyingHandling->fRoll);
		ApplyTurnForce(GetRight() * fRollAccel * fForwSpeed * m_fTurnMass * (*Timer_GetTimeStep()), GetUp());

		CVector vecFRight = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
		CVector vecStabilise = (GetUp().z > 0.0f) ? vecFRight : -vecFRight;
		// Delegate the speed of the stabilisation to the C implementation
		float fStabiliseSpeed = VehicleLogic_PlaneRollStabilise(pFlyingHandling->fRollStab, GetRight().z, DotProduct(GetRight(), vecStabilise), GetForward().z);
		ApplyTurnForce(fStabiliseSpeed * m_fTurnMass * GetRight(), GetUp()); // no (*Timer_GetTimeStep()), is it right?

		// up/down
		float fTail = -DotProduct(GetSpeed(vecTail), GetUp());
		// Delegate the acceleration around the pitch axis to the C implementation
		float fPitchAccel = VehicleLogic_PlaneAxisAccel(pFlyingHandling->fPitchStab, fTail, pFlyingHandling->fPitch, fSteerUD, fForwSpeed);
		ApplyTurnForce(fPitchAccel * m_fTurnMass * GetUp() * (*Timer_GetTimeStep()), vecTail);

		float fLift = DotProduct(GetMoveSpeed(), GetUp()) / General_MaxFloat(0.01f, GetMoveSpeed().Magnitude()); //accel*angle
		// Delegate the lift to the C implementation
		float fLiftAccel = VehicleLogic_PlaneLiftAccel(pFlyingHandling->fFormLift, pFlyingHandling->fAttackLift, fLift, fForwSpeed);
		float fLiftImpulse = fLiftAccel * m_fMass * (*Timer_GetTimeStep());
		if (GRAVITY * (*Timer_GetTimeStep()) * m_fMass < fLiftImpulse) {
			if (flightModel == FLIGHT_MODEL_RCPLANE && GetPosition().z > 50.0f)
				fLiftImpulse = (*Timer_GetTimeStep()) * 0.9f*GRAVITY * m_fMass;
			else if (flightModel == FLIGHT_MODEL_SEAPLANE && GetPosition().z > 80.0f)
				fLiftImpulse = (*Timer_GetTimeStep()) * 0.9f*GRAVITY * m_fMass;
#ifdef BETTER_ALLCARSAREDODO_CHEAT
			else if(bAllDodosCheat && GetPosition().z > 170.0f)
				fLiftImpulse = (*Timer_GetTimeStep()) * 0.9f * GRAVITY * m_fMass;
#endif
		}
		ApplyMoveForce(fLiftImpulse * GetUp());

		CVector vecResistance;
		vecResistance = pFlyingHandling->vecTurnRes;
		// Delegate the resistance per time step to the C implementation
		float rX = VehicleLogic_ResistancePerStep(vecResistance.x, (*Timer_GetTimeStep()));
		float rZ = VehicleLogic_ResistancePerStep(vecResistance.z, (*Timer_GetTimeStep()));
		CVector vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());
		vecTurnSpeed.x *= rX;
		// Delegate the resistance to rolling to the C implementation
		float fResistance = VehicleLogic_PlaneRollResistance(vecTurnSpeed.y, vecResistance.y, pFlyingHandling->vecSpeedRes.y, (*Timer_GetTimeStep()));
		vecTurnSpeed.z *= rZ;
		m_vecTurnSpeed = Multiply3x3(GetMatrix(), vecTurnSpeed);
		ApplyTurnForce(-GetUp() * fResistance * m_fTurnMass, GetRight() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));


		// Delegate the limits of the speeds to the C implementation
		m_vecMoveSpeed *= VehicleLogic_SpeedLimitScale(m_vecMoveSpeed.MagnitudeSqr(), 1.5f);
		m_vecTurnSpeed *= VehicleLogic_SpeedLimitScale(m_vecTurnSpeed.MagnitudeSqr(), 0.2f);
		break;
	}
	case FLIGHT_MODEL_RCHELI:
	case FLIGHT_MODEL_HELI:
	{
#ifdef RESTORE_ALLCARSHELI_CHEAT
		tFlyingHandlingData* flyingHandling = bAllCarCheat && !IsRealHeli() ? mod_HandlingManager.GetFlyingPointer(HANDLING_MAVERICK) : pFlyingHandling;
#else
		tFlyingHandlingData* flyingHandling = pFlyingHandling;
#endif
		// Delegate the resistance per time step to the C implementation
		float rm = VehicleLogic_ResistancePerStep(flyingHandling->fMoveRes, (*Timer_GetTimeStep()));
		m_vecMoveSpeed *= rm;
		if (GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE)
			return;
		float fUpSpeed = DotProduct(m_vecMoveSpeed, GetUp());
		float fPedalState = (CPad::GetPad(0)->GetAccelerate() - CPad::GetPad(0)->GetBrake()) / 255.0f;
		if(flightModel == FLIGHT_MODEL_RCHELI)
			ApplyMoveForce(GRAVITY * CVector(0.0f, 0.0f, 0.5f) * m_fMass * (*Timer_GetTimeStep()));

		// Delegate the thrust of the helicopter to the C implementation
		float fThrust = VehicleLogic_HeliThrust(fPedalState, flightModel == FLIGHT_MODEL_RCHELI, flyingHandling->fThrust, flyingHandling->fThrustFallOff, fUpSpeed, GetPosition().z);
		ApplyMoveForce(GRAVITY * GetUp() * fThrust * m_fMass * (*Timer_GetTimeStep()));

		// Delegate the lean of the helicopter to the C implementation
		float upRight, upFwd;
		VehicleLogic_HeliStabilise(GetUp().z, GetRight().z, GetForward().z, flyingHandling->fFormLift, &upRight, &upFwd);
		float upImpulseRight = -upRight * flyingHandling->fAttackLift * m_fTurnMass * (*Timer_GetTimeStep());
		ApplyTurnForce(upImpulseRight * GetUp(), GetRight());

		float upImpulseFwd = -upFwd * flyingHandling->fAttackLift * m_fTurnMass * (*Timer_GetTimeStep());
		ApplyTurnForce(upImpulseFwd * GetUp(), GetForward());

		float fRoll, fPitch, fYaw;
		if (bCheat5) {
			fPitch = CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
			fRoll = CPad::GetPad(0)->GetLookLeft();
			if (CPad::GetPad(0)->GetLookRight())
				fRoll = -1.0f;
			fYaw = CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
		} else {
			fPitch = CPad::GetPad(0)->GetSteeringUpDown() / 128.0f;
			fRoll = -CPad::GetPad(0)->GetSteeringLeftRight() / 128.0f;
			fYaw = CPad::GetPad(0)->GetLookRight();
			if (CPad::GetPad(0)->GetLookLeft())
				fYaw = -1.0f;
#ifdef FREE_CAM
			if (!CCamera::bFreeCam || (CCamera::bFreeCam && !Input_affectedByController)) // rouz edit (ChatGPT)
#endif
			if(Abs(CPad::GetPad(0)->GetCarGunLeftRight()) > 1.0f)
				fYaw = CPad::GetPad(0)->GetCarGunLeftRight() / 128.0f;
		}
#ifdef FREE_CAM
		if(!CCamera::bFreeCam || (CCamera::bFreeCam && !Input_affectedByController)) // rouz edit (ChatGPT)
#endif
		if(Abs(CPad::GetPad(0)->GetCarGunUpDown()) > 1.0f)
			fPitch = -CPad::GetPad(0)->GetCarGunUpDown() / 128.0f;
		if (CPad::GetPad(0)->GetHorn()) {
			fYaw = 0.0f;
			fPitch = VehicleLogic_StabilityClamp(flyingHandling->fPitchStab, DotProduct(m_vecMoveSpeed, GetForward()));
			fRoll = VehicleLogic_StabilityClamp(flyingHandling->fRollStab, DotProduct(m_vecMoveSpeed, GetRight()));
		}
		ApplyTurnForce(fPitch * GetUp() * flyingHandling->fPitch * m_fTurnMass * (*Timer_GetTimeStep()), GetForward());
		ApplyTurnForce(fRoll * GetUp() * flyingHandling->fRoll * m_fTurnMass * (*Timer_GetTimeStep()), GetRight());

		float fSideSpeed = -DotProduct(GetMoveSpeed(), GetRight());
		// Delegate the sideways acceleration to the C implementation
		float fSideSlipAccel = VehicleLogic_SideSlipAccel(flyingHandling->fSideSlip, fSideSpeed);
		ApplyMoveForce(m_fMass * GetRight() * fSideSlipAccel * (*Timer_GetTimeStep()));
		float fYawAccel = flyingHandling->fYawStab * fSideSpeed * Abs(fSideSpeed) + flyingHandling->fYaw * fYaw;
		ApplyTurnForce(fYawAccel * GetRight() * m_fTurnMass * (*Timer_GetTimeStep()), -GetForward());

		ApplyTurnForce(fYaw * GetForward() * flyingHandling->fYaw * m_fTurnMass * (*Timer_GetTimeStep()), GetRight());

		// Delegate the resistance per time step to the C implementation
		float rX = VehicleLogic_ResistancePerStep(flyingHandling->vecTurnRes.x, (*Timer_GetTimeStep()));
		float rY = VehicleLogic_ResistancePerStep(flyingHandling->vecTurnRes.y, (*Timer_GetTimeStep()));
		CVector vecTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());
		// Delegate the resistance to yawing to the C implementation
		float fResistance;
		float fResistanceMultiplier = VehicleLogic_HeliYawResistance(vecTurnSpeed.z, flyingHandling->vecTurnRes.z, flyingHandling->vecSpeedRes.z, (*Timer_GetTimeStep()), &fResistance);
		vecTurnSpeed.x *= rX;
		vecTurnSpeed.y *= rY;
		vecTurnSpeed.z *= fResistanceMultiplier;
		m_vecTurnSpeed = Multiply3x3(GetMatrix(), vecTurnSpeed);
		ApplyTurnForce(-GetRight() * fResistance * m_fTurnMass, GetForward() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));
		break;
	}
	}
}

static void DestroyStaticRotorModel(void);
static CColModel rotorColModel=ColModel_InitStatic(&rotorColModel,DestroyStaticRotorModel);
static void DestroyStaticRotorModel(void)
{
    // Release the rotor model through the complete C owner at process exit
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&rotorColModel);
}
static CColSphere rotorColSphere;
float ROTOR_SEMI_THICKNESS = 0.05f;
float ROTOR_TURN_SPEED = 0.2f;
float ROTOR_DISGUARD_MULT = 0.3f;
float ROTOR_COL_ELASTICITY = 1.0f;
float ROTOR_COL_TURNMULT = -0.001f;
float ROTOR_DEFAULT_DAMAGE = 100.0f;

bool
CVehicle::DoBladeCollision(CVector pos, CMatrix &matrix, int16 rotorType, float radius, float damageMult)
{
	CVector max(radius, radius, radius);
	CVector min(-radius, -radius, -radius);

	switch(rotorType){
	case ROTOR_TOP:
	case ROTOR_BOTTOM:
		min.z = -ROTOR_SEMI_THICKNESS;
		max.z = ROTOR_SEMI_THICKNESS;
		break;
	case ROTOR_FRONT:
	case ROTOR_BACK:
		min.y = -ROTOR_SEMI_THICKNESS;
		max.y = ROTOR_SEMI_THICKNESS;
		break;
	case ROTOR_RIGHT:
	case ROTOR_LEFT:
		min.x = -ROTOR_SEMI_THICKNESS;
		max.x = ROTOR_SEMI_THICKNESS;
		break;
	}

	min += pos;
	max += pos;
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum0=min;
	    const CVector &boxMaximum0=max;
	    Box_Set(&rotorColModel.boundingBox,COLLISION_POINT_REF(boxMinimum0),COLLISION_POINT_REF(boxMaximum0));
	}
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter0=pos;
	    Sphere_Set(&rotorColModel.boundingSphere,radius,COLLISION_POINT_REF(sphereCenter0));
	}
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter1=pos;
	    ColSphere_Set(&rotorColSphere,radius,COLLISION_POINT_REF(sphereCenter1),0,0);
	}
	rotorColModel.spheres = &rotorColSphere;
	rotorColModel.numSpheres = 1;

	pos = matrix * pos;
	bool hadCollision = false;
	// Delegate the range of sectors to the C implementation
	int32_t minX, minY, maxX, maxY;
	World_SectorRange(pos.x - radius, pos.y - radius, pos.x + radius, pos.y + radius, &minX, &minY, &maxX, &maxY);

	CWorld::AdvanceCurrentScanCode();
	for(int curY = minY; curY <= maxY; curY++) {
		for(int curX = minX; curX <= maxX; curX++) {
			CSector *sector = CWorld::GetSector(curX, curY);
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_BUILDINGS], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_VEHICLES], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_PEDS], rotorColModel, matrix, rotorType, 0.0f))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_PEDS_OVERLAP], rotorColModel, matrix, rotorType, 0.0f))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_OBJECTS], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
			if(BladeColSectorList(sector->m_lists[ENTITYLIST_OBJECTS_OVERLAP], rotorColModel, matrix, rotorType, damageMult))
				hadCollision = true;
		}
	}
	rotorColModel.spheres = nil;
	rotorColModel.numSpheres = 0;

	return hadCollision;
}

bool
CVehicle::BladeColSectorList(CPtrList &list, CColModel &rotorColModel, CMatrix &matrix, int16 rotorType, float damageMult)
{
	int i;
	CVector axis;
	CVector turnSpeed(0.0f, 0.0f, 0.0f);
	switch(rotorType){
	case ROTOR_TOP:
		turnSpeed.z = -ROTOR_TURN_SPEED;
		axis = -matrix.GetUp();
		break;
	case ROTOR_BOTTOM:
		turnSpeed.z = ROTOR_TURN_SPEED;
		axis = matrix.GetUp();
		break;

	case ROTOR_FRONT:
		turnSpeed.y = -ROTOR_TURN_SPEED;
		axis = -matrix.GetForward();
		break;
	case ROTOR_BACK:
		turnSpeed.y = ROTOR_TURN_SPEED;
		axis = matrix.GetForward();
		break;

	case ROTOR_RIGHT:
		turnSpeed.x = -ROTOR_TURN_SPEED;
		axis = -matrix.GetRight();
		break;
	case ROTOR_LEFT:
		turnSpeed.x = ROTOR_TURN_SPEED;
		axis = matrix.GetRight();
		break;
	}
	turnSpeed = Multiply3x3(matrix, turnSpeed);
	CVector center = CollisionVector_ToNative(rotorColModel.boundingSphere.center);
	center = matrix*center;

	for(CPtrNode *node = list.first; node; node = node->next) {
		CEntity *entity = (CEntity *)node->item;
		if(entity == (CEntity*)this ||
		   !entity->bUsesCollision ||
		   entity->m_scanCode == CWorld::GetCurrentScanCode())
			continue;

		entity->m_scanCode = CWorld::GetCurrentScanCode();

		int numCollisions;
		CColModel *entityCol;
		if(entity->IsPed())
			entityCol = ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(entity->GetModelIndex())))->AnimatePedColModelSkinned(entity->GetClump()); // rouz edit (ChatGPT)
		else
			entityCol = entity->GetColModel();
		if(entityCol)
			numCollisions = CCollision::ProcessColModels(matrix, rotorColModel, entity->GetMatrix(), *entityCol,
				CWorld::m_aTempColPts, nil, nil);
		else
			numCollisions = 0;

		if(numCollisions > 0 && entity->IsPed()){
			CPed *ped = (CPed*)entity;
			CVector2D dirToRotor = GetPosition() - entity->GetPosition();
			dirToRotor.Normalise();
			int localDir = ped->GetLocalDirection(dirToRotor);
			if(ped->m_attachedTo == nil){
				ped->bIsStanding = false;
				ped->ApplyMoveForce(-5.0f*dirToRotor.x, -5.0f*dirToRotor.y, 5.0f);
			}
			ped->InflictDamage(this, WEAPONTYPE_RUNOVERBYCAR, 1000.0f, PEDPIECE_TORSO, localDir);

			if(CGame::nastyGame && ped->GetIsOnScreen()){
				for(i = 0; i < 16; i++)
					CParticle::AddParticle(PARTICLE_BLOOD_SMALL, ped->GetPosition(), CVector(dirToRotor.x, dirToRotor.y, 1.0f) * 0.01f);
				CParticle::AddParticle(PARTICLE_TEST, ped->GetPosition(), CVector(0.0f, 0.0f, 0.02f), nil, 0.1f);
				CParticle::AddParticle(PARTICLE_TEST, ped->GetPosition()+CVector(0.0f, 0.0f, 0.2f), CVector(0.0f, 0.0f, -0.01f), nil, 0.1f);
			}
		}else if(numCollisions > 0 && entity->GetModelIndex() != MI_MISSILE){
			float impulse = 0.0f;
			bool hadCollision = false;
			float savedElasticity = m_fElasticity;
			m_fElasticity = ROTOR_COL_ELASTICITY;

			for(i = 0; i < numCollisions; i++){
				CVector colpos = CollisionVector_ToNative(CWorld::m_aTempColPts[i].point);
				CVector localColpos = colpos - center;
				float axisDir = DotProduct(axis, localColpos);
				float colDir = DotProduct(CollisionVector_ToNative(CWorld::m_aTempColPts[i].normal), localColpos);

				// Delegate the check that the collision misses the rotor to the C implementation
				if(VehicleLogic_RotorMissesCollision(axisDir, colDir, ROTOR_SEMI_THICKNESS, ROTOR_DISGUARD_MULT))
					continue;

				hadCollision = true;
				colpos -= axisDir*axis;	// get rid of axis component

				CVector tangentSpeed = CrossProduct(turnSpeed, colpos - center);

				// Particles
				for(int j = 0; j < 4; j++){
					CParticle::AddParticle(PARTICLE_SPARK_SMALL, colpos, (tangentSpeed+m_vecMoveSpeed)/2.0f);
					CParticle::AddParticle(PARTICLE_SPARK, colpos, 0.1f*CollisionVector_ToNative(CWorld::m_aTempColPts[i].normal));
				}

				// Apply Collision
				if(IsCar()){
					CAutomobile *heli = (CAutomobile*)this;
					if(heli->m_aWheelSpeed[1] > 0.15f){
						ApplyCollision(CWorld::m_aTempColPts[i], impulse);
						ApplyTurnForce(m_fTurnMass*ROTOR_COL_TURNMULT*tangentSpeed, colpos - center);
						heli->m_aWheelSpeed[1] = 0.15f;
					}else if(heli->m_aWheelSpeed[1] < 0.075f && heli->m_aWheelSpeed[1] > 0.0f)
						heli->m_aWheelSpeed[1] *= -1.0f;
				}

				float damageImpulse = damageMult * General_MaxFloat(impulse, ROTOR_DEFAULT_DAMAGE*m_fMass/3000.0f);
				if(damageImpulse > m_fDamageImpulse)
					SetDamagedPieceRecord(0, damageImpulse, entity, CollisionVector_ToNative(CWorld::m_aTempColPts[i].normal));

			}

			if(hadCollision && !entity->IsPed())
				DMAudio_ReportCollision(this, entity, SURFACE_CAR_PANEL, SURFACE_TARMAC, 50.0f, 0.09f);
			m_fElasticity = savedElasticity;
		}
	}
	return false;
}


float fBurstSpeedMax = 0.3f;
float fBurstTyreMod = 0.13f;

void
CVehicle::ProcessWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
	int32 wheelsOnGround, float thrust, float brake, float adhesion, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, uint16 wheelStatus)
{
	// BUG: using statics here is probably a bad idea
	static bool bAlreadySkidding = false;	// this is never reset
	static bool bBraking;
	static bool bDriving;

#ifdef FIX_SIGNIFICANT_BUGS
	bAlreadySkidding = false;
#endif

	// Delegate the friction of the wheel to the C implementation
	float contactSpeedFwd = DotProduct(wheelContactSpeed, wheelFwd);
	float contactSpeedRight = DotProduct(wheelContactSpeed, wheelRight);

	VehicleWheelInput wheelInput;
	wheelInput.contactSpeedFwd = contactSpeedFwd;
	wheelInput.contactSpeedRight = contactSpeedRight;
	wheelInput.wheelsOnGround = wheelsOnGround;
	wheelInput.thrust = thrust;
	wheelInput.brake = brake;
	wheelInput.adhesion = adhesion;
	wheelInput.timeStep = (*Timer_GetTimeStep());
	wheelInput.tractionLoss = pHandling->fTractionLoss;
	wheelInput.slippery = rouz.slippery != 0;
	wheelInput.isFrontWheel = wheelId == CARWHEEL_FRONT_LEFT || wheelId == CARWHEEL_FRONT_RIGHT;
	wheelInput.isBurst = wheelStatus == WHEEL_STATUS_BURST;
	wheelInput.burstSpeedMax = fBurstSpeedMax;
	wheelInput.burstTyreMod = fBurstTyreMod;
	wheelInput.gasPedal = m_fGasPedal;
	wheelInput.isBike = IsBike();
	wheelInput.isRCBandit = GetModelIndex() == MI_RCBANDIT;
	wheelInput.mass = pHandling->fMass;
	wheelInput.wheelFriction = mod_HandlingManager.fWheelFriction;
#ifdef FIX_BUGS
	wheelInput.useTimeStepFix = true;
#else
	wheelInput.useTimeStepFix = false;
#endif
	wheelInput.timeStepFix = Timer_GetTimeStepFix();
	wheelInput.wheelState = *wheelState;
	VehicleWheelOutput wheelOutput;
	VehicleLogic_ProcessWheel(&wheelInput, &bAlreadySkidding, &wheelOutput);
	float fwd = wheelOutput.fwd;
	float right = wheelOutput.right;
	*wheelState = (tWheelState)wheelOutput.wheelState;
	bBraking = wheelOutput.braking;
	bDriving = wheelOutput.driving;

	if(fwd != 0.0f || right != 0.0f){
		CVector totalSpeed = fwd*wheelFwd + right*wheelRight;

		CVector turnDirection = totalSpeed;
		bool separateTurnForce = false;	// BUG: not initialized on PC
		if(pHandling->fSuspensionAntidiveMultiplier > 0.0f){
			if(bBraking){
				separateTurnForce = true;
				turnDirection = totalSpeed - pHandling->fSuspensionAntidiveMultiplier*fwd*wheelFwd;
			}else if(bDriving){
				separateTurnForce = true;
				turnDirection = totalSpeed - 0.5f*pHandling->fSuspensionAntidiveMultiplier*fwd*wheelFwd;
			}
		}

		CVector direction = totalSpeed;

		float speed = totalSpeed.Magnitude();
		float turnSpeed;
		if(separateTurnForce)
			turnSpeed = turnDirection.Magnitude();
		else
			turnSpeed = speed;
		direction.Normalise();
		if(separateTurnForce)
			turnDirection.Normalise();
		else
			turnDirection = direction;

		float impulse = speed*m_fMass;
		float turnImpulse = turnSpeed*GetMass(wheelContactPoint, turnDirection);

		ApplyMoveForce(impulse * direction);
		ApplyTurnForce(turnImpulse * turnDirection, wheelContactPoint);
	}
}

float fBurstBikeSpeedMax = 0.12f;
float fBurstBikeTyreMod = 0.05f;
float fTweakBikeWheelTurnForce = 2.0f;

void
CVehicle::ProcessBikeWheel(CVector &wheelFwd, CVector &wheelRight, CVector &wheelContactSpeed, CVector &wheelContactPoint,
	int32 wheelsOnGround, float thrust, float brake, float adhesion, float destabTraction, int8 wheelId, float *wheelSpeed, tWheelState *wheelState, eBikeWheelSpecial special, uint16 wheelStatus)
{
	// BUG: using statics here is probably a bad idea
	static bool bAlreadySkidding = false;	// this is never reset
	static bool bBraking;
	static bool bDriving;
	static bool bReversing;

#ifdef FIX_SIGNIFICANT_BUGS
	bAlreadySkidding = false;
#endif

	// Delegate the friction of the wheel to the C implementation
	float contactSpeedFwd = DotProduct(wheelContactSpeed, wheelFwd);
	float contactSpeedRight;
	if(special == BIKE_WHEELSPEC_2 || special == BIKE_WHEELSPEC_3)
		contactSpeedRight = 0.0f;
	else
		contactSpeedRight = DotProduct(wheelContactSpeed, wheelRight);

	VehicleBikeWheelInput wheelInput;
	wheelInput.contactSpeedFwd = contactSpeedFwd;
	wheelInput.contactSpeedRight = contactSpeedRight;
	wheelInput.wheelsOnGround = wheelsOnGround;
	wheelInput.thrust = thrust;
	wheelInput.brake = brake;
	wheelInput.adhesion = adhesion;
	wheelInput.destabTraction = destabTraction;
	wheelInput.timeStep = (*Timer_GetTimeStep());
	wheelInput.tractionLoss = pHandling->fTractionLoss;
	wheelInput.isBurst = wheelStatus == WHEEL_STATUS_BURST;
	wheelInput.burstSpeedMax = fBurstBikeSpeedMax;
	wheelInput.burstTyreMod = fBurstBikeTyreMod;
	wheelInput.gasPedal = m_fGasPedal;
	wheelInput.isBike = IsBike();
	wheelInput.isRCBandit = GetModelIndex() == MI_RCBANDIT;
	wheelInput.handlingMass = pHandling->fMass;
	wheelInput.mass = m_fMass;
	wheelInput.wheelFriction = mod_HandlingManager.fWheelFriction;
#ifdef FIX_BUGS
	wheelInput.useTimeStepFix = true;
#else
	wheelInput.useTimeStepFix = false;
#endif
	wheelInput.timeStepFix = Timer_GetTimeStepFix();
	wheelInput.wheelState = *wheelState;
	VehicleBikeWheelOutput wheelOutput;
	VehicleLogic_ProcessBikeWheel(&wheelInput, &bAlreadySkidding, &wheelOutput);
	float fwd = wheelOutput.fwd;
	float right = wheelOutput.right;
	*wheelState = (tWheelState)wheelOutput.wheelState;
	bBraking = wheelOutput.braking;
	bDriving = wheelOutput.driving;
	bReversing = wheelOutput.reversing;

	if(fwd != 0.0f || right != 0.0f){
		CVector direction = fwd*wheelFwd + right*wheelRight;

		float speed = direction.Magnitude();
		direction.Normalise();

		float impulse = speed*m_fMass;
		float turnImpulse = speed*GetMass(wheelContactPoint, direction);
		CVector vTurnImpulse = turnImpulse * direction;
		ApplyMoveForce(impulse * direction);

		float turnRight = DotProduct(vTurnImpulse, GetRight());
		float contactRight = DotProduct(wheelContactPoint, GetRight());
		float contactFwd = DotProduct(wheelContactPoint, GetForward());

		if(wheelId != BIKEWHEEL_REAR || !bBraking && !bReversing)
			ApplyTurnForce((vTurnImpulse - turnRight*GetRight()) * fTweakBikeWheelTurnForce,
				wheelContactPoint - contactRight*GetRight());

		ApplyTurnForce(turnRight*GetRight(), contactFwd*GetForward());
	}
}

float
CVehicle::ProcessWheelRotation(tWheelState state, const CVector &fwd, const CVector &speed, float radius)
{
	// Delegate to the C implementation
	return Vehicle_ProcessWheelRotation(state, DotProduct(fwd, speed), radius, (*Timer_GetTimeStep()));
}

int
CVehicle::FindTyreNearestPoint(float x, float y)
{
	// The offsets are computed here and the choice made in C
	CVector pos = CVector(x - GetPosition().x, y - GetPosition().y, 0.0f);
	return Vehicle_FindTyreNearestPoint(IsBike(), DotProduct(GetForward(), pos), DotProduct(GetRight(), pos));
}

void
CVehicle::InflictDamage(CEntity *damagedBy, eWeaponType weaponType, float damage, CVector pos)
{
	if (!bCanBeDamaged)
		return;
	if(GetStatus() == STATUS_PLAYER && Stats_GetPercentageProgress() >= 100.0f)
		damage *= 0.5f;
	if (GetStatus() != STATUS_PLAYER && bOnlyDamagedByPlayer && (damagedBy != FindPlayerPed() && damagedBy != FindPlayerVehicle()))
		return;

	if(damage > 10.0f && (damagedBy == FindPlayerPed() || damagedBy == FindPlayerVehicle()) && GetStatus() != STATUS_WRECKED){
		CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel += 2;
		CWorld::Players[CWorld::PlayerInFocus].m_fMediaAttention += 1.0f;
		Stats_PropertyDestroyed += General_GetRandomNumberInRangeInt(5, 25);
	}

	// Delegate the kind of damage of the weapon to the C implementation
	bool bFrightensDriver = false;
	switch (VehicleLogic_DamageProofKind(weaponType)) {
	case VEHICLE_DAMAGE_MELEE:
		if (bMeleeProof)
			return;
		break;
	case VEHICLE_DAMAGE_BULLET:
		if (bBulletProof)
			return;
		bFrightensDriver = true;
		break;
	case VEHICLE_DAMAGE_EXPLOSION:
		if (bExplosionProof)
			return;
		bFrightensDriver = true;
		break;
	case VEHICLE_DAMAGE_FIRE:
		if (bFireProof)
			return;
		break;
	case VEHICLE_DAMAGE_COLLISION:
		if (bCollisionProof)
			return;
		break;
	default:
		break;
	}
	if(bFrightensDriver && GetStatus() == STATUS_PLAYER && m_fHealth < 250.0f)
		return;

	// Pop tires
	if(damagedBy && damagedBy->IsPed() && (IsCar() || IsBike())){
		// Delegate the chance to burst a tyre to the C implementation
		int accuracy = VehicleLogic_TyreBurstAccuracy(weaponType, ((CPed*)damagedBy)->IsPlayer());
		if(((CPed*)damagedBy)->IsPlayer() && (CCamera::m_bUseMouse3rdPerson || TheCamera.Using1stPersonWeaponMode()))
			accuracy = 0;

		if(accuracy != 0 && !bTyresDontBurst && (General_GetRandomNumber()&0x7F) < accuracy){
			if(IsBike())
				BurstTyre(FindTyreNearestPoint(pos.x, pos.y) + CAR_PIECE_WHEEL_LF, false);
			else if(GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR)
				BurstTyre(FindTyreNearestPoint(pos.x, pos.y) + CAR_PIECE_WHEEL_LF, true);
		}
	}

	if (m_fHealth > 0.0f) {
		if (VehicleCreatedBy == RANDOM_VEHICLE && pDriver &&
			(GetStatus() == STATUS_SIMPLE || GetStatus() == STATUS_PHYSICS) &&
			AutoPilot.m_nCarMission == MISSION_CRUISE) {
			if (m_randomSeed < DAMAGE_FLEE_IN_CAR_PROBABILITY_VALUE) {
				CCarCtrl::SwitchVehicleToRealPhysics(this);
				AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_AVOID_CARS;
				AutoPilot.m_nCruiseSpeed = GAME_SPEED_TO_CARAI_SPEED * pHandling->Transmission.fMaxCruiseVelocity;
				SetStatus(STATUS_PHYSICS);
			}
		}
		m_nLastWeaponDamage = weaponType;
		m_pLastDamageEntity = damagedBy;
		float oldHealth = m_fHealth;
		if (m_fHealth > damage) {
			m_fHealth -= damage;
			if (VehicleCreatedBy == RANDOM_VEHICLE && !IsBoat()){
				switch (GetStatus()) {
				case STATUS_SIMPLE:
				case STATUS_PHYSICS:
					if(AutoPilot.m_nDrivingStyle == DRIVINGSTYLE_PLOUGH_THROUGH ||
					   General_GetRandomNumberInRangeFloat(0.0f, 1.0f) > 0.5f && AutoPilot.m_nCarMission == MISSION_CRUISE){
						// Drive away like a maniac
						if(pDriver && pDriver->m_objective != OBJECTIVE_LEAVE_CAR){
							if(AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_PLOUGH_THROUGH)
								AutoPilot.m_nCruiseSpeed *= 1.5f;
							AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_PLOUGH_THROUGH;
						}
					}else{
						// Leave vehicle
						if (pDriver && pDriver->CharCreatedBy != MISSION_CHAR) {
							SetStatus(STATUS_ABANDONED);
							pDriver->bFleeAfterExitingCar = true;
							pDriver->SetObjective(OBJECTIVE_LEAVE_CAR, this);
							pDriver->Say(SOUND_PED_FLEE_SPRINT);
						}
						int time = 200;
						for (int i = 0; i < m_nNumMaxPassengers; i++) {
							if (pPassengers[i] &&
							    pPassengers[i]->m_objective != OBJECTIVE_LEAVE_CAR &&
							    pPassengers[i]->CharCreatedBy != MISSION_CHAR) {
								pPassengers[i]->bFleeAfterExitingCar = true;
								pPassengers[i]->SetObjective(OBJECTIVE_LEAVE_CAR, this);
								pPassengers[i]->m_objectiveTimer = (*Timer_GetTimeInMilliseconds()) + time;
								pPassengers[i]->Say(SOUND_PED_FLEE_SPRINT);
								time += 200;
							}
						}
					}
					break;
				default:
					break;
				}
			}
			if (oldHealth >= DAMAGE_HEALTH_TO_CATCH_FIRE && m_fHealth < DAMAGE_HEALTH_TO_CATCH_FIRE) {
				if (IsCar()) {
					CAutomobile* pThisCar = (CAutomobile*)this;
					Damage_SetEngineStatus(&pThisCar->Damage, ENGINE_STATUS_ON_FIRE);
					pThisCar->m_pSetOnFireEntity = damagedBy;
					if (damagedBy)
						damagedBy->RegisterReference((CEntity**)&pThisCar->m_pSetOnFireEntity);
				}
			}
		}
		else {
			m_fHealth = 0.0f;
			if (weaponType == WEAPONTYPE_EXPLOSION) {
				// between 1000 and 3047. Also not very nice: can't be saved by respray or cheat
				m_nBombTimer = 1000 + General_GetRandomNumber() & 0x7FF;
				m_pBlowUpEntity = damagedBy;
				if (damagedBy)
					damagedBy->RegisterReference((CEntity**)&m_pBlowUpEntity);
			}
			else
				BlowUpCar(damagedBy);
		}
	}
#ifdef FIX_BUGS // removing dumb case when shooting police car in player's own garage gives wanted level
	if (GetModelIndex() == MI_POLICE && damagedBy == FindPlayerPed() && damagedBy != nil && !bHasBeenOwnedByPlayer)
#else
	if (GetModelIndex() == MI_POLICE && damagedBy == FindPlayerPed())
#endif
		FindPlayerPed()->SetWantedLevelNoDrop(1);
}

void
CVehicle::DoFixedMachineGuns(void)
{
	if(TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_FORWARD){
		if(CPad::GetPad(0)->GetCarGunFired() && !bGunSwitchedOff){
			FireFixedMachineGuns();
		}else{
			if((*Timer_GetTimeInMilliseconds()) > m_nGunFiringTime + 1400)
				m_nAmmoInClip = 20;
		}
	}
}

void
CVehicle::FireFixedMachineGuns(void)
{
	if ((*Timer_GetTimeInMilliseconds()) <= m_nGunFiringTime + 150)
		return;
	CVector source, target;
	// Delegate the direction of the shots to the C implementation
	float dx, dy;
	VehicleLogic_FixedGunDirection(GetForward().x, GetForward().y, &dx, &dy);

	m_nGunFiringTime = (*Timer_GetTimeInMilliseconds());

	source = GetMatrix() * CVector(2.0f, 2.5f, 1.0f);
	target = source + CVector(dx, dy, 0.0f) * 60.0f;
	target += CVector(
		((General_GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((General_GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((General_GetRandomNumber() & 0xFF) - 128) * 0.02f);
	CWeapon::DoTankDoomAiming(this, pDriver, &source, &target);
	FireOneInstantHitRound(&source, &target, 15);

	source = GetMatrix() * CVector(-2.0f, 2.5f, 1.0f);
	target = source + CVector(dx, dy, 0.0f) * 60.0f;
	target += CVector(
		((General_GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((General_GetRandomNumber() & 0xFF) - 128) * 0.015f,
		((General_GetRandomNumber() & 0xFF) - 128) * 0.02f);
	CWeapon::DoTankDoomAiming(this, pDriver, &source, &target);
	FireOneInstantHitRound(&source, &target, 15);

	DMAudio_PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);

	m_nAmmoInClip--;
	if (m_nAmmoInClip == 0) {
		m_nAmmoInClip = 20;
		m_nGunFiringTime = (*Timer_GetTimeInMilliseconds()) + 1400;
	}
}

void
CVehicle::ActivateBomb(void)
{
	if(m_bombType == CARBOMB_TIMED){
		m_bombType = CARBOMB_TIMEDACTIVE;
		m_nBombTimer = 7000;
		m_pBlowUpEntity = FindPlayerPed();
		CGarages::TriggerMessage("GA_12", -1, 3000, -1);
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_BOMB_TIMED_ACTIVATED, 1.0f);
	}else if(m_bombType == CARBOMB_ONIGNITION){
		m_bombType = CARBOMB_ONIGNITIONACTIVE;
		CGarages::TriggerMessage("GA_12", -1, 3000, -1);
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_BOMB_ONIGNITION_ACTIVATED, 1.0f);
	}
}

void
CVehicle::ActivateBombWhenEntered(void)
{
	if(pDriver){
		if(!bDriverLastFrame && m_bombType == CARBOMB_ONIGNITIONACTIVE){
			// If someone enters the car and there is a bomb, detonate
			m_nBombTimer = 1000;
			m_pBlowUpEntity = m_pBombRigger;
			if(m_pBlowUpEntity)
				m_pBlowUpEntity->RegisterReference((CEntity**)&m_pBlowUpEntity);
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_BOMB_TICK, 1.0f);
		}
		bDriverLastFrame = true;
	}else
		bDriverLastFrame = false;
}

void
CVehicle::ExtinguishCarFire(void)
{
	if(GetStatus() != STATUS_WRECKED)
		// Delegate the limit to the C implementation
		m_fHealth = General_MaxFloat(m_fHealth, 300.0f);
	if(m_pCarFire)
		Fire_Extinguish(m_pCarFire);
	if(IsCar()){
		CAutomobile *car = (CAutomobile*)this;
		if(Damage_GetEngineStatus(&car->Damage) >= ENGINE_STATUS_ON_FIRE)
			Damage_SetEngineStatus(&car->Damage, ENGINE_STATUS_ON_FIRE-10);
		car->m_fFireBlowUpTimer = 0.0f;
	}
}

bool
CVehicle::ShufflePassengersToMakeSpace(void)
{
	// The seat shuffling is done in C, asking this vehicle whether a ped can leave through a door
	return VehicleLogic_ShufflePassengersToMakeSpace(this,
		[](void *context, uint32_t door) { return ((CVehicle*)context)->IsRoomForPedToLeaveCar(door, nil); }, this);
}

void
CVehicle::MakeNonDraggedPedsLeaveVehicle(CPed *ped1, CPed *ped2, CPlayerPed *&player, CCopPed *&cop)
{
	int i;
	player = nil;
	cop = nil;

	if(ped1->IsPlayer() && ped2->m_nPedType == PEDTYPE_COP &&
	   Wanted_GetWantedLevel(((CPlayerPed*)ped1)->m_pWanted) > 0 &&
	   ped2->m_pedInObjective == ped1){
		player = (CPlayerPed*)ped1;
		cop = (CCopPed*)ped2;
		return;
	}

	bool ped1IsDriver = ped1 == pDriver;

	// Just what the hell is this weird code?
	CPed *peds[9];
	CPed *peds2[9];
	int numPeds = 0;
	int numPeds2 = 0;
	for(i = 0; i < m_nNumMaxPassengers; i++){
		CPed *p = pPassengers[i];
		if(p && p != ped1 && !p->bStayInCarOnJack){
			peds[numPeds++] = p;
			// uhh what?
			if(i > 0 || ped1IsDriver)
				peds2[numPeds2++] = p;
		}
	}

	// So we're copying this array for no reason...
	CPed *peds3[9];
	int numPeds3 = 0;
	for(i = 0; i < numPeds; i++){
		if(peds[i]->IsPlayer() && ped2->m_nPedType == PEDTYPE_COP &&
		   Wanted_GetWantedLevel(((CPlayerPed*)peds[i])->m_pWanted) > 0 &&
		   ped2->m_pedInObjective == peds[i]){
			player = (CPlayerPed*)peds[i];
			cop = (CCopPed*)ped2;
			return;
		}
		peds3[numPeds3++] = peds[i];
	}

	int time = 1800;
	for(i = 0; i < numPeds3; i++){
		peds3[i]->m_leaveCarTimer = (*Timer_GetTimeInMilliseconds()) + time;
		peds3[i]->SetObjective(OBJECTIVE_LEAVE_CAR, this);
		time += General_GetRandomNumberInRangeFloat(300.0f, 600.0f);
	}

	if(IsCar() && numPeds2 > 0 && General_GetRandomTrueFalse())
		for(i = 0; i < numPeds2; i++)
			if(peds2[i]->IsFemale() || General_GetRandomTrueFalse()){
				peds2[i]->m_leaveCarTimer = (*Timer_GetTimeInMilliseconds()) + 10000;
				peds2[i]->bHeldHostageInCar = true;
				peds2[i]->bFleeAfterExitingCar = true;
			}
}

void
CVehicle::ProcessDelayedExplosion(void)
{
	if(m_nBombTimer == 0)
		return;

	int tick = (*Timer_GetTimeStep())/60.0f*1000.0f;
	int16 prev = m_nBombTimer;
	if(tick > m_nBombTimer)
		m_nBombTimer = 0;
	else
		m_nBombTimer -= tick;

	if(IsCar() && ((CAutomobile*)this)->m_bombType == CARBOMB_TIMEDACTIVE && (m_nBombTimer & 0xFE00) != (prev & 0xFE00))
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_BOMB_TICK, 0.0f);

	if (m_nBombTimer == 0)
		BlowUpCar(m_pBlowUpEntity);
}

bool
CVehicle::IsLawEnforcementVehicle(void)
{
	// Delegate to the C implementation
	return Vehicle_IsLawEnforcementModel(GetModelIndex());
}

bool
CVehicle::UsesSiren(void)
{
	// Delegate to the C implementation
	return Vehicle_UsesSirenModel(GetModelIndex());
}

bool
CVehicle::IsVehicleNormal(void)
{
	if (!pDriver || m_nNumPassengers != 0 || GetStatus() == STATUS_WRECKED)
		return false;
	return GetModelInfo()->m_vehicleClass != -1;
}

bool
CVehicle::CarHasRoof(void)
{
	// Delegate to the C implementation
	return VehicleLogic_CarHasRoof(this);
}

bool
CVehicle::IsUpsideDown(void)
{
	// Delegate to the C implementation
	return VehicleLogic_IsUpsideDown(this);
}

bool
CVehicle::IsOnItsSide(void)
{
	// Delegate to the C implementation
	return VehicleLogic_IsOnItsSide(this);
}

bool
CVehicle::CanBeDeleted(void)
{
	// Delegate to the C implementation
	return VehicleLogic_CanBeDeleted(this);
}

bool
CVehicle::CanPedOpenLocks(CPed *ped)
{
	// Delegate to the C implementation
	return VehicleLogic_CanPedOpenLocks(this, ped->IsPlayer());
}

bool
CVehicle::CanDoorsBeDamaged(void)
{
	// Delegate to the C implementation
	return VehicleLogic_CanDoorsBeDamaged(this);
}

bool
CVehicle::CanPedEnterCar(void)
{
	// Delegate to the C implementation
	return VehicleLogic_CanPedEnterCar(this, this, this);
}

bool
CVehicle::CanPedExitCar(bool jumpExit)
{
	// Delegate to the C implementation
	return VehicleLogic_CanPedExitCar(this, this, this, jumpExit);
}

bool
CVehicle::CanPedJumpOutCar(void)
{
	return true;	// rouz edit
	if(GetUp().z < 0.3f)
		return false;
	float speed = m_vecMoveSpeed.MagnitudeSqr();
	return speed < 0.1f || speed > 0.5f ? false : true;
}

bool
CVehicle::CanPedJumpOffBike(void)
{
	return true; // rouz edit
	if(pPassengers[0])
		return false;
	return m_vecMoveSpeed.MagnitudeSqr() < 0.07f ? false : true;
}

void
CVehicle::ChangeLawEnforcerState(uint8 enable)
{
	// Delegate to the C implementation, the flag is a bit field so it goes through a temporary
	bool isLawEnforcer = bIsLawEnforcer;
	Vehicle_ChangeLawEnforcerState(&isLawEnforcer, &CCarCtrl::NumLawEnforcerCars, enable != 0);
	bIsLawEnforcer = isLawEnforcer;
}

CPed*
CVehicle::SetUpDriver(void)
{
	if(pDriver)
		return pDriver;
	if(VehicleCreatedBy != RANDOM_VEHICLE)
		return nil;

	pDriver = CPopulation::AddPedInCar(this, true);
	pDriver->m_pMyVehicle = this;
	pDriver->m_pMyVehicle->RegisterReference((CEntity**)&pDriver->m_pMyVehicle);
	pDriver->bInVehicle = true;
	pDriver->SetPedState(PED_DRIVING);
	if(bIsBus)
		pDriver->bRenderPedInCar = false;
	return pDriver;
}

CPed*
CVehicle::SetupPassenger(int n)
{
	int i;

	if(pPassengers[n])
		return pPassengers[n];

	if((IsTaxi() || IsLimo()) && n == 0)
		pPassengers[0] = nil;
	else{
		CPed *passenger = CPopulation::AddPedInCar(this, false);
		pPassengers[n] = passenger;
		passenger->m_pMyVehicle = this;
		passenger->m_pMyVehicle->RegisterReference((CEntity**)&pPassengers[n]->m_pMyVehicle);
		passenger->bInVehicle = true;
		passenger->SetPedState(PED_DRIVING);

		if(passenger->m_nPedType == PEDTYPE_CIVMALE || passenger->m_nPedType == PEDTYPE_CIVFEMALE)
			for(i = 0; i < n; i++)
				if(pPassengers[i] && pPassengers[n] &&
				   (pPassengers[i]->m_nPedType == PEDTYPE_CIVMALE || pPassengers[i]->m_nPedType == PEDTYPE_CIVFEMALE) &&
				   passenger->GetModelIndex() == pPassengers[i]->GetModelIndex()){
					pPassengers[n] = nil;
					CPopulation::RemovePed(passenger);
				}
	}
	if(bIsBus && pPassengers[n])
		pPassengers[n]->bRenderPedInCar = false;
	++m_nNumPassengers;
	return pPassengers[n];
}

void
CVehicle::SetDriver(CPed *driver)
{
	pDriver = driver;
	pDriver->RegisterReference((CEntity**)&pDriver);

	if(bFreebies && driver == FindPlayerPed()){
		bFreebies = false;
		switch(GetModelIndex()){
		case MI_AMBULAN:
			// Delegate the health of the freebie to the C implementation
			FindPlayerPed()->m_fHealth = VehicleLogic_AmbulanceFreebieHealth(FindPlayerPed()->m_fHealth, CWorld::Players[0].m_nMaxHealth);
			break;

		case MI_TAXI:
		case MI_CABBIE:
		case MI_ZEBRA:
		case MI_KAUFMAN:
			CWorld::Players[CWorld::PlayerInFocus].m_nMoney += 12;
			break;

		case MI_POLICE:
			Streaming_RequestModel(MI_SHOTGUN, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
			bFreebies = true;
			break;

		case MI_ENFORCER:
			// Delegate the armour of the freebie to the C implementation
			driver->m_fArmour = VehicleLogic_EnforcerFreebieArmour(driver->m_fArmour, CWorld::Players[0].m_nMaxArmour);
			break;

		case MI_CADDY:
			if(!(driver->IsPlayer() && ((CPlayerPed*)driver)->DoesPlayerWantNewWeapon(WEAPONTYPE_GOLFCLUB, true)))
				Streaming_RequestModel(MI_GOLFCLUB, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
			break;
		}
	}

	if(IsBike())
		ApplyMoveForce(-0.02f*driver->m_fMass * GetUp());
	else
		ApplyTurnForce(0.0f, 0.0f, -0.02f*driver->m_fMass,
			driver->GetPosition().x - GetPosition().x,
			driver->GetPosition().y - GetPosition().y,
			0.0f);
}

bool
CVehicle::AddPassenger(CPed *passenger)
{
	if(IsBike())
		ApplyTurnForce(-0.02f*passenger->m_fMass * GetUp(), -0.1f*GetForward());
	else
		ApplyTurnForce(0.0f, 0.0f, -0.2f*passenger->m_fMass,
			passenger->GetPosition().x - GetPosition().x,
			passenger->GetPosition().y - GetPosition().y,
			0.0f);

	// Seat the passenger in C
	return VehicleLogic_AddPassengerToFirstFreeSeat(this, passenger);
}

bool
CVehicle::AddPassenger(CPed *passenger, uint8 n)
{
	if(bIsBus)
		return AddPassenger(passenger);

	if(IsBike())
		ApplyTurnForce(-0.02f*passenger->m_fMass * GetUp(), -0.1f*GetForward());
	else
		ApplyTurnForce(0.0f, 0.0f, -0.2f*passenger->m_fMass,
			passenger->GetPosition().x - GetPosition().x,
			passenger->GetPosition().y - GetPosition().y,
			0.0f);

	// Seat the passenger in C
	return VehicleLogic_AddPassengerToSeat(this, passenger, n);
}

void
CVehicle::RemoveDriver(void)
{
#ifdef FIX_BUGS
	if (GetStatus() != STATUS_WRECKED)
#endif
		SetStatus(STATUS_ABANDONED);
	if(pDriver == FindPlayerPed()){
		if(GetModelIndex() == MI_POLICE && Streaming_HasModelLoaded(MI_SHOTGUN)){ // rouz edit (ChatGPT)
			if(bFreebies){
				if(((CPlayerPed*)pDriver)->DoesPlayerWantNewWeapon(WEAPONTYPE_SHOTGUN, true))
					pDriver->GiveWeapon(WEAPONTYPE_SHOTGUN, 5, true);
				else
					pDriver->GrantAmmo(WEAPONTYPE_SHOTGUN, 5);
				bFreebies = false;
			}
			Streaming_SetModelIsDeletable(MI_SHOTGUN); // rouz edit (ChatGPT)
		}else if(GetModelIndex() == MI_CADDY && Streaming_HasModelLoaded(MI_GOLFCLUB)){ // rouz edit (ChatGPT)
			if(((CPlayerPed*)pDriver)->DoesPlayerWantNewWeapon(WEAPONTYPE_GOLFCLUB, true))
				pDriver->GiveWeapon(WEAPONTYPE_GOLFCLUB, 1, true);
			Streaming_SetModelIsDeletable(MI_GOLFCLUB); // rouz edit (ChatGPT)
		}
	}
	pDriver = nil;
}

void
CVehicle::RemovePassenger(CPed *p)
{
	// Delegate to the C implementation
	VehicleLogic_RemovePassenger(this, p, IsTrain());
}

bool
CVehicle::IsDriver(CPed *ped)
{
	// Delegate to the C implementation
	return VehicleLogic_IsDriver(this, ped);
}

bool
CVehicle::IsDriver(int32 model)
{
	return pDriver && pDriver->GetModelIndex() == model;
}

bool
CVehicle::IsPassenger(CPed *ped)
{
	// Delegate to the C implementation
	return VehicleLogic_IsPassenger(this, ped);
}

bool
CVehicle::IsPassenger(int32 model)
{
	int i;
	for(i = 0; i < 8; i++)
		if(pPassengers[i] && pPassengers[i]->GetModelIndex() == model)
			return true;
	return false;
}

void
CVehicle::UpdatePassengerList(void)
{
	// Delegate to the C implementation
	VehicleLogic_UpdatePassengerList(this);
}

void
CVehicle::ProcessCarAlarm(void)
{
	// Delegate to the C implementation
	VehicleLogic_ProcessCarAlarm(this, Timer_GetTimeStepInMilliseconds());
}

bool
CVehicle::IsSphereTouchingVehicle(float sx, float sy, float sz, float radius)
{
	float x, y, z;
	// sphere relative to vehicle
	CVector sph = CVector(sx, sy, sz) - GetPosition();
	CColModel *colmodel = GetColModel();

	x = DotProduct(sph, GetRight());
	if(colmodel->boundingBox.min.x - radius > x ||
	   colmodel->boundingBox.max.x + radius < x)
		return false;
	y = DotProduct(sph, GetForward());
	if(colmodel->boundingBox.min.y - radius > y ||
	   colmodel->boundingBox.max.y + radius < y)
		return false;
	z = DotProduct(sph, GetUp());
	if(colmodel->boundingBox.min.z - radius > z ||
	   colmodel->boundingBox.max.z + radius < z)
		return false;

	return true;
}

RpMaterial*
SetCompAlphaCB(RpMaterial *material, void *data)
{
	uint32 alpha = (uint32)(uintptr)data;
	RwRGBA *col = (RwRGBA*)RpMaterialGetColor(material);	// get rid of const
	col->alpha = alpha;
	return material;
}

void
CVehicle::SetComponentAtomicAlpha(RpAtomic *atomic, int32 alpha)
{
	RpGeometry *geo = RpAtomicGetGeometry(atomic);
	RpGeometrySetFlags(geo, RpGeometryGetFlags(geo) | rpGEOMETRYMODULATEMATERIALCOLOR);
	RpGeometryForAllMaterials(geo, SetCompAlphaCB, (void*)alpha);
}

void
CVehicle::UpdateClumpAlpha(void)
{
	int clumpAlpha = CVisibilityPlugins::GetClumpAlpha((RpClump*)m_rwObject);

	// The next alpha is worked out in C
	clumpAlpha = Vehicle_UpdateClumpAlpha(clumpAlpha, bFadeOut);
	CVisibilityPlugins::SetClumpAlpha((RpClump*)m_rwObject, clumpAlpha);
}

void
CVehicle::HeliDustGenerate(CEntity *heli, float radius, float ground, int rnd)
{
	int i;
	float angle;
	CColPoint point;
	CEntity *entity;
	uint8 r, g, b;

	if(heli == nil)
		return;

	uint8 surface = SURFACE_TARMAC;
	int frm = (*Timer_GetFrameCounter()) & 7;
	float testLowZ = ground - 10.0f;
	float dustSize = 0.0f;
	float baseSize = 1.0f;
	float offset = 1.0f;	// when heli is tilted
	float particleZ = -101.0f;
	int n = 0;

	if(heli->GetModelIndex() == MI_RCGOBLIN || heli->GetModelIndex() == MI_RCRAIDER){
		radius = 3.0f;
		dustSize = 0.04f;
		baseSize = 0.07f;
		offset = 0.3f;
	}

	CVector heliPos = heli->GetPosition();

	if(heli->IsVehicle() && ((CVehicle*)heli)->IsCar()){
		heliPos.x -= (heliPos.z - ground)*heli->GetUp().x*offset*0.5f;
		heliPos.y -= (heliPos.z - ground)*heli->GetUp().y*offset*0.5f;
	}

	float steamSize = 0.25f * radius * baseSize;
	float splashSize = 0.3f * radius * baseSize;

	i = 0;
	for(i = 0; i < 32+rnd; i++){
		angle = i * TWOPI/32.0f;
		CVector pos(radius*Cos(angle), radius*Sin(angle), 0.0f);
		CVector dir = CVector(pos.x, pos.y, 1.0f)*0.01f;
		pos += heliPos;

		if(i < 32 && i == 4*frm){
			if(CWorld::ProcessVerticalLine(pos, testLowZ, point, entity, true, false, false, false, true, false, nil)){
				n = rnd;
				particleZ = point.point.z;
				surface = point.surfaceB;
			}else
				n = 0;

			float waterLevel = 0.0f;
			if(CWaterLevel::GetWaterLevel(pos, &waterLevel, false) && waterLevel > particleZ){
				surface = SURFACE_WATER;
				n = rnd;
				particleZ = waterLevel;
			}
		}

		if(n){
			pos.z = particleZ;
			if(surface == SURFACE_WATER){
				// Delegate the colour of the spray to the C implementation
				r = VehicleLogic_HeliWaterChannel(TimeCycle_GetDirectionalRed(), TimeCycle_GetAmbientRed_Obj());
				g = VehicleLogic_HeliWaterChannel(TimeCycle_GetDirectionalGreen(), TimeCycle_GetAmbientGreen_Obj());
				b = VehicleLogic_HeliWaterChannel(TimeCycle_GetDirectionalBlue(), TimeCycle_GetAmbientBlue_Obj());
				RwRGBA col1 = { r, g, b, (RwUInt8)General_GetRandomNumberInRangeInt(8, 32) };
				RwRGBA col2 = { 255, 255, 255, 32 };

				if(n&1)
					CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, pos, dir, nil, steamSize, col2);
				else
					CParticle::AddParticle(PARTICLE_CAR_SPLASH, pos, dir, nil, splashSize, col1,
						General_GetRandomNumberInRangeFloat(0.0f, 10.0f),
						General_GetRandomNumberInRangeFloat(0.0f, 90.0f), 1);
			}else{
				// Delegate the colour of the dust to the C implementation
				VehicleLogic_HeliDustColor(surface, &r, &g, &b);
				RwRGBA col = { r, g, b, 32 };
				if(heliPos.z - pos.z < 20.0f)
					CParticle::AddParticle(PARTICLE_HELI_DUST, pos, dir, nil, dustSize, col);
			}

			n--;
		}
	}
}

#define GLARE_MIN_DIST (13.0f)
#define GLARE_FULL_DIST (30.0f)
#define GLARE_MIN_ANGLE (0.99f)
#define GLARE_FULL_ANGLE (0.995f)

void
CVehicle::DoSunGlare(void)
{
	if(bRenderScorched || GetPosition().z < 0.0f ||
	   GetVehicleAppearance() != VEHICLE_APPEARANCE_CAR || Weather_SunGlare <= 0.0f)
		return;

	CVector camDir = TheCamera.GetPosition() - GetPosition();
	float dist = camDir.Magnitude();
	camDir *= 2.0f/dist;
	CVector glareVec = camDir + TimeCycle_ToNativeVector(*TimeCycle_GetSunDirection());
	CVector localGlareVec;
	localGlareVec.x = DotProduct(glareVec, GetRight());
	localGlareVec.y = DotProduct(glareVec, GetForward());
	localGlareVec.z = 0.0;
	localGlareVec.Normalise();

	CVector2D fwd2D = GetForward();
	fwd2D.Normalise();
	CVector2D camDir2D = camDir;
	camDir2D.Normalise();
	float fwdness = Abs(DotProduct2D(fwd2D, camDir2D));

	// Delegate the strength of the glare to the C implementation
	float strength;
	if(!VehicleLogic_SunGlareStrength(fwdness, dist, &strength))
		return;

	float intens = 0.8f * strength * Weather_SunGlare;
	int r = VehicleLogic_SunGlareChannel(intens, TimeCycle_GetSunCoreRed());
	int g = VehicleLogic_SunGlareChannel(intens, TimeCycle_GetSunCoreGreen());
	int b = VehicleLogic_SunGlareChannel(intens, TimeCycle_GetSunCoreBlue());

	CColModel *colmodel = GetColModel();
	CCollision::CalculateTrianglePlanes(colmodel);

	int i;
	for(i = 0; i < colmodel->numTriangles-2; i += 2){
		int a1 = colmodel->triangles[i].a;
		int b1 = colmodel->triangles[i].b;
		int c1 = colmodel->triangles[i].c;
		int a2 = colmodel->triangles[i+1].a;
		int b2 = colmodel->triangles[i+1].b;
		int c2 = colmodel->triangles[i+1].c;
		// Copy complete decoded values before the original native vector expression
		const CollisionVector3 collisionVertexValue0_0=CompressedVector_Get(&colmodel->vertices[a1]);
		CVector vert1 = CVector(collisionVertexValue0_0.x,collisionVertexValue0_0.y,collisionVertexValue0_0.z);
		CVector vert4;
		// Need an upward surface
		if(vert1.z <= 0.0f)
			continue;

		// Delegate the search of the fourth vertex of the quad to the C implementation
		int fourthVertex = 0;
		int numTri2Verts = VehicleLogic_FindQuadVertex(a1, b1, c1, a2, b2, c2, &fourthVertex);
		if(numTri2Verts == 1){
			// Copy complete decoded values before the original native vector expression
			const CollisionVector3 collisionVertexValue1_0=CompressedVector_Get(&colmodel->vertices[fourthVertex]);
			vert4 = CVector(collisionVertexValue1_0.x,collisionVertexValue1_0.y,collisionVertexValue1_0.z);
		}
		// Need exactly one vertex from tri2 for a quad with tri1
		if(numTri2Verts != 1)
			continue;

		// Copy complete decoded values before the original native vector expression
		const CollisionVector3 collisionVertexValue4_0=CompressedVector_Get(&colmodel->vertices[b1]);
		const CollisionVector3 collisionVertexValue4_1=CompressedVector_Get(&colmodel->vertices[c1]);
		CVector mid = (vert1 + CVector(collisionVertexValue4_0.x,collisionVertexValue4_0.y,collisionVertexValue4_0.z) + CVector(collisionVertexValue4_1.x,collisionVertexValue4_1.y,collisionVertexValue4_1.z) + vert4)/4.0f;
		float dy = mid.y - vert1.y;
		float dx = mid.x - vert1.x;
		// Delegate the distance of the glare to the C implementation
		float dist = VehicleLogic_GlareDistance(dx, dy);
		if(dist > 0.6f){
			CVector pos = GetMatrix() * (dist * localGlareVec + mid) + camDir;
			CCoronas::RegisterCorona((uintptr)this + 27 + i,
				r, g, b, 255,
				pos, 0.9f*Weather_SunGlare, 90.0f,
				CCoronas::TYPE_STAR, CCoronas::FLARE_NONE,
				CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF,
				CCoronas::STREAK_OFF, 0.0f);
		}
	}
}

void
CVehicle::KillPedsInVehicle(void)
{
	int i;
	if(pDriver){
		Darkel_RegisterKillByPlayer(pDriver,WEAPONTYPE_EXPLOSION,false);
		if(pDriver->GetPedState() == PED_DRIVING){
			pDriver->SetDead();
			if(!pDriver->IsPlayer())
				pDriver->FlagToDestroyWhenNextProcessed();
		}else
			pDriver->SetDie();
	}
	for(i = 0; i < m_nNumMaxPassengers; i++){
		if(pPassengers[i]){
			Darkel_RegisterKillByPlayer(pPassengers[i],WEAPONTYPE_EXPLOSION,false);
			if(pPassengers[i]->GetPedState() == PED_DRIVING){
				pPassengers[i]->SetDead();
				if(!pPassengers[i]->IsPlayer())
					pPassengers[i]->FlagToDestroyWhenNextProcessed();
			}else
				pPassengers[i]->SetDie();
		}
	}
}

void
DestroyVehicleAndDriverAndPassengers(CVehicle* pVehicle)
{
	if (pVehicle->pDriver) {
		Darkel_RegisterKillByPlayer(pVehicle->pDriver,WEAPONTYPE_UNIDENTIFIED,false);
		pVehicle->pDriver->FlagToDestroyWhenNextProcessed();
	}
	for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
		if (pVehicle->pPassengers[i]) {
			Darkel_RegisterKillByPlayer(pVehicle->pPassengers[i],WEAPONTYPE_UNIDENTIFIED,false);
			pVehicle->pPassengers[i]->FlagToDestroyWhenNextProcessed();
		}
	}
	CWorld::Remove(pVehicle);
	// Destroy and release the vehicle without invoking C++ delete.
	pVehicle->~CVehicle();
		// Access raw storage through the C store or pool API
	CPool_Delete(CPools::GetVehiclePool(), pVehicle);
	}

#ifdef COMPATIBLE_SAVES
void
CVehicle::Save(uint8*& buf)
{
		// Transfer save data through the C buffer API with explicit sizes
	ZeroSaveBuf(&buf, 4);
	WriteSaveBuf(&buf, &(GetRight().x), sizeof(GetRight().x));
	WriteSaveBuf(&buf, &(GetRight().y), sizeof(GetRight().y));
	WriteSaveBuf(&buf, &(GetRight().z), sizeof(GetRight().z));
	ZeroSaveBuf(&buf, 4);
	WriteSaveBuf(&buf, &(GetForward().x), sizeof(GetForward().x));
	WriteSaveBuf(&buf, &(GetForward().y), sizeof(GetForward().y));
	WriteSaveBuf(&buf, &(GetForward().z), sizeof(GetForward().z));
	ZeroSaveBuf(&buf, 4);
	WriteSaveBuf(&buf, &(GetUp().x), sizeof(GetUp().x));
	WriteSaveBuf(&buf, &(GetUp().y), sizeof(GetUp().y));
	WriteSaveBuf(&buf, &(GetUp().z), sizeof(GetUp().z));
	ZeroSaveBuf(&buf, 4);
	WriteSaveBuf(&buf, &(GetPosition().x), sizeof(GetPosition().x));
	WriteSaveBuf(&buf, &(GetPosition().y), sizeof(GetPosition().y));
	WriteSaveBuf(&buf, &(GetPosition().z), sizeof(GetPosition().z));
	ZeroSaveBuf(&buf, 16);
		SaveEntityFlags(buf);
		// Transfer save data through the C buffer API with explicit sizes
	ZeroSaveBuf(&buf, 208);
		AutoPilot_Save(&AutoPilot,&(buf));
		// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &m_currentColour1, sizeof(m_currentColour1));
	WriteSaveBuf(&buf, &m_currentColour2, sizeof(m_currentColour2));
	ZeroSaveBuf(&buf, 2);
	WriteSaveBuf(&buf, &m_nAlarmState, sizeof(m_nAlarmState));
	ZeroSaveBuf(&buf, 42);
	WriteSaveBuf(&buf, &m_nNumMaxPassengers, sizeof(m_nNumMaxPassengers));
	ZeroSaveBuf(&buf, 3);
	WriteSaveBuf(&buf, &field_1D0[0], sizeof(field_1D0[0]));
	WriteSaveBuf(&buf, &field_1D0[1], sizeof(field_1D0[1]));
	WriteSaveBuf(&buf, &field_1D0[2], sizeof(field_1D0[2]));
	WriteSaveBuf(&buf, &field_1D0[3], sizeof(field_1D0[3]));
	ZeroSaveBuf(&buf, 8);
	WriteSaveBuf(&buf, &m_fSteerAngle, sizeof(m_fSteerAngle));
	WriteSaveBuf(&buf, &m_fGasPedal, sizeof(m_fGasPedal));
	WriteSaveBuf(&buf, &m_fBrakePedal, sizeof(m_fBrakePedal));
	WriteSaveBuf(&buf, &VehicleCreatedBy, sizeof(VehicleCreatedBy));
		uint8 flags = 0;
	if (bIsLawEnforcer) flags |= BIT(0);
	if (bIsLocked) flags |= BIT(3);
	if (bEngineOn) flags |= BIT(4);
	if (bIsHandbrakeOn) flags |= BIT(5);
	if (bLightsOn) flags |= BIT(6);
	if (bFreebies) flags |= BIT(7);
		// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &flags, sizeof(flags));
	ZeroSaveBuf(&buf, 10);
	WriteSaveBuf(&buf, &m_fHealth, sizeof(m_fHealth));
	WriteSaveBuf(&buf, &m_nCurrentGear, sizeof(m_nCurrentGear));
	ZeroSaveBuf(&buf, 3);
	WriteSaveBuf(&buf, &m_fChangeGearTime, sizeof(m_fChangeGearTime));
	ZeroSaveBuf(&buf, 12);
	WriteSaveBuf(&buf, &m_nTimeOfDeath, sizeof(m_nTimeOfDeath));
	ZeroSaveBuf(&buf, 2);
	WriteSaveBuf(&buf, &m_nBombTimer, sizeof(m_nBombTimer));
	ZeroSaveBuf(&buf, 12);
	WriteSaveBuf(&buf, &m_nDoorLock, sizeof(m_nDoorLock));
	ZeroSaveBuf(&buf, 108);
	}

void
CVehicle::Load(uint8*& buf)
{
	CMatrix tmp;
		// Transfer save data through the C buffer API with explicit sizes
	SkipSaveBuf(&buf, 4);
	ReadSaveBuf(&tmp.GetRight().x, &buf, sizeof(tmp.GetRight().x));
	ReadSaveBuf(&tmp.GetRight().y, &buf, sizeof(tmp.GetRight().y));
	ReadSaveBuf(&tmp.GetRight().z, &buf, sizeof(tmp.GetRight().z));
	SkipSaveBuf(&buf, 4);
	ReadSaveBuf(&tmp.GetForward().x, &buf, sizeof(tmp.GetForward().x));
	ReadSaveBuf(&tmp.GetForward().y, &buf, sizeof(tmp.GetForward().y));
	ReadSaveBuf(&tmp.GetForward().z, &buf, sizeof(tmp.GetForward().z));
	SkipSaveBuf(&buf, 4);
	ReadSaveBuf(&tmp.GetUp().x, &buf, sizeof(tmp.GetUp().x));
	ReadSaveBuf(&tmp.GetUp().y, &buf, sizeof(tmp.GetUp().y));
	ReadSaveBuf(&tmp.GetUp().z, &buf, sizeof(tmp.GetUp().z));
	SkipSaveBuf(&buf, 4);
	ReadSaveBuf(&tmp.GetPosition().x, &buf, sizeof(tmp.GetPosition().x));
	ReadSaveBuf(&tmp.GetPosition().y, &buf, sizeof(tmp.GetPosition().y));
	ReadSaveBuf(&tmp.GetPosition().z, &buf, sizeof(tmp.GetPosition().z));
		m_matrix = tmp;
		// Transfer save data through the C buffer API with explicit sizes
	SkipSaveBuf(&buf, 16);
		LoadEntityFlags(buf);
		// Transfer save data through the C buffer API with explicit sizes
	SkipSaveBuf(&buf, 208);
		AutoPilot_Load(&AutoPilot,&(buf));
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&m_currentColour1, &buf, sizeof(m_currentColour1));
	ReadSaveBuf(&m_currentColour2, &buf, sizeof(m_currentColour2));
	SkipSaveBuf(&buf, 2);
	ReadSaveBuf(&m_nAlarmState, &buf, sizeof(m_nAlarmState));
	SkipSaveBuf(&buf, 42);
	ReadSaveBuf(&m_nNumMaxPassengers, &buf, sizeof(m_nNumMaxPassengers));
	SkipSaveBuf(&buf, 3);
	ReadSaveBuf(&field_1D0[0], &buf, sizeof(field_1D0[0]));
	ReadSaveBuf(&field_1D0[1], &buf, sizeof(field_1D0[1]));
	ReadSaveBuf(&field_1D0[2], &buf, sizeof(field_1D0[2]));
	ReadSaveBuf(&field_1D0[3], &buf, sizeof(field_1D0[3]));
	SkipSaveBuf(&buf, 8);
	ReadSaveBuf(&m_fSteerAngle, &buf, sizeof(m_fSteerAngle));
	ReadSaveBuf(&m_fGasPedal, &buf, sizeof(m_fGasPedal));
	ReadSaveBuf(&m_fBrakePedal, &buf, sizeof(m_fBrakePedal));
	ReadSaveBuf(&VehicleCreatedBy, &buf, sizeof(VehicleCreatedBy));
		uint8 flags;
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&flags, &buf, sizeof(flags));
		bIsLawEnforcer = !!(flags & BIT(0));
	bIsLocked = !!(flags & BIT(3));
	bEngineOn = !!(flags & BIT(4));
	bIsHandbrakeOn = !!(flags & BIT(5));
	bLightsOn = !!(flags & BIT(6));
	bFreebies = !!(flags & BIT(7));
		// Transfer save data through the C buffer API with explicit sizes
	SkipSaveBuf(&buf, 10);
	ReadSaveBuf(&m_fHealth, &buf, sizeof(m_fHealth));
	ReadSaveBuf(&m_nCurrentGear, &buf, sizeof(m_nCurrentGear));
	SkipSaveBuf(&buf, 3);
	ReadSaveBuf(&m_fChangeGearTime, &buf, sizeof(m_fChangeGearTime));
	SkipSaveBuf(&buf, 12);
	ReadSaveBuf(&m_nTimeOfDeath, &buf, sizeof(m_nTimeOfDeath));
	SkipSaveBuf(&buf, 2);
	ReadSaveBuf(&m_nBombTimer, &buf, sizeof(m_nBombTimer));
	SkipSaveBuf(&buf, 12);
	ReadSaveBuf(&m_nDoorLock, &buf, sizeof(m_nDoorLock));
	SkipSaveBuf(&buf, 108);
	}
#endif

eVehicleAppearance
CVehicle::GetVehicleAppearance(void)
{
	// Delegate to the C implementation
	return (eVehicleAppearance)Vehicle_GetVehicleAppearance(pHandling->Flags);
}

bool
IsVehiclePointerValid(CVehicle* pVehicle)
{
	if (!pVehicle)
		return false;
		// Access raw storage through the C store or pool API
	int index = CPool_GetJustIndex_NoFreeAssert(CPools::GetVehiclePool(), pVehicle);
	#ifdef FIX_BUGS
	if (index < 0 || index >= NUMVEHICLES)
#else
	if (index < 0 || index > NUMVEHICLES)
#endif
		return false;
	return pVehicle->m_vehType == VEHICLE_TYPE_PLANE || pVehicle->m_entryInfoList.first;
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
