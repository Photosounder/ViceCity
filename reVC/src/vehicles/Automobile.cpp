//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"
#include "CollisionVector_native.h"
#include "ColLine_native.h"
#include "main.h"

#include "General.h"
#include "RwHelper.h"
#include "Pad.h"
#include "ModelIndices.h"
#include "VisibilityPlugins.h"
#include "DMAudio.h"
#include "Clock.h"
#include "Timecycle.h"
#include "ZoneCull.h"
#include "Camera.h"
#include "Darkel.h"
#include "Rubbish.h"
#include "Fire.h"
#include "Explosion.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "Glass.h"
#include "Antennas.h"
#include "Antennas_backend.h"
#include "Skidmarks.h"
#include "Skidmarks_backend.h"
#include "WindModifiers.h"
#include "WindModifiers_backend.h"
#include "Shadows.h"
#include "PointLights.h"
#include "Coronas.h"
#include "SpecialFX.h"
#include "WaterCannon.h"
#include "WaterLevel.h"
#include "Floater.h"
#include "WorldLogic.h"
#include "World.h"
#include "SurfaceTable.h"
#include "Weather.h"
#include "HandlingMgr.h"
#include "Record.h"
#include "Remote.h"
#include "Population.h"
#include "CarCtrl.h"
#include "CarAI.h"
#include "Stats.h"
#include "Garages.h"
#include "PathFind.h"
#include "Replay.h"
#include "AnimManager.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Object.h"
#include "Automobile.h"
#include "AutomobileLogic.h"
#include "PathLogic.h"
#include "Bike.h"
#include "Wanted.h"
#include "SaveBuf.h"
#include "Pools.h" // rouz edit (ChatGPT)

bool bAllCarCheat;

RwObject *GetCurrentAtomicObjectCB(RwObject *object, void *data);

bool CAutomobile::m_sAllTaxiLights;

const uint32 CAutomobile::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	1500;
#else
	sizeof(CAutomobile);
#endif

CAutomobile::CAutomobile(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
    // Initialize damage before the former door member constructors
    Damage_Init(&Damage);
    // Initialize every door at the former member-constructor boundary
    for(int doorIndex=0;doorIndex<VEHICLE_DOOR_COUNT;++doorIndex)
        Door_Init(&Doors[doorIndex]);
	int i;

	m_vehType = VEHICLE_TYPE_CAR;

	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(id)); // rouz edit (ChatGPT)
	m_fFireBlowUpTimer = 0.0f;
	m_doingBurnout = 0;
	bTaxiLight = m_sAllTaxiLights;
	bFixedColour = false;
	bBigWheels = false;
	bWaterTight = false;

	SetModelIndex(id);

	// Already done in CVehicle...
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
	}

	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)mi->m_handlingId);
	pFlyingHandling =  mod_HandlingManager.GetFlyingPointer((tVehicleType)mi->m_handlingId);

	m_auto_unused1 = 20.0f;
	m_auto_unused2 = 0;

	mi->ChooseVehicleColour(m_currentColour1, m_currentColour2);

	bIsVan = !!(pHandling->Flags & HANDLING_IS_VAN);
	bIsBig = !!(pHandling->Flags & HANDLING_IS_BIG);
	bIsBus = !!(pHandling->Flags & HANDLING_IS_BUS);
	bLowVehicle = !!(pHandling->Flags & HANDLING_IS_LOW);

	// Doors
	if(bIsBus){
		Door_SetLimits(&Doors[DOOR_FRONT_LEFT], -HALFPI, 0.0f, 0, 2);
		Door_SetLimits(&Doors[DOOR_FRONT_RIGHT], 0.0f, HALFPI, 1, 2);
	}else{
		Door_SetLimits(&Doors[DOOR_FRONT_LEFT], -PI*0.4f, 0.0f, 0, 2);
		Door_SetLimits(&Doors[DOOR_FRONT_RIGHT], 0.0f, PI*0.4f, 1, 2);
	}
	if(bIsVan){
		Door_SetLimits(&Doors[DOOR_REAR_LEFT], -HALFPI, 0.0f, 1, 2);
		Door_SetLimits(&Doors[DOOR_REAR_RIGHT], 0.0f, HALFPI, 0, 2);
	}else{
		Door_SetLimits(&Doors[DOOR_REAR_LEFT], -PI*0.4f, 0.0f, 0, 2);
		Door_SetLimits(&Doors[DOOR_REAR_RIGHT], 0.0f, PI*0.4f, 1, 2);
	}
	if(pHandling->Flags & HANDLING_REV_BONNET)
		Door_SetLimits(&Doors[DOOR_BONNET], -PI*0.3f, 0.0f, 1, 0);
	else
		Door_SetLimits(&Doors[DOOR_BONNET], 0.0f, PI*0.3f, 1, 0);
	if(pHandling->Flags & HANDLING_HANGING_BOOT)
		Door_SetLimits(&Doors[DOOR_BOOT], -PI*0.4f, 0.0f, 0, 0);
	else if(pHandling->Flags & HANDLING_TAILGATE_BOOT)
		Door_SetLimits(&Doors[DOOR_BOOT], 0.0, HALFPI, 1, 0);
	else
		Door_SetLimits(&Doors[DOOR_BOOT], -PI*0.3f, 0.0f, 1, 0);
	if(pHandling->Flags & HANDLING_NO_DOORS){
		Damage_SetDoorStatus(&Damage, DOOR_FRONT_LEFT, DOOR_STATUS_MISSING);
		Damage_SetDoorStatus(&Damage, DOOR_FRONT_RIGHT, DOOR_STATUS_MISSING);
		Damage_SetDoorStatus(&Damage, DOOR_REAR_LEFT, DOOR_STATUS_MISSING);
		Damage_SetDoorStatus(&Damage, DOOR_REAR_RIGHT, DOOR_STATUS_MISSING);
	}

	for(i = 0; i < 6; i++)
		m_randomValues[i] = General_GetRandomNumberInRangeFloat(-0.15f, 0.15f);

	m_fMass = pHandling->fMass;
	m_fTurnMass = pHandling->fTurnMass;
	m_vecCentreOfMass = pHandling->CentreOfMass;
	m_fAirResistance = pHandling->Dimension.x*pHandling->Dimension.z/m_fMass;
	m_fElasticity = 0.05f;
	m_fBuoyancy = pHandling->fBuoyancy;

	m_fOrientation = m_fPlaneSteer = 0.0f;

	m_nBusDoorTimerEnd = 0;
	m_nBusDoorTimerStart = 0;

	m_fSteerAngle = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;
	m_pSetOnFireEntity = nil;
	m_fGasPedalAudio = 0.0f;
	bNotDamagedUpsideDown = false;
	bMoreResistantToDamage = false;
	bTankDetonateCars = true;
	bStuckInSand = false;
	bHeliDestroyed = false;
	m_fVelocityChangeForAudio = 0.0f;
	m_hydraulicState = 0;

	for(i = 0; i < 4; i++){
		m_aGroundPhysical[i] = nil;
		m_aGroundOffset[i] = CVector(0.0f, 0.0f, 0.0f);
		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i] = 1.0f;
		m_aWheelTimer[i] = 0.0f;
		m_aWheelRotation[i] = 0.0f;
		m_aWheelSpeed[i] = 0.0f;
		m_aWheelState[i] = WHEEL_STATE_NORMAL;
		m_aWheelSkidmarkType[i] = SKIDMARK_NORMAL;
		m_aWheelSkidmarkBloody[i] = false;
	}

	m_nWheelsOnGround = 0;
	m_nDriveWheelsOnGround = 0;
	m_nDriveWheelsOnGroundPrev = 0;
	m_fHeightAboveRoad = 0.0f;
	m_fTraction = 1.0f;
	m_fTireTemperature = 1.0f;

	CColModel *colModel = mi->GetColModel();
	if(colModel->lines == nil){
		colModel->lines = (CColLine*)RwMalloc(4*sizeof(CColLine));
		colModel->numLines = 4;
	}

	SetupSuspensionLines();

	SetStatus(STATUS_SIMPLE);
	bUseCollisionRecords = true;

	m_nNumPassengers = 0;

	if(m_nDoorLock == CARLOCK_UNLOCKED &&
	   (id == MI_POLICE || id == MI_ENFORCER || id == MI_RHINO))
		m_nDoorLock = CARLOCK_LOCKED_INITIALLY;

	m_fCarGunLR = 0.0f;
	m_fCarGunUD = 0.05f;
	m_fPropellerRotation = 0.0f;
	m_fHeliOrientation = -1.0f;
	m_weaponDoorTimerLeft = 0.0f;
	m_weaponDoorTimerRight = m_weaponDoorTimerLeft;

	if(GetModelIndex() == MI_DODO){
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF])), 0);
		CMatrix mat1;
		mat1.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		CMatrix mat2(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
		mat1.GetPosition() += CVector(mat2.GetPosition().x + 0.1f, 0.0f, mat2.GetPosition().z);
		mat1.UpdateRW();
	}else if(GetModelIndex() == MI_HUNTER){
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB])), 0);
	}else if(IsRealHeli()){
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB])), 0);
	}else if(GetModelIndex() == MI_RHINO){
		bExplosionProof = true;
		bBulletProof = true;
	}
}

void
CAutomobile::SetModelIndex(uint32 id)
{
	CVehicle::SetModelIndex(id);
	SetupModelNodes();
}

#define SAND_SLOWDOWN (0.01f)
float CAR_BALANCE_MULT = 0.3f;
float HELI_ROTOR_DOTPROD_LIMIT = 0.95f;
CVector vecSeaSparrowGunPos(-0.5f, 2.4f, -0.785f);
CVector vecHunterGunPos(0.0f, 4.8f, -1.3f);
CVector vecHunterRocketPos(2.5f, 1.0f, -0.5f);
CVector vecDAMAGE_ENGINE_POS_SMALL(-0.1f, -0.1f, 0.0f);
CVector vecDAMAGE_ENGINE_POS_BIG(-0.5f, -0.3f, 0.0f);

#pragma optimize("", off) // a workaround for another compiler bug

void
CAutomobile::ProcessControl(void)
{
	int i;
	float wheelRot;
	CColModel *colModel;
	float brake = 0.0f;

	// rouz edit, fix wheel rotation bug
	for(i=0; i < 4; i++)
		m_aWheelRotation[i] = rangewrap(m_aWheelRotation[i], 0., TWOPI);

	if(bUsingSpecialColModel)
		colModel = &World_Players[World_PlayerInFocus].m_ColModel;
	else
		colModel = GetColModel();
	bool drivingInSand = false;
	bWarnedPeds = false;
	m_doingBurnout = 0;
	bStuckInSand = false;
	bRestingOnPhysical = false;

	bool carHasNitro = bAllTaxisHaveNitro && GetStatus() == STATUS_PLAYER && IsTaxi();

	if(Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;

	// Heli wind
	if(IsRealHeli())
		if((GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PHYSICS) && m_aWheelSpeed[1] > 0.075f ||
		   GetStatus() == STATUS_SIMPLE)
			WindModifiers_RegisterOne(WindModifiers_BackendVector(&GetPosition()), 1);

	UpdatePassengerList();

	// Improve grip of vehicles in certain cases
	// Delegate the improved grip to the C implementation
	bool strongGrip1, strongGrip2;
	bool playerDrivesOtherVehicle = FindPlayerVehicle() && this != FindPlayerVehicle();
	Automobile_StrongGrip(playerDrivesOtherVehicle, playerDrivesOtherVehicle ? Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) : 0, AutoPilot.m_nCarMission,
		FindPlayerVehicle() ? FindPlayerSpeed().Magnitude() : 0.0f, m_vecMoveSpeed.Magnitude(), playerDrivesOtherVehicle ? (GetPosition() - FindPlayerCoors()).Magnitude() : 0.0f,
		GetModelIndex() == MI_RCBANDIT, GetStatus() != STATUS_PLAYER_REMOTE, &strongGrip1, &strongGrip2);

	if(bIsBus)
		ProcessAutoBusDoors();

	ProcessCarAlarm();

	// Scan if this car sees the player committing any crimes
	if(GetStatus() != STATUS_ABANDONED && GetStatus() != STATUS_WRECKED &&
	   GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE && GetStatus() != STATUS_PLAYER_DISABLED){
		switch(GetModelIndex())
		case MI_FBIRANCH:
		case MI_POLICE:
		case MI_ENFORCER:
		case MI_SECURICA:
		case MI_RHINO:
		case MI_BARRACKS:
			ScanForCrimes();
	}

	// Process driver
	if(pDriver)
		if(IsUpsideDown() && CanPedEnterCar()){
			if(!pDriver->IsPlayer() &&
			   !(pDriver->m_leader && pDriver->m_leader->bInVehicle) &&
			   pDriver->CharCreatedBy != MISSION_CHAR)
				pDriver->SetObjective(OBJECTIVE_LEAVE_CAR, this);
		}

	ActivateBombWhenEntered();

	// Process passengers
	if(m_nNumPassengers != 0 && IsUpsideDown() && CanPedEnterCar()){
		for(i = 0; i < m_nNumMaxPassengers; i++)
			if(pPassengers[i])
				if(!pPassengers[i]->IsPlayer() &&
				   !(pPassengers[i]->m_leader && pPassengers[i]->m_leader->bInVehicle) &&
				   pPassengers[i]->CharCreatedBy != MISSION_CHAR)
					pPassengers[i]->SetObjective(OBJECTIVE_LEAVE_CAR, this);
	}

	// Enter the C rubbish algorithm with the original owner expressions
	Rubbish_StirUp(this);

	UpdateClumpAlpha();

	AutoPilot.m_bSlowedDownBecauseOfCars = false;
	AutoPilot.m_bSlowedDownBecauseOfPeds = false;

	// Set Center of Mass to make car more stable
	if(strongGrip1 || bCheat3)
		m_vecCentreOfMass.z = 0.3f*m_aSuspensionSpringLength[0] + -1.0f*m_fHeightAboveRoad;
	else if(pHandling->Flags & HANDLING_NONPLAYER_STABILISER && GetStatus() == STATUS_PHYSICS)
		m_vecCentreOfMass.z = pHandling->CentreOfMass.z - 0.2f*pHandling->Dimension.z;
	else
		m_vecCentreOfMass = pHandling->CentreOfMass;

	// Park car
	if(bCanPark && !bParking && VehicleCreatedBy != MISSION_VEHICLE && AutoPilot.m_nCarMission == MISSION_CRUISE &&
	   (((*Timer_GetFrameCounter()) + m_randomSeed)&0xF) == 0 && !IsTaxi()){
		CVector parkPosition = GetPosition() + 3.0f*GetRight() + 10.0f*GetForward();
		CEntity *ent = nil;
		CColPoint colpoint;
		if(!World_ProcessLineOfSight(GetPosition(), parkPosition, colpoint, ent, true, true, true, false, false, false) ||
		   ent == this)
			CarAI_GetCarToParkAtCoors(this, &parkPosition);
	}

	// Process depending on status

	bool playerRemote = false;
	switch(GetStatus()){
	case STATUS_PLAYER_REMOTE:
#ifdef FIX_BUGS
		if(CPad::GetPad(0)->CarGunJustDown() && !bDisableRemoteDetonation){
#else
		if(CPad::GetPad(0)->WeaponJustDown() && !bDisableRemoteDetonation){
#endif
			BlowUpCar(FindPlayerPed());
			Remote_TakeRemoteControlledCarFromPlayer(true); // rouz edit (ChatGPT)
		}

		if(GetModelIndex() == MI_RCBANDIT && !bDisableRemoteDetonationOnContact){
			//CVector pos = GetPosition();
			// FindPlayerCoors unused
			if(RcbanditCheckHitWheels() || bIsInWater){
				Remote_TakeRemoteControlledCarFromPlayer(true); // rouz edit (ChatGPT)
				BlowUpCar(FindPlayerPed());
			}
		}

		if(World_Players[World_PlayerInFocus].m_pRemoteVehicle == this)
			playerRemote = true;
		// fall through
	case STATUS_PLAYER:
		if(playerRemote ||
		   pDriver && pDriver->GetPedState() != PED_EXIT_CAR && pDriver->GetPedState() != PED_DRAG_FROM_CAR && pDriver->GetPedState() != PED_ARRESTED){
			// process control input if controlled by player
			if(playerRemote || pDriver->m_nPedType == PEDTYPE_PLAYER1)
				ProcessControlInputs(0);

			PruneReferences();

			if(GetStatus() == STATUS_PLAYER && !RecordChase_IsRecording()) // rouz edit (ChatGPT)
				DoDriveByShootings();

			// Tweak center on mass when driving on two wheels
			int twoWheelTime = World_Players[World_PlayerInFocus].m_nTimeNotFullyOnGround;
			// Delegate the height of the centre of mass to the C implementation
			float centreZ;
			if(Automobile_TwoWheelCentreOfMassZ(pHandling->CentreOfMass.z, twoWheelTime, CPad::GetPad(0)->GetSteeringLeftRight(), CAR_BALANCE_MULT, colModel->boundingBox.max.z,
				GetUp().z, GetRight().z, IsRealHeli() || IsRealPlane(), &centreZ))
				m_vecCentreOfMass.z = centreZ;

			if(bHoverCheat)
				DoHoverSuspensionRatios();

			if(m_aSuspensionSpringRatio[0] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[0].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
			   m_aSuspensionSpringRatio[1] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[1].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
			   m_aSuspensionSpringRatio[2] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[2].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
			   m_aSuspensionSpringRatio[3] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[3].surfaceB) == ADHESIVE_SAND){ // rouz edit (ChatGPT)
				if(GetModelIndex() != MI_RCBANDIT && GetModelIndex() != MI_RHINO){
					// Delegate the slowdown in the sand to the C implementation
					CVector parallelSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetUp())*GetUp();
					float parallelScale;
					bool stuckInSand;
					float slowdown = Automobile_SandSlowdown(parallelSpeed.MagnitudeSqr(), (pHandling->Flags & HANDLING_GOOD_INSAND) != 0, Weather_WetRoads, SAND_SLOWDOWN, &parallelScale, &stuckInSand);
					parallelSpeed *= parallelScale;
					if(stuckInSand)
						bStuckInSand = true;
					ApplyMoveForce(parallelSpeed * -(*Timer_GetTimeStep())*slowdown*m_fMass);
					drivingInSand = true;
				}
			}
		}else if(pDriver && pDriver->IsPlayer() &&
		         (pDriver->GetPedState() == PED_ARRESTED ||
		          pDriver->GetPedState() == PED_DRAG_FROM_CAR ||
		          (pDriver->GetPedState() == PED_EXIT_CAR || pDriver->m_objective == OBJECTIVE_LEAVE_CAR) && !CanPedJumpOutCar())){
			bIsHandbrakeOn = true;
			m_fBrakePedal = 1.0f;
			m_fGasPedal = 0.0f;
		}
		if(CPad::GetPad(0)->CarGunJustDown())
			ActivateBomb();
		break;

	case STATUS_SIMPLE:
		CarAI_UpdateCarAI(this);
		CPhysical::ProcessControl();
		CarCtrl_UpdateCarOnRails(this);

		m_nWheelsOnGround = 4;
		m_nDriveWheelsOnGroundPrev = m_nDriveWheelsOnGround;
		m_nDriveWheelsOnGround = 4;

		Transmission_CalculateGearForSimpleCar(&pHandling->Transmission, AutoPilot.m_fMaxTrafficSpeed/50.0f, &m_nCurrentGear);

		wheelRot = ProcessWheelRotation(WHEEL_STATE_NORMAL, GetForward(), m_vecMoveSpeed, 0.35f);
		for(i = 0; i < 4; i++)
			m_aWheelRotation[i] += wheelRot;

		PlayHornIfNecessary();
		ReduceHornCounter();
		bVehicleColProcessed = false;
		bAudioChangingGear = false;
		// that's all we do for simple vehicles
		return;

	case STATUS_PHYSICS:
		CarAI_UpdateCarAI(this);
		CarCtrl_SteerAICarWithPhysics(this);
		PlayHornIfNecessary();

		if(bIsBeingCarJacked){
			m_fGasPedal = 0.0f;
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}

		if(m_aSuspensionSpringRatio[0] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[0].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
		   m_aSuspensionSpringRatio[1] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[1].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
		   m_aSuspensionSpringRatio[2] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[2].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
		   m_aSuspensionSpringRatio[3] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[3].surfaceB) == ADHESIVE_SAND){ // rouz edit (ChatGPT)
			if(GetModelIndex() != MI_RCBANDIT && GetModelIndex() != MI_SANDKING && GetModelIndex() != MI_BFINJECT){
				bStuckInSand = true;
				if(Weather_WetRoads > 0.0f)
					ApplyMoveForce(m_vecMoveSpeed * -(*Timer_GetTimeStep())*SAND_SLOWDOWN*m_fMass * (1.0f-Weather_WetRoads));
				else
					ApplyMoveForce(m_vecMoveSpeed * -(*Timer_GetTimeStep())*SAND_SLOWDOWN*m_fMass);
			}
		}
		break;

	case STATUS_ABANDONED:
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f))
			m_fBrakePedal = 0.2f;
		else
			m_fBrakePedal = 0.0f;
		bIsHandbrakeOn = false;

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
		if(!IsAlarmOn())
			m_nCarHornTimer = 0;

		if(bIsBeingCarJacked){
			m_fGasPedal = 0.0f;
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}

		// rouz remote bomb
		if (rouz.remote_bomb && this == rouz.bomb_veh)
		{
			m_fGasPedal = 1.6f;
			m_fBrakePedal = 0.0f;

			// Set yield
			float yield = 0.f;
			if (Input_GetCharJustDown('B')) // rouz edit (ChatGPT)
				yield = 1.f;

			if (yield > 0.f)
			{
				DMAudio_PlayFrontEndSound(SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM, 1.f);
				BlowUpCar(nil);
				Explosion_AddExplosion(this, nil, EXPLOSION_HELI2, GetPosition(), 0);
				rouz.remote_bomb = 0;
				rouz.bomb_veh = NULL;
			}
		}

		// rouz remote control
		if (rouz.remote_control && this == rouz.rc_veh)
		{
			bIsHandbrakeOn = false;

			m_fGasPedal = 1.f * ((float) Input_GetChar('I') - (float) Input_GetChar('K')); // rouz edit (ChatGPT)
			m_fBrakePedal = 1.f * (float) Input_GetChar('K'); // rouz edit (ChatGPT)
			m_fSteerAngle = 30.f*PI/180.f * ((float) Input_GetChar('J') - (float) Input_GetChar('L')); // rouz edit (ChatGPT)

			if (DotProduct(m_vecMoveSpeed, GetForward()) <= 0.f)
				m_fBrakePedal = 0.f;

			static CVector player_offset;
			static int player_offset_set=0;
			/*if (Input_GetChar('P')) // rouz edit (ChatGPT)
			{
				if (player_offset_set==0)
				{
					player_offset_set = 1;
					player_offset = FindPlayerPed()->GetPosition() - GetPosition();
				}
				//m_vecMoveSpeed.z = 0.011f;
				FindPlayerPed()->SetPosition(player_offset + GetPosition());
				FindPlayerPed()->m_vecMoveSpeed = m_vecMoveSpeed;
				FindPlayerPed()->GetMatrix().Translate(FindPlayerPed()->m_vecMoveSpeed * (*Timer_GetTimeStep()));
				//FindPlayerPed()->bAffectedByGravity = 0;
				FindPlayerPed()->bIsFrozen = 1;
				FindPlayerPed()->bUsesCollision = 0;
			}
			else
			{
				player_offset_set = 0;
				FindPlayerPed()->bAffectedByGravity = 1;
				FindPlayerPed()->bIsFrozen = 0;
				FindPlayerPed()->bUsesCollision = 1;
			}*/
			//SetTurnSpeed(0.f, 0.0f, 0.01f);
		}
		break;

	case STATUS_WRECKED:
		m_fBrakePedal = 0.05f;
		bIsHandbrakeOn = true;

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
		if(!IsAlarmOn())
			m_nCarHornTimer = 0;
		break;

	case STATUS_PLAYER_DISABLED:
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f) ||
		   (pDriver && pDriver->IsPlayer() &&
		    (pDriver->GetPedState() == PED_ARRESTED ||
		     pDriver->GetPedState() == PED_DRAG_FROM_CAR ||
		     (pDriver->GetPedState() == PED_EXIT_CAR || pDriver->m_objective == OBJECTIVE_LEAVE_CAR) && !CanPedJumpOutCar()))){
			bIsHandbrakeOn = true;
			m_fBrakePedal = 1.0f;
			m_fGasPedal = 0.0f;
		}else{
			m_fBrakePedal = 0.0f;
			bIsHandbrakeOn = false;
		}

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
		if(!IsAlarmOn())
			m_nCarHornTimer = 0;
		break;
	default: break;
	}

	// rouz window control
	rouz_veh_apply_controls_from_window((void *) this);

	// Skip physics if object is found to have been static recently
	bool skipPhysics = false;
	if (!((rouz.remote_bomb && rouz.bomb_veh == this) || (rouz.remote_control && rouz.rc_veh == this)))
	if(!bIsStuck && (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED)){
		bool makeStatic = false;
		if(!bVehicleColProcessed &&
		   m_vecMoveSpeed.IsZero() &&
		// BUG? m_aSuspensionSpringRatioPrev[3] is checked twice in the game. also, why 3?
		   m_aSuspensionSpringRatioPrev[3] != 1.0f)
			makeStatic = true;

		m_vecMoveSpeedAvg = (m_vecMoveSpeedAvg + m_vecMoveSpeed)/2.0f;
		m_vecTurnSpeedAvg = (m_vecTurnSpeedAvg + m_vecTurnSpeed)/2.0f;

		// Delegate the test for the car that barely moves to the C implementation
		if(Automobile_IsBarelyMoving(GetStatus() == STATUS_WRECKED, m_vecMoveSpeedAvg.MagnitudeSqr(), m_vecTurnSpeedAvg.MagnitudeSqr(), (*Timer_GetTimeStep()), m_fDistanceTravelled,
			m_fDamageImpulse > 0.0f && m_pDamageEntity && m_pDamageEntity->IsPed(), makeStatic)){
			m_nStaticFrames++;

			if(m_nStaticFrames > 10 || makeStatic)
				if(!CarCtrl_MapCouldMoveInThisArea(GetPosition().x, GetPosition().y)){
					if(!makeStatic || m_nStaticFrames > 10)
						m_nStaticFrames = 10;

					skipPhysics = true;

					m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
					m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
				}
		}else
			m_nStaticFrames = 0;
		if(IsRealHeli() && m_aWheelSpeed[1] > 0.0f){
			skipPhysics = false;
			m_nStaticFrames = 0;
		}
	}

	// Postpone
	for(i = 0; i < 4; i++)
		if(m_aGroundPhysical[i]){
			bRestingOnPhysical = true;
			if(!World_bForceProcessControl && m_aGroundPhysical[i]->bIsInSafePosition){
				bWasPostponed = true;
				return;
			}
		}

	if(bRestingOnPhysical){
		skipPhysics = false;
		m_nStaticFrames = 0;
	}

	VehicleDamage(0.0f, 0);

	// special control
	switch(GetModelIndex()){
	case MI_FIRETRUCK:
		FireTruckControl();
		break;
	case MI_RHINO:
		TankControl();
		BlowUpCarsInPath();
		break;
	case MI_VOODOO:
		HydraulicControl();
		break;
	default:
		if(CVehicle::bCheat3 || carHasNitro){
			// Make vehicle jump when horn is sounded
			if(GetStatus() == STATUS_PLAYER && m_vecMoveSpeed.MagnitudeSqr() > sq(0.2f) &&
			// BUG: game checks [0] four times, instead of all wheels
			   m_aSuspensionSpringRatio[0] < 1.0f &&
			   CPad::GetPad(0)->HornJustDown()){

				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP, 1.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					CollisionVector_ToNative(m_aWheelColPoints[0].point) + 0.5f*GetUp(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					CollisionVector_ToNative(m_aWheelColPoints[0].point) + 0.5f*GetUp(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					CollisionVector_ToNative(m_aWheelColPoints[2].point) + 0.5f*GetUp(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					CollisionVector_ToNative(m_aWheelColPoints[2].point) + 0.5f*GetUp(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					CollisionVector_ToNative(m_aWheelColPoints[0].point) + 0.5f*GetUp() - GetForward(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					CollisionVector_ToNative(m_aWheelColPoints[0].point) + 0.5f*GetUp() - GetForward(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				CParticle::AddParticle(PARTICLE_ENGINE_STEAM,
					CollisionVector_ToNative(m_aWheelColPoints[2].point) + 0.5f*GetUp() - GetForward(),
					1.3f*m_vecMoveSpeed, nil, 2.5f);
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,
					CollisionVector_ToNative(m_aWheelColPoints[2].point) + 0.5f*GetUp() - GetForward(),
					1.2f*m_vecMoveSpeed, nil, 2.0f);

				ApplyMoveForce(CVector(0.0f, 0.0f, 1.0f)*m_fMass*0.4f);
				ApplyTurnForce(GetUp()*m_fTurnMass*0.01f, GetForward()*1.0f);
			}
		}
		break;
	}

	if(GetStatus() == STATUS_PHYSICS || GetStatus() == STATUS_SIMPLE)
		if(AutoPilot.m_nCarMission == MISSION_HELI_FLYTOCOORS ||
		   AutoPilot.m_nCarMission == MISSION_PLANE_FLYTOCOORS)
			skipPhysics = true;

	if(skipPhysics){
		bHasContacted = false;
		bIsInSafePosition = false;
		bWasPostponed = false;
		bHasHitWall = false;
		m_nCollisionRecords = 0;
		bHasCollided = false;
		bVehicleColProcessed = false;
		bAudioChangingGear = false;
		m_nDamagePieceType = 0;
		m_fDamageImpulse = 0.0f;
		m_pDamageEntity = nil;
		m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
		m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		m_fTireTemperature = 1.0f;
	}else{

		// This has to be done if ProcessEntityCollision wasn't called
		if(!bVehicleColProcessed){
			CMatrix mat(GetMatrix());
			bIsStuck = false;
			bHasContacted = false;
			bIsInSafePosition = false;
			bWasPostponed = false;
			bHasHitWall = false;
			m_fDistanceTravelled = 0.0f;
			m_bIsVehicleBeingShifted = false;
			bSkipLineCol = false;
			ApplyMoveSpeed();
			ApplyTurnSpeed();
			for(i = 0; CheckCollision() && i < 5; i++){
				GetMatrix() = mat;
				ApplyMoveSpeed();
				ApplyTurnSpeed();
			}
			bIsInSafePosition = true;
			bIsStuck = false;			
		}

		CPhysical::ProcessControl();

		ProcessBuoyancy();

		// Rescale spring ratios, i.e. subtract wheel radius
		for(i = 0; i < 4; i++){
			// Delegate the rescaling of the spring ratio to the C implementation, 0.0 is fully compressed and 1.0 is fully extended
			m_aSuspensionSpringRatio[i] = Automobile_PrevSpringRatio(m_aSuspensionSpringRatio[i], m_aSuspensionSpringLength[i], m_aSuspensionLineLength[i]);
		}

		float fwdSpeed = Abs(DotProduct(m_vecMoveSpeed, GetForward()));
		CVector contactPoints[4];	// relative to model
		CVector contactSpeeds[4];	// speed at contact points
		CVector springDirections[4];	// normalized, in world space

		for(i = 0; i < 4; i++){
			// Set spring under certain circumstances
			if(Damage_GetWheelStatus(&Damage, i) == WHEEL_STATUS_MISSING)
				m_aSuspensionSpringRatio[i] = 1.0f;
			else if(Damage_GetWheelStatus(&Damage, i) == WHEEL_STATUS_BURST){
				// wheel more bumpy the faster we are
				if(General_GetRandomNumberInRangeInt(0, (uint16)(40*fwdSpeed) + 98) < 100){
					m_aSuspensionSpringRatio[i] += 0.3f*(m_aSuspensionLineLength[i]-m_aSuspensionSpringLength[i])/m_aSuspensionSpringLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}
			}else if(SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[i].surfaceB) == ADHESIVE_SAND && // rouz edit (ChatGPT)
			         GetModelIndex() != MI_RHINO){
				fwdSpeed *= 0.7f;
				// Delegate the spring ratio in the sand to the C implementation
				m_aSuspensionSpringRatio[i] = Automobile_SandSpringRatio(m_aSuspensionSpringRatio[i], fwdSpeed, Weather_WetRoads, m_aSuspensionLineLength[i], m_aSuspensionSpringLength[i]);
			}

			// get points and directions if spring is compressed
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				contactPoints[i] = CollisionVector_ToNative(m_aWheelColPoints[i].point) - GetPosition();
				springDirections[i] = Multiply3x3(GetMatrix(), CollisionVector_ToNative(colModel->lines[i].p1) - CollisionVector_ToNative(colModel->lines[i].p0));
				springDirections[i].Normalise();
			}
		}

		// Make springs push up vehicle
		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				float bias = pHandling->fSuspensionBias;
				if(i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
					bias = 1.0f - bias;

				ApplySpringCollisionAlt(pHandling->fSuspensionForceLevel, springDirections[i], contactPoints[i], m_aSuspensionSpringRatio[i], bias, m_aWheelColPoints[i]);

				m_aWheelSkidmarkUnk[i] = false;
				if(m_aWheelColPoints[i].surfaceB == SURFACE_GRASS ||
				   m_aWheelColPoints[i].surfaceB == SURFACE_MUD_DRY)
					m_aWheelSkidmarkType[i] = SKIDMARK_MUDDY;
				else if(m_aWheelColPoints[i].surfaceB == SURFACE_SAND ||
				        m_aWheelColPoints[i].surfaceB == SURFACE_SAND_BEACH){
					m_aWheelSkidmarkType[i] = SKIDMARK_SANDY;
					m_aWheelSkidmarkUnk[i] = true;
				}else
					m_aWheelSkidmarkType[i] = SKIDMARK_NORMAL;
			}else{
				contactPoints[i] = Multiply3x3(GetMatrix(), CollisionVector_ToNative(colModel->lines[i].p1));
			}
		}

		// Get speed at contact points
		for(i = 0; i < 4; i++){
			contactSpeeds[i] = GetSpeed(contactPoints[i]);
			if(m_aGroundPhysical[i]){
				// subtract movement of physical we're standing on
				contactSpeeds[i] -= m_aGroundPhysical[i]->GetSpeed(m_aGroundOffset[i]);
#ifndef FIX_BUGS
				// this shouldn't be reset because we still need it below
				m_aGroundPhysical[i] = nil;
#endif
			}
			if(m_aSuspensionSpringRatio[i] < 1.0f && m_aWheelColPoints[i].normal.z > 0.35f)
				springDirections[i] = -CollisionVector_ToNative(m_aWheelColPoints[i].normal);
		}

		// dampen springs
		for(i = 0; i < 4; i++)
			if(m_aSuspensionSpringRatio[i] < 0.99999f)
				ApplySpringDampening(pHandling->fSuspensionDampingLevel,
					springDirections[i], contactPoints[i], contactSpeeds[i]);

		// Get speed at contact points again
		for(i = 0; i < 4; i++){
			contactSpeeds[i] = GetSpeed(contactPoints[i]);
			if(m_aGroundPhysical[i]){
				// subtract movement of physical we're standing on
				contactSpeeds[i] -= m_aGroundPhysical[i]->GetSpeed(m_aGroundOffset[i]);
				m_aGroundPhysical[i] = nil;
			}
		}

		bool gripCheat = true;
		fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward());
		if(!strongGrip1 && !CVehicle::bCheat3)
			gripCheat = false;
		float acceleration = Transmission_CalculateDriveAcceleration(&pHandling->Transmission, &m_fGasPedal, &m_nCurrentGear, &m_fChangeGearTime, &fwdSpeed, gripCheat);
		acceleration /= m_fForceMultiplier;

		if(IsRealHeli() || IsRealPlane())
			acceleration = 0.0f;

		if(bAudioChangingGear && m_fGasPedal > 0.4f && m_fBrakePedal < 0.1f && fwdSpeed > 0.15f &&
		   this == FindPlayerVehicle() && TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON){
			if(GetStatus() == STATUS_PLAYER && !(pHandling->Flags & HANDLING_IS_BUS)){
				if(m_nBusDoorTimerEnd == 0)
					m_nBusDoorTimerEnd = 1000;
				else {
					uint32 timeStepInMs = Timer_GetTimeStepInMilliseconds();
					if(m_nBusDoorTimerEnd > timeStepInMs)
						m_nBusDoorTimerEnd -= timeStepInMs;
					else
						m_nBusDoorTimerEnd = 0;
				}
			}

			// Delegate the anti roll force to the C implementation
			if(Automobile_WheelsOnBothSides(m_aSuspensionSpringRatio))
				ApplyTurnForce(Automobile_AntiRollStrength(GRAVITY, m_fTurnMass)*GetUp(), -1.0f*GetForward());
		}

		brake = m_fBrakePedal * pHandling->fBrakeDeceleration * (*Timer_GetTimeStep());
		// Delegate the biases of the wheels to the C implementation
		bool neutralHandling = GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE && (pHandling->Flags & HANDLING_NEUTRALHANDLING);
		float wheelBiases[4];
		Automobile_WheelBiases(neutralHandling, pHandling->fBrakeBias, pHandling->fTractionBias, wheelBiases);
		float brakeBiasFront = wheelBiases[0];
		float brakeBiasRear  = wheelBiases[1];	// looks like a bug, but it was correct in III...
		float tractionBiasFront = wheelBiases[2];
		float tractionBiasRear  = wheelBiases[3];

		// Count how many wheels are touching the ground

		m_nWheelsOnGround = 0;
		m_nDriveWheelsOnGroundPrev = m_nDriveWheelsOnGround;
		m_nDriveWheelsOnGround = 0;

		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f)
				m_aWheelTimer[i] = 4.0f;
			else
				// Delegate the countdown of the timer to the C implementation
				m_aWheelTimer[i] = VehicleLogic_DecayToZero(m_aWheelTimer[i], (*Timer_GetTimeStep()));

			if(m_aWheelTimer[i] > 0.0f){
				m_nWheelsOnGround++;
				switch(pHandling->Transmission.nDriveType){
				case '4':
					m_nDriveWheelsOnGround++;
					break;
				case 'F':
					if(i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
						m_nDriveWheelsOnGround++;
					break;
				case 'R':
					if(i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT)
						m_nDriveWheelsOnGround++;
					break;
				}
			}
		}

		float traction;
		if(GetStatus() == STATUS_PHYSICS)
			traction = 0.004f * m_fTraction;
		else
			traction = 0.004f;
		traction *= pHandling->fTractionMultiplier / 4.0f;
		traction /= m_fForceMultiplier;
		if(CVehicle::bCheat3)
			traction *= 4.0f;

		if(FindPlayerVehicle() != this && (strongGrip1 || CVehicle::bCheat3)){
			traction *= 1.2f;
			acceleration *= 1.4f;
			if(strongGrip2 || CVehicle::bCheat3){
				traction *= 1.3f;
				acceleration *= 1.4f;
			}
		}

		static float fThrust;
		static tWheelState WheelState[4];

		bool rearWheelsFirst = !!(pHandling->Flags & HANDLING_REARWHEEL_1ST);

		// Process front wheels on ground - first try

		if(!rearWheelsFirst){
		if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f || m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
			float s = Sin(m_fSteerAngle);
			float c = Cos(m_fSteerAngle);

			CVector wheelFwd, wheelRight, tmp;

			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier))
					fThrust = acceleration;
				else
					fThrust = 0.0f;

				// Delegate the directions of the steered wheel to the C implementation
				{
					CVector fwd0 = GetForward();
					CVector n0 = CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal);
					Automobile_SteeredWheelAxes(&fwd0, &n0, s, c, &wheelFwd, &wheelRight);
				}

				m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesion = Automobile_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[CARWHEEL_FRONT_LEFT])), traction, GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB) : 1.0f);
				WheelState[CARWHEEL_FRONT_LEFT] = m_aWheelState[CARWHEEL_FRONT_LEFT];

				if(Damage_GetWheelStatus(&Damage, CARWHEEL_FRONT_LEFT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_OK);
			}

			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier))
					fThrust = acceleration;
				else
					fThrust = 0.0f;

				// Delegate the directions of the steered wheel to the C implementation
				{
					CVector fwd0 = GetForward();
					CVector n0 = CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal);
					Automobile_SteeredWheelAxes(&fwd0, &n0, s, c, &wheelFwd, &wheelRight);
				}

				m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesion = Automobile_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT])), traction, GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB) : 1.0f);
				WheelState[CARWHEEL_FRONT_RIGHT] = m_aWheelState[CARWHEEL_FRONT_RIGHT];

				if(Damage_GetWheelStatus(&Damage, CARWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_OK);
			}
		}

		// Process front wheels off ground

		if(!IsRealHeli()){
			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] <= 0.0f){
				// Delegate the speed of the wheel to the C implementation
				m_aWheelSpeed[CARWHEEL_FRONT_LEFT] = Automobile_FreeWheelSpeed(m_aWheelSpeed[CARWHEEL_FRONT_LEFT], acceleration, mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier), false);
				m_aWheelRotation[CARWHEEL_FRONT_LEFT] += m_aWheelSpeed[CARWHEEL_FRONT_LEFT];
			}
			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] <= 0.0f){
				// Delegate the speed of the wheel to the C implementation
				m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] = Automobile_FreeWheelSpeed(m_aWheelSpeed[CARWHEEL_FRONT_RIGHT], acceleration, mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier), false);
				m_aWheelRotation[CARWHEEL_FRONT_RIGHT] += m_aWheelSpeed[CARWHEEL_FRONT_RIGHT];
			}
		}
		}

		// Process rear wheels

		if(m_aWheelTimer[CARWHEEL_REAR_LEFT] > 0.0f || m_aWheelTimer[CARWHEEL_REAR_RIGHT] > 0.0f){
			CVector wheelFwd = GetForward();
			CVector wheelRight = GetRight();	// overwritten for resp. wheel

			float rearBrake = brake;
			float rearTraction = traction;
			if(bIsHandbrakeOn){
#ifdef FIX_BUGS
				// Not sure if this is needed, but brake usually has timestep as a factor
				rearBrake = 20000.0f * Timer_GetTimeStepFix();
#else
				rearBrake = 20000.0f;
#endif
				if(fwdSpeed > 0.1f && pHandling->Flags & HANDLING_HANDBRAKE_TYRE){
					m_fTireTemperature += 0.005*(*Timer_GetTimeStep());
					if(m_fTireTemperature > 2.0f)
						m_fTireTemperature = 2.0f;
				}
			}else if(m_doingBurnout && mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier)){
				rearBrake = 0.0f;
				rearTraction = 0.0f;
				// BUG: missing timestep
				ApplyTurnForce(contactPoints[CARWHEEL_REAR_LEFT], -0.001f*m_fTurnMass*m_fSteerAngle*GetRight());
			}else if(m_fTireTemperature > 1.0f){
				rearTraction *= m_fTireTemperature;
			}

			if(m_aWheelTimer[CARWHEEL_REAR_LEFT] > 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier))
					fThrust = acceleration;
				else
					fThrust = 0.0f;

				// Delegate the directions of the wheel along the surface to the C implementation
				{
					CVector n0 = CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_LEFT].normal);
					CVector fwdOut;
					Automobile_SurfaceWheelAxes(&wheelFwd, &n0, &fwdOut, &wheelRight);
					wheelFwd = CVector(fwdOut.x, fwdOut.y, fwdOut.z);
				}

				m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesion = Automobile_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[CARWHEEL_REAR_LEFT])), rearTraction, GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB) : 1.0f);
				WheelState[CARWHEEL_REAR_LEFT] = m_aWheelState[CARWHEEL_REAR_LEFT];

				if(Damage_GetWheelStatus(&Damage, CARWHEEL_REAR_LEFT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_LEFT], contactPoints[CARWHEEL_REAR_LEFT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear*Damage.m_fWheelDamageEffect,
						CARWHEEL_REAR_LEFT,
						&m_aWheelSpeed[CARWHEEL_REAR_LEFT],
						&WheelState[CARWHEEL_REAR_LEFT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_LEFT], contactPoints[CARWHEEL_REAR_LEFT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear,
						CARWHEEL_REAR_LEFT,
						&m_aWheelSpeed[CARWHEEL_REAR_LEFT],
						&WheelState[CARWHEEL_REAR_LEFT],
						WHEEL_STATUS_OK);
			}

#ifdef FIX_BUGS
			// Shouldn't we reset these after the left wheel?
			wheelFwd = GetForward();
			wheelRight = GetRight();	// actually useless
#endif

			if(m_aWheelTimer[CARWHEEL_REAR_RIGHT] > 0.0f){
				if(mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier))
					fThrust = acceleration;
				else
					fThrust = 0.0f;

				// Delegate the directions of the wheel along the surface to the C implementation
				{
					CVector n0 = CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_RIGHT].normal);
					CVector fwdOut;
					Automobile_SurfaceWheelAxes(&wheelFwd, &n0, &fwdOut, &wheelRight);
					wheelFwd = CVector(fwdOut.x, fwdOut.y, fwdOut.z);
				}

				m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesion = Automobile_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[CARWHEEL_REAR_RIGHT])), rearTraction, GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB) : 1.0f);
				WheelState[CARWHEEL_REAR_RIGHT] = m_aWheelState[CARWHEEL_REAR_RIGHT];

				if(Damage_GetWheelStatus(&Damage, CARWHEEL_REAR_RIGHT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_RIGHT], contactPoints[CARWHEEL_REAR_RIGHT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear*Damage.m_fWheelDamageEffect,
						CARWHEEL_REAR_RIGHT,
						&m_aWheelSpeed[CARWHEEL_REAR_RIGHT],
						&WheelState[CARWHEEL_REAR_RIGHT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_REAR_RIGHT], contactPoints[CARWHEEL_REAR_RIGHT],
						m_nWheelsOnGround, fThrust,
						rearBrake*brakeBiasRear,
						adhesion*tractionBiasRear,
						CARWHEEL_REAR_RIGHT,
						&m_aWheelSpeed[CARWHEEL_REAR_RIGHT],
						&WheelState[CARWHEEL_REAR_RIGHT],
						WHEEL_STATUS_OK);
			}
		}

		// Delegate the temperature of the tyres to the C implementation
		m_fTireTemperature = Automobile_TireTemperature(m_fTireTemperature,
			m_doingBurnout && mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier) &&
			(m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING || m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING), (*Timer_GetTimeStep()));

		// Process rear wheels off ground

		if(!IsRealHeli()){
			if(m_aWheelTimer[CARWHEEL_REAR_LEFT] <= 0.0f){
				// Delegate the speed of the wheel to the C implementation
				m_aWheelSpeed[CARWHEEL_REAR_LEFT] = Automobile_FreeWheelSpeed(m_aWheelSpeed[CARWHEEL_REAR_LEFT], acceleration, mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier), bIsHandbrakeOn);
				m_aWheelRotation[CARWHEEL_REAR_LEFT] += m_aWheelSpeed[CARWHEEL_REAR_LEFT];
			}
			if(m_aWheelTimer[CARWHEEL_REAR_RIGHT] <= 0.0f){
				// Delegate the speed of the wheel to the C implementation
				m_aWheelSpeed[CARWHEEL_REAR_RIGHT] = Automobile_FreeWheelSpeed(m_aWheelSpeed[CARWHEEL_REAR_RIGHT], acceleration, mod_HandlingManager.HasRearWheelDrive(pHandling->nIdentifier), bIsHandbrakeOn);
				m_aWheelRotation[CARWHEEL_REAR_RIGHT] += m_aWheelSpeed[CARWHEEL_REAR_RIGHT];
			}
		}

		// Process front wheels on ground - second try

		if(rearWheelsFirst){
		if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f || m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
			float s = Sin(m_fSteerAngle);
			float c = Cos(m_fSteerAngle);

			CVector wheelFwd, wheelRight, tmp;

			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] > 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier))
					fThrust = acceleration;
				else
					fThrust = 0.0f;

				// Delegate the directions of the steered wheel to the C implementation
				{
					CVector fwd0 = GetForward();
					CVector n0 = CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].normal);
					Automobile_SteeredWheelAxes(&fwd0, &n0, s, c, &wheelFwd, &wheelRight);
				}

				m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesion = Automobile_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[CARWHEEL_FRONT_LEFT])), traction, GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB) : 1.0f);
				WheelState[CARWHEEL_FRONT_LEFT] = m_aWheelState[CARWHEEL_FRONT_LEFT];

				if(Damage_GetWheelStatus(&Damage, CARWHEEL_FRONT_LEFT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_LEFT], contactPoints[CARWHEEL_FRONT_LEFT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_LEFT,
						&m_aWheelSpeed[CARWHEEL_FRONT_LEFT],
						&WheelState[CARWHEEL_FRONT_LEFT],
						WHEEL_STATUS_OK);
			}

			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] > 0.0f){
				if(mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier))
					fThrust = acceleration;
				else
					fThrust = 0.0f;

				// Delegate the directions of the steered wheel to the C implementation
				{
					CVector fwd0 = GetForward();
					CVector n0 = CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].normal);
					Automobile_SteeredWheelAxes(&fwd0, &n0, s, c, &wheelFwd, &wheelRight);
				}

				m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesion = Automobile_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT])), traction, GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB) : 1.0f);
				WheelState[CARWHEEL_FRONT_RIGHT] = m_aWheelState[CARWHEEL_FRONT_RIGHT];

				if(Damage_GetWheelStatus(&Damage, CARWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront*Damage.m_fWheelDamageEffect,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_BURST);
				else
					ProcessWheel(wheelFwd, wheelRight,
						contactSpeeds[CARWHEEL_FRONT_RIGHT], contactPoints[CARWHEEL_FRONT_RIGHT],
						m_nWheelsOnGround, fThrust,
						brake*brakeBiasFront,
						adhesion*tractionBiasFront,
						CARWHEEL_FRONT_RIGHT,
						&m_aWheelSpeed[CARWHEEL_FRONT_RIGHT],
						&WheelState[CARWHEEL_FRONT_RIGHT],
						WHEEL_STATUS_OK);
			}
		}

		// Process front wheels off ground

		if (!IsRealHeli()) {
			if(m_aWheelTimer[CARWHEEL_FRONT_LEFT] <= 0.0f){
				// Delegate the speed of the wheel to the C implementation
				m_aWheelSpeed[CARWHEEL_FRONT_LEFT] = Automobile_FreeWheelSpeed(m_aWheelSpeed[CARWHEEL_FRONT_LEFT], acceleration, mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier), false);
				m_aWheelRotation[CARWHEEL_FRONT_LEFT] += m_aWheelSpeed[CARWHEEL_FRONT_LEFT];
			}
			if(m_aWheelTimer[CARWHEEL_FRONT_RIGHT] <= 0.0f){
				// Delegate the speed of the wheel to the C implementation
				m_aWheelSpeed[CARWHEEL_FRONT_RIGHT] = Automobile_FreeWheelSpeed(m_aWheelSpeed[CARWHEEL_FRONT_RIGHT], acceleration, mod_HandlingManager.HasFrontWheelDrive(pHandling->nIdentifier), false);
				m_aWheelRotation[CARWHEEL_FRONT_RIGHT] += m_aWheelSpeed[CARWHEEL_FRONT_RIGHT];
			}
		}
		}

		// Delegate the heights of the wheels to the C implementation
		for(i = 0; i < 4; i++)
			m_aWheelPosition[i] = Automobile_WheelPosition(colModel->lines[i].p0.z, m_aSuspensionSpringRatio[i], m_aSuspensionSpringLength[i], GetModelIndex() == MI_VOODOO && bUsingSpecialColModel, m_aWheelPosition[i]);
		for(i = 0; i < 4; i++)
			m_aWheelState[i] = WheelState[i];
		if(m_fGasPedal < 0.0f){
			if(m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SPINNING)
				m_aWheelState[CARWHEEL_REAR_LEFT] = WHEEL_STATE_NORMAL;
			if(m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SPINNING)
				m_aWheelState[CARWHEEL_REAR_RIGHT] = WHEEL_STATE_NORMAL;
		}

		// Process horn

		if(GetStatus() != STATUS_PLAYER){
			if(!IsAlarmOn())
				ReduceHornCounter();
		}else{
			if(UsesSiren()){
				if(Pads[0].bHornHistory[Pads[0].iCurrHornHistory]){
					if(Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+CPad::HORNHISTORY_SIZE-1) % CPad::HORNHISTORY_SIZE] &&
					   Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+CPad::HORNHISTORY_SIZE-2) % CPad::HORNHISTORY_SIZE])
						m_nCarHornTimer = 1;
					else
						m_nCarHornTimer = 0;
				}else if(Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+CPad::HORNHISTORY_SIZE-1) % CPad::HORNHISTORY_SIZE] &&
				         !Pads[0].bHornHistory[(Pads[0].iCurrHornHistory+1) % CPad::HORNHISTORY_SIZE]){
					m_nCarHornTimer = 0;
					m_bSirenOrAlarm = !m_bSirenOrAlarm;
				}else
					m_nCarHornTimer = 0;
			}else if(GetModelIndex() != MI_VOODOO && !CVehicle::bCheat3 && !carHasNitro){
				if(!IsAlarmOn()){
					if(Pads[0].GetHorn())
						m_nCarHornTimer = 1;
					else
						m_nCarHornTimer = 0;
				}
			}
		}

		// Flying

		bool playRotorSound = false;
		bool isPlane = GetModelIndex() == MI_DODO || bAllDodosCheat;
#ifdef FIX_BUGS
		isPlane = isPlane && !IsRealHeli();
#endif
		if(GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE && GetStatus() != STATUS_PHYSICS){
			if(IsRealHeli()){
				bEngineOn = false;
				// Delegate the slowing of the rotor to the C implementation
				m_aWheelSpeed[1] = VehicleLogic_DecayToZero(m_aWheelSpeed[1], 0.0005f);
				if(GetModelIndex() != MI_RCRAIDER && GetModelIndex() != MI_RCGOBLIN)
					if(m_aWheelSpeed[1] < 0.154f && m_aWheelSpeed[1] > 0.0044f)
						playRotorSound = true;
			}
		}else if(isPlane && m_vecMoveSpeed.Magnitude() > 0.0f && (*Timer_GetTimeStep()) > 0.0f){
			if(GetModelIndex() == MI_DODO)
				FlyingControl(FLIGHT_MODEL_DODO);
			else
				FlyingControl(FLIGHT_MODEL_PLANE);
		}else if(GetModelIndex() == MI_RCBARON){
			FlyingControl(FLIGHT_MODEL_RCPLANE);
		}else if(IsRealHeli() || bAllCarCheat){
#ifdef RESTORE_ALLCARSHELI_CHEAT
			if (bAllCarCheat)
				FlyingControl(FLIGHT_MODEL_HELI);
			else
#endif
			{
				// Speed up rotor
				if (m_aWheelSpeed[1] < 0.22f && !bIsInWater) {
					if (GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN)
						m_aWheelSpeed[1] += 0.003f;
					else
						m_aWheelSpeed[1] += 0.001f;
				}

				// Fly
				if (m_aWheelSpeed[1] > 0.15f) {
					if (GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN)
						FlyingControl(FLIGHT_MODEL_RCHELI);
					else if (m_nWheelsOnGround < 4 && !(GetModelIndex() == MI_SEASPAR && bTouchingWater) ||
						CPad::GetPad(0)->GetAccelerate() != 0 ||
#ifndef FREE_CAM
						CPad::GetPad(0)->GetCarGunUpDown() > 1.0f ||
#else
						((!CCamera::bFreeCam || (CCamera::bFreeCam && !Input_affectedByController)) && CPad::GetPad(0)->GetCarGunUpDown() > 1.0f) || // rouz edit (ChatGPT)
#endif
						VehicleLogic_AnySpeedAbove(m_vecMoveSpeed.x, m_vecMoveSpeed.y, m_vecMoveSpeed.z, 0.02f)) // Delegated to the C implementation
						FlyingControl(FLIGHT_MODEL_HELI);
				}
			}

			// Blade collision
			if(m_aWheelSpeed[1] > 0.015f && m_aCarNodes[CAR_BONNET]){
				CMatrix mat;
				mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BONNET]));
				if(GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN)
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 0.72f, 0.9f);
				else if(GetModelIndex() == MI_SPARROW || GetModelIndex() == MI_SEASPAR)
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 5.15f, 0.8f);
				else if(GetModelIndex() == MI_HUNTER)
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 6.15f, 0.5f);
				else
					DoBladeCollision(mat.GetPosition(), GetMatrix(), ROTOR_TOP, 6.15f, 1.0f);
			}

			// Heli weapons
			if(GetModelIndex() == MI_HUNTER && GetStatus() == STATUS_PLAYER){
				// Hunter rockets
				if(CPad::GetPad(0)->CarGunJustDown() && (*Timer_GetTimeInMilliseconds()) > m_nGunFiringTime+350){
					CWeapon gun(WEAPONTYPE_ROCKETLAUNCHER, 100);
					// Delegate the lead of the rockets to the C implementation
					CVector rocketLead;
					Automobile_RocketLead(DotProduct(m_vecMoveSpeed, GetForward()), &GetForward(), (*Timer_GetTimeStep()), &rocketLead);
					CVector source = vecHunterRocketPos;
					source = GetMatrix()*source + CVector(rocketLead.x, rocketLead.y, rocketLead.z);
					gun.FireProjectile(this, &source, 0.0f);

					source = vecHunterRocketPos;
					source.x = -source.x;
					source = GetMatrix()*source + CVector(rocketLead.x, rocketLead.y, rocketLead.z);
					gun.FireProjectile(this, &source, 0.0f);

					Stats_RoundsFiredByPlayer++;
					DMAudio_PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
					m_nGunFiringTime = (*Timer_GetTimeInMilliseconds());
				// Hunter gun
				}else if(CPad::GetPad(0)->GetHandBrake() && (*Timer_GetTimeInMilliseconds()) > m_nGunFiringTime+60){
					CWeapon gun(WEAPONTYPE_HELICANNON, 5000);
					CVector source = vecHunterGunPos;
					source = GetMatrix()*source + m_vecMoveSpeed*(*Timer_GetTimeStep());
					gun.FireInstantHit(this, &source);
					gun.AddGunshell(this, source, CVector2D(0.0f, 0.1f), 0.025f);
					Stats_RoundsFiredByPlayer++;
					DMAudio_PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
					m_nGunFiringTime = (*Timer_GetTimeInMilliseconds());
				}
			}else if(GetModelIndex() == MI_SEASPAR && GetStatus() == STATUS_PLAYER){
				// Sea sparrow gun
				if(CPad::GetPad(0)->GetHandBrake() && (*Timer_GetTimeInMilliseconds()) > m_nGunFiringTime+40){
					CWeapon gun(WEAPONTYPE_M4, 5000);
					CVector source = vecSeaSparrowGunPos;
					source = GetMatrix()*source + m_vecMoveSpeed*(*Timer_GetTimeStep());
					gun.FireInstantHit(this, &source);
					gun.AddGunshell(this, source, CVector2D(0.0f, 0.1f), 0.025f);
					Stats_RoundsFiredByPlayer++;
					DMAudio_PlayOneShot(m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);
					m_nGunFiringTime = (*Timer_GetTimeInMilliseconds());
				}
			}

			if(GetModelIndex() != MI_RCRAIDER && GetModelIndex() != MI_RCGOBLIN)
				if(m_aWheelSpeed[1] < 0.154f && m_aWheelSpeed[1] > 0.0044f)
					playRotorSound = true;
		}

		// Play rotor sound
		if(playRotorSound && m_aCarNodes[CAR_BONNET]){
			CVector camDist = TheCamera.GetPosition() - GetPosition();
			float distSq = camDist.MagnitudeSqr();
			// Delegate the rotor sound check to the C implementation
			if(Automobile_RotorSoundCheck(distSq, m_fPropellerRotation, m_aWheelRotation[1])){
				CMatrix mat;
				mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BONNET]));
				CVector blade = mat.GetRight();
				blade = Multiply3x3(GetMatrix(), blade);
				// Delegate the distance to the C implementation
				camDist /= Automobile_SafeDistance(distSq);
				if(Automobile_RotorBladeInLine(DotProduct(camDist, blade), HELI_ROTOR_DOTPROD_LIMIT)){
					DMAudio_PlayOneShot(m_audioEntityId, SOUND_HELI_BLADE, 0.0f);
					m_fPropellerRotation = m_aWheelRotation[1];
				}
			}
		}
	}



	// Process car on fire
	// A similar calculation of damagePos is done elsewhere for smoke

	uint8 engineStatus = Damage_GetEngineStatus(&Damage);
	CVector damagePos = ((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())))->m_positions[CAR_POS_HEADLIGHTS]; // rouz edit (ChatGPT)

	switch(Damage_GetDoorStatus(&Damage, DOOR_BONNET)){
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		// Bonnet is still there, smoke comes out at the edge
		damagePos += vecDAMAGE_ENGINE_POS_SMALL;
		break;
	case DOOR_STATUS_SWINGING:
	case DOOR_STATUS_MISSING:
		// Bonnet is gone, smoke comes out at the engine
		damagePos += vecDAMAGE_ENGINE_POS_BIG;
		break;
	}

	// move fire forward if in first person
	if(this == FindPlayerVehicle() && TheCamera.GetLookingForwardFirstPerson())
		if(m_fHealth < 250.0f && GetStatus() != STATUS_WRECKED){
			if(GetModelIndex() == MI_FIRETRUCK)
				damagePos += CVector(0.0f, 3.0f, -0.2f);
			else
				damagePos += CVector(0.0f, 1.2f, -0.8f);
		}

	damagePos = GetMatrix()*damagePos;
	damagePos.z += 0.15f;

	if(m_fHealth < 250.0f && GetStatus() != STATUS_WRECKED){
		// Car is on fire

		CParticle::AddParticle(PARTICLE_CARFLAME, damagePos,
			CVector(0.0f, 0.0f, General_GetRandomNumberInRangeFloat(0.01125f, 0.09f)),
			nil, 0.63f);

		CVector coors = damagePos;
		coors.x += General_GetRandomNumberInRangeFloat(-0.5625f, 0.5625f),
		coors.y += General_GetRandomNumberInRangeFloat(-0.5625f, 0.5625f),
		coors.z += General_GetRandomNumberInRangeFloat(0.5625f, 2.25f);
		CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE, coors, CVector(0.0f, 0.0f, 0.0f));

		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, CVector(0.0f, 0.0f, 0.0f), nil, 0.5f);

		// Blow up car after 5 seconds
		m_fFireBlowUpTimer += Timer_GetTimeStepInMilliseconds();
		if(m_fFireBlowUpTimer > 5000.0f)
			BlowUpCar(m_pSetOnFireEntity);
	}else
		m_fFireBlowUpTimer = 0.0f;

	// Decrease car health if engine is damaged badly
	if(engineStatus > ENGINE_STATUS_ON_FIRE && m_fHealth > 250.0f)
		m_fHealth -= 2.0f;

	ProcessDelayedExplosion();


	if(m_bSirenOrAlarm && ((*Timer_GetFrameCounter())&7) == 5 &&
	   UsesSiren() && GetModelIndex() != MI_MRWHOOP)
		CarAI_MakeWayForCarWithSiren(this);


	// Find out how much to shake the pad depending on suspension and ground surface

	float suspShake = 0.0f;
	float surfShake = 0.0f;
	float speedsq = m_vecMoveSpeed.MagnitudeSqr();
	for(i = 0; i < 4; i++){
		float suspChange = m_aSuspensionSpringRatioPrev[i] - m_aSuspensionSpringRatio[i];
		if(suspChange > 0.3f && !drivingInSand && speedsq > SQR(0.2f)){
			if(Damage_GetWheelStatus(&Damage, i) == WHEEL_STATUS_BURST)
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP_2, suspChange);
			else
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP, suspChange);
			if(suspChange > suspShake)
				suspShake = suspChange;
		}

		// Delegate the shake from the surface to the C implementation
		surfShake = Automobile_SurfaceShake(m_aWheelColPoints[i].surfaceB, surfShake);

		if(this == FindPlayerVehicle())
// BUG: this only observes one of the wheels
			TheCamera.m_bVehicleSuspenHigh = General_AbsAbove(suspChange, 0.05f);

		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i];
		m_aSuspensionSpringRatio[i] = 1.0f;
	}

	// Shake pad

	if(!drivingInSand && (suspShake > 0.0f || surfShake > 0.0f) && GetStatus() == STATUS_PLAYER){
		float speed = m_vecMoveSpeed.MagnitudeSqr();
		if(speed > sq(0.1f)){
			speed = Sqrt(speed);
			if(suspShake > 0.0f){
				uint8 freq = Automobile_PadShakeFrequency(suspShake, speed, m_fMass, 100.0f, 250.0f);
				CPad::GetPad(0)->StartShake(20000.0f*(*Timer_GetTimeStep())/freq, freq);
			}else{
				uint8 freq = Automobile_PadShakeFrequency(surfShake, speed, m_fMass, 40.0f, 150.0f);
				CPad::GetPad(0)->StartShake(5000.0f*(*Timer_GetTimeStep())/freq, freq);
			}
		}
	}

	bVehicleColProcessed = false;
	bAudioChangingGear = false;

	if(!bWarnedPeds && GetVehicleAppearance() != VEHICLE_APPEARANCE_HELI && GetVehicleAppearance() != VEHICLE_APPEARANCE_PLANE)
		CarCtrl_ScanForPedDanger(this);


	// Turn around at the edge of the world
	// TODO: make the numbers defines

	// rouz edit, world edge bounce disabled
	/*float heading;
	if(GetPosition().x > 1950.0f-400.0f){
		if(m_vecMoveSpeed.x > 0.0f)
			m_vecMoveSpeed.x *= -1.0f;
		heading = GetForward().Heading();
		if(heading > 0.0f)	// going west
			SetHeading(-heading);
	}else if(GetPosition().x < -1950.0f-400.0f){
		if(m_vecMoveSpeed.x < 0.0f)
			m_vecMoveSpeed.x *= -1.0f;
		heading = GetForward().Heading();
		if(heading < 0.0f)	// going east
			SetHeading(-heading);
	}
	if(GetPosition().y > 1950.0f){
		if(m_vecMoveSpeed.y > 0.0f)
			m_vecMoveSpeed.y *= -1.0f;
		heading = GetForward().Heading();
		if(heading < HALFPI && heading > 0.0f)
			SetHeading(PI-heading);
		else if(heading > -HALFPI && heading < 0.0f)
			SetHeading(-PI-heading);
	}else if(GetPosition().y < -1950.0f){
		if(m_vecMoveSpeed.y < 0.0f)
			m_vecMoveSpeed.y *= -1.0f;
		heading = GetForward().Heading();
		if(heading > HALFPI)
			SetHeading(PI-heading);
		else if(heading < -HALFPI)
			SetHeading(-PI-heading);
	}*/

	if(bInfiniteMass){
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
	}else if(!skipPhysics &&
	         (m_fGasPedal == 0.0f && brake == 0.0f || GetStatus() == STATUS_WRECKED)){
		// Delegate the check that the vehicle is nearly still to the C implementation
		if(VehicleLogic_AllSpeedsBelow(m_vecMoveSpeed.x, m_vecMoveSpeed.y, m_vecMoveSpeed.z, 0.005f) &&
		   !(m_fDamageImpulse > 0.0f && m_pDamageEntity == FindPlayerPed()) &&
		   (m_aSuspensionSpringRatioPrev[0] < 1.0f || m_aSuspensionSpringRatioPrev[1] < 1.0f ||
		    m_aSuspensionSpringRatioPrev[2] < 1.0f || m_aSuspensionSpringRatioPrev[3] < 1.0f)){
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			m_vecTurnSpeed.z = 0.0f;
		}
	}

	if(IsRealHeli() && bHeliDestroyed && !bRenderScorched){
		ApplyMoveForce(0.0f, 0.0f, -2.0f*(*Timer_GetTimeStep()));
		m_vecTurnSpeed.z += -0.002f*(*Timer_GetTimeStep());
		m_vecTurnSpeed.x += -0.0002f*(*Timer_GetTimeStep());

		RwRGBA col = { 84, 84, 84, 255 };
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, GetMatrix()*CVector(0.0f, 0.0f, -10.0f),
			CVector(0.0f, 0.0f, 0.0f), nil, 0.7f, col, 0, 0, 0, 3000);

		if(World_TestSphereAgainstWorld(GetPosition(), 10.0f, this, true, false, false, false, false, false) ||
		   GetPosition().z < 6.0f)
			if(!bRenderScorched){	// we already know this is true...
				Explosion_AddExplosion(this, nil, EXPLOSION_CAR, GetPosition(), 0);
				bRenderScorched = true;
			}
	}
}

#pragma optimize("", on)

void
CAutomobile::Teleport(CVector pos)
{
	World_Remove(this);

	SetPosition(pos);
	SetOrientation(0.0f, 0.0f, 0.0f);
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);

	ResetSuspension();

	World_Add(this);
}

void
CAutomobile::PreRender(void)
{
	int i, j, n;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)

	if(GetModelIndex() == MI_RHINO && m_aCarNodes[CAR_WINDSCREEN]){
		// Rotate Rhino turret
		CMatrix m;
		CVector p;
		m.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WINDSCREEN]));
		p = m.GetPosition();
		m.SetRotateZ(m_fCarGunLR);
		m.Translate(p);
		m.UpdateRW();
	}

	if(GetModelIndex() == MI_RCBANDIT){
		CVector pos = GetMatrix() * CVector(0.218f, -0.444f, 0.391f);
		// Register the C antenna with typed vector copies
		Antennas_RegisterOne((uintptr)this, Antennas_BackendVector(&GetUp()), Antennas_BackendVector(&pos), 1.0f);
	}

	float fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward())*180.0f;


	// Wheel particles

	if(GetModelIndex() == MI_DODO || GetVehicleAppearance() != VEHICLE_APPEARANCE_CAR){
		; // nothing
	}else if(GetModelIndex() == MI_RCBANDIT){
		for(i = 0; i < 4; i++){
			// Game has same code three times here
			switch(m_aWheelState[i]){
			case WHEEL_STATE_SPINNING:
			case WHEEL_STATE_SKIDDING:
			case WHEEL_STATE_FIXED:
				CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
					CollisionVector_ToNative(m_aWheelColPoints[i].point) + CVector(0.0f, 0.0f, 0.05f),
					CVector(0.0f, 0.0f, 0.0f), nil, 0.1f);
				break;
			default: break;
			}
		}
	}else{
		if(GetStatus() == STATUS_SIMPLE){
			CMatrix mat;
			CVector pos;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RB]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_REAR_RIGHT];
			m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point = CollisionVector_FromNative(GetMatrix() * pos);
			m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB = SURFACE_DEFAULT;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LB]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_REAR_LEFT];
			m_aWheelColPoints[CARWHEEL_REAR_LEFT].point = CollisionVector_FromNative(GetMatrix() * pos);
			m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB = SURFACE_DEFAULT;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
			m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].point = CollisionVector_FromNative(GetMatrix() * pos);
			m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB = SURFACE_DEFAULT;

			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
			pos = mat.GetPosition();
			pos.z = 1.5f*m_aWheelPosition[CARWHEEL_FRONT_LEFT];
			m_aWheelColPoints[CARWHEEL_FRONT_LEFT].point = CollisionVector_FromNative(GetMatrix() * pos);
			m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB = SURFACE_DEFAULT;
		}

		int drawParticles = Automobile_SpeedBelow(fwdSpeed, 90.0f);
		if(GetStatus() == STATUS_SIMPLE || GetStatus() == STATUS_PHYSICS ||
		   GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PLAYER_PLAYBACKFROMBUFFER){
			bool rearSkidding = false;
			if(m_aWheelState[CARWHEEL_REAR_LEFT] == WHEEL_STATE_SKIDDING ||
			   m_aWheelState[CARWHEEL_REAR_RIGHT] == WHEEL_STATE_SKIDDING)
				rearSkidding = true;

			for(i = 0; i < 4; i++){
				if(m_aSuspensionSpringRatioPrev[i] < 1.0f && m_aWheelColPoints[i].surfaceB != SURFACE_WATER)
				switch(m_aWheelState[i]){
				case WHEEL_STATE_SPINNING:
					if(AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles)){
						CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
							CollisionVector_ToNative(m_aWheelColPoints[i].point) + CVector(0.0f, 0.0f, 0.25f),
							CVector(0.0f, 0.0f, 0.0f));

						CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
							CollisionVector_ToNative(m_aWheelColPoints[i].point) + CVector(0.0f, 0.0f, 0.25f),
							CVector(0.0f, 0.0f, 0.05f));
					}

					CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
						CollisionVector_ToNative(m_aWheelColPoints[i].point) + CVector(0.0f, 0.0f, 0.25f),
						CVector(0.0f, 0.0f, 0.0f));

					if(m_aWheelTimer[i] > 0.0f)
						// Enter the C skidmark algorithm with the original owner expressions
						Skidmarks_RegisterOneType((uintptr)this + i, Skidmarks_BackendPosition(CollisionVector_ToNative(m_aWheelColPoints[i].point)),
							GetForward().x, GetForward().y,
							m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
					break;

				case WHEEL_STATE_SKIDDING:
					if(i == CARWHEEL_REAR_LEFT || i == CARWHEEL_REAR_RIGHT || rearSkidding){
						// same as below

						if(Automobile_SpeedAbove(fwdSpeed, 5.0f)){
							AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles);

							CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
								CollisionVector_ToNative(m_aWheelColPoints[i].point) + CVector(0.0f, 0.0f, 0.25f),
								CVector(0.0f, 0.0f, 0.0f));
						}

						if(m_aWheelTimer[i] > 0.0f)
							// Enter the C skidmark algorithm with the original owner expressions
							Skidmarks_RegisterOneType((uintptr)this + i, Skidmarks_BackendPosition(CollisionVector_ToNative(m_aWheelColPoints[i].point)),
								GetForward().x, GetForward().y,
								m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
					}
					break;

				case WHEEL_STATE_FIXED:
					if(Automobile_SpeedAbove(fwdSpeed, 5.0f)){
						AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles);

						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
							CollisionVector_ToNative(m_aWheelColPoints[i].point) + CVector(0.0f, 0.0f, 0.25f),
							CVector(0.0f, 0.0f, 0.0f));
					}

					if(m_aWheelTimer[i] > 0.0f)
						// Enter the C skidmark algorithm with the original owner expressions
						Skidmarks_RegisterOneType((uintptr)this + i, Skidmarks_BackendPosition(CollisionVector_ToNative(m_aWheelColPoints[i].point)),
							GetForward().x, GetForward().y,
							m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
					break;

				default:
					if(Automobile_SpeedAbove(fwdSpeed, 5.0f))
						AddWheelDirtAndWater(&m_aWheelColPoints[i], drawParticles);
					if((m_aWheelSkidmarkBloody[i] || m_aWheelSkidmarkUnk[i]) && m_aWheelTimer[i] > 0.0f)
						// Enter the C skidmark algorithm with the original owner expressions
						Skidmarks_RegisterOneType((uintptr)this + i, Skidmarks_BackendPosition(CollisionVector_ToNative(m_aWheelColPoints[i].point)),
							GetForward().x, GetForward().y,
							m_aWheelSkidmarkType[i], &m_aWheelSkidmarkBloody[i]);
				}

				// Sparks for friction of burst wheels
				if(Damage_GetWheelStatus(&Damage, i) == WHEEL_STATUS_BURST && m_aSuspensionSpringRatioPrev[i] < 1.0f){
					// Delegate the number of sparks to the C implementation
					int sparkCount = Automobile_BurstWheelSparkCount(m_vecMoveSpeed.MagnitudeSqr(), m_aWheelColPoints[i].surfaceB);
					if(sparkCount > 0){
						CVector normalSpeed = CollisionVector_ToNative(m_aWheelColPoints[i].normal) * DotProduct(CollisionVector_ToNative(m_aWheelColPoints[i].normal), m_vecMoveSpeed);
						CVector frictionSpeed = m_vecMoveSpeed - normalSpeed;
						if(i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_REAR_LEFT)
							frictionSpeed -= 0.05f*GetRight();
						else
							frictionSpeed += 0.05f*GetRight();
						CVector unusedRight = 0.15f*GetRight();
						CVector sparkDir = 0.25f*frictionSpeed;
						for(int spark = 0; spark < sparkCount; spark++)
							CParticle::AddParticle(PARTICLE_SPARK_SMALL, CollisionVector_ToNative(m_aWheelColPoints[i].point), sparkDir);
					}
				}
			}
		}
	}

	if(m_aCarNodes[CAR_WHEEL_RM]){
		// assume middle wheels are two units before rear ones
		CVector offset = GetForward()*2.0f;

		switch(m_aWheelState[CARWHEEL_REAR_LEFT]){
		// Game has same code three times here
		case WHEEL_STATE_SPINNING:
		case WHEEL_STATE_SKIDDING:
		case WHEEL_STATE_FIXED:
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
				CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_LEFT].point) + CVector(0.0f, 0.0f, 0.25f) + offset,
				CVector(0.0f, 0.0f, 0.0f));

			if(m_aWheelTimer[CARWHEEL_REAR_LEFT] > 0.0f)
				// Enter the C skidmark algorithm with the original owner expressions
				Skidmarks_RegisterOneType((uintptr)this + 5, Skidmarks_BackendPosition(CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_LEFT].point) + offset),
					GetForward().x, GetForward().y,
					m_aWheelSkidmarkType[CARWHEEL_REAR_LEFT], &m_aWheelSkidmarkBloody[CARWHEEL_REAR_LEFT]);
			break;
		default: break;
		}

		switch(m_aWheelState[CARWHEEL_REAR_RIGHT]){
		// Game has same code three times here
		case WHEEL_STATE_SPINNING:
		case WHEEL_STATE_SKIDDING:
		case WHEEL_STATE_FIXED:
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
				CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point) + CVector(0.0f, 0.0f, 0.25f) + offset,
				CVector(0.0f, 0.0f, 0.0f));

			if(m_aWheelTimer[CARWHEEL_REAR_RIGHT] > 0.0f)
				// Enter the C skidmark algorithm with the original owner expressions
				Skidmarks_RegisterOneType((uintptr)this + 6, Skidmarks_BackendPosition(CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point) + offset),
					GetForward().x, GetForward().y,
					m_aWheelSkidmarkType[CARWHEEL_REAR_RIGHT], &m_aWheelSkidmarkBloody[CARWHEEL_REAR_RIGHT]);
			break;
		default: break;
		}
	}


	// Rain on roof
	if(!CullZones_CamNoRain() && !CullZones_PlayerNoRain() &&
	   Automobile_SpeedBelow(fwdSpeed, 20.0f) && Weather_Rain > 0.02f){
		CColModel *colModel = GetColModel();

		for(i = 0; i < colModel->numTriangles; i++){
			CVector p1, p2, p3, c;

			// Invoke the complete C collision owner at the original native lifetime or operation boundary
			p1 = CollisionVector_ToNative(ColModel_GetTrianglePoint(colModel,colModel->triangles[i].a));
			p1 = GetMatrix() * p1;
			// Invoke the complete C collision owner at the original native lifetime or operation boundary
			p2 = CollisionVector_ToNative(ColModel_GetTrianglePoint(colModel,colModel->triangles[i].b));
			p2 = GetMatrix() * p2;
			// Invoke the complete C collision owner at the original native lifetime or operation boundary
			p3 = CollisionVector_ToNative(ColModel_GetTrianglePoint(colModel,colModel->triangles[i].c));
			p3 = GetMatrix() * p3;
			c = (p1 + p2 + p3)/3.0f;

			n = 6.0f*Weather_Rain;
			for(j = 0; j <= n; j++)
				CParticle::AddParticle(PARTICLE_RAIN_SPLASHUP,
					c + CVector(General_GetRandomNumberInRangeFloat(-0.4f, 0.4f), General_GetRandomNumberInRangeFloat(-0.4f, 0.4f), 0.0f),
					CVector(0.0f, 0.0f, 0.0f),
					nil, 0.0f, 0, 0, General_GetRandomNumber() & 1);
		}
	}

	AddDamagedVehicleParticles();

	// Exhaust smoke
	if(bEngineOn && !(pHandling->Flags & HANDLING_NO_EXHAUST) && fwdSpeed < 130.0f){
		CVector exhaustPos = mi->m_positions[CAR_POS_EXHAUST];
		CVector pos1, pos2, dir1, dir2;

		if(exhaustPos != CVector(0.0f, 0.0f, 0.0f)){
			dir1.z = 0.0f;
			dir2.z = 0.0f;
			if(fwdSpeed < 10.0f){
				float steerFwdXY[2];
				Automobile_SteerForward(m_fSteerAngle, steerFwdXY);
				CVector steerFwd(steerFwdXY[0], steerFwdXY[1], 0.0f);
				steerFwd = Multiply3x3(GetMatrix(), steerFwd);
				float r = General_GetRandomNumberInRangeFloat(-0.06f, -0.03f);
				dir1.x = steerFwd.x * r;
				dir1.y = steerFwd.y * r;
			}else{
				dir1.x = m_vecMoveSpeed.x;
				dir1.y = m_vecMoveSpeed.y;
			}

			pos1 = GetMatrix() * exhaustPos;
			if(pHandling->Flags & HANDLING_DBL_EXHAUST){
				pos2 = exhaustPos;
				pos2.x = -pos2.x;
				pos2 = GetMatrix() * pos2;
				dir2 = dir1;
			}

			static float fumesLimit = 2.0f;
			if(General_GetRandomNumberInRangeFloat(1.0f, 3.0f)*(m_fGasPedal+1.1f) > fumesLimit)
				for(i = 0; i < 4;){
					CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos1, dir1);
					if(pHandling->Flags & HANDLING_DBL_EXHAUST)
						CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos2, dir2);

					static float extraFumesLimit = 0.5f;
					if(m_fGasPedal > extraFumesLimit && m_nCurrentGear < 3){
						if(General_GetRandomNumber() & 1)
							CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos1, dir1);
						else if(pHandling->Flags & HANDLING_DBL_EXHAUST)
							CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos2, dir2);
					}

					// Fire on Cuban hermes
					if(GetModelIndex() == MI_CUBAN && i == 1 && m_fGasPedal > 0.9f){
						if(m_nCurrentGear == 1 || m_nCurrentGear == 3 && ((*Timer_GetTimeInMilliseconds())%1500) > 750){
							if(General_GetRandomNumber() & 1){
								CParticle::AddParticle(PARTICLE_FIREBALL, pos1, dir1, nil, 0.05f, 0, 0, 2, 200);
								CParticle::AddParticle(PARTICLE_FIREBALL, pos1, dir1, nil, 0.05f, 0, 0, 2, 200);
							}else{
								CParticle::AddParticle(PARTICLE_FIREBALL, pos2, dir2, nil, 0.05f, 0, 0, 2, 200);
								CParticle::AddParticle(PARTICLE_FIREBALL, pos2, dir2, nil, 0.05f, 0, 0, 2, 200);
							}
						}
					}

					if(GetStatus() == STATUS_PLAYER && ((*Timer_GetFrameCounter())&3) == 0 &&
					   Weather_Rain == 0.0f && i == 0){
						CVector camDist = GetPosition() - TheCamera.GetPosition();
						if(DotProduct(GetForward(), camDist) > 0.0f ||
						   TheCamera.GetLookDirection() == LOOKING_LEFT ||
						   TheCamera.GetLookDirection() == LOOKING_RIGHT){
							CParticle::AddParticle(PARTICLE_HEATHAZE, pos1, CVector(0.0f, 0.0f, 0.0f));
							if(pHandling->Flags & HANDLING_DBL_EXHAUST)
								CParticle::AddParticle(PARTICLE_HEATHAZE, pos2, CVector(0.0f, 0.0f, 0.0f));

							CParticle::AddParticle(PARTICLE_HEATHAZE, pos1, CVector(0.0f, 0.0f, 0.0f));
							if(pHandling->Flags & HANDLING_DBL_EXHAUST)
								CParticle::AddParticle(PARTICLE_HEATHAZE, pos2, CVector(0.0f, 0.0f, 0.0f));
						}
					}

					if(GetModelIndex() == MI_CUBAN && i < 1){
						i = 1;
						pos1 = GetMatrix() * CVector(1.134f, -1.276f, -0.56f);
						pos2 = GetMatrix() * CVector(-1.134f, -1.276f, -0.56f);
						dir1 += 0.05f*GetRight();
						dir2 -= 0.05f*GetRight();
					}else
						i = 99;
				}
		}
	}


	// Siren and taxi lights
	switch(GetModelIndex()){
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_POLICE:
	case MI_ENFORCER:
		if(m_bSirenOrAlarm){
			CVector pos1, pos2;
			uint8 r1, g1, b1;
			uint8 r2, g2, b2;
			uint8 r, g, b;

			// Delegate the places and the colours of the sirens to the C implementation
			int sirenKind = AUTOMOBILE_SIREN_ENFORCER;
			switch(GetModelIndex()){
			case MI_FIRETRUCK: sirenKind = AUTOMOBILE_SIREN_FIRETRUCK; break;
			case MI_AMBULAN: sirenKind = AUTOMOBILE_SIREN_AMBULANCE; break;
			case MI_POLICE: sirenKind = AUTOMOBILE_SIREN_POLICE; break;
			case MI_ENFORCER: sirenKind = AUTOMOBILE_SIREN_ENFORCER; break;
			}
			AutomobileSirenSetup sirenSetup = Automobile_SirenSetup(sirenKind);
			pos1 = CVector(sirenSetup.pos1[0], sirenSetup.pos1[1], sirenSetup.pos1[2]);
			pos2 = CVector(sirenSetup.pos2[0], sirenSetup.pos2[1], sirenSetup.pos2[2]);
			r1 = sirenSetup.color1[0]; g1 = sirenSetup.color1[1]; b1 = sirenSetup.color1[2];
			r2 = sirenSetup.color2[0]; g2 = sirenSetup.color2[1]; b2 = sirenSetup.color2[2];

			unsigned char sirenColor[3];
			Automobile_SirenLightColor((*Timer_GetTimeInMilliseconds()), sirenSetup.color1, sirenSetup.color2, sirenColor);
			r = sirenColor[0];
			g = sirenColor[1];
			b = sirenColor[2];

			CVector pos = GetPosition();
			float angle = ((*Timer_GetTimeInMilliseconds()) & 0x3FF)*TWOPI/0x3FF;
			float s = 8.0f*Sin(angle);
			float c = 8.0f*Cos(angle);
			//CShadows::StoreCarLightShadow(this, (uintptr)this + 21, gpShadowHeadLightsTex,
			//	&pos, c, s, s, -c, r, g, b, 8.0f);

			// Enter the C point-light algorithm with the original owner expressions
			PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_POINT,
				pos + GetUp()*2.0f, CVector(0.0f, 0.0f, 0.0f), 12.0f,
				r*0.02f, g*0.02f, b*0.02f, POINTLIGHTS_FOG_NONE, true);

			pos1 = GetMatrix() * pos1;
			pos2 = GetMatrix() * pos2;

			for(i = 0; i < 4; i++){
				uint8 sirenTimer = (((*Timer_GetTimeInMilliseconds()) + (i<<6))>>8) & 3;
				// Delegate the position of the siren light to the C implementation
				{
					const float p1[3] = { pos1.x, pos1.y, pos1.z };
					const float p2[3] = { pos2.x, pos2.y, pos2.z };
					float po[3];
					Automobile_SirenPosition(p1, p2, i, po);
					pos = CVector(po[0], po[1], po[2]);
				}

				switch(sirenTimer){
				case 0:
					CCoronas::RegisterCorona((uintptr)this + 21 + i,
						r1, g1, b1, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						i == 1 ? CCoronas::FLARE_HEADLIGHTS : CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
					break;
				case 2:
					CCoronas::RegisterCorona((uintptr)this + 21 + i,
						r2, g2, b2, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
					break;
				default:
					CCoronas::UpdateCoronaCoors((uintptr)this + 21 + i, pos, 50.0f, 0.0f);
					break;
				}
			}
		}
		break;

	case MI_FBIRANCH:
	case MI_VICECHEE:
		if(m_bSirenOrAlarm){
			CVector pos = GetMatrix() * CVector(0.4f, 0.6f, 0.3f);
			if((*Timer_GetTimeInMilliseconds()) & 0x100 &&
			   DotProduct(GetForward(), GetPosition() - TheCamera.GetPosition()) < 0.0f)
				if(GetModelIndex() == MI_VICECHEE)
					CCoronas::RegisterCorona((uintptr)this + 21,
						255, 70, 70, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
				else
					CCoronas::RegisterCorona((uintptr)this + 21,
						0, 0, 255, 255,
						pos, 0.4f, 50.0f,
						CCoronas::TYPE_STAR,
						CCoronas::FLARE_NONE,
						CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
			else
				CCoronas::UpdateCoronaCoors((uintptr)this + 21, pos, 50.0f, 0.0f);
		}
		break;

	case MI_TAXI:
	case MI_CABBIE:
	case MI_ZEBRA:
	case MI_KAUFMAN:
		if(bTaxiLight){
			CVector pos = GetPosition() + GetUp()*0.95f;
			CCoronas::RegisterCorona((uintptr)this + 21,
				128, 128, 0, 255,
				pos, 0.8f, 50.0f,
				CCoronas::TYPE_NORMAL,
				CCoronas::FLARE_NONE,
				CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f);
			// Enter the C point-light algorithm with the original owner expressions
			PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_POINT,
				pos, CVector(0.0f, 0.0f, 0.0f), 10.0f,
				1.0f, 1.0f, 0.5f, POINTLIGHTS_FOG_NONE, true);
		}
		break;
	}

	if(GetModelIndex() != MI_RCBANDIT && GetModelIndex() != MI_DODO &&
	   GetModelIndex() != MI_RHINO && GetModelIndex() != MI_RCBARON &&
	   GetVehicleAppearance() != VEHICLE_APPEARANCE_HELI) {
	// Process lights

	// Turn lights on/off
	// Delegate the state of the lights to the C implementation
	bool shouldLightsBeOn = VehicleLogic_ShouldLightsBeOn(Clock_GetHours(), Clock_GetMinutes(), m_randomSeed, Weather_Foggyness, Weather_WetRoads);
	bLightsOn = VehicleLogic_NextLightsState(shouldLightsBeOn, bLightsOn, GetStatus() == STATUS_WRECKED, GetStatus() == STATUS_ABANDONED,
		Automobile_CameraIsFarAway(TheCamera.GetPosition().x - GetPosition().x, TheCamera.GetPosition().y - GetPosition().y, 100.0f));

	// Actually render the lights
	bool alarmOn, alarmOff;
	// Delegate the flashing of the alarm to the C implementation
	VehicleLogic_AlarmFlash(IsAlarmOn(), (*Timer_GetTimeInMilliseconds()), &alarmOn, &alarmOff);
	if(bEngineOn && bLightsOn || alarmOn || alarmOff){
		CVector lookVector = GetPosition() - TheCamera.GetPosition();
		float camDist = lookVector.Magnitude();
		if(camDist != 0.0f)
			lookVector *= 1.0f/camDist;
		else
			lookVector = CVector(1.0f, 0.0f, 0.0f);

		// 1.0 if directly behind car, -1.0 if in front
		// Delegate the clamping of the behindness and its angle to the C implementation
		float angle;
		float behindness = Automobile_Behindness(DotProduct(lookVector, GetForward()), &angle);	// 0.0 if behind car, PI if in front

		// Headlights

		CVector headLightPos = mi->m_positions[CAR_POS_HEADLIGHTS];
		CVector lightR = GetMatrix() * headLightPos;
		CVector lightL = lightR;
		// Delegate the position of the left light to the C implementation
		{
			CVector right = GetRight();
			const float lr[3] = { lightL.x, lightL.y, lightL.z };
			const float rt[3] = { right.x, right.y, right.z };
			float ll[3];
			VehicleLogic_MirrorLight(lr, rt, headLightPos.x, ll);
			lightL = CVector(ll[0], ll[1], ll[2]);
		}

		// Headlight coronas
		if(DotProduct(lightR-TheCamera.GetPosition(), GetForward()) < 0.0f &&
		   (TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_1STPERSON || this != FindPlayerVehicle())){
			// In front of car
			// Delegate the glow of the headlight to the C implementation
			float intensity, size;
			VehicleLogic_HeadlightGlow(behindness, &intensity, &size);

			if(behindness < -0.97f && camDist < 30.0f){
				// Directly in front and not too far away
				if(pHandling->Flags & HANDLING_HALOGEN_LIGHTS){
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 6, 150, 150, 195, 255,
							lightL, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 7, 150, 150, 195, 255,
							lightR, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
				}else{
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 6, 160, 160, 140, 255,
							lightL, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 7, 160, 160, 140, 255,
							lightR, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
				}
			}

			if(alarmOff){
				if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this, 0, 0, 0, 0,
						lightL, size, 0.0f,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this + 1, 0, 0, 0, 0,
						lightR, size, 0.0f,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
			}else{
				if(pHandling->Flags & HANDLING_HALOGEN_LIGHTS){
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this, 190*intensity, 190*intensity, 255*intensity, 255,
							lightL, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 1, 190*intensity, 190*intensity, 255*intensity, 255,
							lightR, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}else{
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this, 210*intensity, 210*intensity, 195*intensity, 255,
							lightL, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 1, 210*intensity, 210*intensity, 195*intensity, 255,
							lightR, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}
			}
		}else{
			// Behind car
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this, lightL, 50.0f*TheCamera.LODDistMultiplier, angle);
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 1, lightR, 50.0f*TheCamera.LODDistMultiplier, angle);
		}

		// bright lights
		if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK && !bNoBrightHeadLights)
			// Enter the C bright-light algorithm with the original arguments
			BrightLights_RegisterOneLegacy(lightL, GetUp(), GetRight(), GetForward(), pHandling->FrontLights + BRIGHTLIGHT_FRONT);
		if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK && !bNoBrightHeadLights)
			// Enter the C bright-light algorithm with the original arguments
			BrightLights_RegisterOneLegacy(lightR, GetUp(), GetRight(), GetForward(), pHandling->FrontLights + BRIGHTLIGHT_FRONT);

		// Taillights

		CVector tailLightPos = mi->m_positions[CAR_POS_TAILLIGHTS];
		lightR = GetMatrix() * tailLightPos;
		lightL = lightR;
		// Delegate the position of the left light to the C implementation
		{
			CVector right = GetRight();
			const float lr[3] = { lightL.x, lightL.y, lightL.z };
			const float rt[3] = { right.x, right.y, right.z };
			float ll[3];
			VehicleLogic_MirrorLight(lr, rt, tailLightPos.x, ll);
			lightL = CVector(ll[0], ll[1], ll[2]);
		}

		// Taillight coronas
		if(DotProduct(lightR-TheCamera.GetPosition(), GetForward()) > 0.0f){
			// Behind car
			float intensity = (behindness + 1.0f)*0.4f;
			float size = (behindness + 1.0f)*0.5f;

			if(m_fGasPedal < 0.0f){
				// reversing
				intensity += 0.4f;
				size += 0.3f;
				if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this + 14, 128*intensity, 128*intensity, 128*intensity, 255,
						lightL, size, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
					CCoronas::RegisterCorona((uintptr)this + 15, 128*intensity, 128*intensity, 128*intensity, 255,
						lightR, size, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
			}else{
				if(m_fBrakePedal > 0.0f){
					intensity += 0.4f;
					size += 0.3f;
				}

				if(alarmOff){
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 14, 0, 0, 0, 0,
							lightL, size, 0.0f,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 15, 0, 0, 0, 0,
							lightR, size, 0.0f,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}else{
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 14, 128*intensity, 0, 0, 255,
							lightL, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 15, 128*intensity, 0, 0, 255,
							lightR, size, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}
			}
		}else{
			// In front of car
			// missing LODDistMultiplier probably a BUG
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 14, lightL, 50.0f, angle);
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 15, lightR, 50.0f, angle);
		}

		// bright lights
		if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
			// Enter the C bright-light algorithm with the original arguments
			BrightLights_RegisterOneLegacy(lightL, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);
		if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
			// Enter the C bright-light algorithm with the original arguments
			BrightLights_RegisterOneLegacy(lightR, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);

		// Light shadows
		if(!alarmOff){
			CVector pos = GetPosition();
			CVector2D fwd(GetForward());
			fwd.Normalise();
			float f = headLightPos.y + 6.0f;
			pos += CVector(f*fwd.x, f*fwd.y, 2.0f);

			if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK ||
			   Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
				CShadows::StoreCarLightShadow(this, (uintptr)this + 22, gpShadowHeadLightsTex, &pos,
					7.0f*fwd.x, 7.0f*fwd.y, 5.5f*fwd.y, -5.5f*fwd.x, 45, 45, 45, 7.0f);

			f = (tailLightPos.y - 2.5f) - (headLightPos.y + 6.0f);
			pos += CVector(f*fwd.x, f*fwd.y, 0.0f);
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK ||
			   Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
				CShadows::StoreCarLightShadow(this, (uintptr)this + 25, gpShadowExplosionTex, &pos,
					3.0f, 0.0f, 0.0f, -3.0f, 35, 0, 0, 4.0f);
		}

		if(this == FindPlayerVehicle() && !alarmOff){
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_LEFT) == LIGHT_STATUS_OK ||
			   Damage_GetLightStatus(&Damage, VEHLIGHT_FRONT_RIGHT) == LIGHT_STATUS_OK)
				// Enter the C point-light algorithm with the original owner expressions
				PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_DIRECTIONAL, GetPosition(), GetForward(),
					20.0f, 1.0f, 1.0f, 1.0f,
					FindPlayerVehicle()->m_vecMoveSpeed.MagnitudeSqr2D() < sq(0.45f) ? POINTLIGHTS_FOG_NORMAL : POINTLIGHTS_FOG_NONE,
					false);
			CVector pos = GetPosition() - 4.0f*GetForward();
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK ||
			   Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK) {
				if(m_fBrakePedal > 0.0f)
					// Enter the C point-light algorithm with the original owner expressions
					PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_POINT, pos, CVector(0.0f, 0.0f, 0.0f),
						10.0f, 1.0f, 0.0f, 0.0f,
						POINTLIGHTS_FOG_NONE, false);
				else
					// Enter the C point-light algorithm with the original owner expressions
					PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_POINT, pos, CVector(0.0f, 0.0f, 0.0f),
						7.0f, 0.6f, 0.0f, 0.0f,
						POINTLIGHTS_FOG_NONE, false);
			}
		}
	}else if(GetStatus() != STATUS_ABANDONED && GetStatus() != STATUS_WRECKED){
		// Lights off

		CVector lightPos = mi->m_positions[CAR_POS_TAILLIGHTS];
		CVector lightR = GetMatrix() * lightPos;
		CVector lightL = lightR;
		// Delegate the position of the left light to the C implementation
		{
			CVector right = GetRight();
			const float lr[3] = { lightL.x, lightL.y, lightL.z };
			const float rt[3] = { right.x, right.y, right.z };
			float ll[3];
			VehicleLogic_MirrorLight(lr, rt, lightPos.x, ll);
			lightL = CVector(ll[0], ll[1], ll[2]);
		}

		if(m_fBrakePedal > 0.0f || m_fGasPedal < 0.0f){
			CVector lookVector = GetPosition() - TheCamera.GetPosition();
			lookVector.Normalise();
			float behindness = DotProduct(lookVector, GetForward());
			if(behindness > 0.0f){
				if(m_fGasPedal < 0.0f){
					// reversing
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 14, 120, 120, 120, 255,
							lightL, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 15, 120, 120, 120, 255,
							lightR, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						// Enter the C bright-light algorithm with the original arguments
						BrightLights_RegisterOneLegacy(lightL, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_FRONT);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						// Enter the C bright-light algorithm with the original arguments
						BrightLights_RegisterOneLegacy(lightR, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_FRONT);
				}else{
					// braking
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 14, 120, 0, 0, 255,
							lightL, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						CCoronas::RegisterCorona((uintptr)this + 15, 120, 0, 0, 255,
							lightR, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
							CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
							CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
						// Enter the C bright-light algorithm with the original arguments
						BrightLights_RegisterOneLegacy(lightL, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);
					if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
						// Enter the C bright-light algorithm with the original arguments
						BrightLights_RegisterOneLegacy(lightR, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);
				}
			}else{
				if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
					CCoronas::UpdateCoronaCoors((uintptr)this + 14, lightL, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
				if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
					CCoronas::UpdateCoronaCoors((uintptr)this + 15, lightR, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
			}
		}else{
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_LEFT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 14, lightL, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
			if(Damage_GetLightStatus(&Damage, VEHLIGHT_REAR_RIGHT) == LIGHT_STATUS_OK)
				CCoronas::UpdateCoronaCoors((uintptr)this + 15, lightR, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
		}
	}
	// end of lights
	}

	if (IsRealHeli())
		CShadows::StoreShadowForVehicle(this, VEH_SHD_TYPE_HELI);
	else if ( GetModelIndex() == MI_RCBARON)
		CShadows::StoreShadowForVehicle(this, VEH_SHD_TYPE_RCPLANE);
	else
		CShadows::StoreShadowForVehicle(this, VEH_SHD_TYPE_CAR);

	DoSunGlare();

	// Heli dust
	if(IsRealHeli() && m_aWheelSpeed[1] > 0.1125f && GetPosition().z < 30.0f){
		bool foundGround = false;
		float waterZ = -1000.0f;
		float groundZ = World_FindGroundZFor3DCoord(GetPosition().x, GetPosition().y, GetPosition().z, &foundGround);
		if(!WaterLevel_GetWaterLevel(GetPosition(), &waterZ, false))
			waterZ = 0.0f;
		// Delegate the higher height to the C implementation
		groundZ = General_MaxFloat(groundZ, waterZ);
		// Delegate the amount of dust to the C implementation
		float rnd = Automobile_HeliDustAmount(m_aWheelSpeed[1], (*Timer_GetTimeStep()));
		float radius = 10.0f;
		if(GetModelIndex() == MI_RCGOBLIN || GetModelIndex() == MI_RCRAIDER)
			radius = 3.0f;
		if(GetPosition().z - groundZ < radius)
			HeliDustGenerate(this, radius-(GetPosition().z - groundZ), groundZ, Ceil(rnd));
	}

	CMatrix mat;
	CVector pos;

	bool onlyFrontWheels = false;
	if(IsRealHeli()){
		// top rotor
		m_aWheelRotation[1] += m_aWheelSpeed[1]*(*Timer_GetTimeStep());
		// Delegate the angle wrapping to the C implementation
		m_aWheelRotation[1] = General_ReduceAbove(m_aWheelRotation[1], TWOPI, TWOPI);
		// rear rotor
		m_aWheelRotation[3] += m_aWheelSpeed[1]*(*Timer_GetTimeStep());
		// Delegate the angle wrapping to the C implementation
		m_aWheelRotation[3] = General_ReduceAbove(m_aWheelRotation[3], TWOPI, TWOPI);
		onlyFrontWheels = true;
	}

	CVector contactPoints[4];	// relative to model
	CVector contactSpeeds[4];	// speed at contact points
	float steerFwd[2];
	// Delegate the direction of the front wheels to the C implementation
	Automobile_SteerForward(m_fSteerAngle, steerFwd);
	CVector frontWheelFwd = Multiply3x3(GetMatrix(), CVector(steerFwd[0], steerFwd[1], 0.0f));
	CVector rearWheelFwd = GetForward();
	for(i = 0; i < 4; i++){
		if (m_aWheelTimer[i] > 0.0f && (!onlyFrontWheels || i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)) {
			contactPoints[i] = CollisionVector_ToNative(m_aWheelColPoints[i].point) - GetPosition();
			contactSpeeds[i] = GetSpeed(contactPoints[i]);
			if (i == CARWHEEL_FRONT_LEFT || i == CARWHEEL_FRONT_RIGHT)
				m_aWheelSpeed[i] = ProcessWheelRotation(m_aWheelState[i], frontWheelFwd, contactSpeeds[i], 0.5f*mi->m_wheelScale);
			else
				m_aWheelSpeed[i] = ProcessWheelRotation(m_aWheelState[i], rearWheelFwd, contactSpeeds[i], 0.5f*mi->m_wheelScale);
			m_aWheelRotation[i] += m_aWheelSpeed[i];
		}
	}

	RwRGBA hoverParticleCol = { 255, 255, 255, 32 };

	// Rear right wheel
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RB]));
	pos = mat.GetPosition();
	pos.z = m_aWheelPosition[CARWHEEL_REAR_RIGHT];
	if(Damage_GetWheelStatus(&Damage, CARWHEEL_REAR_RIGHT) == WHEEL_STATUS_BURST)
		mat.SetRotate(m_aWheelRotation[CARWHEEL_REAR_RIGHT], 0.0f, Automobile_WheelWobble(m_aWheelRotation[CARWHEEL_REAR_RIGHT]));
	else
		mat.SetRotateX(m_aWheelRotation[CARWHEEL_REAR_RIGHT]);
	if(GetStatus() == STATUS_PLAYER){
		if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_RIGHT] < 1.0f &&
		   m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB == SURFACE_WATER){
			// hovering on water
			mat.RotateY(-HALFPI);
			if(((*Timer_GetFrameCounter())+CARWHEEL_REAR_RIGHT) & 1){
				CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point),
					0.5f*m_vecMoveSpeed+0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
			}else{
				CParticle::AddParticle(PARTICLE_CAR_SPLASH, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_RIGHT].point),
					0.3f*m_vecMoveSpeed+0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
					General_GetRandomNumberInRangeFloat(0.0f, 10.0f),
					General_GetRandomNumberInRangeFloat(0.0f, 90.0f), 1);
			}
#ifdef BETTER_ALLCARSAREDODO_CHEAT
		} else if (bAllDodosCheat && m_nDriveWheelsOnGround == 0 && m_nDriveWheelsOnGroundPrev == 0) {
			mat.RotateY(-HALFPI);
#endif
		}else{
			// tilt wheel depending oh how much it presses on ground
			// Delegate the tilt of the wheel to the C implementation
			mat.RotateY(Automobile_WheelTilt(pos.z, m_fHeightAboveRoad, mi->m_wheelScale, GetModelIndex() == MI_VOODOO, true));
		}
	}
	if(pHandling->Flags & HANDLING_FAT_REARW)
		mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
	else
		mat.Scale(mi->m_wheelScale);
	mat.Translate(pos);
	mat.UpdateRW();

	// Rear left wheel
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LB]));
	pos = mat.GetPosition();
	pos.z = m_aWheelPosition[CARWHEEL_REAR_LEFT];
	if(Damage_GetWheelStatus(&Damage, CARWHEEL_REAR_LEFT) == WHEEL_STATUS_BURST)
		mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI+Automobile_WheelWobble(m_aWheelRotation[CARWHEEL_REAR_LEFT]));
	else
		mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI);
	if(GetStatus() == STATUS_PLAYER){
		if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_LEFT] < 1.0f &&
		   m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB == SURFACE_WATER){
			// hovering on water
			mat.RotateY(HALFPI);
			if(((*Timer_GetFrameCounter())+CARWHEEL_REAR_LEFT) & 1){
				CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_LEFT].point),
					0.5f*m_vecMoveSpeed-0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
			}else{
				CParticle::AddParticle(PARTICLE_CAR_SPLASH, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_REAR_LEFT].point),
					0.3f*m_vecMoveSpeed-0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
					General_GetRandomNumberInRangeFloat(0.0f, 10.0f),
					General_GetRandomNumberInRangeFloat(0.0f, 90.0f), 1);
			}
#ifdef BETTER_ALLCARSAREDODO_CHEAT
		} else if (bAllDodosCheat && m_nDriveWheelsOnGround == 0 && m_nDriveWheelsOnGroundPrev == 0) {
			mat.RotateY(HALFPI);
#endif
		}else{
			// tilt wheel depending oh how much it presses on ground
			// Delegate the tilt of the wheel to the C implementation
			mat.RotateY(Automobile_WheelTilt(pos.z, m_fHeightAboveRoad, mi->m_wheelScale, GetModelIndex() == MI_VOODOO, false));
		}
	}
	if(pHandling->Flags & HANDLING_FAT_REARW)
		mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
	else
		mat.Scale(mi->m_wheelScale);
	mat.Translate(pos);
	mat.UpdateRW();

	// Mid right wheel
	if(m_aCarNodes[CAR_WHEEL_RM]){
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RM]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_REAR_RIGHT];
		if(Damage_GetWheelStatus(&Damage, CARWHEEL_REAR_RIGHT) == WHEEL_STATUS_BURST)
			mat.SetRotate(m_aWheelRotation[CARWHEEL_REAR_RIGHT], 0.0f, Automobile_WheelWobble(m_aWheelRotation[CARWHEEL_REAR_RIGHT]));
		else
			mat.SetRotateX(m_aWheelRotation[CARWHEEL_REAR_RIGHT]);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_RIGHT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_REAR_RIGHT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(-HALFPI);
#ifdef BETTER_ALLCARSAREDODO_CHEAT
			} else if (bAllDodosCheat && m_nDriveWheelsOnGround == 0 && m_nDriveWheelsOnGroundPrev == 0) {
				mat.RotateY(-HALFPI);
#endif
			}else{
				// tilt wheel depending oh how much it presses on ground
				// Delegate the tilt of the wheel to the C implementation
				mat.RotateY(Automobile_WheelTilt(pos.z, m_fHeightAboveRoad, mi->m_wheelScale, GetModelIndex() == MI_VOODOO, true));
			}
		}
		if(pHandling->Flags & HANDLING_FAT_REARW)
			mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Mid left wheel
	if(m_aCarNodes[CAR_WHEEL_LM]){
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LM]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_REAR_LEFT];
		if(Damage_GetWheelStatus(&Damage, CARWHEEL_REAR_LEFT) == WHEEL_STATUS_BURST)
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI+Automobile_WheelWobble(m_aWheelRotation[CARWHEEL_REAR_LEFT]));
		else
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_REAR_LEFT], 0.0f, PI);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_REAR_LEFT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_REAR_LEFT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(HALFPI);
#ifdef BETTER_ALLCARSAREDODO_CHEAT
			} else if (bAllDodosCheat && m_nDriveWheelsOnGround == 0 && m_nDriveWheelsOnGroundPrev == 0) {
				mat.RotateY(HALFPI);
#endif
			}else{
				// tilt wheel depending oh how much it presses on ground
				// Delegate the tilt of the wheel to the C implementation
				mat.RotateY(Automobile_WheelTilt(pos.z, m_fHeightAboveRoad, mi->m_wheelScale, GetModelIndex() == MI_VOODOO, false));
			}
		}
		if(pHandling->Flags & HANDLING_FAT_REARW)
			mat.Scale(1.15f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	if(GetModelIndex() == MI_DODO){
		// Front wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
		if(Damage_GetWheelStatus(&Damage, CARWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle+Automobile_WheelWobble(m_aWheelRotation[CARWHEEL_FRONT_RIGHT]));
		else
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle);
		mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		// Rotate propeller
		if(m_aCarNodes[CAR_WINDSCREEN]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WINDSCREEN]));
			pos = mat.GetPosition();
			mat.SetRotateY(m_fPropellerRotation);
			mat.Translate(pos);
			mat.UpdateRW();

			m_fPropellerRotation += m_fGasPedal != 0.0f ? TWOPI/13.0f : TWOPI/26.0f;
			if(m_fPropellerRotation > TWOPI)
				m_fPropellerRotation -= TWOPI;
		}

		// Rudder
		if(Damage_GetDoorStatus(&Damage, DOOR_BOOT) != DOOR_STATUS_MISSING && m_aCarNodes[CAR_BOOT]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
			pos = mat.GetPosition();
			mat.SetRotate(0.0f, 0.0f, -m_fSteerAngle);
			mat.Rotate(0.0f, Sin(m_fSteerAngle)*DEGTORAD(22.0f), 0.0f);
			mat.Translate(pos);
			mat.UpdateRW();
		}

		ProcessSwingingDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT);
		ProcessSwingingDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
	}else if(GetModelIndex() == MI_RHINO){
		// Front right wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
		// no damaged wheels or steering
		mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, 0.0f);
		mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		// Front left wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_LEFT];
		// no damaged wheels or steering
		mat.SetRotate(-m_aWheelRotation[CARWHEEL_FRONT_LEFT], 0.0f, PI);
		mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();
	}else if(IsRealHeli()){
		// Top rotor
		if(m_aCarNodes[CAR_BONNET]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BONNET]));
			pos = mat.GetPosition();
			mat.SetRotateZ(m_aWheelRotation[1]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
		// Blurred top rotor
		if(m_aCarNodes[CAR_WINDSCREEN]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WINDSCREEN]));
			pos = mat.GetPosition();
			mat.SetRotateZ(-m_aWheelRotation[1]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
		// Rear rotor
		if(m_aCarNodes[CAR_BOOT]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
			pos = mat.GetPosition();
			mat.SetRotateX(m_aWheelRotation[3]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
		// Blurred rear rotor
		if(m_aCarNodes[CAR_BUMP_REAR]){
			mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_BUMP_REAR]));
			pos = mat.GetPosition();
			mat.SetRotateX(-m_aWheelRotation[3]);
			mat.Translate(pos);
			mat.UpdateRW();
		}
	}else{
		// Front right wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_RIGHT];
		if(Damage_GetWheelStatus(&Damage, CARWHEEL_FRONT_RIGHT) == WHEEL_STATUS_BURST)
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle+Automobile_WheelWobble(m_aWheelRotation[CARWHEEL_FRONT_RIGHT]));
		else
			mat.SetRotate(m_aWheelRotation[CARWHEEL_FRONT_RIGHT], 0.0f, m_fSteerAngle);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_FRONT_RIGHT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(-HALFPI);
				if(((*Timer_GetFrameCounter())+CARWHEEL_FRONT_RIGHT) & 1){
					CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].point),
						0.5f*m_vecMoveSpeed+0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
				}else{
					CParticle::AddParticle(PARTICLE_CAR_SPLASH, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_RIGHT].point),
						0.3f*m_vecMoveSpeed+0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
						General_GetRandomNumberInRangeFloat(0.0f, 90.0f),
						General_GetRandomNumberInRangeFloat(0.0f, 10.0f), 1);
				}
#ifdef BETTER_ALLCARSAREDODO_CHEAT
			} else if (bAllDodosCheat && m_nDriveWheelsOnGround == 0 && m_nDriveWheelsOnGroundPrev == 0) {
				mat.RotateY(-HALFPI);
#endif
			}else{
				// tilt wheel depending oh how much it presses on ground
				// Delegate the tilt of the wheel to the C implementation
				mat.RotateY(Automobile_WheelTilt(pos.z, m_fHeightAboveRoad, mi->m_wheelScale, GetModelIndex() == MI_VOODOO, true));
			}
		}
		if(pHandling->Flags & HANDLING_NARROW_FRONTW)
			mat.Scale(0.7f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		// Front left wheel
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
		pos = mat.GetPosition();
		pos.z = m_aWheelPosition[CARWHEEL_FRONT_LEFT];
		if(Damage_GetWheelStatus(&Damage, CARWHEEL_FRONT_LEFT) == WHEEL_STATUS_BURST)
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_FRONT_LEFT], 0.0f, PI+m_fSteerAngle+Automobile_WheelWobble(m_aWheelRotation[CARWHEEL_FRONT_LEFT]));
		else
			mat.SetRotate(-m_aWheelRotation[CARWHEEL_FRONT_LEFT], 0.0f, PI+m_fSteerAngle);
		if(GetStatus() == STATUS_PLAYER){
			if(bHoverCheat && m_aSuspensionSpringRatioPrev[CARWHEEL_FRONT_LEFT] < 1.0f &&
			   m_aWheelColPoints[CARWHEEL_FRONT_LEFT].surfaceB == SURFACE_WATER){
				// hovering on water
				mat.RotateY(HALFPI);
				if(((*Timer_GetFrameCounter())+CARWHEEL_FRONT_LEFT) & 1){
					CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].point),
						0.5f*m_vecMoveSpeed-0.1f*GetRight(), nil, 0.4f, hoverParticleCol);
				}else{
					CParticle::AddParticle(PARTICLE_CAR_SPLASH, CollisionVector_ToNative(m_aWheelColPoints[CARWHEEL_FRONT_LEFT].point),
						0.3f*m_vecMoveSpeed-0.15f*GetRight()+CVector(0.0f, 0.0f, 0.1f), nil, 0.15f, hoverParticleCol,
						General_GetRandomNumberInRangeFloat(0.0f, 90.0f),
						General_GetRandomNumberInRangeFloat(0.0f, 10.0f), 1);
				}
#ifdef BETTER_ALLCARSAREDODO_CHEAT
			} else if (bAllDodosCheat && m_nDriveWheelsOnGround == 0 && m_nDriveWheelsOnGroundPrev == 0) {
				mat.RotateY(HALFPI);
#endif
			}else{
				// tilt wheel depending oh how much it presses on ground
				// Delegate the tilt of the wheel to the C implementation
				mat.RotateY(Automobile_WheelTilt(pos.z, m_fHeightAboveRoad, mi->m_wheelScale, GetModelIndex() == MI_VOODOO, false));
			}
		}
		if(pHandling->Flags & HANDLING_NARROW_FRONTW)
			mat.Scale(0.7f*mi->m_wheelScale, mi->m_wheelScale, mi->m_wheelScale);
		else
			mat.Scale(mi->m_wheelScale);
		mat.Translate(pos);
		mat.UpdateRW();

		ProcessSwingingDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT);
		ProcessSwingingDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
		ProcessSwingingDoor(CAR_DOOR_LR, DOOR_REAR_LEFT);
		ProcessSwingingDoor(CAR_DOOR_RR, DOOR_REAR_RIGHT);
		ProcessSwingingDoor(CAR_BONNET, DOOR_BONNET);
		ProcessSwingingDoor(CAR_BOOT, DOOR_BOOT);
	}

	if((GetModelIndex() == MI_PHEONIX || GetModelIndex() == MI_BFINJECT) &&
	   GetStatus() == STATUS_PLAYER && m_aCarNodes[CAR_WING_LR]){
		// Delegate the rotation of the propeller to the C implementation
		float rotation = Automobile_WingPropeller(GetModelIndex() == MI_BFINJECT, Abs(m_fGasPedal) > 0.0f, &m_fPropellerRotation, (*Timer_GetTimeStep()), (*Timer_GetTimeInMilliseconds()));

		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WING_LR]));
		pos = mat.GetPosition();
		if(GetModelIndex() == MI_BFINJECT)
			mat.SetRotateY(rotation);
		else
			mat.SetRotateX(rotation);
		mat.Translate(pos);
		mat.UpdateRW();
	}
}

void
CAutomobile::Render(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)

	mi->SetVehicleColour(m_currentColour1, m_currentColour2);

	if(IsRealHeli()){
		RpAtomic *atomic = nil;
		// Delegate the alpha of the rotors to the C implementation
		int rotorAlpha, blurAlpha;
		Automobile_HeliRotorAlpha(m_aWheelSpeed[1], &rotorAlpha, &blurAlpha);

		// Top rotor
		if(m_aCarNodes[CAR_BONNET]){
			RwFrameForAllObjects(m_aCarNodes[CAR_BONNET], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, rotorAlpha);
		}
		atomic = nil;
		// Rear rotor
		if(m_aCarNodes[CAR_BOOT]){
			RwFrameForAllObjects(m_aCarNodes[CAR_BOOT], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, rotorAlpha);
		}
		atomic = nil;
		// Blurred top rotor
		if(m_aCarNodes[CAR_WINDSCREEN]){
			RwFrameForAllObjects(m_aCarNodes[CAR_WINDSCREEN], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, blurAlpha);
		}
		atomic = nil;
		// Blurred rear rotor
		if(m_aCarNodes[CAR_BUMP_REAR]){
			RwFrameForAllObjects(m_aCarNodes[CAR_BUMP_REAR], GetCurrentAtomicObjectCB, &atomic);
			if(atomic)
				SetComponentAtomicAlpha(atomic, blurAlpha);
		}
	}

	if(CVehicle::bWheelsOnlyCheat){
		// Use the C helper and retain the original typed SDK result
		RpAtomicRender((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB])));
		// Use the C helper and retain the original typed SDK result
		RpAtomicRender((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB])));
		// Use the C helper and retain the original typed SDK result
		RpAtomicRender((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF])));
		// Use the C helper and retain the original typed SDK result
		RpAtomicRender((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF])));
		if(m_aCarNodes[CAR_WHEEL_RM])
			// Use the C helper and retain the original typed SDK result
			RpAtomicRender((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RM])));
		if(m_aCarNodes[CAR_WHEEL_LM])
			// Use the C helper and retain the original typed SDK result
			RpAtomicRender((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LM])));
	}else
		CEntity::Render();
}

int32
CAutomobile::ProcessEntityCollision(CEntity *ent, CColPoint *colpoints)
{
	int i;
	CColModel *colModel;

	if(GetStatus() != STATUS_SIMPLE)
		bVehicleColProcessed = true;

	if(bUsingSpecialColModel)
		colModel = &World_Players[World_PlayerInFocus].m_ColModel;
	else
		colModel = GetColModel();

	int numWheelCollisions = 0;
	float prevRatios[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
	for(i = 0; i < 4; i++)
		prevRatios[i] = m_aSuspensionSpringRatio[i];

	if(m_bIsVehicleBeingShifted || bSkipLineCol || ent->IsPed() ||
	   GetModelIndex() == MI_DODO && ent->IsVehicle())
		colModel->numLines = 0;

	int numCollisions = Collision_ProcessColModels(GetMatrix(), *colModel,
		ent->GetMatrix(), *ent->GetColModel(),
		colpoints,
		m_aWheelColPoints, m_aSuspensionSpringRatio);

	// m_aSuspensionSpringRatio are now set to the point where the tyre touches ground.
	// In ProcessControl these will be re-normalized to ignore the tyre radius.

	if(colModel->numLines){
		for(i = 0; i < 4; i++)
			if(m_aSuspensionSpringRatio[i] < 1.0f && m_aSuspensionSpringRatio[i] < prevRatios[i]){
				numWheelCollisions++;

				// wheel is touching a physical
				if(ent->IsVehicle() || ent->IsObject()){
					CPhysical *phys = (CPhysical*)ent;

					m_aGroundPhysical[i] = phys;
					phys->RegisterReference((CEntity**)&m_aGroundPhysical[i]);
					m_aGroundOffset[i] = CollisionVector_ToNative(m_aWheelColPoints[i].point) - phys->GetPosition();
				}

				m_nSurfaceTouched = m_aWheelColPoints[i].surfaceB;
				if(ent->IsBuilding())
					m_pCurGroundEntity = ent;
			}
	}else
		colModel->numLines = 4;

	if(numCollisions > 0 || numWheelCollisions > 0){
		AddCollisionRecord(ent);
		if(!ent->IsBuilding())
			((CPhysical*)ent)->AddCollisionRecord(this);

		if(numCollisions > 0)
			if(ent->IsBuilding() ||
			   ent->IsObject() && ((CPhysical*)ent)->bInfiniteMass)
				bHasHitWall = true;
	}

	return numCollisions;
}

static int16 nLastControlInput;
static float fMouseCentreRange = 0.35f;
static float fMouseSteerSens = -0.0035f;
static float fMouseCentreMult = 0.975f;

void
CAutomobile::ProcessControlInputs(uint8 pad)
{
	float speed = DotProduct(m_vecMoveSpeed, GetForward());

	if(!CPad::GetPad(pad)->GetExitVehicle() ||
	   pDriver && pDriver->m_pVehicleAnim && (pDriver->m_pVehicleAnim->animId == ANIM_STD_ROLLOUT_LHS ||
	                                          pDriver->m_pVehicleAnim->animId == ANIM_STD_ROLLOUT_RHS))
		bIsHandbrakeOn = !!CPad::GetPad(pad)->GetHandBrake();
	else
		bIsHandbrakeOn = true;

	// Steer left/right
	if(CCamera::m_bUseMouse3rdPerson && !CVehicle::m_bDisableMouseSteering){
		if(Input_GetMouseX() != 0.0f){ // rouz edit (ChatGPT)
			// Delegate the steering with the mouse to the C implementation
			m_fSteerInput = AutomobileLogic_SteerMouse(m_fSteerInput, Input_GetMouseX(), fMouseSteerSens, fMouseCentreRange, fMouseCentreMult, (*Timer_GetTimeStep()));
			nLastControlInput = 2;
		}else if(CPad::GetPad(pad)->GetSteeringLeftRight() || nLastControlInput != 2){
			// mouse hasn't move, steer with pad like below
			// Delegate the steering step to the C implementation
			m_fSteerInput = AutomobileLogic_SteerPad(m_fSteerInput, CPad::GetPad(pad)->GetSteeringLeftRight(), (*Timer_GetTimeStep()));
			nLastControlInput = 0;
		}
	}else{
		// Delegate the steering step to the C implementation
		m_fSteerInput = AutomobileLogic_SteerPad(m_fSteerInput, CPad::GetPad(pad)->GetSteeringLeftRight(), (*Timer_GetTimeStep()));
		nLastControlInput = 0;
	}
	// Delegate the limit to the C implementation
	m_fSteerInput = VehicleLogic_ClampValue(m_fSteerInput, -1.0f, 1.0f);

	// Accelerate/Brake
	// Delegate the pedals to the C implementation
	bool burnout;
	AutomobileLogic_Pedals(speed, CPad::GetPad(pad)->GetAccelerate(), CPad::GetPad(pad)->GetBrake(), GetModelIndex() == MI_DODO, &m_fGasPedal, &m_fBrakePedal, &burnout);
	if(burnout)
		m_doingBurnout = 1;

	// Actually turn wheels
	// Delegate the angle of the wheels to the C implementation
	m_fSteerAngle = AutomobileLogic_WheelAngle(m_fSteerInput, pHandling->fSteeringLock);

	if(bComedyControls){
		// Delegate the comedy controls to the C implementation
		m_comedyControlState = AutomobileLogic_ComedyControls(m_comedyControlState, General_GetRandomNumber() % 10, &m_fSteerAngle);
	}else
		m_comedyControlState = 0;

	// Brake if player isn't in control
	// BUG: game always uses pad 0 here
#ifdef FIX_BUGS
	if(CPad::GetPad(pad)->ArePlayerControlsDisabled()){
#else
	if(CPad::GetPad(0)->ArePlayerControlsDisabled()){
#endif
		m_fBrakePedal = 1.0f;
		bIsHandbrakeOn = true;
		m_fGasPedal = 0.0f;

		FindPlayerPed()->KeepAreaAroundPlayerClear();

		// slow down car immediately
		speed = m_vecMoveSpeed.Magnitude();
		if(speed > 0.28f)
			m_vecMoveSpeed *= 0.28f/speed;
	}
}

void
CAutomobile::FireTruckControl(void)
{
	if(this == FindPlayerVehicle()){
		if(!CPad::GetPad(0)->GetCarGunFired())
			return;
#ifdef FREE_CAM
		if (!CCamera::bFreeCam)
#endif 
		{
			m_fCarGunLR += CPad::GetPad(0)->GetCarGunLeftRight() * 0.00025f * (*Timer_GetTimeStep());
			m_fCarGunUD += CPad::GetPad(0)->GetCarGunUpDown() * 0.0001f * (*Timer_GetTimeStep());
		}
		// Delegate the limit to the C implementation
		m_fCarGunUD = VehicleLogic_ClampValue(m_fCarGunUD, 0.05f, 0.3f);


		CVector cannonPos(0.0f, 1.5f, 1.9f);
		cannonPos = GetMatrix() * cannonPos;
		// Delegate the direction of the cannon to the C implementation
		CVector cannonDirC;
		Automobile_CannonDirection(m_fCarGunLR, m_fCarGunUD, &cannonDirC);
		CVector cannonDir(cannonDirC.x, cannonDirC.y, cannonDirC.z);
		cannonDir = Multiply3x3(GetMatrix(), cannonDir);
		cannonDir.z += (General_GetRandomNumber()&0xF)/1000.0f;
		// Enter the C water-cannon algorithm with the original owner expressions
		WaterCannons_UpdateOneLegacy((uintptr)this, &cannonPos, &cannonDir);
	}else if(GetStatus() == STATUS_PHYSICS){
		CFire *fire = FireManager_FindFurthestFire_NeverMindFireMen(&gFireManager,Fire_VectorFromNative(GetPosition()), 10.0f, 35.0f);
		if(fire == nil)
			return;

		// Target cannon onto fire
		float targetAngle = General_GetATanOfXY(fire->m_vecPos.x-GetPosition().x, fire->m_vecPos.y-GetPosition().y);
		float fwdAngle = General_GetATanOfXY(GetForward().x, GetForward().y);
		float targetCannonAngle = fwdAngle - targetAngle;
		float angleDelta = (*Timer_GetTimeStep())*0.01f;
		float cannonDelta = targetCannonAngle - m_fCarGunLR;
		// Delegate the angle wrapping to the C implementation
		cannonDelta = General_RaiseBelow(cannonDelta, PI, TWOPI);
		cannonDelta = General_ReduceAbove(cannonDelta, PI, TWOPI);
		// Delegate the turning of the cannon towards the fire to the C implementation
		m_fCarGunLR = Automobile_CannonTrackStep(m_fCarGunLR, targetCannonAngle, cannonDelta, angleDelta);

		// Go up and down a bit
		// Delegate the vertical angle of the cannon to the C implementation
		m_fCarGunUD = Automobile_AIFireTruckGunUD((*Timer_GetTimeInMilliseconds()));

		// Spray water every once in a while
		if(((*Timer_GetTimeInMilliseconds())>>10) & 3){
			CVector cannonPos(0.0f, 0.0f, 2.2f);	// different position than player's firetruck!
			cannonPos = GetMatrix() * cannonPos;
			// Delegate the direction of the cannon to the C implementation
			CVector cannonDirC;
			Automobile_CannonDirection(m_fCarGunLR, m_fCarGunUD, &cannonDirC);
			CVector cannonDir(cannonDirC.x, cannonDirC.y, cannonDirC.z);
			cannonDir = Multiply3x3(GetMatrix(), cannonDir);
			cannonDir.z += (General_GetRandomNumber()&0xF)/1000.0f;
			// Enter the C water-cannon algorithm with the original owner expressions
			WaterCannons_UpdateOneLegacy((uintptr)this, &cannonPos, &cannonDir);
		}
	}
}

void
CAutomobile::TankControl(void)
{
	int i;

	// These coords are 1 unit higher then they should be relative to model center
	CVector turrentBase(0.0f, -1.394f, 2.296f);
	CVector gunEnd(0.0f, 1.813f, 2.979f);
	CVector baseToEnd = gunEnd - turrentBase;

	if(this != FindPlayerVehicle())
		return;
	if(World_Players[World_PlayerInFocus].m_WBState != WBSTATE_PLAYING)
		return;

	// Rotate turret
	float prevAngle = m_fCarGunLR;
#ifdef FREE_CAM
	if(!CCamera::bFreeCam)
#endif
		m_fCarGunLR -= CPad::GetPad(0)->GetCarGunLeftRight() * 0.00015f * (*Timer_GetTimeStep());

	// Delegate the wrapping of the angle of the turret to the C implementation
	m_fCarGunLR = Automobile_TankTurretAngle(m_fCarGunLR);
	if(m_fCarGunLR != prevAngle)
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_TANK_TURRET_ROTATE, Abs(m_fCarGunLR - prevAngle));

	// Shoot
	if(CPad::GetPad(0)->CarGunJustDown() &&
	   (*Timer_GetTimeInMilliseconds()) > World_Players[World_PlayerInFocus].m_nTimeTankShotGun + 800){
		World_Players[World_PlayerInFocus].m_nTimeTankShotGun = (*Timer_GetTimeInMilliseconds());

		// more like -sin(angle), cos(angle), i.e. rotated (0,1,0)
		// Delegate the direction of the turret to the C implementation
		float turretXY[2];
		Automobile_TurretDirection(m_fCarGunLR, turretXY);
		CVector turretDir = CVector(turretXY[0], turretXY[1], 0.0f);
		turretDir = Multiply3x3(GetMatrix(), turretDir);

		// Delegate the end of the gun to the C implementation
		CVector rotatedEndC;
		Automobile_TankGunEnd(m_fCarGunLR, &baseToEnd, &turrentBase, &rotatedEndC);
		CVector rotatedEnd(rotatedEndC.x, rotatedEndC.y, rotatedEndC.z);

		CVector point1 = GetMatrix() * rotatedEnd;
		CVector point2 = point1 + 60.0f*turretDir;
		m_vecMoveSpeed -= 0.06f*turretDir;
		m_vecMoveSpeed.z += 0.05f;

		CWeapon::DoTankDoomAiming(FindPlayerVehicle(), FindPlayerPed(), &point1, &point2);
		CColPoint colpoint;
		CEntity *entity = nil;
		World_ProcessLineOfSight(point1, point2, colpoint, entity, true, true, true, true, true, true, false);
		if(entity)
			point2 = CollisionVector_ToNative(colpoint.point) - 0.04f*(CollisionVector_ToNative(colpoint.point) - point1);

		Explosion_AddExplosion(nil, FindPlayerPed(), EXPLOSION_TANK_GRENADE, point2, 0);

		// Add particles on the way to the explosion;
		float shotDist = (point2 - point1).Magnitude();
		int n = shotDist/4.0f;
		RwRGBA black = { 0, 0, 0, 0 };
		for(i = 0; i < n; i++){
			float f = (float)i/n;
			CParticle::AddParticle(PARTICLE_HELI_DUST,
				point1 + f*(point2 - point1),
				CVector(0.0f, 0.0f, 0.0f),
				nil, 0.1f, black);
		}

		// More particles
		CVector shotDir = point2 - point1;
		shotDir.Normalise();
		for(i = 0; i < 15; i++){
			float f = i/15.0f;
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, point1,
				shotDir*General_GetRandomNumberInRangeFloat(0.3f, 1.0f)*f,
				nil, General_GetRandomNumberInRangeFloat(0.5f, 1.5f)*f, black);
		}

		// And some gun flashes near the gun
		CVector flashPos = point1;
		CVector nullDir(0.0f, 0.0f, 0.0f);
		int lifeSpan = 250;
		if(m_vecMoveSpeed.Magnitude() > 0.08f){
			lifeSpan = 125;
			flashPos.x += 5.0f*m_vecMoveSpeed.x;
			flashPos.y += 5.0f*m_vecMoveSpeed.y;
		}
		CParticle::AddParticle(PARTICLE_GUNFLASH, flashPos, nullDir, nil, 0.4f, black, 0, 0, 0, lifeSpan);
		flashPos += 0.3f*shotDir;
		CParticle::AddParticle(PARTICLE_GUNFLASH, flashPos, nullDir, nil, 0.2f, black, 0, 0, 0, lifeSpan);
		flashPos += 0.1f*shotDir;
		CParticle::AddParticle(PARTICLE_GUNFLASH, flashPos, nullDir, nil, 0.15f, black, 0, 0, 0, lifeSpan);
	}
}

#define HYDRAULIC_UPPER_EXT (-0.16f)
#define HYDRAULIC_LOWER_EXT (0.16f)

void
CAutomobile::HydraulicControl(void)
{
	int i;
	float wheelPositions[4];
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)
	CColModel *normalColModel = mi->GetColModel();
	float wheelRadius = 0.5f*mi->m_wheelScale;
	CPlayerInfo *playerInfo = &World_Players[World_PlayerInFocus];
	CColModel *specialColModel = &playerInfo->m_ColModel;

	if(GetStatus() != STATUS_PLAYER){
		// reset hydraulics for non-player cars

		if(!bUsingSpecialColModel)
			return;
		if(specialColModel != nil)	// this is always true
			for(i = 0; i < 4; i++)
				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
		for(i = 0; i < 4; i++){
			m_aSuspensionSpringLength[i] = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
			m_aSuspensionLineLength[i] = normalColModel->lines[i].p0.z - normalColModel->lines[i].p1.z;
			m_aSuspensionSpringRatio[i] = (normalColModel->lines[i].p0.z - wheelPositions[i]) / m_aSuspensionLineLength[i];
			if(m_aSuspensionSpringRatio[i] > 1.0f)
				m_aSuspensionSpringRatio[i] = 1.0f;
		}

		if(m_hydraulicState == 0)
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
		else if(m_hydraulicState >= 100)
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);

		if(playerInfo->m_pVehicleEx == this)
			playerInfo->m_pVehicleEx = nil;
		bUsingSpecialColModel = false;
		m_hydraulicState = 0;
		return;
	}

	// Player car

	float normalUpperLimit = pHandling->fSuspensionUpperLimit;
	float normalLowerLimit = pHandling->fSuspensionLowerLimit;
	float normalSpringLength = normalUpperLimit - normalLowerLimit;
	float extendedUpperLimit = normalUpperLimit - 0.2f;
	float extendedLowerLimit = normalLowerLimit - 0.2f;
	float extendedSpringLength = extendedUpperLimit - extendedLowerLimit;

	if(!bUsingSpecialColModel){
		// Init special col model

		if(playerInfo->m_pVehicleEx && playerInfo->m_pVehicleEx == this)
			playerInfo->m_pVehicleEx->bUsingSpecialColModel = false;
		playerInfo->m_pVehicleEx = this;
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_Assign(&playerInfo->m_ColModel,normalColModel);
		bUsingSpecialColModel = true;
		specialColModel = &playerInfo->m_ColModel;

		if(m_fVelocityChangeForAudio > 0.1f)
			m_hydraulicState = 20;
		else{
			m_hydraulicState = 0;
			normalUpperLimit += HYDRAULIC_UPPER_EXT;
			normalSpringLength = normalUpperLimit - (normalLowerLimit+HYDRAULIC_LOWER_EXT);
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}

		// Setup suspension
		float normalLineLength = normalSpringLength + wheelRadius;
		CVector pos;
		for(i = 0; i < 4; i++){
			wheelPositions[i] = normalColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
			mi->GetWheelPosn(i, pos);
			pos.z += normalUpperLimit;
			specialColModel->lines[i].p0 = CollisionVector_FromNative(pos);
			pos.z -= normalLineLength;
			specialColModel->lines[i].p1 = CollisionVector_FromNative(pos);
			m_aSuspensionSpringLength[i] = normalSpringLength;
			m_aSuspensionLineLength[i] = normalLineLength;

			// Delegate the update of the spring ratio to the C implementation
			m_aSuspensionSpringRatio[i] = Automobile_HydraulicSpringRatio(m_aSuspensionSpringRatio[i], specialColModel->lines[i].p0.z, wheelPositions[i], m_aSuspensionLineLength[i]);
		}
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);

		// Adjust col model
		mi->GetWheelPosn(0, pos);
		float minz = pos.z + extendedLowerLimit - wheelRadius;
		if(minz < specialColModel->boundingBox.min.z)
			specialColModel->boundingBox.min.z = minz;
		// Delegate the radius of the vehicle to the C implementation
		CVector radiusMin = CollisionVector_ToNative(specialColModel->boundingBox.min);
		CVector radiusMax = CollisionVector_ToNative(specialColModel->boundingBox.max);
		float radius = VehicleLogic_BoundingRadius(&radiusMin, &radiusMax);
		if(specialColModel->boundingSphere.radius < radius)
			specialColModel->boundingSphere.radius = radius;
		return;
	}

	if(playerInfo->m_WBState != WBSTATE_PLAYING)
		return;

	bool setPrevRatio = false;
	if(m_hydraulicState < 20 && m_fVelocityChangeForAudio > 0.2f){
		if(m_hydraulicState == 0){
			m_hydraulicState = 20;
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
			setPrevRatio = true;
		}else{
			m_hydraulicState++;
		}
	}else if(m_hydraulicState != 0){	// must always be true
		if(m_hydraulicState < 21 && m_fVelocityChangeForAudio < 0.1f){
			m_hydraulicState--;
			if(m_hydraulicState == 0)
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}
	}

	if(CPad::GetPad(0)->HornJustDown()){
		// Switch between normal and extended

		if(m_hydraulicState < 100)
			m_hydraulicState = 100;
		else{
			if(m_fVelocityChangeForAudio > 0.1f)
				m_hydraulicState = 20;
			else
				m_hydraulicState = 0;
		}

		if(m_hydraulicState < 100){
			if(m_hydraulicState == 0){
				normalUpperLimit += HYDRAULIC_UPPER_EXT;
				normalLowerLimit += HYDRAULIC_LOWER_EXT;
				normalSpringLength = normalUpperLimit - normalLowerLimit;
			}

			// Reset suspension to normal
			float normalLineLength = normalSpringLength + wheelRadius;
			CVector pos;
			for(i = 0; i < 4; i++){
				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += normalUpperLimit;
				specialColModel->lines[i].p0 = CollisionVector_FromNative(pos);
				pos.z -= normalLineLength;
				specialColModel->lines[i].p1 = CollisionVector_FromNative(pos);
				m_aSuspensionSpringLength[i] = normalSpringLength;
				m_aSuspensionLineLength[i] = normalLineLength;

				// Delegate the update of the spring ratio to the C implementation
				m_aSuspensionSpringRatio[i] = Automobile_HydraulicSpringRatio(m_aSuspensionSpringRatio[i], specialColModel->lines[i].p0.z, wheelPositions[i], m_aSuspensionLineLength[i]);
			}
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}else{
			// Reset suspension to extended
			float extendedLineLength = extendedSpringLength + wheelRadius;
			CVector pos;
			for(i = 0; i < 4; i++){
				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += extendedUpperLimit;
				specialColModel->lines[i].p0 = CollisionVector_FromNative(pos);
				pos.z -= extendedLineLength;
				specialColModel->lines[i].p1 = CollisionVector_FromNative(pos);
				m_aSuspensionSpringLength[i] = extendedSpringLength;
				m_aSuspensionLineLength[i] = extendedLineLength;

				// Delegate the update of the spring ratio to the C implementation
				m_aSuspensionSpringRatio[i] = Automobile_HydraulicSpringRatio(m_aSuspensionSpringRatio[i], specialColModel->lines[i].p0.z, wheelPositions[i], m_aSuspensionLineLength[i]);

				setPrevRatio = true;
			}
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
		}
	}else{
		float suspChange[4];
		float maxDelta = 0.0f;

		// Delegate the lift of the wheels to the C implementation
		Automobile_HydraulicWheelLift(CPad::GetPad(0)->GetCarGunUpDown(), CPad::GetPad(0)->GetCarGunLeftRight(), suspChange);

		if(m_hydraulicState < 100){
			// Lowered, move wheels up

			if(m_hydraulicState == 0){
				normalUpperLimit += HYDRAULIC_UPPER_EXT;
				normalLowerLimit += HYDRAULIC_LOWER_EXT;
				normalSpringLength = normalUpperLimit - normalLowerLimit;
			}

			// Set suspension
			CVector pos;
			for(i = 0; i < 4; i++){
				if(suspChange[i] > 1.0f)
					suspChange[i] = 1.0f;

				float upperLimit = suspChange[i]*(extendedUpperLimit-normalUpperLimit) + normalUpperLimit;
				float springLength = suspChange[i]*(extendedSpringLength-normalSpringLength) + normalSpringLength;
				float lineLength = springLength + wheelRadius;

				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += upperLimit;
				specialColModel->lines[i].p0 = CollisionVector_FromNative(pos);
				pos.z -= lineLength;
				if(Automobile_LineDeltaExceeds(pos.z, specialColModel->lines[i].p1.z, maxDelta))
					maxDelta = pos.z - specialColModel->lines[i].p1.z;
				specialColModel->lines[i].p1 = CollisionVector_FromNative(pos);
				m_aSuspensionSpringLength[i] = springLength;
				m_aSuspensionLineLength[i] = lineLength;

				// Delegate the update of the spring ratio to the C implementation
				m_aSuspensionSpringRatio[i] = Automobile_HydraulicSpringRatio(m_aSuspensionSpringRatio[i], specialColModel->lines[i].p0.z, wheelPositions[i], m_aSuspensionLineLength[i]);
			}
		}else{
			if(m_hydraulicState < 104)
				m_hydraulicState++;

			if(m_fVelocityChangeForAudio < 0.1f){
				normalUpperLimit += HYDRAULIC_UPPER_EXT;
				normalLowerLimit += HYDRAULIC_LOWER_EXT;
				normalSpringLength = normalUpperLimit - normalLowerLimit;
			}

			// Set suspension
			CVector pos;
			for(i = 0; i < 4; i++){
				if(suspChange[i] > 1.0f)
					suspChange[i] = 1.0f;

				float upperLimit = suspChange[i]*(normalUpperLimit-extendedUpperLimit) + extendedUpperLimit;
				float springLength = suspChange[i]*(normalSpringLength-extendedSpringLength) + extendedSpringLength;
				float lineLength = springLength + wheelRadius;

				wheelPositions[i] = specialColModel->lines[i].p0.z - m_aSuspensionSpringRatio[i]*m_aSuspensionLineLength[i];
				mi->GetWheelPosn(i, pos);
				pos.z += upperLimit;
				specialColModel->lines[i].p0 = CollisionVector_FromNative(pos);
				pos.z -= lineLength;
				if(Automobile_LineDeltaExceeds(pos.z, specialColModel->lines[i].p1.z, maxDelta))
					maxDelta = pos.z - specialColModel->lines[i].p1.z;
				specialColModel->lines[i].p1 = CollisionVector_FromNative(pos);
				m_aSuspensionSpringLength[i] = springLength;
				m_aSuspensionLineLength[i] = lineLength;

				// Delegate the update of the spring ratio to the C implementation
				m_aSuspensionSpringRatio[i] = Automobile_HydraulicSpringRatio(m_aSuspensionSpringRatio[i], specialColModel->lines[i].p0.z, wheelPositions[i], m_aSuspensionLineLength[i]);
			}
		}

		float limitDiff = extendedLowerLimit - normalLowerLimit;
		float f;

		// Delegate the loudness of the hydraulics to the C implementation
		if(Automobile_HydraulicSoundLevel(maxDelta, limitDiff, &f)){
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_3, f);
			if(f < 0.4f || f > 0.6f)
				setPrevRatio = true;
			if(f < 0.25f)
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_2, 0.0f);
			else if(f > 0.75f)
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_HYDRAULIC_1, 0.0f);
		}
	}

	if(setPrevRatio)
		for(i = 0; i < 4; i++){
			// Delegate the previous spring ratio to the C implementation
			m_aSuspensionSpringRatioPrev[i] = Automobile_PrevSpringRatio(m_aSuspensionSpringRatio[i], m_aSuspensionSpringLength[i], m_aSuspensionLineLength[i]);
		}
}

void
CAutomobile::ProcessBuoyancy(void)	
{
	int i;
	CVector impulse, point;

	if(mod_Buoyancy.ProcessBuoyancy(this, m_fBuoyancy, &point, &impulse)){
		bTouchingWater = true;
		// Delegate the ratio of the impulse and the water resistance to the C implementation
		float impulseRatio = Automobile_WaterImpulseRatio(impulse.z, m_fMass, (*Timer_GetTimeStep()));
		float waterResistance = Automobile_WaterResistance(impulseRatio, (*Timer_GetTimeStep()));
		m_vecMoveSpeed *= waterResistance;
		m_vecTurnSpeed *= waterResistance;

		bool heliHitWaterHard = false;
		if(IsRealHeli() && m_aWheelSpeed[1] > 0.15f){
			if(GetModelIndex() == MI_SEASPAR){
				if(impulseRatio > 3.0f){
					m_aWheelSpeed[1] = 0.0f;
					heliHitWaterHard = true;
				}
			}else{
				// Delegate the divisor of the impulse to the C implementation
				float strength = Automobile_WaterImpactDivisor(impulseRatio);
				ApplyMoveForce(-2.0f*impulse/strength);
				ApplyTurnForce(-impulse/strength, point);
				if(impulseRatio > 0.9f){
					m_aWheelSpeed[1] = 0.0f;
					heliHitWaterHard = true;
				}else
					return;
			}
		}

		bTouchingWater = true;
		ApplyMoveForce(impulse);
		ApplyTurnForce(impulse, point);
		CVector initialSpeed = m_vecMoveSpeed;

		// Delegate the test for the drowning to the C implementation
		if(Automobile_IsDrowning(m_modelIndex == MI_SEASPAR, impulseRatio, GetUp().z, CVehicle::bHoverCheat && GetStatus() == STATUS_PLAYER && GetUp().z > 0.1f, heliHitWaterHard,
			m_aSuspensionSpringRatio[0] == 1.0f || m_aSuspensionSpringRatio[1] == 1.0f || m_aSuspensionSpringRatio[2] == 1.0f || m_aSuspensionSpringRatio[3] == 1.0f)){
			bIsInWater = true;
			bIsDrowning = true;
			if(m_vecMoveSpeed.z < -0.1f)
				m_vecMoveSpeed.z = -0.1f;

			if(pDriver){
				pDriver->bIsInWater = true;
				if(pDriver->IsPlayer() || !bWaterTight)
					pDriver->InflictDamage(nil, WEAPONTYPE_DROWNING, (*Timer_GetTimeStep()), PEDPIECE_TORSO, 0);
			}
			for(i = 0; i < m_nNumMaxPassengers; i++)
				if(pPassengers[i]){
					pPassengers[i]->bIsInWater = true;
					if(pPassengers[i]->IsPlayer() || !bWaterTight)
						pPassengers[i]->InflictDamage(nil, WEAPONTYPE_DROWNING, (*Timer_GetTimeStep()), PEDPIECE_TORSO, 0);
				}
		}else{
			bIsInWater = false;
			bIsDrowning = false;
		}
		static uint32 nGenerateRaindrops = 0;
		static uint32 nGenerateWaterCircles = 0;

		if(initialSpeed.z < -0.1f && impulse.z > 0.3f || heliHitWaterHard){
			RwRGBA color;
			// Delegate the colour channels of the splash to the C implementation
			color.red = Automobile_WaterSplashChannel(TimeCycle_GetDirectionalRed(), TimeCycle_GetAmbientRed_Obj());
			color.green = Automobile_WaterSplashChannel(TimeCycle_GetDirectionalGreen(), TimeCycle_GetAmbientGreen_Obj());
			color.blue = Automobile_WaterSplashChannel(TimeCycle_GetDirectionalBlue(), TimeCycle_GetAmbientBlue_Obj());
			color.alpha = General_GetRandomNumberInRangeInt(0, 32) + 128;
			CVector target = CVector(0.0f, 0.0f, General_GetRandomNumberInRangeFloat(0.15f, 0.45f));
			CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, GetPosition(),
				target, 0.0f, 75, color, true);

			nGenerateRaindrops = (*Timer_GetTimeInMilliseconds()) + 300;
			nGenerateWaterCircles = (*Timer_GetTimeInMilliseconds()) + 60;

			if(heliHitWaterHard){
				CVector right = CrossProduct(GetForward(), CVector(0.0f, 0.0f, 1.0f));
				CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, GetPosition() + right,
					target, 0.0f, 75, color, true);
				CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, GetPosition() - right,
					target, 0.0f, 75, color, true);
			}

			if(m_vecMoveSpeed.z < -0.2f)
				m_vecMoveSpeed.z = -0.2f;
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_WATER_FALL, 0.0f);
		}

		if(nGenerateWaterCircles > 0 && nGenerateWaterCircles <= (*Timer_GetTimeInMilliseconds())){
			CVector pos = GetPosition();
			float waterLevel = 0.0f;
			if(WaterLevel_GetWaterLevel(pos.x, pos.y, pos.z, &waterLevel, false))
				pos.z = waterLevel;
			static RwRGBA black;
			if(pos.z != 0.0f){
				nGenerateWaterCircles = 0;
				pos.z += 1.0f;
				for(i = 0; i < 4; i++){
					CVector p = pos;
					p.x += General_GetRandomNumberInRangeFloat(-2.5f, 2.5f);
					p.y += General_GetRandomNumberInRangeFloat(-2.5f, 2.5f);
					CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW,
						p, CVector(0.0f, 0.0f, 0.0f),
						nil, 0.0f, black, 0, 0, 0, 0);
				}
			}
		}

		if(nGenerateRaindrops > 0 && nGenerateRaindrops <= (*Timer_GetTimeInMilliseconds())){
			CVector pos = GetPosition();
			float waterLevel = 0.0f;
			if(WaterLevel_GetWaterLevel(pos.x, pos.y, pos.z, &waterLevel, false))
				pos.z = waterLevel;
			static RwRGBA black;
			if(pos.z >= 0.0f){
				nGenerateRaindrops = 0;
				pos.z += 0.5f;
				CParticleObject::AddObject(POBJECT_SPLASHES_AROUND,
					pos, CVector(0.0f, 0.0f, 0.0f), 6.5f, 2500, black, true);
			}
		}
	}else{
		bIsInWater = false;
		bIsDrowning = false;
		bTouchingWater = false;

		static RwRGBA splashCol = {155, 155, 185, 196};
		static RwRGBA smokeCol = {255, 255, 255, 255};

		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f && m_aWheelColPoints[i].surfaceB == SURFACE_WATER){
				CVector pos = CollisionVector_ToNative(m_aWheelColPoints[i].point) + 0.3f*GetUp() - GetPosition();
				CVector vSpeed = GetSpeed(pos);
				vSpeed.z = 0.0f;
				float fSpeed = vSpeed.MagnitudeSqr();
				if(fSpeed > sq(0.05f)){
					fSpeed = Sqrt(fSpeed);

					float size = Automobile_WheelSplashSize(fSpeed);
					CVector right = 0.2f*fSpeed*GetRight() + 0.2f*vSpeed;

					CParticle::AddParticle(PARTICLE_PED_SPLASH,
						pos + GetPosition(), -0.5f*right,
						nil, size, splashCol,
						General_GetRandomNumberInRangeFloat(0.0f, 10.0f),
						General_GetRandomNumberInRangeFloat(0.0f, 90.0f), 1, 0);

					CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
						pos + GetPosition(), -0.6f*right,
						nil, size, smokeCol, 0, 0, 0, 0);
				
					if(((*Timer_GetFrameCounter()) & 0xF) == 0)
						DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_SPLASH, 2000.0f*fSpeed);
				}
			}
		}
	}
}

void
CAutomobile::DoDriveByShootings(void)
{
	CAnimBlendAssociation *anim = nil;
	CPlayerInfo* playerInfo = ((CPlayerPed*)pDriver)->GetPlayerInfoForThisPlayerPed();
	if (playerInfo && !playerInfo->m_bDriveByAllowed)
		return;

	CWeapon *weapon = pDriver->GetWeapon();
	if(WeaponInfo_GetWeaponInfo(weapon->m_eWeaponType)->m_nWeaponSlot != WEAPONSLOT_SUBMACHINEGUN)
		return;

	weapon->Update(pDriver->m_audioEntityId, nil);

	bool lookingLeft = false;
	bool lookingRight = false;
	if(TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_TOPDOWN ||
	   TheCamera.m_bObbeCinematicCarCamOn){
		if(CPad::GetPad(0)->GetLookLeft())
			lookingLeft = true;
		if(CPad::GetPad(0)->GetLookRight())
			lookingRight = true;
	}else{
		if(TheCamera.Cams[TheCamera.ActiveCam].LookingLeft)
			lookingLeft = true;
		if(TheCamera.Cams[TheCamera.ActiveCam].LookingRight)
			lookingRight = true;
	}

	AnimationId rightAnim = ANIM_STD_CAR_DRIVEBY_RIGHT;
	AnimationId leftAnim = ANIM_STD_CAR_DRIVEBY_LEFT;
	if (pDriver->m_pMyVehicle->bLowVehicle) {
		rightAnim = ANIM_STD_CAR_DRIVEBY_RIGHT_LO;
		leftAnim = ANIM_STD_CAR_DRIVEBY_LEFT_LO;
	}

	if(lookingLeft || lookingRight){
		if(lookingLeft){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), rightAnim);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), leftAnim);
			if(anim == nil || anim->blendDelta < 0.0f)
				// Use the complete C animation manager at the original call boundary
				anim = AnimManager_AddAnimation(pDriver->GetClump(), ASSOCGRP_STD, leftAnim);
		}else if(pDriver->m_pMyVehicle->pPassengers[0] == nil || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_1STPERSON){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), leftAnim);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), rightAnim);
			if(anim == nil || anim->blendDelta < 0.0f)
				// Use the complete C animation manager at the original call boundary
				anim = AnimManager_AddAnimation(pDriver->GetClump(), ASSOCGRP_STD, rightAnim);
		}

		// Use the complete C association at the original native operation boundary
		if (!anim || !AnimAssociation_IsRunning(anim)) {
			if (CPad::GetPad(0)->GetCarGunFired() && (*Timer_GetTimeInMilliseconds()) > weapon->m_nTimer) {
				weapon->FireFromCar(this, lookingLeft, true);
				weapon->m_nTimer = (*Timer_GetTimeInMilliseconds()) + 70;
			}
		}
	}else{
		weapon->Reload();
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), leftAnim);
		if(anim)
			anim->blendDelta = -1000.0f;
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), rightAnim);
		if(anim)
			anim->blendDelta = -1000.0f;
	}

	// TODO: what is this?
	if(!lookingLeft && m_weaponDoorTimerLeft > 0.0f){
		// Delegate the countdown of the timer to the C implementation
		m_weaponDoorTimerLeft = VehicleLogic_DecayToZero(m_weaponDoorTimerLeft, (*Timer_GetTimeStep())*0.1f);
		ProcessOpenDoor(CAR_DOOR_LF, ANIM_STD_NUM, m_weaponDoorTimerLeft);
	}
	if(!lookingRight && m_weaponDoorTimerRight > 0.0f){
		// Delegate the countdown of the timer to the C implementation
		m_weaponDoorTimerRight = VehicleLogic_DecayToZero(m_weaponDoorTimerRight, (*Timer_GetTimeStep())*0.1f);
		ProcessOpenDoor(CAR_DOOR_RF, ANIM_STD_NUM, m_weaponDoorTimerRight);
	}
}

void
CAutomobile::DoHoverSuspensionRatios(void)
{
	int i;

	if(GetUp().z < 0.1f)
		return;

	CColModel *colmodel = GetColModel();
	for(i = 0; i < 4; i++){
		float waterZ;
		CVector upper = GetMatrix() * CollisionVector_ToNative(colmodel->lines[i].p0);
		CVector lower = GetMatrix() * CollisionVector_ToNative(colmodel->lines[i].p1);
		// see if touching water, the wheel is changed in C
		if(WaterLevel_GetWaterLevel(lower, &waterZ, false))
			Automobile_HoverWheelInWater(this, i, &upper.x, &lower.x, waterZ);
	}
}

int32
CAutomobile::RcbanditCheckHitWheels(void)
{
	int x, xmin, xmax;
	int y, ymin, ymax;

	// Delegate the range of sectors to the C implementation
	World_SectorRange(GetPosition().x - 2.0f, GetPosition().y - 2.0f, GetPosition().x + 2.0f, GetPosition().y + 2.0f, &xmin, &ymin, &xmax, &ymax);

	World_AdvanceCurrentScanCode();

	for(y = ymin; y <= ymax; y++)
		for(x = xmin; x <= xmax; x++){
			CSector *s = World_GetSector(x, y);
			if(RcbanditCheck1CarWheels(s->m_lists[ENTITYLIST_VEHICLES]) ||
			   RcbanditCheck1CarWheels(s->m_lists[ENTITYLIST_VEHICLES_OVERLAP]))
				return 1;
		}
	return 0;
}

int32
CAutomobile::RcbanditCheck1CarWheels(CPtrList &list)
{
	static CMatrix matW2B;
	int i;
	CPtrNode *node;
	CAutomobile *car;
	CColModel *colModel = GetColModel();
	CVehicleModelInfo *mi;

	for(node = list.first; node; node = node->next){
		car = (CAutomobile*)node->item;
		if(this != car && car->IsCar() && car->GetModelIndex() != MI_RCBANDIT &&
		   car->m_scanCode != World_GetCurrentScanCode()){
			car->m_scanCode = World_GetCurrentScanCode();

			// Delegate the check that the other car is close to the C implementation
			if(VehicleLogic_WithinSquare(this->GetPosition().x, this->GetPosition().y, car->GetPosition().x, car->GetPosition().y, 10.0f)){
				mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(car->GetModelIndex())); // rouz edit (ChatGPT)

				for(i = 0; i < 4; i++){
					if(car->m_aSuspensionSpringRatioPrev[i] < 1.0f || car->GetStatus() == STATUS_SIMPLE){
						CVector wheelPos;
						CColSphere sph;
						mi->GetWheelPosn(i, wheelPos);
						matW2B = Invert(GetMatrix());
						sph.bounds.center = CollisionVector_FromNative(matW2B * (car->GetMatrix() * wheelPos));
						sph.bounds.radius = mi->m_wheelScale*0.25f;
						if(Collision_TestSphereBox(sph.bounds, colModel->boundingBox))
							return 1;
					}
				}
			}
		}
	}
	return 0;
}

void
CAutomobile::PlaceOnRoadProperly(void)
{
	CColPoint point;
	CEntity *entity;
	CColModel *colModel = GetColModel();
	float lenFwd, lenBack;
	float frontZ, rearZ;

	lenFwd = colModel->boundingBox.max.y;
	lenBack = -colModel->boundingBox.min.y;

	CVector front(GetPosition().x + GetForward().x*lenFwd,
	              GetPosition().y + GetForward().y*lenFwd,
	              GetPosition().z + 5.0f);
	if(World_ProcessVerticalLine(front, GetPosition().z - 5.0f, point, entity,
			true, false, false, false, false, false, nil)){
		frontZ = point.point.z;
		m_pCurGroundEntity = entity;
	}else{
		frontZ = m_fMapObjectHeightAhead;
	}

	CVector rear(GetPosition().x - GetForward().x*lenBack,
	             GetPosition().y - GetForward().y*lenBack,
	             GetPosition().z + 5.0f);
	if(World_ProcessVerticalLine(rear, GetPosition().z - 5.0f, point, entity,
			true, false, false, false, false, false, nil)){
		rearZ = point.point.z;
		m_pCurGroundEntity = entity;
	}else{
		rearZ = m_fMapObjectHeightBehind;
	}

	float len = lenFwd + lenBack;
	// Delegate the orientation and position of the car on the road to the C implementation
	const float frontXY[2] = { front.x, front.y };
	const float rearXY[2] = { rear.x, rear.y };
	CVector rightC;
	CVector forwardC;
	CVector upC;
	CVector posC;
	Automobile_RoadAlignedMatrix(frontXY, rearXY, frontZ, rearZ, len, GetHeightAboveRoad(), &rightC, &forwardC, &upC, &posC);
	GetMatrix().GetRight() = CVector(rightC.x, rightC.y, rightC.z);
	GetMatrix().GetForward() = CVector(forwardC.x, forwardC.y, forwardC.z);
	GetMatrix().GetUp() = CVector(upC.x, upC.y, upC.z);
	GetMatrix().GetPosition() = CVector(posC.x, posC.y, posC.z);
}

void
CAutomobile::VehicleDamage(float impulse, uint16 damagedPiece)
{
	int i;
	float damageMultiplier = 0.333f;

	if(impulse == 0.0f){
		impulse = m_fDamageImpulse;
		damagedPiece = m_nDamagePieceType;
		damageMultiplier = 1.0f;
	}

	if(GetStatus() == STATUS_PLAYER && Stats_GetPercentageProgress() >= 100.0f)
		impulse *= 0.5f;

	CVector pos(0.0f, 0.0f, 0.0f);

	if(!bCanBeDamaged)
		return;

	if(m_pDamageEntity && m_pDamageEntity->IsPed() && ((CPed*)m_pDamageEntity)->bIsStanding){
		// Delegate the impulse of the ped to the C implementation
		impulse = Automobile_PedImpulseReduction(impulse, ((CPed*)m_pDamageEntity)->m_vecAnimMoveDelta.y, DotProduct(GetForward(), m_vecDamageNormal), ((CPed*)m_pDamageEntity)->m_fMass);
	}

	// damage flipped over car
	if(GetUp().z < 0.0f && this != FindPlayerVehicle()){
		if(bNotDamagedUpsideDown || GetStatus() == STATUS_PLAYER_REMOTE || bIsInWater)
			return;
		if(GetStatus() != STATUS_WRECKED)
			// Delegate the loss of health to the C implementation
			m_fHealth = VehicleLogic_DecayToZero(m_fHealth, 4.0f*(*Timer_GetTimeStep()));
	}

	float minImpulse = GetModelIndex() == MI_RCRAIDER || GetModelIndex() == MI_RCGOBLIN ? 1.0f : 25.0f;
	if(impulse > minImpulse && GetStatus() != STATUS_WRECKED){
		if(bIsLawEnforcer &&
		   FindPlayerVehicle() && FindPlayerVehicle() == m_pDamageEntity &&
		   GetStatus() != STATUS_ABANDONED &&
		   FindPlayerVehicle()->m_vecMoveSpeed.Magnitude() >= m_vecMoveSpeed.Magnitude() &&
		   FindPlayerVehicle()->m_vecMoveSpeed.Magnitude() > 0.1f)
			FindPlayerPed()->SetWantedLevelNoDrop(1);

		if(GetStatus() == STATUS_PLAYER && impulse > 50.0f){
			// Delegate the frequency of the shake to the C implementation
			uint8 freq = Automobile_ShakeFrequency(impulse, m_fMass);
			CPad::GetPad(0)->StartShake(40000/freq, freq);
		}

		if(GetStatus() != STATUS_PLAYER && bOnlyDamagedByPlayer){
			if(m_pDamageEntity != FindPlayerPed() &&
			   m_pDamageEntity != FindPlayerVehicle())
				return;
		}

		if(m_pDamageEntity && m_pDamageEntity->IsVehicle()){
			m_nLastWeaponDamage = WEAPONTYPE_RAMMEDBYCAR;
			m_pLastDamageEntity = m_pDamageEntity;
		}

		if(bCollisionProof)
			return;

		if(m_pDamageEntity){
			if(m_pDamageEntity->IsBuilding() &&
			   DotProduct(m_vecDamageNormal, GetUp()) > 0.6f)
				return;
		}

		int oldLightStatus[4];
		for(i = 0; i < 4; i++)
			oldLightStatus[i] = Damage_GetLightStatus(&Damage, (eLights)i);

		if(GetUp().z > 0.0f || m_vecMoveSpeed.MagnitudeSqr() > 0.1f){
			float impulseMult = bMoreResistantToDamage ? 0.5f : 4.0f;

			switch(damagedPiece){
			case CAR_PIECE_BUMP_FRONT:
				GetComponentWorldPosition(CAR_BUMP_FRONT, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_BUMPER_FRONT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetBumperDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT);
				if(m_aCarNodes[CAR_BONNET] && Damage_GetPanelStatus(&Damage, VEHBUMPER_FRONT) == PANEL_STATUS_MISSING){
			case CAR_PIECE_BONNET:
					GetComponentWorldPosition(CAR_BONNET, pos);
					dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
					if(GetModelIndex() != MI_DODO)
					if(Damage_ApplyDamage(&Damage, COMPONENT_DOOR_BONNET, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
						SetDoorDamage(CAR_BONNET, DOOR_BONNET);
				}
				break;

			case CAR_PIECE_BUMP_REAR:
				GetComponentWorldPosition(CAR_BUMP_REAR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_BUMPER_REAR, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetBumperDamage(CAR_BUMP_REAR, VEHBUMPER_REAR);
				if(m_aCarNodes[CAR_BOOT] && Damage_GetPanelStatus(&Damage, VEHBUMPER_REAR) == PANEL_STATUS_MISSING){
			case CAR_PIECE_BOOT:
					GetComponentWorldPosition(CAR_BOOT, pos);
					dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
					if(Damage_ApplyDamage(&Damage, COMPONENT_DOOR_BOOT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
						SetDoorDamage(CAR_BOOT, DOOR_BOOT);
				}
				break;

			case CAR_PIECE_DOOR_LF:
				GetComponentWorldPosition(CAR_DOOR_LF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_DOOR_FRONT_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT);
				break;
			case CAR_PIECE_DOOR_RF:
				GetComponentWorldPosition(CAR_DOOR_RF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_DOOR_FRONT_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
				break;
			case CAR_PIECE_DOOR_LR:
				GetComponentWorldPosition(CAR_DOOR_LR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_DOOR_REAR_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT);
				break;
			case CAR_PIECE_DOOR_RR:
				GetComponentWorldPosition(CAR_DOOR_RR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_DOOR_REAR_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT);
				break;

			case CAR_PIECE_WING_LF:
				GetComponentWorldPosition(CAR_WING_LF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_PANEL_FRONT_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_LF, VEHPANEL_FRONT_LEFT);
				break;
			case CAR_PIECE_WING_RF:
				GetComponentWorldPosition(CAR_WING_RF, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_PANEL_FRONT_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_RF, VEHPANEL_FRONT_RIGHT);
				break;
			case CAR_PIECE_WING_LR:
				GetComponentWorldPosition(CAR_WING_LR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_PANEL_REAR_LEFT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_LR, VEHPANEL_REAR_LEFT);
				break;
			case CAR_PIECE_WING_RR:
				GetComponentWorldPosition(CAR_WING_RR, pos);
				dmgDrawCarCollidingParticles(pos, impulse*damageMultiplier);
				if(Damage_ApplyDamage(&Damage, COMPONENT_PANEL_REAR_RIGHT, impulse*impulseMult, pHandling->fCollisionDamageMultiplier))
					SetPanelDamage(CAR_WING_RR, VEHPANEL_REAR_RIGHT);
				break;

			case CAR_PIECE_WHEEL_LF:
			case CAR_PIECE_WHEEL_LR:
			case CAR_PIECE_WHEEL_RF:
			case CAR_PIECE_WHEEL_RR:
				break;

			case CAR_PIECE_WINDSCREEN:
				if(Damage_ApplyDamage(&Damage, COMPONENT_PANEL_WINDSCREEN, impulse*impulseMult, pHandling->fCollisionDamageMultiplier)){
					uint8 oldStatus = Damage_GetPanelStatus(&Damage, VEHPANEL_WINDSCREEN);
					SetPanelDamage(CAR_WINDSCREEN, VEHPANEL_WINDSCREEN);
					if(oldStatus != Damage_GetPanelStatus(&Damage, VEHPANEL_WINDSCREEN)){
					//	DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_WINDSHIELD_CRACK, 0.0f);
					}
				}
				break;
			}
		}

		// Delegate the damage to the C implementation
		float damage = Automobile_CollisionDamage(impulse, minImpulse, pHandling->fCollisionDamageMultiplier, damageMultiplier,
			GetModelIndex() == MI_SECURICA && m_pDamageEntity && m_pDamageEntity->GetStatus() == STATUS_PLAYER,
			GetModelIndex() == MI_RCGOBLIN || GetModelIndex() == MI_RCRAIDER);

		if(damage > 0.0f){
			if(damage > 5.0f &&
			   pDriver &&
			   m_pDamageEntity && m_pDamageEntity->IsVehicle() &&
			   (this != FindPlayerVehicle() || ((CVehicle*)m_pDamageEntity)->VehicleCreatedBy == MISSION_VEHICLE) &&
			   ((CVehicle*)m_pDamageEntity)->pDriver){
				if(GetVehicleAppearance() == VEHICLE_APPEARANCE_CAR)
					pDriver->Say(SOUND_PED_CRASH_CAR);
				else
					pDriver->Say(SOUND_PED_CRASH_VEHICLE);
			}

			// Delegate the health left to the C implementation
			m_fHealth = Automobile_HealthAfterDamage(m_fHealth, damage, this == FindPlayerVehicle(), bTakeLessDamage, m_pDamageEntity && m_pDamageEntity == FindPlayerVehicle());
		}

		// play sound if a light broke
		for(i = 0; i < 4; i++)
			if(oldLightStatus[i] != 1 && Damage_GetLightStatus(&Damage, (eLights)i) == 1){
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_LIGHT_BREAK, i);	// BUG? i?
				break;
			}
	}

	if(m_fHealth < 250.0f){
		// Car is on fire
		if(Damage_GetEngineStatus(&Damage) < ENGINE_STATUS_ON_FIRE){
			// Set engine on fire and remember who did this
			Damage_SetEngineStatus(&Damage, ENGINE_STATUS_ON_FIRE);
			m_fFireBlowUpTimer = 0.0f;
			m_pSetOnFireEntity = m_pDamageEntity;
			if(m_pSetOnFireEntity)
				m_pSetOnFireEntity->RegisterReference(&m_pSetOnFireEntity);
		}
	}else{
		if(GetModelIndex() == MI_BFINJECT){
			if(m_fHealth < 400.0f)
				Damage_SetEngineStatus(&Damage, 200);
			else if(m_fHealth < 600.0f)
				Damage_SetEngineStatus(&Damage, 100);
		}
	}
}

void
CAutomobile::dmgDrawCarCollidingParticles(const CVector &pos, float amount)
{
	int i, n;

	if(!GetIsOnScreen())
		return;

	// FindPlayerSpeed() unused

	n = (int)amount/20;

	for(i = 0; i < ((n+4)&0x1F); i++)
		CParticle::AddParticle(PARTICLE_SPARK_SMALL, pos,
			CVector(General_GetRandomNumberInRangeFloat(-0.1f, 0.1f),
			        General_GetRandomNumberInRangeFloat(-0.1f, 0.1f),
			        0.006f));

	for(i = 0; i < n+2; i++)
		CParticle::AddParticle(PARTICLE_CARCOLLISION_DUST,
			CVector(General_GetRandomNumberInRangeFloat(-1.2f, 1.2f) + pos.x,
			        General_GetRandomNumberInRangeFloat(-1.2f, 1.2f) + pos.y,
			        pos.z),
			CVector(0.0f, 0.0f, 0.0f), nil, 0.5f);

	n = (int)amount/50 + 1;
	for(i = 0; i < n; i++)
		CParticle::AddParticle(PARTICLE_CAR_DEBRIS, pos,
			CVector(General_GetRandomNumberInRangeFloat(-0.25f, 0.25f),
			        General_GetRandomNumberInRangeFloat(-0.25f, 0.25f),
			        General_GetRandomNumberInRangeFloat(0.1f, 0.25f)),
			nil,
			General_GetRandomNumberInRangeFloat(0.02f, 0.08f),
			CVehicleModelInfo::ms_vehicleColourTable[m_currentColour1],
			General_GetRandomNumberInRangeInt(-40, 40),
			0,
			General_GetRandomNumberInRangeInt(0, 4));
}

float fDamagePosSpeedShift = 0.4f;
float fSpeedMult[] = {
	0.8f,
	0.75f,
	0.85f,
	0.9f,
	0.85f,
	0.85f
};

void
CAutomobile::AddDamagedVehicleParticles(void)
{
	int i, n;

	if(this == FindPlayerVehicle() && TheCamera.GetLookingForwardFirstPerson())
		return;
	if(this != FindPlayerVehicle() && ((*Timer_GetFrameCounter()) + m_randomSeed) & 1)
		return;
	if(m_fHealth >= 650.0f)
		return;

	CVector direction = fSpeedMult[5]*m_vecMoveSpeed;
	CVector damagePos = ((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())))->m_positions[CAR_POS_HEADLIGHTS]; // rouz edit (ChatGPT)

	switch(Damage_GetDoorStatus(&Damage, DOOR_BONNET)){
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		// Bonnet is still there, smoke comes out at the edge
		damagePos += vecDAMAGE_ENGINE_POS_SMALL;
		break;
	case DOOR_STATUS_SWINGING:
	case DOOR_STATUS_MISSING:
		// Bonnet is gone, smoke comes out at the engine
		damagePos += vecDAMAGE_ENGINE_POS_BIG;
		break;
	}

	if(GetModelIndex() == MI_BFINJECT)
		damagePos = CVector(0.3f, -1.5f, -0.1f);
	else if(GetModelIndex() == MI_CADDY)
		damagePos = CVector(0.6f, -1.0f, -0.25f);
	else if(IsRealHeli()){
		damagePos.x = 0.4f*GetColModel()->boundingBox.max.x;
		damagePos.y = 0.2f*GetColModel()->boundingBox.min.y;
		damagePos.z = 0.3f*GetColModel()->boundingBox.max.z;
	}else
		damagePos.z += fDamagePosSpeedShift*(GetColModel()->boundingBox.max.z-damagePos.z) * DotProduct(GetForward(), m_vecMoveSpeed);
	damagePos = GetMatrix()*damagePos;
	damagePos.z += 0.15f;

	bool electric = pHandling->Transmission.nEngineType == 'E';

	if(electric && m_fHealth < 320.0f && m_fHealth > 1.0f){
		direction = 0.85f*m_vecMoveSpeed;
		direction += GetRight() * General_GetRandomNumberInRangeFloat(0.0f, 0.04f) * (1.0f - 2.0f*m_vecMoveSpeed.Magnitude());
		direction.z += 0.001f;
		n = (General_GetRandomNumber() & 7) + 2;
		for(i = 0; i < n; i++)
			CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
		if((((*Timer_GetFrameCounter()) + m_randomSeed) & 7) == 0)
			CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, 0.8f*m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
	}else if(electric && m_fHealth < 460.0f){
		direction = 0.85f*m_vecMoveSpeed;
		direction += GetRight() * General_GetRandomNumberInRangeFloat(0.0f, 0.04f) * (1.0f - 2.0f*m_vecMoveSpeed.Magnitude());
		direction.z += 0.001f;
		n = (General_GetRandomNumber() & 3) + 1;
		for(i = 0; i < n; i++)
			CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
		if((((*Timer_GetFrameCounter()) + m_randomSeed) & 0xF) == 0)
			CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, 0.8f*m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
	}else if(m_fHealth < 250.0f){
		// nothing
	}else if(m_fHealth < 320.0f){
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, fSpeedMult[0]*direction);
	}else if(m_fHealth < 390.0f){
		CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, fSpeedMult[1]*direction);
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, fSpeedMult[2]*direction);
	}else if(m_fHealth < 460.0f){
		if((((*Timer_GetFrameCounter()) + m_randomSeed) & 3) == 0 ||
		   (((*Timer_GetFrameCounter()) + m_randomSeed) & 3) == 2)
			CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, fSpeedMult[3]*direction);
	}else{
		int rnd = (*Timer_GetFrameCounter()) + m_randomSeed;
		if(rnd < 10 ||
		   rnd < 70 && rnd > 25 ||
		   rnd < 160 && rnd > 100 ||
		   rnd < 200 && rnd > 175 ||
		   rnd > 235)
			return;
		// Delegate the lift of the smoke to the C implementation
		direction.z += Automobile_SmokeLift(m_vecMoveSpeed.Magnitude());
		if(electric){
			direction = 0.85f*m_vecMoveSpeed;
			direction += GetRight() * General_GetRandomNumberInRangeFloat(0.0f, 0.04f) * (1.0f - 2.0f*m_vecMoveSpeed.Magnitude());
			direction.z += 0.001f;
			n = (General_GetRandomNumber() & 2) + 2;
			for(i = 0; i < n; i++)
				CParticle::AddParticle(PARTICLE_SPARK_SMALL, damagePos, direction);
			if((((*Timer_GetFrameCounter()) + m_randomSeed) & 0xF) == 0)
				CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, 0.8f*m_vecMoveSpeed, nil, 0.1f, 0, 0, 0, 1000);
		}else{
			if(TheCamera.GetLookDirection() != LOOKING_FORWARD)
				CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, direction);
			else if((((*Timer_GetFrameCounter()) + m_randomSeed) & 1) == 0)
				CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, fSpeedMult[4]*m_vecMoveSpeed);
		}
	}
}

int32
CAutomobile::AddWheelDirtAndWater(CColPoint *colpoint, uint32 belowEffectSpeed)
{
	int i;
	CVector dir;
	static RwRGBA grassCol = { 8, 24, 8, 255 };
	static RwRGBA gravelCol = { 64, 64, 64, 255 };
	static RwRGBA mudCol = { 64, 32, 16, 255 };
	static RwRGBA sandCol = { 170, 165, 140, 255 };
	static RwRGBA waterCol = { 48, 48, 64, 0 };

	if(!belowEffectSpeed &&
	   colpoint->surfaceB != SURFACE_SAND && colpoint->surfaceB != SURFACE_SAND_BEACH)
		return 0;

	switch(colpoint->surfaceB){
	case SURFACE_GRASS:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = General_GetRandomNumberInRangeFloat(0.03f, 0.06f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, CollisionVector_ToNative(colpoint->point), dir, nil,
				General_GetRandomNumberInRangeFloat(0.02f, 0.1f), grassCol);
		}
		return 0;
	case SURFACE_GRAVEL:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = General_GetRandomNumberInRangeFloat(0.03f, 0.06f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, CollisionVector_ToNative(colpoint->point), dir, nil,
				General_GetRandomNumberInRangeFloat(0.05f, 0.09f), gravelCol);
		}
		return 1;
	case SURFACE_MUD_DRY:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = General_GetRandomNumberInRangeFloat(0.03f, 0.06f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, CollisionVector_ToNative(colpoint->point), dir, nil,
				General_GetRandomNumberInRangeFloat(0.02f, 0.06f), mudCol);
		}
		return 0;
	case SURFACE_SAND:
	case SURFACE_SAND_BEACH:
		if((*Timer_GetFrameCounter()) & 2 ||
		   Weather_WetRoads > 0.0f && General_GetRandomNumberInRangeFloat(Weather_WetRoads, 1.01f) > 0.5f)
			return 0;
		dir.x = 0.5f*m_vecMoveSpeed.x;
		dir.y = 0.5f*m_vecMoveSpeed.y;
		for(i = 0; i < 1; i++){
			dir.z = General_GetRandomNumberInRangeFloat(0.02f, 0.055f);
			CParticle::AddParticle(PARTICLE_SAND, CollisionVector_ToNative(colpoint->point), dir, nil,
				2.0f*m_vecMoveSpeed.Magnitude(), sandCol);
		}
		return 0;
	default:
		if(Weather_WetRoads > 0.01f){
			if((*Timer_GetFrameCounter()) & 1)
				CParticle::AddParticle(
					PARTICLE_WATERSPRAY,
					CollisionVector_ToNative(colpoint->point) + CVector(0.0f, 0.0f, 0.25f+0.25f),
					CVector(0.0f, 0.0f, General_GetRandomNumberInRangeFloat(0.005f, 0.04f)),
					nil,
					General_GetRandomNumberInRangeFloat(0.1f, 0.5f), waterCol);
			return 0;
		}
		return 1;
	}
}

void
CAutomobile::GetComponentWorldPosition(int32 component, CVector &pos)
{
	if(m_aCarNodes[component] == nil){
		printf("CarNode missing: %d %d\n", GetModelIndex(), component);
		return;
	}
	RwMatrix *ltm = RwFrameGetLTM(m_aCarNodes[component]);
	pos = *RwMatrixGetPos(ltm);
}

bool
CAutomobile::IsComponentPresent(int32 comp)
{
	// Delegate to the C implementation
	return Automobile_IsComponentPresent(this, comp);
}

void
CAutomobile::SetComponentRotation(int32 component, CVector rotation)
{
	CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector pos = mat.GetPosition();
	// BUG: all these set the whole matrix
	mat.SetRotateX(DEGTORAD(rotation.x));
	mat.SetRotateY(DEGTORAD(rotation.y));
	mat.SetRotateZ(DEGTORAD(rotation.z));
	mat.Translate(pos);
	mat.UpdateRW();
}

void
CAutomobile::OpenDoor(int32 component, eDoors door, float openRatio)
{
	CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector pos = mat.GetPosition();
	float axes[3] = { 0.0f, 0.0f, 0.0f };
	float wasClosed = false;

	if(Door_IsClosed(&Doors[door])){
		// enable angle cull for closed doors
		RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::ClearAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
		wasClosed = true;
	}

	Door_Open(&Doors[door], openRatio);

	if(wasClosed && Door_RetAngleWhenClosed(&Doors[door]) != Doors[door].m_fAngle){
		// door opened
		HideAllComps();
		// turn off angle cull for swinging door
		RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::SetAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_DOOR_OPEN_BONNET + door, 0.0f);
	}

	if(!wasClosed && openRatio == 0.0f){
		// door closed
		if(Damage_GetDoorStatus(&Damage, door) == DOOR_STATUS_SWINGING)
			Damage_SetDoorStatus(&Damage, door, DOOR_STATUS_OK);	// huh?
		ShowAllComps();
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_DOOR_CLOSE_BONNET + door, 0.0f);
	}

	axes[Doors[door].m_nAxis] = Doors[door].m_fAngle;
	mat.SetRotate(axes[0], axes[1], axes[2]);
	mat.Translate(pos);
	mat.UpdateRW();
}

inline void ProcessDoorOpenAnimation(CAutomobile *car, uint32 component, eDoors door, float time, float start, float end)
{
	if(time > start && time < end){
		float ratio = (time - start)/(end - start);
		if(Door_GetAngleOpenRatio(&car->Doors[door]) < ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > end){
		car->OpenDoor(component, door, 1.0f);
	}
}

inline void ProcessDoorCloseAnimation(CAutomobile *car, uint32 component, eDoors door, float time, float start, float end)
{
	if(time > start && time < end){
		float ratio = 1.0f - (time - start)/(end - start);
		if(Door_GetAngleOpenRatio(&car->Doors[door]) > ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > end){
		car->OpenDoor(component, door, 0.0f);
	}
}

inline void ProcessDoorOpenCloseAnimation(CAutomobile *car, uint32 component, eDoors door, float time, float start, float mid, float end)
{
	if(time > start && time < mid){
		// open
		float ratio = (time - start)/(mid - start);
		if(Door_GetAngleOpenRatio(&car->Doors[door]) < ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > mid && time < end){
		// close
		float ratio = 1.0f - (time - mid)/(end - mid);
		if(Door_GetAngleOpenRatio(&car->Doors[door]) > ratio)
			car->OpenDoor(component, door, ratio);
	}else if(time > end){
		car->OpenDoor(component, door, 0.0f);
	}
}

void
CAutomobile::ProcessOpenDoor(uint32 component, uint32 anim, float time)
{
	eDoors door;

	switch(component){
	case CAR_DOOR_RF: door = DOOR_FRONT_RIGHT; break;
	case CAR_DOOR_RR: door = DOOR_REAR_RIGHT; break;
	case CAR_DOOR_LF: door = DOOR_FRONT_LEFT; break;
	case CAR_DOOR_LR: door = DOOR_REAR_LEFT; break;
	default: assert(0);
	}

	if(IsDoorMissing(door))
		return;

	switch(anim){
	case ANIM_STD_QUICKJACK:
	case ANIM_STD_CAR_OPEN_DOOR_LHS:
	case ANIM_STD_CAR_OPEN_DOOR_RHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.41f, 0.89f);
		break;
	case ANIM_STD_CAR_CLOSE_DOOR_LHS:
	case ANIM_STD_CAR_CLOSE_DOOR_LO_LHS:
	case ANIM_STD_CAR_CLOSE_DOOR_RHS:
	case ANIM_STD_CAR_CLOSE_DOOR_LO_RHS:
		ProcessDoorCloseAnimation(this, component, door, time, 0.2f, 0.45f);
		break;
	case ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS:
	case ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS:
		ProcessDoorOpenCloseAnimation(this, component, door, time, 0.1f, 0.6f, 0.95f);
		break;
	case ANIM_STD_GETOUT_LHS:
	case ANIM_STD_GETOUT_LO_LHS:
	case ANIM_STD_GETOUT_RHS:
	case ANIM_STD_GETOUT_LO_RHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.06f, 0.43f);
		break;
	case ANIM_STD_CAR_CLOSE_LHS:
	case ANIM_STD_CAR_CLOSE_RHS:
		ProcessDoorCloseAnimation(this, component, door, time, 0.1f, 0.23f);
		break;
	case ANIM_STD_CAR_PULL_OUT_PED_RHS:
	case ANIM_STD_CAR_PULL_OUT_PED_LO_RHS:
		OpenDoor(component, door, 1.0f);
		break;
	case ANIM_STD_COACH_OPEN_LHS:
	case ANIM_STD_COACH_OPEN_RHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.66f, 0.8f);
		break;
	case ANIM_STD_COACH_GET_OUT_LHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.0f, 0.3f);
		break;
	case ANIM_STD_VAN_OPEN_DOOR_REAR_LHS:
	case ANIM_STD_VAN_OPEN_DOOR_REAR_RHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.37f, 0.55f);
		break;
	case ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS:
	case ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS:
		ProcessDoorCloseAnimation(this, component, door, time, 0.5f, 0.8f);
		break;
	case ANIM_STD_VAN_GET_OUT_REAR_LHS:
	case ANIM_STD_VAN_GET_OUT_REAR_RHS:
		ProcessDoorOpenAnimation(this, component, door, time, 0.5f, 0.6f);
		break;
	case ANIM_STD_NUM:
		OpenDoor(component, door, time);
		break;
	}
}

bool
CAutomobile::IsDoorReady(eDoors door)
{
	// Delegate to the C implementation
	return Automobile_IsDoorReady(this, door, m_nGettingInFlags);
}

bool
CAutomobile::IsDoorFullyOpen(eDoors door)
{
	// Delegate to the C implementation
	return Automobile_IsDoorFullyOpen(this, door);
}

bool
CAutomobile::IsDoorClosed(eDoors door)
{
	// Delegate to the C implementation
	return Automobile_IsDoorClosed(this, door);
}

bool
CAutomobile::IsDoorMissing(eDoors door)
{
	// Delegate to the C implementation
	return Automobile_IsDoorMissing(this, door);
}

bool
CAutomobile::IsDoorReady(uint32 door)
{
	// Delegate to the C implementation
	return Automobile_IsDoorReadyByNode(this, door, m_nGettingInFlags);
}

bool
CAutomobile::IsDoorMissing(uint32 door)
{
	// Delegate to the C implementation
	return Automobile_IsDoorMissingByNode(this, door);
}

bool
CAutomobile::IsOpenTopCar(void)
{
	// Delegate to the C implementation
	return Automobile_IsOpenTopCar((CAutomobileStruct*)this);
}

void
CAutomobile::RemoveRefsToVehicle(CEntity *ent)
{
	int i;
	for(i = 0; i < 4; i++)
		if(m_aGroundPhysical[i] == ent)
			m_aGroundPhysical[i] = nil;
}

void
CAutomobile::BlowUpCar(CEntity *culprit)
{
	RpAtomic *atomic;

	if(!bCanBeDamaged)
		return;

	if(culprit == FindPlayerPed() || culprit == FindPlayerVehicle()){
		World_Players[World_PlayerInFocus].m_nHavocLevel += 20;
		World_Players[World_PlayerInFocus].m_fMediaAttention += 10.0f;
		Stats_PropertyDestroyed += General_GetRandomNumber()%6000 + 4000;
	}

	// explosion pushes vehicle up
	m_vecMoveSpeed.z += 0.13f;
	SetStatus(STATUS_WRECKED);
	bRenderScorched = true;
	m_nTimeOfDeath = (*Timer_GetTimeInMilliseconds());
	Damage_FuckCarCompletely(&Damage);

	if(GetModelIndex() != MI_RCBANDIT){
		SetBumperDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT);
		SetBumperDamage(CAR_BUMP_REAR, VEHBUMPER_REAR);
		SetDoorDamage(CAR_BONNET, DOOR_BONNET);
		SetDoorDamage(CAR_BOOT, DOOR_BOOT);
		SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT);
		SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
		SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT);
		SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT);
		SpawnFlyingComponent(CAR_WHEEL_LF, COMPGROUP_WHEEL);
		atomic = nil;
		RwFrameForAllObjects(m_aCarNodes[CAR_WHEEL_LF], GetCurrentAtomicObjectCB, &atomic);
		if(atomic)
			RpAtomicSetFlags(atomic, 0);
	}

	m_fHealth = 0.0f;
	m_nBombTimer = 0;
	m_bombType = CARBOMB_NONE;

	TheCamera.CamShake(0.7f, GetPosition().x, GetPosition().y, GetPosition().z);

	KillPedsInVehicle();

	bEngineOn = false;
	bLightsOn = false;
	m_bSirenOrAlarm = false;
	bTaxiLight = false;
	if(bIsAmbulanceOnDuty){
		bIsAmbulanceOnDuty = false;
		CarCtrl_NumAmbulancesOnDuty--;
	}
	if(bIsFireTruckOnDuty){
		bIsFireTruckOnDuty = false;
		CarCtrl_NumFiretrucksOnDuty--;
	}
	ChangeLawEnforcerState(false);

	FireManager_StartFireOn(&gFireManager,this, culprit, 0.8f, true);
	Darkel_RegisterCarBlownUpByPlayer(this);
	if(GetModelIndex() == MI_RCBANDIT)
		Explosion_AddExplosion(this, culprit, EXPLOSION_CAR_QUICK, GetPosition(), 0);
	else
		Explosion_AddExplosion(this, culprit, EXPLOSION_CAR, GetPosition(), 0);
}

bool
CAutomobile::SetUpWheelColModel(CColModel *colModel)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)
	CColModel *vehColModel = mi->GetColModel();

	if(GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI ||
	   GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE)
		return false;

	colModel->boundingSphere = vehColModel->boundingSphere;
	colModel->boundingBox = vehColModel->boundingBox;

	CMatrix mat;
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter0=mat.GetPosition();
	    ColSphere_Set(&colModel->spheres[0],mi->m_wheelScale / 2,COLLISION_POINT_REF(sphereCenter0),SURFACE_RUBBER,CAR_PIECE_WHEEL_LF);
	}
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LB]));
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter1=mat.GetPosition();
	    ColSphere_Set(&colModel->spheres[1],mi->m_wheelScale / 2,COLLISION_POINT_REF(sphereCenter1),SURFACE_RUBBER,CAR_PIECE_WHEEL_LR);
	}
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter2=mat.GetPosition();
	    ColSphere_Set(&colModel->spheres[2],mi->m_wheelScale / 2,COLLISION_POINT_REF(sphereCenter2),SURFACE_RUBBER,CAR_PIECE_WHEEL_RF);
	}
	mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RB]));
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter3=mat.GetPosition();
	    ColSphere_Set(&colModel->spheres[3],mi->m_wheelScale / 2,COLLISION_POINT_REF(sphereCenter3),SURFACE_RUBBER,CAR_PIECE_WHEEL_RR);
	}

	if(m_aCarNodes[CAR_WHEEL_LM] != nil && m_aCarNodes[CAR_WHEEL_RM] != nil){
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LM]));
		{
		    // Keep the native center expression alive at the former member-call boundary
		    const CVector &sphereCenter4=mat.GetPosition();
		    ColSphere_Set(&colModel->spheres[4],mi->m_wheelScale / 2,COLLISION_POINT_REF(sphereCenter4),SURFACE_RUBBER,CAR_PIECE_WHEEL_LR);
		}
		mat.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RM]));
		{
		    // Keep the native center expression alive at the former member-call boundary
		    const CVector &sphereCenter5=mat.GetPosition();
		    ColSphere_Set(&colModel->spheres[5],mi->m_wheelScale / 2,COLLISION_POINT_REF(sphereCenter5),SURFACE_RUBBER,CAR_PIECE_WHEEL_RR);
		}
		colModel->numSpheres = 6;
	}else
		colModel->numSpheres = 4;

	return true;
}

float fBurstForceMult = 0.03f;

void
CAutomobile::BurstTyre(uint8 wheel, bool applyForces)
{
	if(GetModelIndex() == MI_RHINO || bTyresDontBurst)
		return;

	switch(wheel){
	case CAR_PIECE_WHEEL_LF: wheel = CARWHEEL_FRONT_LEFT; break;
	case CAR_PIECE_WHEEL_RF: wheel = CARWHEEL_FRONT_RIGHT; break;
	case CAR_PIECE_WHEEL_LR: wheel = CARWHEEL_REAR_LEFT; break;
	case CAR_PIECE_WHEEL_RR: wheel = CARWHEEL_REAR_RIGHT; break;
	}

	int status = Damage_GetWheelStatus(&Damage, wheel);
	if(status == WHEEL_STATUS_OK){
		Damage_SetWheelStatus(&Damage, wheel, WHEEL_STATUS_BURST);
		Stats_TyresPopped++;
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_TYRE_POP, 0.0f);

		if(GetStatus() == STATUS_SIMPLE){
			SetStatus(STATUS_PHYSICS);
			CarCtrl_SwitchVehicleToRealPhysics(this);
		}

		if(applyForces){
			ApplyMoveForce(GetRight() * m_fMass * General_GetRandomNumberInRangeFloat(-fBurstForceMult, fBurstForceMult));
			ApplyTurnForce(GetRight() * m_fTurnMass * General_GetRandomNumberInRangeFloat(-fBurstForceMult, fBurstForceMult), GetForward());
		}
	}
}

bool
CAutomobile::IsRoomForPedToLeaveCar(uint32 component, CVector *doorOffset)
{
	CColPoint colpoint;
	CEntity *ent;
	colpoint.point = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.0f));
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)

	CVector seatPos;
	switch(component){
	case CAR_DOOR_RF:
		seatPos = mi->GetFrontSeatPosn();
		break;
	case CAR_DOOR_LF:
		seatPos = mi->GetFrontSeatPosn();
		seatPos.x = -seatPos.x;
		break;
	case CAR_DOOR_RR:
		seatPos = mi->m_positions[CAR_POS_BACKSEAT];
		break;
	case CAR_DOOR_LR:
		seatPos = mi->m_positions[CAR_POS_BACKSEAT];
		seatPos.x = -seatPos.x;
		break;
	}
	seatPos = GetMatrix() * seatPos;

	CVector doorPos = CPed::GetPositionToOpenCarDoor(this, component);
	if(doorOffset){
		CVector off = *doorOffset;
		if(component == CAR_DOOR_RF || component == CAR_DOOR_RR)
			off.x = -off.x;
		doorPos += Multiply3x3(GetMatrix(), off);
	}

	if(GetUp().z < 0.0f){
		seatPos.z += 0.5f;
		doorPos.z += 0.5f;
	}

	CVector dist = doorPos - seatPos;

	// Removing that makes this func. return false for van doors.
	doorPos.z += 0.5f;
	float length = dist.Magnitude();
	// Delegate the scale of the vector to the C implementation
	CVector pedPos = seatPos + dist*VehicleLogic_ExitCheckScale(length);

	if(!World_GetIsLineOfSightClear(seatPos, pedPos, true, false, false, true, false, false))
		return false;
	if(World_TestSphereAgainstWorld(doorPos, 0.6f, this, true, true, false, true, false, false))
		return false;
	if(World_ProcessVerticalLine(doorPos, 1000.0f, colpoint, ent, true, false, false, true, false, false, nil))
		// Delegate the test for the ceiling to the C implementation
		if(VehicleLogic_ExitBlockedAbove(colpoint.point.z, doorPos.z))
			return false;
	float upperZ = colpoint.point.z;
	if(!World_ProcessVerticalLine(doorPos, -1000.0f, colpoint, ent, true, false, false, true, false, false, nil))
		return false;
	// Delegate the test for the ground to the C implementation
	if(VehicleLogic_ExitBlockedBelow(upperZ, colpoint.point.z))
		return false;
	return true;
}

float
CAutomobile::GetHeightAboveRoad(void)
{
	return m_fHeightAboveRoad;
}

void
CAutomobile::PlayCarHorn(void)
{
	// The honking is decided in C, the driver shouts here
	if (VehicleLogic_PlayCarHorn(this, IsAlarmOn(), General_GetRandomNumber) && pDriver)
		pDriver->Say(SOUND_PED_ANNOYED_DRIVER);
}

void
CAutomobile::PlayHornIfNecessary(void)
{
	if(AutoPilot.m_bSlowedDownBecauseOfPeds ||
	   AutoPilot.m_bSlowedDownBecauseOfCars)
		if(!HasCarStoppedBecauseOfLight())
			PlayCarHorn();
}

void
CAutomobile::ResetSuspension(void)
{
	// Delegate to the C implementation
	Automobile_ResetSuspension(this);
}

void
CAutomobile::SetupSuspensionLines(void)
{
	int i;
	CVector posn;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)
	CColModel *colModel = mi->GetColModel();

	// Each suspension line starts at the uppermost wheel position
	// and extends down to the lowermost point on the tyre
	for(i = 0; i < 4; i++){
		mi->GetWheelPosn(i, posn);
		m_aWheelPosition[i] = posn.z;

		// uppermost wheel position
		posn.z += pHandling->fSuspensionUpperLimit;
		colModel->lines[i].p0 = CollisionVector_FromNative(posn);

		// lowermost wheel position
		posn.z += pHandling->fSuspensionLowerLimit - pHandling->fSuspensionUpperLimit;
		// lowest point on tyre
		posn.z -= mi->m_wheelScale*0.5f;
		colModel->lines[i].p1 = CollisionVector_FromNative(posn);

		// this is length of the spring at rest
		m_aSuspensionSpringLength[i] = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
		m_aSuspensionLineLength[i] = colModel->lines[i].p0.z - colModel->lines[i].p1.z;
	}

	// Compress spring somewhat to get normal height on road
	m_fHeightAboveRoad = m_aSuspensionSpringLength[0]*(1.0f - 1.0f/(4.0f*pHandling->fSuspensionForceLevel))
			- colModel->lines[0].p0.z + mi->m_wheelScale*0.5f;
	for(i = 0; i < 4; i++)
		m_aWheelPosition[i] = mi->m_wheelScale*0.5f - m_fHeightAboveRoad;

	// adjust col model to include suspension lines
	if(colModel->boundingBox.min.z > colModel->lines[0].p1.z)
		colModel->boundingBox.min.z = colModel->lines[0].p1.z;
	// Delegate the radius of the vehicle to the C implementation
	CVector radiusMin = CollisionVector_ToNative(colModel->boundingBox.min);
	CVector radiusMax = CollisionVector_ToNative(colModel->boundingBox.max);
	float radius = VehicleLogic_BoundingRadius(&radiusMin, &radiusMax);
	if(colModel->boundingSphere.radius < radius)
		colModel->boundingSphere.radius = radius;

	if(GetModelIndex() == MI_RCBANDIT){
		colModel->boundingSphere.radius = 2.0f;
		for(i = 0; i < colModel->numSpheres; i++)
			colModel->spheres[i].bounds.radius = 0.3f;
	}
}

// called on police cars
void
CAutomobile::ScanForCrimes(void)
{
	if(FindPlayerVehicle() && FindPlayerVehicle()->IsCar())
		if(FindPlayerVehicle()->IsAlarmOn())
			// if player's alarm is on, increase wanted level
			if((FindPlayerVehicle()->GetPosition() - GetPosition()).MagnitudeSqr() < sq(20.0f))
				World_Players[World_PlayerInFocus].m_pPed->SetWantedLevelNoDrop(1);
}

void
CAutomobile::BlowUpCarsInPath(void)
{
	int i;

	if(m_vecMoveSpeed.Magnitude() > 0.1f && bTankDetonateCars)
		for(i = 0; i < m_nCollisionRecords; i++)
			if(m_aCollisionRecords[i] &&
			   m_aCollisionRecords[i]->IsVehicle() &&
			   m_aCollisionRecords[i]->GetModelIndex() != MI_RHINO &&
			   !m_aCollisionRecords[i]->bRenderScorched){
				if(this == FindPlayerVehicle())
					Events_RegisterEntity(EVENT_EXPLOSION,EVENT_ENTITY_VEHICLE,m_aCollisionRecords[i],FindPlayerPed(),2000);
				((CVehicle*)m_aCollisionRecords[i])->BlowUpCar(this);
			}
}

bool
CAutomobile::HasCarStoppedBecauseOfLight(void)
{
	if(GetStatus() != STATUS_SIMPLE && GetStatus() != STATUS_PHYSICS)
		return false;

	// The link tests are done in C
	if(AutoPilot.m_nCurrentRouteNode && AutoPilot.m_nNextRouteNode &&
	   PathLogic_LinkHasTrafficLight(ThePaths.m_pathNodes, ThePaths.m_connections, ThePaths.m_carPathConnections, ThePaths.m_carPathLinks,
			AutoPilot.m_nCurrentRouteNode, AutoPilot.m_nNextRouteNode))
		return true;

	if(AutoPilot.m_nCurrentRouteNode && AutoPilot.m_nPrevRouteNode &&
	   PathLogic_LinkHasTrafficLight(ThePaths.m_pathNodes, ThePaths.m_connections, ThePaths.m_carPathConnections, ThePaths.m_carPathLinks,
			AutoPilot.m_nCurrentRouteNode, AutoPilot.m_nPrevRouteNode))
		return true;

	return false;
}

void
CPed::DeadPedMakesTyresBloody(void)
{
	// Delegate the range of sectors to the C implementation
	int32_t minX, minY, maxX, maxY;
	World_SectorRange(GetPosition().x - 2.0f, GetPosition().y - 2.0f, GetPosition().x + 2.0f, GetPosition().y + 2.0f, &minX, &minY, &maxX, &maxY);

	World_AdvanceCurrentScanCode();

	for (int curY = minY; curY <= maxY; curY++) {
		for (int curX = minX; curX <= maxX; curX++) {
			CSector *sector = World_GetSector(curX, curY);
			MakeTyresMuddySectorList(sector->m_lists[ENTITYLIST_VEHICLES]);
			MakeTyresMuddySectorList(sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP]);
		}
	}
}

void
CPed::MakeTyresMuddySectorList(CPtrList &list)
{
	CAutomobile *car = nil;
	CBike *bike = nil;
	for (CPtrNode *node = list.first; node; node = node->next) {
		CVehicle *veh = (CVehicle*)node->item;
		if (veh->m_scanCode != World_GetCurrentScanCode()) {
			veh->m_scanCode = World_GetCurrentScanCode();

			// Delegate the check that the other vehicle is close to the C implementation
			if (VehicleLogic_WithinSquare(GetPosition().x, GetPosition().y, veh->GetPosition().x, veh->GetPosition().y, 10.0f)) {				
				if (veh->IsCar()) {
					bike = nil;
					car = (CAutomobile*)veh;
				} else if (veh->IsBike()) {
					bike = (CBike*)veh;
					car = nil;
				}
				if (veh->m_vecMoveSpeed.MagnitudeSqr2D() > 0.05f) {
					if (car) {
						for (int wheel = 0; wheel < 4; wheel++) {
							if (!car->m_aWheelSkidmarkBloody[wheel] && car->m_aSuspensionSpringRatio[wheel] < 1.0f) {

								CColModel* vehCol = car->GetModelInfo()->GetColModel();
								// Delegate the rough position of the wheel to the C implementation
								VectorMath3 approxWheel = AutomobileLogic_ApproxWheelOffset(wheel, vehCol->boundingBox.max.x, vehCol->boundingBox.max.y, vehCol->boundingBox.min.y);
								CVector approxWheelOffset(approxWheel.x, approxWheel.y, approxWheel.z);

								// I hope so
								CVector wheelPos = car->GetMatrix() * approxWheelOffset;
								if (General_AbsDiffBelow(wheelPos.z, GetPosition().z, 2.0f)) {

									if ((wheelPos - GetPosition()).MagnitudeSqr2D() < 1.0f) {
										if (Game_nastyGame) {
											car->m_aWheelSkidmarkBloody[wheel] = true;
											DMAudio_PlayOneShot(car->m_audioEntityId, SOUND_SPLATTER, 0.0f);
										}
										if (car->m_fMass > 500.f) {
											car->ApplyMoveForce(CVector(0.0f, 0.0f, Automobile_WheelNudgeForce(m_fMass, 0.001f)));

											CVector vehAndWheelDist = wheelPos - car->GetPosition();
											car->ApplyTurnForce(CVector(0.0f, 0.0f, Automobile_WheelNudgeForce(m_fTurnMass, 0.0005f)), vehAndWheelDist);
											if (car == FindPlayerVehicle()) {
												CPad::GetPad(0)->StartShake(300, 70);
											}
										}
									}
								}
							}
						}
					} else if (bike) {
						for (int wheel = 0; wheel < 2; wheel++) {
							if (!bike->m_aWheelSkidmarkBloody[wheel] && bike->m_aSuspensionSpringRatio[wheel] < 1.0f) {

								CColModel* vehCol = bike->GetModelInfo()->GetColModel();
								// Delegate the rough position of the wheel to the C implementation
								VectorMath3 approxWheel = AutomobileLogic_ApproxBikeWheelOffset(wheel, vehCol->boundingBox.max.y, vehCol->boundingBox.min.y);
								CVector approxWheelOffset(approxWheel.x, approxWheel.y, approxWheel.z);

								// I hope so
								CVector wheelPos = bike->GetMatrix() * approxWheelOffset;
								if (General_AbsDiffBelow(wheelPos.z, GetPosition().z, 2.0f)) {

									if ((wheelPos - GetPosition()).MagnitudeSqr2D() < 1.0f) {
										if (Game_nastyGame) {
											bike->m_aWheelSkidmarkBloody[wheel] = true;
											DMAudio_PlayOneShot(bike->m_audioEntityId, SOUND_SPLATTER, 0.0f);
										}
										if (bike->m_fMass > 100.0f) {
											bike->ApplyMoveForce(CVector(0.0f, 0.0f, 10.0f));

											CVector vehAndWheelDist = wheelPos - bike->GetPosition();
											bike->ApplyTurnForce(CVector(0.0f, 0.0f, 10.0f), vehAndWheelDist);

											if (bike == FindPlayerVehicle()) {
												CPad::GetPad(0)->StartShake(300, 70);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void
CAutomobile::SetBusDoorTimer(uint32 timer, uint8 type)
{
	// Delegate to the C implementation
	Automobile_SetBusDoorTimer(this, timer, type, (*Timer_GetTimeInMilliseconds()));
}

void
CAutomobile::ProcessAutoBusDoors(void)
{
	// The decisions are made in C, the doors are opened here
	const bool doorMissing[2] = { IsDoorMissing(DOOR_FRONT_LEFT), IsDoorMissing(DOOR_FRONT_RIGHT) };
	const bool doorClosed[2] = { IsDoorClosed(DOOR_FRONT_LEFT), IsDoorClosed(DOOR_FRONT_RIGHT) };
	AutomobileDoorAction actions[2];
	int numActions = Automobile_ProcessAutoBusDoors((CAutomobileStruct*)this, (*Timer_GetTimeInMilliseconds()), doorMissing, doorClosed, actions);
	for(int i = 0; i < numActions; i++)
		OpenDoor(actions[i].component, (eDoors)actions[i].door, actions[i].ratio);
}

void
CAutomobile::ProcessSwingingDoor(int32 component, eDoors door)
{
	if(Damage_GetDoorStatus(&Damage, door) != DOOR_STATUS_SWINGING)
		return;

	if (m_aCarNodes[component] == nil)
		return;

	CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector pos = mat.GetPosition();
	float axes[3] = { 0.0f, 0.0f, 0.0f };

	Door_Process(&Doors[door], this);
	axes[Doors[door].m_nAxis] = Doors[door].m_fAngle;
	mat.SetRotate(axes[0], axes[1], axes[2]);
	mat.Translate(pos);
	mat.UpdateRW();

	// make wind rip off bonnet
	if(door == DOOR_BONNET && Doors[door].m_nDoorState == DOORST_OPEN &&
	   DotProduct(m_vecMoveSpeed, GetForward()) > 0.4f){
#ifdef FIX_BUGS
		CObject *comp = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_BONNET);
#else
		CObject *comp = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_DOOR);
#endif
		// make both doors invisible on car
		SetComponentVisibility(m_aCarNodes[CAR_BONNET], ATOMIC_FLAG_NONE);
		Damage_SetDoorStatus(&Damage, DOOR_BONNET, DOOR_STATUS_MISSING);

		if(comp){
			if(General_GetRandomNumber() & 1)
				comp->m_vecMoveSpeed = 0.4f*m_vecMoveSpeed + 0.1f*GetRight() + 0.5f*GetUp();
			else
				comp->m_vecMoveSpeed = 0.4f*m_vecMoveSpeed - 0.1f*GetRight() + 0.5f*GetUp();
			comp->ApplyTurnForce(10.0f*GetUp(), GetForward());
		}
	}
}

void
CAutomobile::Fix(void)
{
	int component;

	Damage_ResetDamageStatus(&Damage);

	if(pHandling->Flags & HANDLING_NO_DOORS){
		Damage_SetDoorStatus(&Damage, DOOR_FRONT_LEFT, DOOR_STATUS_MISSING);
		Damage_SetDoorStatus(&Damage, DOOR_FRONT_RIGHT, DOOR_STATUS_MISSING);
		Damage_SetDoorStatus(&Damage, DOOR_REAR_LEFT, DOOR_STATUS_MISSING);
		Damage_SetDoorStatus(&Damage, DOOR_REAR_RIGHT, DOOR_STATUS_MISSING);
	}

	bIsDamaged = false;
	RpClumpForAllAtomics((RpClump*)m_rwObject, CVehicleModelInfo::HideAllComponentsAtomicCB, (void*)ATOMIC_FLAG_DAM);

	for(component = CAR_BUMP_FRONT; component < NUM_CAR_NODES; component++){
		if(m_aCarNodes[component]){
			CMatrix mat(RwFrameGetMatrix(m_aCarNodes[component]));
			mat.SetTranslate(mat.GetPosition());
			mat.UpdateRW();
		}
	}

	for(component = 0; component < 4; component++)
		Damage_SetWheelStatus(&Damage, component, WHEEL_STATUS_OK);

	if(GetModelIndex() == MI_HUNTER){
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB])), 0);
	}else if(IsRealHeli()){
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LB])), 0);
		// Use the C helper and retain the original typed SDK result
		RpAtomicSetFlags((RpAtomic*)((RwObject*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RB])), 0);
	}
}

void
CAutomobile::SetupDamageAfterLoad(void)
{
	if(m_aCarNodes[CAR_BUMP_FRONT])
		SetBumperDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT);
	if(m_aCarNodes[CAR_BONNET])
		SetDoorDamage(CAR_BONNET, DOOR_BONNET);
	if(m_aCarNodes[CAR_BUMP_REAR])
		SetBumperDamage(CAR_BUMP_REAR, VEHBUMPER_REAR);
	if(m_aCarNodes[CAR_BOOT])
		SetDoorDamage(CAR_BOOT, DOOR_BOOT);
	if(m_aCarNodes[CAR_DOOR_LF])
		SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT);
	if(m_aCarNodes[CAR_DOOR_RF])
		SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT);
	if(m_aCarNodes[CAR_DOOR_LR])
		SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT);
	if(m_aCarNodes[CAR_DOOR_RR])
		SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT);
	if(m_aCarNodes[CAR_WING_LF])
		SetPanelDamage(CAR_WING_LF, VEHPANEL_FRONT_LEFT);
	if(m_aCarNodes[CAR_WING_RF])
		SetPanelDamage(CAR_WING_RF, VEHPANEL_FRONT_RIGHT);
	if(m_aCarNodes[CAR_WING_LR])
		SetPanelDamage(CAR_WING_LR, VEHPANEL_REAR_LEFT);
	if(m_aCarNodes[CAR_WING_RR])
		SetPanelDamage(CAR_WING_RR, VEHPANEL_REAR_RIGHT);
}

RwObject*
GetCurrentAtomicObjectCB(RwObject *object, void *data)
{
	RpAtomic *atomic = (RpAtomic*)object;
	assert(RwObjectGetType(object) == rpATOMIC);
	if(RpAtomicGetFlags(atomic) & rpATOMICRENDER)
		*(RpAtomic**)data = atomic;
	return object;
}

CObject*
CAutomobile::SpawnFlyingComponent(int32 component, uint32 type)
{
	RpAtomic *atomic;
	RwFrame *frame;
	RwMatrix *matrix;
	CObject *obj;

	if(CObject::nNoTempObjects >= NUMTEMPOBJECTS)
		return nil;

	atomic = nil;
	RwFrameForAllObjects(m_aCarNodes[component], GetCurrentAtomicObjectCB, &atomic);
	if(atomic == nil)
		return nil;

	// Allocate the flying automobile component without invoking C++ new.
	CObjectPool *objectPool = Pools_GetObjectPool();
		// Access raw storage through the C store or pool API
	obj = ((CObject*)CPool_New(objectPool));
	#ifdef FIX_BUGS
	if (!obj) {
				// Access raw storage through the C store or pool API
		for (int32 i = 0; i < CPool_GetSize(objectPool); i++) {
			// Access raw storage through the C store or pool API
			CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
					if (existing && existing->ObjectCreatedBy == TEMP_OBJECT) {
								// Access raw storage through the C store or pool API
				int32 handle = CPool_GetIndex(objectPool, existing);
								World_Remove(existing);
				existing->~CObject();
								// Access raw storage through the C store or pool API
				CPool_Delete(objectPool, existing);
				obj = ((CObject*)CPool_NewAt(objectPool, handle));
								break;
			}
		}
	}
	if (obj)
#endif
		std::allocator<CObject>().construct(obj);
	if(obj == nil)
		return nil;

	if(component == CAR_WINDSCREEN){
		obj->SetModelIndexNoCreate(MI_CAR_BONNET);
	}else switch(type){
	case COMPGROUP_BUMPER:
		obj->SetModelIndexNoCreate(MI_CAR_BUMPER);
		break;
	case COMPGROUP_WHEEL:
		obj->SetModelIndexNoCreate(MI_CAR_WHEEL);
		break;
	case COMPGROUP_DOOR:
		obj->SetModelIndexNoCreate(MI_CAR_DOOR);
		obj->SetCenterOfMass(0.0f, -0.5f, 0.0f);
		obj->bDrawLast = true;
		break;
	case COMPGROUP_BONNET:
		obj->SetModelIndexNoCreate(MI_CAR_BONNET);
		obj->SetCenterOfMass(0.0f, 0.4f, 0.0f);
		break;
	case COMPGROUP_BOOT:
		obj->SetModelIndexNoCreate(MI_CAR_BOOT);
		obj->SetCenterOfMass(0.0f, -0.3f, 0.0f);
		break;
	case COMPGROUP_PANEL:
	default:
		obj->SetModelIndexNoCreate(MI_CAR_PANEL);
		break;
	}

	// object needs base model
	obj->RefModelInfo(GetModelIndex());

	// create new atomic
	matrix = RwFrameGetLTM(m_aCarNodes[component]);
	frame = RwFrameCreate();
	atomic = RpAtomicClone(atomic);
	*RwFrameGetMatrix(frame) = *matrix;
	RpAtomicSetFrame(atomic, frame);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, nil);
	obj->AttachToRwObject((RwObject*)atomic);
	obj->bDontStream = true;

	// init object
	obj->m_fMass = 10.0f;
	obj->m_fTurnMass = 25.0f;
	obj->m_fAirResistance = 0.97f;
	obj->m_fElasticity = 0.1f;
	obj->m_fBuoyancy = obj->m_fMass*GRAVITY/0.75f;
	obj->ObjectCreatedBy = TEMP_OBJECT;
	obj->SetIsStatic(false);
	obj->bIsPickup = false;
	obj->bUseVehicleColours = true;
	obj->m_colour1 = m_currentColour1;
	obj->m_colour2 = m_currentColour2;

	// life time - the more objects the are, the shorter this one will live
	CObject::nNoTempObjects++;
	if(CObject::nNoTempObjects > 20)
		obj->m_nEndOfLifeTime = (*Timer_GetTimeInMilliseconds()) + 20000/5.0f;
	else if(CObject::nNoTempObjects > 10)
		obj->m_nEndOfLifeTime = (*Timer_GetTimeInMilliseconds()) + 20000/2.0f;
	else
		obj->m_nEndOfLifeTime = (*Timer_GetTimeInMilliseconds()) + 20000;

	// Delegate the speed of the component to the C implementation
	Automobile_FlyingComponentSpeed(&m_vecMoveSpeed, GetUp().z, component == COMPGROUP_BONNET || component == COMPGROUP_BOOT || component == CAR_WINDSCREEN, &obj->m_vecMoveSpeed);

	obj->m_vecTurnSpeed = m_vecTurnSpeed*2.0f;

	// push component away from car
	CVector dist = obj->GetPosition() - GetPosition();
	dist.Normalise();
	if(component == COMPGROUP_BONNET || component == COMPGROUP_BOOT || component == CAR_WINDSCREEN){
		// push these up some
		dist += GetUp();
		if(GetUp().z > 0.0f){
			// simulate fast upward movement if going fast
			float speed = CVector2D(m_vecMoveSpeed).Magnitude();
			obj->GetMatrix().Translate(GetUp()*speed);
		}
	}
	obj->ApplyMoveForce(dist);

	if(type == COMPGROUP_WHEEL){
		obj->m_fTurnMass = 5.0f;
		obj->m_vecTurnSpeed.x = 0.5f;
		obj->m_fAirResistance = 0.99f;
	}

	if(GetStatus() == STATUS_WRECKED && IsVisible() && DotProduct(dist, TheCamera.GetPosition() - GetPosition()) > -0.5f){
		dist = TheCamera.GetPosition() - GetPosition();
		dist.Normalise();
		dist.z += 0.3f;
		ApplyMoveForce(5.0f*dist);
	}

	if(Collision_ProcessColModels(obj->GetMatrix(), *obj->GetColModel(),
			this->GetMatrix(), *this->GetColModel(),
			World_m_aTempColPts, nil, nil) > 0)
		obj->m_pCollidingEntity = this;

	if(bRenderScorched)
		obj->bRenderScorched = true;

	World_Add(obj);

	return obj;
}

CObject*
CAutomobile::RemoveBonnetInPedCollision(void)
{
	CObject *obj;

	if(Damage_GetDoorStatus(&Damage, DOOR_BONNET) == DOOR_STATUS_SWINGING &&
	   Door_RetAngleWhenOpen(&Doors[DOOR_BONNET])*0.4f < Doors[DOOR_BONNET].m_fAngle){
#ifdef FIX_BUGS
		obj = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_BONNET);
#else
		obj = SpawnFlyingComponent(CAR_BONNET, COMPGROUP_DOOR);
#endif
		// make both doors invisible on car
		SetComponentVisibility(m_aCarNodes[CAR_BONNET], ATOMIC_FLAG_NONE);
		Damage_SetDoorStatus(&Damage, DOOR_BONNET, DOOR_STATUS_MISSING);
		return obj;
	}
	return nil;
}

void
CAutomobile::SetPanelDamage(int32 component, ePanels panel, bool noFlyingComponents)
{
	int status = Damage_GetPanelStatus(&Damage, panel);
	if(m_aCarNodes[component] == nil)
		return;
	if(status == PANEL_STATUS_SMASHED1){
		if(panel == VEHPANEL_WINDSCREEN)
			DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_WINDSHIELD_CRACK, 0.0f);
		// show damaged part
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
	}else if(status == PANEL_STATUS_MISSING){
		if(!noFlyingComponents)
			SpawnFlyingComponent(component, COMPGROUP_PANEL);
		else if(panel == VEHPANEL_WINDSCREEN)
			Glass_CarWindscreenShatters(this, false);
		// hide both
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
	}
}

void
CAutomobile::SetBumperDamage(int32 component, ePanels panel, bool noFlyingComponents)
{
	int status = Damage_GetPanelStatus(&Damage, panel);
	if(m_aCarNodes[component] == nil){
		printf("Trying to damage component %d of %s\n",
			component, ((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex()))->GetModelName()); // rouz edit (ChatGPT)
		return;
	}
	if(status == PANEL_STATUS_SMASHED1){
		// show damaged part
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
	}else if(status == PANEL_STATUS_MISSING){
		if(!noFlyingComponents)
			SpawnFlyingComponent(component, COMPGROUP_BUMPER);
		// hide both
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
	}
}

void
CAutomobile::SetDoorDamage(int32 component, eDoors door, bool noFlyingComponents)
{
	int status = Damage_GetDoorStatus(&Damage, door);
	if(m_aCarNodes[component] == nil){
		printf("Trying to damage component %d of %s\n",
			component, ((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex()))->GetModelName()); // rouz edit (ChatGPT)
		return;
	}

	if(!CanDoorsBeDamaged() && status > DOOR_STATUS_SMASHED && door != DOOR_BONNET && door != DOOR_BOOT){
		Damage_SetDoorStatus(&Damage, door, DOOR_STATUS_SMASHED);
		status = DOOR_STATUS_SMASHED;
	}

	if(door == DOOR_BOOT && status == DOOR_STATUS_SWINGING && pHandling->Flags & HANDLING_NOSWING_BOOT){
		Damage_SetDoorStatus(&Damage, DOOR_BOOT, DOOR_STATUS_MISSING);
		status = DOOR_STATUS_MISSING;
	}

	switch(status){
	case DOOR_STATUS_SMASHED:
		// show damaged part
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_DAM);
		break;
	case DOOR_STATUS_SWINGING:
		// turn off angle cull for swinging doors
		RwFrameForAllObjects(m_aCarNodes[component], CVehicleModelInfo::SetAtomicFlagCB, (void*)ATOMIC_FLAG_NOCULL);
		break;
	case DOOR_STATUS_MISSING:
		if(!noFlyingComponents){
			if(door == DOOR_BONNET)
				SpawnFlyingComponent(component, COMPGROUP_BONNET);
			else if(door == DOOR_BOOT)
				SpawnFlyingComponent(component, COMPGROUP_BOOT);
			else
				SpawnFlyingComponent(component, COMPGROUP_DOOR);
		}
		// hide both
		SetComponentVisibility(m_aCarNodes[component], ATOMIC_FLAG_NONE);
		break;
	}
}


static RwObject*
SetVehicleAtomicVisibilityCB(RwObject *object, void *data)
{
	uint32 flags = (uint32)(uintptr)data;
	RpAtomic *atomic = (RpAtomic*)object;
	if((CVisibilityPlugins::GetAtomicId(atomic) & (ATOMIC_FLAG_OK|ATOMIC_FLAG_DAM)) == flags)
		RpAtomicSetFlags(atomic, rpATOMICRENDER);
	else
		RpAtomicSetFlags(atomic, 0);
	return object;
}

void
CAutomobile::SetComponentVisibility(RwFrame *frame, uint32 flags)
{
	HideAllComps();
	bIsDamaged = true;
	RwFrameForAllObjects(frame, SetVehicleAtomicVisibilityCB, (void*)flags);
}

void
CAutomobile::SetupModelNodes(void)
{
	// Clear the node array in C before filling it
	Automobile_ClearCarNodes(this);
	CClumpModelInfo::FillFrameArray(GetClump(), m_aCarNodes);
}

void
CAutomobile::SetTaxiLight(bool light)
{
	bTaxiLight = light;
}

bool
CAutomobile::GetAllWheelsOffGround(void)
{
	// Delegate to the C implementation
	return Automobile_GetAllWheelsOffGround(this);
}

void
CAutomobile::HideAllComps(void)
{
	// empty
}

void
CAutomobile::ShowAllComps(void)
{
	// empty
}

void
CAutomobile::ReduceHornCounter(void)
{
	// Delegate to the C implementation
	Automobile_ReduceHornCounter(this);
}

void
CAutomobile::SetAllTaxiLights(bool set)
{
	m_sAllTaxiLights = set;
}

void
CAutomobile::TellHeliToGoToCoors(float x, float y, float z, uint8 speed)
{
	// Delegate to the C implementation
	Automobile_TellHeliToGoToCoors((CAutomobileStruct*)this, x, y, z, speed);
}

void
CAutomobile::TellPlaneToGoToCoors(float x, float y, float z, uint8 speed)
{
	// Delegate to the C implementation
	Automobile_TellPlaneToGoToCoors((CAutomobileStruct*)this, x, y, z, speed);
}

void
CAutomobile::PopBoot(void)
{
	switch(Damage_GetDoorStatus(&Damage, DOOR_BOOT)){
	case DOOR_STATUS_OK:
	case DOOR_STATUS_SMASHED:
		Doors[DOOR_BOOT].m_fAngle = Doors[DOOR_BOOT].m_fMinAngle;
		CMatrix mat(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
		CVector pos = mat.GetPosition();
		float axes[3] = { 0.0f, 0.0f, 0.0f };
		axes[Doors[DOOR_BOOT].m_nAxis] = Doors[DOOR_BOOT].m_fAngle;
		mat.SetRotate(axes[0], axes[1], axes[2]);
		mat.Translate(pos);
		mat.UpdateRW();
	}
}

void
CAutomobile::PopBootUsingPhysics(void)
{
	// Delegate to the C implementation
	Automobile_PopBootUsingPhysics(this);
}

void
CAutomobile::CloseAllDoors(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)
	if(!IsDoorMissing(DOOR_FRONT_LEFT))
		OpenDoor(CAR_DOOR_LF, DOOR_FRONT_LEFT, 0.0f);
	if(mi->m_numDoors > 1){
		if(!IsDoorMissing(DOOR_FRONT_RIGHT))
			OpenDoor(CAR_DOOR_RF, DOOR_FRONT_RIGHT, 0.0f);
		if(mi->m_numDoors > 2){
			if(!IsDoorMissing(DOOR_REAR_LEFT))
				OpenDoor(CAR_DOOR_LR, DOOR_REAR_LEFT, 0.0f);
			if(!IsDoorMissing(DOOR_REAR_RIGHT))
				OpenDoor(CAR_DOOR_RR, DOOR_REAR_RIGHT, 0.0f);
		}
	}
}

CPed*
CAutomobile::KnockPedOutCar(eWeaponType weapon, uint16 door, CPed *ped)
{
	AnimationId anim = ANIM_STD_KO_FRONT;
	if(ped == nil)
		return nil;

	ped->m_vehDoor = door;
	ped->SetPedState(PED_IDLE);
	// Use the complete C animation manager at the original call boundary
	AnimManager_BlendAnimation(ped->GetClump(), ped->m_animGroup, ANIM_STD_IDLE, 100.0f);
	CPed::PedSetOutCarCB(nil, ped);
	ped->SetMoveState(PEDMOVE_STILL);
	if(GetUp().z < 0.0f)
		ped->SetHeading(General_LimitRadianAngle(GetForward().Heading() + PI));
	else
		ped->SetHeading(GetForward().Heading());

	switch(weapon){
	case WEAPONTYPE_UNARMED:
	case WEAPONTYPE_UNIDENTIFIED:
		ped->m_vecMoveSpeed = m_vecMoveSpeed;
		ped->m_pCollidingEntity = this;
		anim = ANIM_STD_NUM;
		break;

	case WEAPONTYPE_BASEBALLBAT:
	case WEAPONTYPE_RAMMEDBYCAR:
	case WEAPONTYPE_FALL:
		ped->m_vecMoveSpeed = m_vecMoveSpeed;
		anim = ANIM_STD_SPINFORWARD_LEFT;
		ApplyMoveForce(4.0f*GetUp() + 8.0f*GetRight());
		break;
	}

	if(weapon != WEAPONTYPE_UNARMED){
		ped->SetFall(1000, anim, 0);
		ped->bIsStanding = false;
		ped->m_headingRate = 0.0f;
	}
	ped->m_pMyVehicle = nil;
	return ped;
}

#ifdef COMPATIBLE_SAVES
void
CAutomobile::Save(uint8*& buf)
{
	CVehicle::Save(buf);
		// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &Damage, sizeof(Damage));
	ZeroSaveBuf(&buf, 1500 - 672 - sizeof(CDamageManager));
	}

void
CAutomobile::Load(uint8*& buf)
{
	CVehicle::Load(buf);
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Damage, &buf, sizeof(Damage));
	SkipSaveBuf(&buf, 1500 - 672 - sizeof(CDamageManager));
		SetupDamageAfterLoad();
}
#endif

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
