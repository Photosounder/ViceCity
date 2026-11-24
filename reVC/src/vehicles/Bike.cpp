//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "Quaternion_native.h"
#include "ColSphere_native.h"
#include "CollisionVector_native.h"
#include "ColLine_native.h"
#include "Skidmarks_backend.h"
#include "main.h"	// rouz edit
#include "General.h"
#include "Pad.h"
#include "DMAudio.h"
#include "Clock.h"
#include "Timecycle.h"
#include "ZoneCull.h"
#include "Camera.h"
#include "Darkel.h"
#include "Rubbish.h"
#include "Explosion.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "Shadows.h"
#include "PointLights.h"
#include "Coronas.h"
#include "SpecialFX.h"
#include "WaterLevel.h"
#include "Floater.h"
#include "World.h"
#include "SurfaceTable.h"
#include "Weather.h"
#include "Record.h"
#include "CarCtrl.h"
#include "CarAI.h"
#include "Script.h"
#include "Stats.h"
#include "Replay.h"
#include "AnimManager.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "DamageManager.h"
#include "Vehicle.h"
#include "VehicleLogic.h"
#include "Automobile.h"
#include "Bike.h"
#include "BikeLogic.h"
#include "AutomobileLogic.h"
#include "Debug.h"
#include "SaveBuf.h"

// Wheel directions along the surface a wheel stands on, from the forward direction and the surface normal
static void
BikeSurfaceWheelAxes(CVector &wheelFwd, CVector &wheelRight, const CVector &normal)
{
	float fwdIn[3] = { wheelFwd.x, wheelFwd.y, wheelFwd.z };
	float nIn[3] = { normal.x, normal.y, normal.z };
	float fwdOut[3], rightOut[3];

	Automobile_SurfaceWheelAxes(fwdIn, nIn, fwdOut, rightOut);
	wheelFwd = CVector(fwdOut[0], fwdOut[1], fwdOut[2]);
	wheelRight = CVector(rightOut[0], rightOut[1], rightOut[2]);
}

const uint32 CBike::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	1260;
#else
	sizeof(CBoat);
#endif


// TODO: maybe put this somewhere else
inline void
GetRelativeMatrix(RwMatrix *mat, RwFrame *frm, RwFrame *end)
{
	*mat = *RwFrameGetMatrix(frm);
	frm = RwFrameGetParent(frm);
	while(frm){
		RwMatrixTransform(mat, RwFrameGetMatrix(frm), rwCOMBINEPOSTCONCAT);
		frm = RwFrameGetParent(frm);
		if(frm == end)
			frm = nil;
	}
}

#define FAKESUSPENSION (99999.992f)

CBike::CBike(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
	int i;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(id)); // rouz edit (ChatGPT)
	switch(id){
	case MI_ANGEL:
	case MI_FREEWAY:
		m_bikeAnimType = ASSOCGRP_BIKE_HARLEY;
		break;
	case MI_PIZZABOY:
	case MI_FAGGIO:
		m_bikeAnimType = ASSOCGRP_BIKE_VESPA;
		break;
	case MI_PCJ600:
		m_bikeAnimType = ASSOCGRP_BIKE_STANDARD;
		break;
	case MI_SANCHEZ:
		m_bikeAnimType = ASSOCGRP_BIKE_DIRT;
		break;
	default: assert(0 && "invalid bike model ID");
	}
	m_vehType = VEHICLE_TYPE_BIKE;

	m_fFireBlowUpTimer = 0.0f;
	m_doingBurnout = 0;
	m_bike_flag01 = false;

	SetModelIndex(id);

	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)mi->m_handlingId);
	pBikeHandling = mod_HandlingManager.GetBikePointer((tVehicleType)mi->m_handlingId);
	pFlyingHandling = mod_HandlingManager.GetFlyingPointer((tVehicleType)mi->m_handlingId);

	m_bike_unused1 = 20.0f;
	m_bike_unused2 = 0;

	mi->ChooseVehicleColour(m_currentColour1, m_currentColour2);

	m_fRearForkLength = 0.0f;
	m_fFrontForkY = 0.0;
	m_fFrontForkZ = 0.0;
	m_fFrontForkSlope = Tan(DEGTORAD(mi->m_bikeSteerAngle));

	m_fMass = pHandling->fMass;
	m_fTurnMass = pHandling->fTurnMass;
	m_vecCentreOfMass = pHandling->CentreOfMass;
	m_vecCentreOfMass.z = 0.1f;
	m_fAirResistance = pHandling->Dimension.x*pHandling->Dimension.z/m_fMass;
	m_fElasticity = 0.05f;
	m_fBuoyancy = pHandling->fBuoyancy;

	m_fSteerAngle = 0.0f;
	m_fWheelAngle = 0.0f;
	m_fLeanLRAngle = 0.0f;
	m_fLeanLRAngle2 = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;
	m_fLeanInput = 0.0f;
	m_fPedLeanAmountLR = 0.0f;
	m_fPedLeanAmountUD = 0.0f;
	m_pSetOnFireEntity = nil;
	m_pBombRigger = nil;
	m_fGasPedalAudio = 0.0f;
	m_bike_flag02 = false;
	bWaterTight = false;
	bIsBeingPickedUp = false;
	bIsStanding = false;
	bExtraSpeed = false;
	bIsOnFire = false;
	bWheelieCam = false;

	m_fTireTemperature = 1.0f;
	m_fBrakeDestabilization = 0.0f;
	m_fVelocityChangeForAudio = 0;

	for(i = 0; i < 2; i++){
		m_aWheelRotation[i] = 0.0f;
		m_aWheelSpeed[i] = 0.0f;
		m_aWheelState[i] = WHEEL_STATE_NORMAL;
		m_aWheelSkidmarkType[i] = SKIDMARK_NORMAL;
		m_aWheelSkidmarkBloody[i] = false;
		m_aWheelSkidmarkUnk[0] = false;
		m_wheelStatus[i] = WHEEL_STATUS_OK;
	}

	for(i = 0; i < 4; i++){
		m_aGroundPhysical[i] = nil;
		m_aGroundOffset[i] = CVector(0.0f, 0.0f, 0.0f);
		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i] = 1.0f;
		m_aWheelTimer[i] = 0.0f;
	}

	m_nWheelsOnGround = 0;
	m_nDriveWheelsOnGround = 0;
	m_nDriveWheelsOnGroundPrev = 0;
	m_fHeightAboveRoad = 0.0f;
	m_fTraction = 1.0f;

	CColModel *colModel = mi->GetColModel();
	if(colModel->lines == nil){
		colModel->lines = (CColLine*)RwMalloc(4*sizeof(CColLine));
		colModel->numLines = 4;
	}
	// BUG? this would make more sense in the if above
	colModel->lines[0].p0.z = FAKESUSPENSION;

	SetupSuspensionLines();

	AutoPilot.m_nCarMission = MISSION_NONE;
	AutoPilot.m_nTempAction = TEMPACT_NONE;
	AutoPilot.m_nTimeToStartMission = (*Timer_GetTimeInMilliseconds());
	AutoPilot.m_bStayInCurrentLevel = false;

	SetStatus(STATUS_SIMPLE);
	bUseCollisionRecords = true;
	m_nNumPassengers = 0;
	bIsVan = false;
	bIsBus = false;
	bIsBig = false;
	bLowVehicle = false;
	bPedPhysics = false;

	bLeanMatrixClean = false;
	m_leanMatrix = GetMatrix();
}

void
CBike::SetModelIndex(uint32 id)
{
	CVehicle::SetModelIndex(id);
	SetupModelNodes();
}

#define SAND_SLOWDOWN (0.02f)
CVector vecTestResistance(0.9995f, 0.9f, 0.95f);
float fDAxisX = 1.0f;
float fDAxisXExtra = 100.0f;
float fDAxisY = 1000.0f;
float fInAirXRes = 0.98f;
float fFlySpeedMult = -0.6f;

#pragma optimize("", off) // a workaround for another compiler bug =P, original had optimize off for this function too though

void
CBike::ProcessControl(void)
{
	int i;
	float wheelRot;
	float acceleration = 0.0f;
	bool bBalancedByRider = false;
	bool bStuckInSand = false;
	float brake = 0.0f;
	CColModel *colModel = GetColModel();
	float wheelScale = ((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())))->m_wheelScale; // rouz edit (ChatGPT)
	bWarnedPeds = false;
	bLeanMatrixClean = false;
	m_doingBurnout = 0;
	bExtraSpeed = false;
	bRestingOnPhysical = false;

	// rouz edit, fix wheel rotation bug
	for(i=0; i < 2; i++)
		m_aWheelRotation[i] = rangewrap(m_aWheelRotation[i], 0., TWOPI);

	if(Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;

	ProcessCarAlarm();

	ActivateBombWhenEntered();

	// Enter the C rubbish algorithm with the original owner expressions
	Rubbish_StirUp(this);

	UpdateClumpAlpha();

	AutoPilot.m_bSlowedDownBecauseOfCars = false;
	AutoPilot.m_bSlowedDownBecauseOfPeds = false;

	switch(GetStatus()){
	case STATUS_PLAYER:
		bBalancedByRider = true;
		bIsBeingPickedUp = false;
		if(FindPlayerPed()->GetPedState() != PED_EXIT_CAR && FindPlayerPed()->GetPedState() != PED_DRAG_FROM_CAR){
			ProcessControlInputs(0);

			// Lean back
			if(m_fLeanInput < 0.0f){
				m_vecCentreOfMass.y = pHandling->CentreOfMass.y + pBikeHandling->fLeanBakCOM*m_fLeanInput;
				CVector com = m_vecCentreOfMass;
#ifdef FIX_BUGS
				// center of mass has to have world space orientation. unfortunately we can't do wheelies
				// at high speed then, flipping y here is like riding south without this fix where wheelies work
				com.y = -com.y;
				com = Multiply3x3(GetMatrix(), com);
#endif
				if(m_fBrakePedal == 0.0f && !bIsHandbrakeOn || m_nWheelsOnGround == 0){
					// Delegate the lean back force to the C implementation
					float force = Bike_LeanForce(m_fLeanInput, m_fTurnMass, pBikeHandling->fLeanBackForce, m_vecMoveSpeed.Magnitude());
					force *= Bike_LeanBackGasScale(GetModelIndex() == MI_SANCHEZ, m_fGasPedal);
					ApplyTurnForce(-force*(*Timer_GetTimeStep())*GetUp(), com+GetForward());
				}
			}
			// Lean forward
			else
			{
				m_vecCentreOfMass.y = pHandling->CentreOfMass.y + pBikeHandling->fLeanFwdCOM*m_fLeanInput;
				CVector com = m_vecCentreOfMass;
#ifdef FIX_BUGS
				// see above
				com.y = -com.y;
				com = Multiply3x3(GetMatrix(), com);
#endif
				if(m_fBrakePedal < 0.0f || m_nWheelsOnGround == 0){
					// Delegate the lean forward force to the C implementation
					float force = Bike_LeanForce(m_fLeanInput, m_fTurnMass, pBikeHandling->fLeanFwdForce, m_vecMoveSpeed.Magnitude());
					ApplyTurnForce(-force*(*Timer_GetTimeStep())*GetUp(), com+GetForward());
				}
			}

			PruneReferences();

			if(GetStatus() == STATUS_PLAYER && !RecordChase_IsRecording()) // rouz edit (ChatGPT)
				DoDriveByShootings();

			if(m_aSuspensionSpringRatio[0] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[0].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
			   m_aSuspensionSpringRatio[1] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[1].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
			   m_aSuspensionSpringRatio[2] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[2].surfaceB) == ADHESIVE_SAND || // rouz edit (ChatGPT)
			   m_aSuspensionSpringRatio[3] < 1.0f && SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[3].surfaceB) == ADHESIVE_SAND){ // rouz edit (ChatGPT)
				CVector parallelSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetUp())*GetUp();
				if(m_fGasPedal > 0.3f){
					if(parallelSpeed.MagnitudeSqr() < SQR(0.3f))
						bStuckInSand = true;
					parallelSpeed -= DotProduct(parallelSpeed, GetForward())*GetForward();
				}
				ApplyMoveForce(parallelSpeed * -(*Timer_GetTimeStep())*SAND_SLOWDOWN*m_fMass);
			}
		}
		if(CPad::GetPad(0)->WeaponJustDown())
			ActivateBomb();
		break;

	case STATUS_PLAYER_PLAYBACKFROMBUFFER:
		bBalancedByRider = true;
		break;

	case STATUS_SIMPLE:
		CCarAI::UpdateCarAI(this);
		CPhysical::ProcessControl();
		CCarCtrl::UpdateCarOnRails(this);

		m_nWheelsOnGround = 2;
		m_nDriveWheelsOnGroundPrev = m_nDriveWheelsOnGround;
		m_nDriveWheelsOnGround = 2;

		Transmission_CalculateGearForSimpleCar(&pHandling->Transmission, AutoPilot.m_fMaxTrafficSpeed/50.0f, &m_nCurrentGear);

		wheelRot = ProcessWheelRotation(WHEEL_STATE_NORMAL, GetForward(), m_vecMoveSpeed, 0.5f*wheelScale);
		for(i = 0; i < 2; i++)
			m_aWheelRotation[i] += wheelRot;

		PlayHornIfNecessary();
		ReduceHornCounter();
		bVehicleColProcessed = false;
		bAudioChangingGear = false;
		bWheelieCam = false;
		// that's all we do for simple vehicles
		return;

	case STATUS_PHYSICS:
		CCarAI::UpdateCarAI(this);
		CCarCtrl::SteerAICarWithPhysics(this);
		PlayHornIfNecessary();

		bBalancedByRider = true;
		bWheelieCam = false;

		if(bIsBeingCarJacked){
			m_fGasPedal = 0.0f;
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}else
			bIsBeingPickedUp = false;
		break;

	case STATUS_ABANDONED:
		m_fBrakePedal = 0.0f;
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f) || bIsStanding)
			bIsHandbrakeOn = true;
		else	
			bIsHandbrakeOn = false;

		m_fGasPedal = 0.0f;
#ifdef FIX_BUGS
		if(!IsAlarmOn())
#endif
			m_nCarHornTimer = 0;

		bBalancedByRider = (pDriver || pPassengers[0] || bIsBeingCarJacked) && !bIsStanding;
		m_fPedLeanAmountLR = 0.0f;
		m_fPedLeanAmountUD = 0.0f;
		bWheelieCam = false;

		if(bIsBeingCarJacked){
			m_fGasPedal = 0.0f;
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}

		// rouz remote control
		if (rouz.remote_control && this == rouz.rc_veh)
		{
			bIsHandbrakeOn = false;

			m_fGasPedal = 1.f * ((float) Input_GetChar('I') - (float) Input_GetChar('K')); // rouz edit (ChatGPT)
			m_fBrakePedal = 1.f * (float) Input_GetChar('K'); // rouz edit (ChatGPT)
			m_fSteerAngle = 30.f*PI/180.f * ((float) Input_GetChar('J') - (float) Input_GetChar('L')); // rouz edit (ChatGPT)

			// Handle reverse
			float fwd_speed = DotProduct(m_vecMoveSpeed, GetForward());
			if (fwd_speed <= 0.f)
			{
				// Brake if we're trying to go forward
				if (m_fGasPedal > 0.f && fwd_speed < 0.f)
					m_fBrakePedal = 1.f;
				else
					m_fBrakePedal = 0.f;
			}

			bBalancedByRider = 1;
			bIsStanding = 0;
		}
		break;

	case STATUS_WRECKED:
		m_fBrakePedal = 0.05f;
		bIsHandbrakeOn = true;

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
#ifdef FIX_BUGS
		if(!IsAlarmOn())
#endif
			m_nCarHornTimer = 0;

		bBalancedByRider = false;
		bWheelieCam = false;
		m_fPedLeanAmountLR = 0.0f;
		m_fPedLeanAmountUD = 0.0f;
		break;

	case STATUS_PLAYER_DISABLED:
		if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.1f)){
			m_fBrakePedal = 1.0f;
			bIsHandbrakeOn = true;
		}else{
			m_fBrakePedal = 0.0f;
			bIsHandbrakeOn = false;
		}

		m_fSteerAngle = 0.0f;
		m_fGasPedal = 0.0f;
#ifdef FIX_BUGS
		if(!IsAlarmOn())
#endif
			m_nCarHornTimer = 0;

		bBalancedByRider = true;
		bWheelieCam = false;
		break;
	}

	if(bIsStanding)
		// Delegate the check that the bike tipped over to the C implementation
		if(VehicleLogic_BikeTippedOver(GetRight().z, GetForward().z))
			bIsStanding = false;

	if(bBalancedByRider || bIsBeingPickedUp || bIsStanding){
		// Delegate the turn resistance to the C implementation
		CVector localTurnSpeed = Multiply3x3(m_vecTurnSpeed, GetMatrix());
		float turn[2];
		Bike_TurnResistance(GetStatus() == STATUS_PLAYER, m_aWheelTimer[BIKESUSP_F1] == 0.0f && m_aWheelTimer[BIKESUSP_F2] == 0.0f,
			m_aWheelTimer[BIKESUSP_R1] == 0.0f && m_aWheelTimer[BIKESUSP_R2] == 0.0f, GetForward().z, vecTestResistance.x, vecTestResistance.y,
			fDAxisX, fDAxisXExtra, fDAxisY, fInAirXRes, pBikeHandling->fWheelieAng, pBikeHandling->fStoppieAng,
			localTurnSpeed.x, localTurnSpeed.y, (*Timer_GetTimeStep()), turn);
		float turnX = turn[0];
		float turnY = turn[1];

		CVector res;
		res = -GetUp() * turnY * m_fTurnMass;
		ApplyTurnForce(res, GetRight() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));

		res = GetUp() * turnX * m_fTurnMass;
		ApplyTurnForce(res, GetForward() + Multiply3x3(GetMatrix(), m_vecCentreOfMass));

		if(GetStatus() != STATUS_PLAYER)
			m_vecCentreOfMass = pHandling->CentreOfMass;
	}else{
		m_vecCentreOfMass = pHandling->CentreOfMass;
		m_vecCentreOfMass.z = pBikeHandling->fNoPlayerCOMz;
	}

	// rouz window control
	rouz_veh_apply_controls_from_window((void *) this);

	// Skip physics if object is found to have been static recently
	bool skipPhysics = false;
	if (!((rouz.remote_bomb && rouz.bomb_veh == this) || (rouz.remote_control && rouz.rc_veh == this)))
	if(!bIsStuck && (GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED) && !bIsBeingPickedUp){
		bool makeStatic = false;
		if(!bVehicleColProcessed &&
		   m_vecMoveSpeed.IsZero() &&
		// BUG? m_aSuspensionSpringRatioPrev[3] is checked twice in the game. also, why 3?
		   m_aSuspensionSpringRatioPrev[3] != 1.0f)
			makeStatic = true;

		m_vecMoveSpeedAvg = (m_vecMoveSpeedAvg + m_vecMoveSpeed)/2.0f;
		m_vecTurnSpeedAvg = (m_vecTurnSpeedAvg + m_vecTurnSpeed)/2.0f;

		// Delegate the test for the bike that barely moves to the C implementation shared with cars
		if(Automobile_IsBarelyMoving(GetStatus() == STATUS_WRECKED, m_vecMoveSpeedAvg.MagnitudeSqr(), m_vecTurnSpeedAvg.MagnitudeSqr(), (*Timer_GetTimeStep()), m_fDistanceTravelled,
			false, makeStatic)){
			m_nStaticFrames++;

			if(m_nStaticFrames > 10 || makeStatic)
				if(!CCarCtrl::MapCouldMoveInThisArea(GetPosition().x, GetPosition().y)){
					if(!makeStatic || m_nStaticFrames > 10)
						m_nStaticFrames = 10;

					skipPhysics = true;

					m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
					m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
				}
		}else
			m_nStaticFrames = 0;
	}

	// Postpone
	for(i = 0; i < 4; i++)
		if(m_aGroundPhysical[i]){
			bRestingOnPhysical = true;
			if(!CWorld::bForceProcessControl && m_aGroundPhysical[i]->bIsInSafePosition){
				bWasPostponed = true;
				return;
			}
		}

	if(bRestingOnPhysical){
		skipPhysics = false;
		m_nStaticFrames = 0;
	}

	VehicleDamage();

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
// missing. BUG?
//		m_fTireTemperature = 1.0f;

		if(bIsStanding && m_fWheelAngle < DEGTORAD(20.0f))
			m_fWheelAngle += DEGTORAD(1.0f)*(*Timer_GetTimeStep());
		if(bIsStanding){
			float f = Pow(0.97f, (*Timer_GetTimeStep()));
			m_fLeanLRAngle2 = m_fLeanLRAngle2*f - (General_AsinClamped(GetRight().z)+DEGTORAD(15.0f))*(1.0f-f);
			m_fLeanLRAngle = m_fLeanLRAngle2;
		}
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

		if(!(bBalancedByRider || bIsBeingPickedUp || bIsStanding)){
			if(GetRight().z < 0.0f){
				if(m_fSteerAngle > -DEGTORAD(25.0f))
					m_fSteerAngle -= DEGTORAD(0.5f)*(*Timer_GetTimeStep());
			}else{
				if(m_fSteerAngle < DEGTORAD(25.0f))
					m_fSteerAngle += DEGTORAD(0.5f)*(*Timer_GetTimeStep());
			}
		}

		// Lean forward speed up
		float savedAirResistance = m_fAirResistance;
		if(GetStatus() == STATUS_PLAYER && pDriver){
			CAnimBlendAssociation *assoc = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_LEANF);
			if(assoc && assoc->blendAmount > 0.5f &&
			   assoc->currentTime > 0.06f && assoc->currentTime < 0.14f){
				m_fAirResistance *= 0.6f;
				if(m_fGasPedal > 0.5f && DotProduct(m_vecMoveSpeed, GetForward()) > 0.25f){
					ApplyMoveForce(0.2f*m_fMass*GRAVITY*(*Timer_GetTimeStep())*GetForward());
					bExtraSpeed = true;
				}
			}
		}

		CPhysical::ProcessControl();
		m_fAirResistance = savedAirResistance;

		ProcessBuoyancy();

		// Rescale spring ratios, i.e. subtract wheel radius
		for(i = 0; i < 4; i++){
			// Delegate the rescaling of the spring ratio to the C implementation, 0.0 is fully compressed and 1.0 is fully extended
			m_aSuspensionSpringRatio[i] = Automobile_PrevSpringRatio(m_aSuspensionSpringRatio[i], m_aSuspensionSpringLength[i], m_aSuspensionLineLength[i]);
		}

		int rnd = 0;
		float fwdSpeed = Abs(DotProduct(m_vecMoveSpeed, GetForward()));
		CVector contactPoints[4];	// relative to model
		CVector contactSpeeds[4];	// speed at contact points
		CVector springDirections[4];	// normalized, in model space

		for(i = 0; i < 4; i++){
			// Set spring under certain circumstances
			if(m_wheelStatus[i/2] == WHEEL_STATUS_MISSING)
				m_aSuspensionSpringRatio[i] = 1.0f;
			else if(m_wheelStatus[i/2] == WHEEL_STATUS_BURST){
				// wheel more bumpy the faster we are
				if(i == BIKESUSP_F1 || i == BIKESUSP_R1)
					rnd = General_GetRandomNumberInRangeInt(0, (uint16)(40*fwdSpeed) + 98) < 100;
				if(rnd){
					m_aSuspensionSpringRatio[i] += 0.3f*(m_aSuspensionLineLength[i]-m_aSuspensionSpringLength[i])/m_aSuspensionSpringLength[i];
					if(m_aSuspensionSpringRatio[i] > 1.0f)
						m_aSuspensionSpringRatio[i] = 1.0f;
				}
			}

			// get points and directions if spring is compressed
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				contactPoints[i] = CollisionVector_ToNative(m_aWheelColPoints[i].point) - GetPosition();
				springDirections[i] = Multiply3x3(GetMatrix(), CollisionVector_ToNative(colModel->lines[i].p1) - CollisionVector_ToNative(colModel->lines[i].p0));
				springDirections[i].Normalise();
			}
		}

		m_aWheelSkidmarkType[0] = m_aWheelSkidmarkType[1] = SKIDMARK_NORMAL;
		m_aWheelSkidmarkUnk[0] = m_aWheelSkidmarkUnk[1] = false;

		// Make springs push up vehicle
		for(i = 0; i < 4; i++){
			if(m_aSuspensionSpringRatio[i] < 1.0f){
				float bias = pHandling->fSuspensionBias;
				if(i == BIKESUSP_R1 || i == BIKESUSP_R2)
					bias = 1.0f - bias;

				if(m_aWheelColPoints[i].normal.z > 0.35f)
					ApplySpringCollisionAlt(pHandling->fSuspensionForceLevel, springDirections[i], contactPoints[i], m_aSuspensionSpringRatio[i], bias, m_aWheelColPoints[i]);
				else
					ApplySpringCollision(pHandling->fSuspensionForceLevel,
						springDirections[i], contactPoints[i],
						m_aSuspensionSpringRatio[i], bias);

				if(m_aWheelColPoints[i].surfaceB == SURFACE_GRASS ||
				   m_aWheelColPoints[i].surfaceB == SURFACE_MUD_DRY){
					if(i < 2)
						m_aWheelSkidmarkType[0] = SKIDMARK_MUDDY;
					else
						m_aWheelSkidmarkType[1] = SKIDMARK_MUDDY;
				}else if(m_aWheelColPoints[i].surfaceB == SURFACE_SAND ||
				         m_aWheelColPoints[i].surfaceB == SURFACE_SAND_BEACH){
					if(i < 2){
						m_aWheelSkidmarkType[0] = SKIDMARK_SANDY;
						m_aWheelSkidmarkUnk[0] = true;
					}else{
						m_aWheelSkidmarkType[1] = SKIDMARK_SANDY;
						m_aWheelSkidmarkUnk[1] = true;
					}
				}
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
		}

		CVector normal;
		if(m_aSuspensionSpringRatio[0] < 1.0f || m_aSuspensionSpringRatio[1] < 1.0f){
			normal = m_aSuspensionSpringRatio[0] < 1.0f ? CollisionVector_ToNative(m_aWheelColPoints[0].normal) : CollisionVector_ToNative(m_aWheelColPoints[1].normal);
			if(normal.z > 0.35f)
				springDirections[0] = -normal;
			normal = m_aSuspensionSpringRatio[1] < 1.0f ? CollisionVector_ToNative(m_aWheelColPoints[1].normal) : CollisionVector_ToNative(m_aWheelColPoints[0].normal);
			if(normal.z > 0.35f)
				springDirections[1] = -normal;
		}
		if(m_aSuspensionSpringRatio[2] < 1.0f || m_aSuspensionSpringRatio[3] < 1.0f){
			normal = m_aSuspensionSpringRatio[2] < 1.0f ? CollisionVector_ToNative(m_aWheelColPoints[2].normal) : CollisionVector_ToNative(m_aWheelColPoints[3].normal);
			if(normal.z > 0.35f)
				springDirections[2] = -normal;
			normal = m_aSuspensionSpringRatio[3] < 1.0f ? CollisionVector_ToNative(m_aWheelColPoints[3].normal) : CollisionVector_ToNative(m_aWheelColPoints[2].normal);
			if(normal.z > 0.35f)
				springDirections[3] = -normal;
		}

		// game has dead code here if m_vecMoveSpeed.Magnitude() < 0.01f

		// dampen springs
		for(i = 0; i < 4; i++)
			if(m_aSuspensionSpringRatio[i] < 1.0f)
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
		if(!CVehicle::bCheat3)
			gripCheat = false;
		acceleration = Transmission_CalculateDriveAcceleration(&pHandling->Transmission, &m_fGasPedal, &m_nCurrentGear, &m_fChangeGearTime, &fwdSpeed, gripCheat);
		acceleration /= m_fForceMultiplier;

		brake = m_fBrakePedal * pHandling->fBrakeDeceleration * (*Timer_GetTimeStep());
		bool neutralHandling = GetStatus() != STATUS_PLAYER && GetStatus() != STATUS_PLAYER_REMOTE && (pHandling->Flags & HANDLING_NEUTRALHANDLING);
		// Delegate the biases of the wheels to the C implementation
		float wheelBiases[4];
		Bike_WheelBiases(neutralHandling, pHandling->fBrakeBias, pHandling->fTractionBias, wheelBiases);
		float brakeBiasFront = wheelBiases[0];
		float brakeBiasRear  = wheelBiases[1];
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
				if(i == BIKESUSP_R1 || i == BIKESUSP_R2)
					m_nDriveWheelsOnGround = 1;
				if(m_nWheelsOnGround == 1)
					m_vecAvgSurfaceNormal = CollisionVector_ToNative(m_aWheelColPoints[i].normal);
				else
					m_vecAvgSurfaceNormal += CollisionVector_ToNative(m_aWheelColPoints[i].normal);
			}
		}

		if(m_nWheelsOnGround == 0)
			m_vecAvgSurfaceNormal = CVector(0.0f, 0.0f, 1.0f);
		else{
			m_vecAvgSurfaceNormal /= m_nWheelsOnGround;
			if(DotProduct(m_vecAvgSurfaceNormal, GetUp()) < -0.5f)
				m_vecAvgSurfaceNormal *= -1.0f;
		}

		// Find contact points for wheel processing
		int frontLine = m_aSuspensionSpringRatio[BIKESUSP_F1] < m_aSuspensionSpringRatio[BIKESUSP_F2] ?
			BIKESUSP_F1 : BIKESUSP_F2;
		CVector frontContact(0.0f,
			colModel->lines[BIKESUSP_F1].p0.y,
			colModel->lines[BIKESUSP_F1].p0.z - m_aSuspensionSpringRatio[frontLine]*m_aSuspensionSpringLength[BIKESUSP_F1] - 0.5f*wheelScale);
		frontContact = Multiply3x3(GetMatrix(), frontContact);

		int rearLine = m_aSuspensionSpringRatio[BIKESUSP_R1] < m_aSuspensionSpringRatio[BIKESUSP_R2] ?
			BIKESUSP_R1 : BIKESUSP_R2;
		CVector rearContact(0.0f,
			colModel->lines[BIKESUSP_R1].p0.y,
			colModel->lines[BIKESUSP_R1].p0.z - m_aSuspensionSpringRatio[rearLine]*m_aSuspensionSpringLength[BIKESUSP_R1] - 0.5f*wheelScale);
		rearContact = Multiply3x3(GetMatrix(), rearContact);

		float traction = 0.004f * m_fTraction;
		traction *= pHandling->fTractionMultiplier / 4.0f;

		// Turn wheel
		// Delegate the angle of the front wheel to the C implementation
		bool wheelLimitedByGrip = fwdSpeed > 0.01f && m_aWheelTimer[BIKESUSP_F1] > 0.0f && m_aWheelTimer[BIKESUSP_F2] > 0.0f && GetStatus() == STATUS_PLAYER;
		float gripSteer = 0.0f;
		if(wheelLimitedByGrip){
			CColPoint point;
			point.surfaceA = SURFACE_WHEELBASE;
			point.surfaceB = SURFACE_TARMAC;
			gripSteer = SurfaceTable_GetAdhesiveLimit(&(point))*4.0f*pBikeHandling->fSpeedSteer*traction; // rouz edit (ChatGPT)
			if(SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[rearLine].surfaceB) == ADHESIVE_LOOSE || // rouz edit (ChatGPT)
			   SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[rearLine].surfaceB) == ADHESIVE_SAND) // rouz edit (ChatGPT)
				gripSteer *= pBikeHandling->fSlipSteer;
		}
		m_fWheelAngle = Bike_WheelAngle(GetStatus() == STATUS_PLAYER || !bIsStanding || bIsBeingPickedUp, m_vecMoveSpeed.x, m_vecMoveSpeed.y, m_fSteerAngle, m_fWheelAngle, m_fLeanLRAngle,
			fwdSpeed, GetStatus() == STATUS_PLAYER, wheelLimitedByGrip, gripSteer, pHandling->fSteeringLock, (*Timer_GetTimeStep()));

		// rouz edit, always turn wheels with remote_control
		if (rouz.remote_control && this == rouz.rc_veh && GetStatus()==STATUS_ABANDONED)
			m_fWheelAngle = m_fSteerAngle;

		static float fThrust;
		static tWheelState WheelState[2];
		CVector initialMoveSpeed = m_vecMoveSpeed;
		bool rearWheelsFirst = !!(pHandling->Flags & HANDLING_REARWHEEL_1ST);

		// Process front wheel - first try

		if(!rearWheelsFirst){
			if(m_aWheelTimer[BIKESUSP_F1] > 0.0f || m_aWheelTimer[BIKESUSP_F2] > 0.0f){
				// Wheel on ground
				eBikeWheelSpecial spec;
				if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f)
					spec = BIKE_WHEELSPEC_0;
				else
					spec = BIKE_WHEELSPEC_2;
				// Delegate the direction of the wheel to the C implementation
				float steerXY_wheelFwd[2];
				Automobile_SteerForward(m_fWheelAngle, steerXY_wheelFwd);
				CVector wheelFwd = Multiply3x3(GetMatrix(), CVector(steerXY_wheelFwd[0], steerXY_wheelFwd[1], 0.0f));
				CVector wheelRight;
				// Delegate the wheel axes along the surface to the C implementation
				BikeSurfaceWheelAxes(wheelFwd, wheelRight, CollisionVector_ToNative(m_aWheelColPoints[frontLine].normal));

				fThrust = 0.0f;
				m_aWheelColPoints[frontLine].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesionDestab;
				float adhesion = Bike_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[frontLine])), traction, m_fBrakeDestabilization,
					SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[frontLine].surfaceB), GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[frontLine].surfaceB) : 1.0f,
					m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST, &adhesionDestab);
				WheelState[BIKEWHEEL_FRONT] = m_aWheelState[BIKEWHEEL_FRONT];
				CVector contactSpeed = GetSpeed(frontContact);
				ProcessBikeWheel(wheelFwd, wheelRight,
					contactSpeed, frontContact,
					2, fThrust,
					brake*brakeBiasFront,
					adhesion*tractionBiasFront, adhesionDestab,
					BIKEWHEEL_FRONT,
					&m_aWheelSpeed[BIKEWHEEL_FRONT],
					&WheelState[BIKEWHEEL_FRONT],
					spec,
					m_wheelStatus[BIKEWHEEL_FRONT]);
				if(bStuckInSand && (WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SPINNING || WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SKIDDING))
					WheelState[BIKEWHEEL_FRONT] = WHEEL_STATE_NORMAL;
			}else{
				// Wheel in the air
				m_aWheelSpeed[BIKEWHEEL_FRONT] *= 0.95f;
				m_aWheelRotation[BIKEWHEEL_FRONT] += m_aWheelSpeed[BIKEWHEEL_FRONT];
			}
		}

		// Process rear wheel

		if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f){
			// Wheel on ground
			float rearBrake;
			float rearTraction;

			CVector wheelFwd = GetForward();
			CVector wheelRight = GetRight();
			// Delegate the wheel axes along the surface to the C implementation
			BikeSurfaceWheelAxes(wheelFwd, wheelRight, CollisionVector_ToNative(m_aWheelColPoints[rearLine].normal));

#ifdef FIX_BUGS
			// Not sure if this is needed, but brake usually has timestep as a factor
			float handbrakeBrake = 20000.0f * Timer_GetTimeStepFix();
#else
			float handbrakeBrake = 20000.0f;
#endif
			// Delegate the brake and traction of the rear wheel to the C implementation
			int rearGrip = Bike_RearWheelGrip(bIsHandbrakeOn, handbrakeBrake, m_doingBurnout, m_fTireTemperature, m_fGasPedal, brake, traction, &rearBrake, &rearTraction);
			if(rearGrip == BIKE_REARGRIP_HANDBRAKE){
				m_fTireTemperature = 1.0f;
			}else if(rearGrip == BIKE_REARGRIP_BURNOUT){
				ApplyTurnForce(contactPoints[BIKESUSP_R1], -0.0007f*m_fTurnMass*m_fSteerAngle*GetRight()*(*Timer_GetTimeStep()));
			}else if(rearGrip == BIKE_REARGRIP_COLD_TYRES){
				ApplyTurnForce(contactPoints[BIKESUSP_R1], (1.0f-m_fTireTemperature)*-0.0007f*m_fTurnMass*m_fSteerAngle*GetRight()*(*Timer_GetTimeStep()));
			}

			if(fThrust > 0.0f && brake > 0.0f)
				brake = 0.0f;	// only affects next front wheel. is this intended?
			fThrust = acceleration;
			m_aWheelColPoints[rearLine].surfaceA = SURFACE_WHEELBASE;
			// Delegate the adhesion of the wheel to the C implementation
			float adhesionDestab;
			float adhesion = Bike_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[rearLine])), rearTraction, m_fBrakeDestabilization,
				SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[rearLine].surfaceB), GetStatus() == STATUS_PLAYER,
				GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[rearLine].surfaceB) : 1.0f,
				m_wheelStatus[BIKEWHEEL_REAR] == WHEEL_STATUS_BURST, &adhesionDestab);
			WheelState[BIKEWHEEL_REAR] = m_aWheelState[BIKEWHEEL_REAR];
			CVector contactSpeed = GetSpeed(rearContact);
			ProcessBikeWheel(wheelFwd, wheelRight,
				contactSpeed, rearContact,
				2, fThrust,
				rearBrake*brakeBiasRear,
				adhesion*tractionBiasRear, adhesionDestab,
				BIKEWHEEL_REAR,
				&m_aWheelSpeed[BIKEWHEEL_REAR],
				&WheelState[BIKEWHEEL_REAR],
				BIKE_WHEELSPEC_1,
				m_wheelStatus[BIKEWHEEL_REAR]);
			if(bStuckInSand && (WheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING || WheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SKIDDING))
				WheelState[BIKEWHEEL_REAR] = WHEEL_STATE_NORMAL;
		}else{
			// Wheel in the air
			// Delegate the speed of the rear wheel to the C implementation
			m_aWheelSpeed[BIKEWHEEL_REAR] = Bike_RearWheelAirSpeed(m_aWheelSpeed[BIKEWHEEL_REAR], acceleration, bIsHandbrakeOn);
			m_aWheelRotation[BIKEWHEEL_REAR] += m_aWheelSpeed[BIKEWHEEL_REAR];
		}

		// Delegate the temperature of the tyres to the C implementation
		m_fTireTemperature = Automobile_TireTemperature(m_fTireTemperature, m_doingBurnout && m_aWheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING, (*Timer_GetTimeStep()));

		// Process front wheel - second try

		if(rearWheelsFirst){
			if(m_aWheelTimer[BIKESUSP_F1] > 0.0f || m_aWheelTimer[BIKESUSP_F2] > 0.0f){
				// Wheel on ground
				eBikeWheelSpecial spec;
				if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f)
					spec = BIKE_WHEELSPEC_0;
				else
					spec = BIKE_WHEELSPEC_2;
				// Delegate the direction of the wheel to the C implementation
				float steerXY_wheelFwd[2];
				Automobile_SteerForward(m_fWheelAngle, steerXY_wheelFwd);
				CVector wheelFwd = GetMatrix() * CVector(steerXY_wheelFwd[0], steerXY_wheelFwd[1], 0.0f);
				CVector wheelRight;
				// Delegate the wheel axes along the surface to the C implementation
				BikeSurfaceWheelAxes(wheelFwd, wheelRight, CollisionVector_ToNative(m_aWheelColPoints[frontLine].normal));

				fThrust = 0.0f;
				m_aWheelColPoints[frontLine].surfaceA = SURFACE_WHEELBASE;
				// Delegate the adhesion of the wheel to the C implementation
				float adhesionDestab;
				float adhesion = Bike_WheelAdhesion(SurfaceTable_GetAdhesiveLimit(&(m_aWheelColPoints[frontLine])), traction, m_fBrakeDestabilization,
					SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[frontLine].surfaceB), GetStatus() == STATUS_PLAYER,
					GetStatus() == STATUS_PLAYER ? SurfaceTable_GetWetMultiplier(m_aWheelColPoints[frontLine].surfaceB) : 1.0f,
					m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST, &adhesionDestab);
				WheelState[BIKEWHEEL_FRONT] = m_aWheelState[BIKEWHEEL_FRONT];
				CVector contactSpeed = GetSpeed(frontContact);
				ProcessBikeWheel(wheelFwd, wheelRight,
					contactSpeed, frontContact,
					2, fThrust,
					brake*brakeBiasFront,
					adhesion*tractionBiasFront, adhesionDestab,
					BIKEWHEEL_FRONT,
					&m_aWheelSpeed[BIKEWHEEL_FRONT],
					&WheelState[BIKEWHEEL_FRONT],
					spec,
					m_wheelStatus[BIKEWHEEL_FRONT]);
				if(bStuckInSand && (WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SPINNING || WheelState[BIKEWHEEL_FRONT] == WHEEL_STATE_SKIDDING))
					WheelState[BIKEWHEEL_FRONT] = WHEEL_STATE_NORMAL;
			}else{
				// Wheel in the air
				m_aWheelSpeed[BIKEWHEEL_FRONT] *= 0.95f;
				m_aWheelRotation[BIKEWHEEL_FRONT] += m_aWheelSpeed[BIKEWHEEL_FRONT];
			}
		}

		// Process leaning
		float idleAngle = 0.0f;
		if(pDriver){
			CAnimBlendAssociation *assoc = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_READY);
			if(assoc)
				idleAngle = DEGTORAD(10.0f) * assoc->blendAmount;
		}
		// Delegate the lean angle to the C implementation
		float sideAccel = 0.0f;
		if(bBalancedByRider || bIsBeingPickedUp){
			m_vecAvgSurfaceRight = CrossProduct(GetForward(), m_vecAvgSurfaceNormal);
			m_vecAvgSurfaceRight.Normalise();
			if(m_nWheelsOnGround != 0)
				sideAccel = DotProduct(m_vecMoveSpeed-initialMoveSpeed, m_vecAvgSurfaceRight);
		}
		m_fLeanLRAngle2 = Bike_LeanAngle(bBalancedByRider || bIsBeingPickedUp, bIsStanding, m_nWheelsOnGround, m_fSteerAngle, pHandling->fSteeringLock, sideAccel,
			m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST, pBikeHandling->fMaxLean, pBikeHandling->fDesLean, idleAngle, GetRight().z, m_fLeanLRAngle2, GRAVITY, (*Timer_GetTimeStep()));
		m_fLeanLRAngle = m_fLeanLRAngle2;

		// Destabilize steering when braking
		if((m_aSuspensionSpringRatio[BIKESUSP_F1] < 1.0f || m_aSuspensionSpringRatio[BIKESUSP_F2] < 1.0f) &&
		   m_fBrakePedal - m_fGasPedal > 0.9f &&
		   fwdSpeed > 0.02f &&
		   !bIsHandbrakeOn){
			// Delegate the destabilisation to the C implementation
			m_fBrakeDestabilization = Bike_BrakeDestabilization(m_fBrakeDestabilization, General_GetRandomNumberInRangeFloat(0.5f, 1.0f), (*Timer_GetTimeStep()));
			ApplyTurnForce(contactPoints[BIKESUSP_R1],
				Bike_BrakeDestabForce(m_fTurnMass, m_fBrakeDestabilization, fwdSpeed, m_aSuspensionSpringRatio[BIKESUSP_R1] < 1.0f || m_aSuspensionSpringRatio[BIKESUSP_R2] < 1.0f)*GetRight()*(*Timer_GetTimeStep()));
		}else
			m_fBrakeDestabilization = 0.0f;

		// Update wheel positions from suspension
		// Delegate the heights of the wheels to the C implementation
		m_aWheelPosition[BIKEWHEEL_FRONT] = Automobile_WheelPosition(colModel->lines[frontLine].p0.z, m_aSuspensionSpringRatio[frontLine], m_aSuspensionSpringLength[frontLine], false, m_aWheelPosition[BIKEWHEEL_FRONT]);
		m_aWheelPosition[BIKEWHEEL_REAR] = Automobile_WheelPosition(colModel->lines[rearLine].p0.z, m_aSuspensionSpringRatio[rearLine], m_aSuspensionSpringLength[rearLine], false, m_aWheelPosition[BIKEWHEEL_REAR]);

		for(i = 0; i < 2; i++)
			m_aWheelState[i] = WheelState[i];
		// never spin when moving backwards
		if(m_fGasPedal < 0.0f && m_aWheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING)
			m_aWheelState[BIKEWHEEL_REAR] = WHEEL_STATE_NORMAL;

		// Process horn

		if(GetStatus() != STATUS_PLAYER){
#ifdef FIX_BUGS
			if(!IsAlarmOn())
#endif
				ReduceHornCounter();
		}else{
#ifdef FIX_BUGS
			if(!IsAlarmOn())
#endif
			{
				if(Pads[0].GetHorn())
					m_nCarHornTimer = 1;
				else
					m_nCarHornTimer = 0;
			}
		}
	}

	if(m_fHealth < 250.0f && GetStatus() != STATUS_WRECKED){
		// Car is on fire

		CVector damagePos, fireDir;

		// move fire forward if in first person
		if(this == FindPlayerVehicle() && TheCamera.GetLookingForwardFirstPerson()){
			damagePos = CVector(0.0f, 1.2f, -0.4f);
			fireDir = CVector(0.0f, 0.0f, General_GetRandomNumberInRangeFloat(0.01125f, 0.09f));
		}else{
			damagePos = ((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())))->m_positions[CAR_POS_BACKSEAT]; // rouz edit (ChatGPT)
			damagePos.z -= 0.3f;
			fireDir = General_GetRandomNumberInRangeFloat(0.02025f, 0.09f) * GetRight();
			fireDir -= General_GetRandomNumberInRangeFloat(0.02025f, 0.18f) * GetForward();
			fireDir.z = General_GetRandomNumberInRangeFloat(0.00225f, 0.018f);
		}

		damagePos = GetMatrix()*damagePos;
		CParticle::AddParticle(PARTICLE_CARFLAME, damagePos, fireDir,
			nil, 0.9f);

		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, CVector(0.0f, 0.0f, 0.0f), nil, 0.5f);

		damagePos.x += General_GetRandomNumberInRangeFloat(-0.5625f, 0.5625f),
		damagePos.y += General_GetRandomNumberInRangeFloat(-0.5625f, 0.5625f),
		damagePos.z += General_GetRandomNumberInRangeFloat(0.5625f, 2.25f);
		CParticle::AddParticle(PARTICLE_CARFLAME_SMOKE, damagePos, CVector(0.0f, 0.0f, 0.0f));

		// Blow up car after 5 seconds
		m_fFireBlowUpTimer += Timer_GetTimeStepInMilliseconds();
		if(m_fFireBlowUpTimer > 5000.0f)
			BlowUpCar(m_pSetOnFireEntity);
	}else
		m_fFireBlowUpTimer = 0.0f;

	ProcessDelayedExplosion();

	// Find out how much to shake the pad depending on suspension and ground surface

	float suspShake = 0.0f;
	float surfShake = 0.0f;
	float speedsq = m_vecMoveSpeed.MagnitudeSqr();
	for(i = 0; i < 4; i++){
		float suspChange = m_aSuspensionSpringRatioPrev[i] - m_aSuspensionSpringRatio[i];
		if(suspChange > 0.3f && (i == BIKESUSP_F1 || i == BIKESUSP_R1) && speedsq > 0.04f){
			if(GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PHYSICS){
#ifdef FIX_BUGS
				// only two wheels but 4 suspensions
				if(m_wheelStatus[i/2] == WHEEL_STATUS_BURST)
#else
				if(m_wheelStatus[i] == WHEEL_STATUS_BURST)
#endif
					DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP_2, suspChange);
				else
					DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_JUMP, suspChange);
				if(suspChange > suspShake)
					suspShake = suspChange;
			}
		}

		if(this == FindPlayerVehicle()){
			// Delegate the shake from the surface to the C implementation
			surfShake = Bike_SurfaceShake(m_aWheelColPoints[i].surfaceB, surfShake);

// BUG: this only observes one of the wheels
			TheCamera.m_bVehicleSuspenHigh = General_AbsAbove(suspChange, 0.05f);
		}

		m_aSuspensionSpringRatioPrev[i] = m_aSuspensionSpringRatio[i];
		m_aSuspensionSpringRatio[i] = 1.0f;
	}

	// Shake pad

	if((suspShake > 0.0f || surfShake > 0.0f) && GetStatus() == STATUS_PLAYER){
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

	if(!bWarnedPeds)
		CCarCtrl::ScanForPedDanger(this);

	if(bInfiniteMass){
		m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
		m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
		m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
	}else if(!skipPhysics &&
	         (acceleration == 0.0f && brake == 0.0f || GetStatus() == STATUS_WRECKED)){
		// Delegate the check that the vehicle is nearly still to the C implementation
		if(VehicleLogic_AllSpeedsBelow(m_vecMoveSpeed.x, m_vecMoveSpeed.y, m_vecMoveSpeed.z, 0.005f)){
			m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
			m_vecTurnSpeed.z = 0.0f;
		}
	}

	// Balance bike
	if(bBalancedByRider || bIsBeingPickedUp || bIsStanding){
		float onSideness = DotProduct(GetRight(), m_vecAvgSurfaceNormal);
		// Delegate the limit to the C implementation
		onSideness = VehicleLogic_ClampValue(onSideness, -1.0f, 1.0f);
		CVector worldCOM = Multiply3x3(GetMatrix(), m_vecCentreOfMass);
		// Keep bike upright
		if(bBalancedByRider){
			if (m_nWheelsOnGround || rouz.clean_bike_backflips==0)	// rouz edit, allows flying upside down
				ApplyTurnForce(-0.07f*onSideness*m_fTurnMass*GetUp()*(*Timer_GetTimeStep()), worldCOM+GetRight());
			bIsStanding = false;
		}else
			ApplyTurnForce(-0.1f*onSideness*m_fTurnMass*GetUp()*(*Timer_GetTimeStep()), worldCOM+GetRight());

		// Wheelie/Stoppie stabilization
		if(GetStatus() == STATUS_PLAYER){
			if(m_aWheelTimer[BIKESUSP_F1] == 0.0f && m_aWheelTimer[BIKESUSP_F2] == 0.0f && GetForward().z > 0.0 &&
			   !(m_aWheelTimer[BIKESUSP_R1] == 0.0f && m_aWheelTimer[BIKESUSP_R2] == 0.0f)){
				// Wheelie
				// Delegate the strength of the stabilisation to the C implementation
				float wheelieStab = Bike_WheelieStabilisation(pBikeHandling->fWheelieAng, GetForward().z, pBikeHandling->fWheelieStabMult, m_vecMoveSpeed.Magnitude());
				ApplyTurnForce(0.5f*(*Timer_GetTimeStep())*wheelieStab*m_fTurnMass*GetUp(), worldCOM+GetForward());
				ApplyTurnForce(0.5f*(*Timer_GetTimeStep())*m_fWheelAngle*pBikeHandling->fWheelieSteer*m_fTurnMass*GetRight(), worldCOM+GetForward());
			}else if(m_aWheelTimer[BIKESUSP_R1] == 0.0f && m_aWheelTimer[BIKESUSP_R2] == 0.0f && GetForward().z < 0.0 &&
			         !(m_aWheelTimer[BIKESUSP_F1] == 0.0f && m_aWheelTimer[BIKESUSP_F2] == 0.0f)){
				// Stoppie
				float speed = m_vecMoveSpeed.Magnitude();
				// Delegate the strength of the stabilisation to the C implementation
				float stoppieStab = Bike_StoppieStabilisation(pBikeHandling->fStoppieAng, GetForward().z, pBikeHandling->fStoppieStabMult, speed);
				ApplyTurnForce(0.5f*(*Timer_GetTimeStep())*stoppieStab*m_fTurnMass*GetUp(), worldCOM+GetForward());
				ApplyTurnForce(0.5f*General_MinFloat(5.0f*speed,1.0f)*(*Timer_GetTimeStep())*m_fWheelAngle*pBikeHandling->fWheelieSteer*m_fTurnMass*GetRight(), worldCOM+GetForward());
			}
		}
	}
}

#pragma optimize("", on)

void
CBike::Teleport(CVector pos)
{
	CWorld::Remove(this);

	SetPosition(pos);
	SetOrientation(0.0f, 0.0f, 0.0f);
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);

	ResetSuspension();

	CWorld::Add(this);
}

void
CBike::PreRender(void)
{
	int i;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)

	// Wheel particles

	if(m_aWheelState[BIKEWHEEL_REAR] != WHEEL_STATE_NORMAL &&
	   m_aWheelColPoints[BIKESUSP_R2].surfaceB != SURFACE_WATER && m_aWheelTimer[BIKESUSP_R2] > 0.0f){
		static float smokeSize = 0.2f;
		CVector groundPos = CollisionVector_ToNative(m_aWheelColPoints[BIKESUSP_R2].point);
		if(m_aSuspensionSpringRatioPrev[BIKESUSP_R1] < 1.0f)
			groundPos = (groundPos + CollisionVector_ToNative(m_aWheelColPoints[BIKESUSP_R1].point))/2.0f;
		// Delegate the offset of the smoke to the C implementation
		groundPos += Bike_LeanSmokeOffset(m_fLeanLRAngle, GetColModel()->boundingBox.min.z) * GetRight();
		CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,
			groundPos + CVector(0.0f, 0.0f, 0.25f), CVector(0.0f, 0.0f, 0.0f),
			nil, smokeSize);

		// Enter the C skidmark algorithm with the original owner expressions
		Skidmarks_RegisterOneType((uintptr)this, Skidmarks_BackendPosition(groundPos), GetForward().x, GetForward().y,
			m_aWheelSkidmarkType[BIKEWHEEL_REAR], &m_aWheelSkidmarkBloody[BIKEWHEEL_REAR]);

		if(m_aWheelState[BIKEWHEEL_REAR] == WHEEL_STATE_SPINNING &&
		   (SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[BIKESUSP_R2].surfaceB) == ADHESIVE_HARD || // rouz edit (ChatGPT)
		    SurfaceTable_GetAdhesionGroup(m_aWheelColPoints[BIKESUSP_R2].surfaceB) == ADHESIVE_ROAD)){ // rouz edit (ChatGPT)
			CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
				groundPos + CVector(0.0f, 0.0f, 0.25f),
				CVector(0.0f, 0.0f, 0.0f));
			CParticle::AddParticle(PARTICLE_BURNINGRUBBER_SMOKE,
				groundPos + CVector(0.0f, 0.0f, 0.25f),
				CVector(0.0f, 0.0f, 0.05f));
		}
	}else if(m_aWheelSkidmarkBloody[BIKEWHEEL_REAR] || m_aWheelSkidmarkUnk[BIKEWHEEL_REAR]){
		CVector groundPos = CollisionVector_ToNative(m_aWheelColPoints[BIKESUSP_R2].point);
		// Delegate the offset of the smoke to the C implementation
		groundPos += Bike_LeanSmokeOffset(m_fLeanLRAngle, GetColModel()->boundingBox.min.z) * GetRight();

		// Enter the C skidmark algorithm with the original owner expressions
		Skidmarks_RegisterOneType((uintptr)this, Skidmarks_BackendPosition(groundPos), GetForward().x, GetForward().y,
			m_aWheelSkidmarkType[BIKEWHEEL_REAR], &m_aWheelSkidmarkBloody[BIKEWHEEL_REAR]);
	}

	// Process lights

	// Turn lights on/off
	// Delegate the state of the lights to the C implementation
	bool shouldLightsBeOn = VehicleLogic_ShouldLightsBeOn(Clock_GetHours(), Clock_GetMinutes(), m_randomSeed, Weather_Foggyness, Weather_WetRoads);
	bLightsOn = VehicleLogic_NextLightsState(shouldLightsBeOn, bLightsOn, GetStatus() == STATUS_WRECKED, GetStatus() == STATUS_ABANDONED,
		General_ManhattanAbove(TheCamera.GetPosition().x - GetPosition().x, TheCamera.GetPosition().y - GetPosition().y, 100.0f));

	// Actually render the lights
	bool alarmOn, alarmOff;
	// Delegate the flashing of the alarm to the C implementation
	VehicleLogic_AlarmFlash(IsAlarmOn(), (*Timer_GetTimeInMilliseconds()), &alarmOn, &alarmOff);
	if(bEngineOn && bLightsOn || alarmOn || alarmOff){
		CalculateLeanMatrix();
		CVector lookVector = GetPosition() - TheCamera.GetPosition();
		float camDist = lookVector.Magnitude();
		if(camDist != 0.0f)
			lookVector *= 1.0f/camDist;
		else
			lookVector = CVector(1.0f, 0.0f, 0.0f);

		// 1.0 if directly behind car, -1.0 if in front
		float behindness = DotProduct(lookVector, GetForward());
		// 0.0 if behind car, PI if in front
		// Delegate the angle of the view to the C implementation
		float angle = Bike_ViewAngle(behindness);

		// Headlight

		CMatrix mat;
		CVector headLightPos = mi->m_positions[CAR_POS_HEADLIGHTS];
		if(GetModelIndex() == 152){	// this is the bobcat in VC, but we don't want that effect anyway
			mat.SetUnity();
			mat.RotateZ(m_fWheelAngle);
			mat = m_leanMatrix * mat;
		}else
			mat = m_leanMatrix;
		CVector light = mat * headLightPos;
		if(behindness < 0.0f){
			// In front of bike
			// Delegate the glow of the headlight to the C implementation
			float intensity, size;
			VehicleLogic_HeadlightGlow(behindness, &intensity, &size);

			if(behindness < -0.97f && camDist < 30.0f){
				// Directly in front and not too far away
				if(pHandling->Flags & HANDLING_HALOGEN_LIGHTS){
					CCoronas::RegisterCorona((uintptr)this + 6, 150, 150, 195, 255,
						light, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
				}else{
					CCoronas::RegisterCorona((uintptr)this + 6, 160, 160, 140, 255,
						light, 1.2f, 45.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_HEADLIGHT, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, angle);
				}
			}

			if(alarmOff){
				CCoronas::RegisterCorona((uintptr)this, 0, 0, 0, 0,
					light, size, 0.0f,
					CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
					CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
			}else{
				if(pHandling->Flags & HANDLING_HALOGEN_LIGHTS){
					CCoronas::RegisterCorona((uintptr)this + 1, 190*intensity, 190*intensity, 255*intensity, 255,
						light, size, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}else{
					CCoronas::RegisterCorona((uintptr)this + 1, 210*intensity, 210*intensity, 195*intensity, 255,
						light, size, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}
			}
		}else{
			CCoronas::UpdateCoronaCoors((uintptr)this, light, 50.0f*TheCamera.LODDistMultiplier, angle);
		}

		// bright light
		// Enter the C bright-light algorithm with the original arguments
		BrightLights_RegisterOneLegacy(light, GetUp(), GetRight(), GetForward(), pHandling->FrontLights + BRIGHTLIGHT_FRONT);

		// Taillight

		CVector tailLightPos = mi->m_positions[CAR_POS_TAILLIGHTS];
		light = m_leanMatrix * tailLightPos;

		// Taillight corona
		if(behindness > 0.0f){
			// Behind car
			// Delegate the glow of the taillight to the C implementation
			float intensity, size;
			Bike_TailLightGlow(behindness, m_fGasPedal >= 0.0f && m_fBrakePedal > 0.0f, &intensity, &size);

			if(m_fGasPedal < 0.0f){
				// reversing
				// no lights in this case
			}else{
				if(alarmOff){
					CCoronas::RegisterCorona((uintptr)this + 14, 0, 0, 0, 0,
						light, size, 0.0f,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}else{
					CCoronas::RegisterCorona((uintptr)this + 14, 128*intensity, 0, 0, 255,
						light, size, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STREAK, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, angle);
				}
			}
		}else{
			CCoronas::UpdateCoronaCoors((uintptr)this + 14, light, 50.0f*TheCamera.LODDistMultiplier, angle);
		}

		// bright light
		// Enter the C bright-light algorithm with the original arguments
		BrightLights_RegisterOneLegacy(light, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);

		// Light shadows
		if(!alarmOff){
			CVector pos = GetPosition();
			CVector2D fwd(GetForward());
			fwd.Normalise();
			float f = headLightPos.y + 6.0f;
			pos += CVector(f*fwd.x, f*fwd.y, 2.0f);
			CShadows::StoreCarLightShadow(this, (uintptr)this + 22, gpShadowExplosionTex, &pos,
				7.0f*fwd.x, 7.0f*fwd.y, 3.5f*fwd.y, -3.5f*fwd.x, 45, 45, 45, 7.0f);

			f = (tailLightPos.y - 2.5f) - (headLightPos.y + 6.0f);
			pos += CVector(f*fwd.x, f*fwd.y, 0.0f);
			CShadows::StoreCarLightShadow(this, (uintptr)this + 25, gpShadowExplosionTex, &pos,
				3.0f, 0.0f, 0.0f, -3.0f, 35, 0, 0, 4.0f);
		}

		if(this == FindPlayerVehicle() && !alarmOff){
			// Enter the C point-light algorithm with the original owner expressions
			PointLights_AddLightLegacy(POINTLIGHTS_LIGHT_DIRECTIONAL, GetPosition(), GetForward(),
				20.0f, 1.0f, 1.0f, 1.0f,
				FindPlayerVehicle()->m_vecMoveSpeed.MagnitudeSqr2D() < sq(0.45f) ? POINTLIGHTS_FOG_NORMAL : POINTLIGHTS_FOG_NONE,
				false);
			CVector pos = GetPosition() - 4.0f*GetForward();
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
	}else if(GetStatus() != STATUS_ABANDONED && GetStatus() != STATUS_WRECKED){
		// Lights off
		CalculateLeanMatrix();

		CVector tailLightPos = mi->m_positions[CAR_POS_TAILLIGHTS];
		CVector light = m_leanMatrix * tailLightPos;

		if(m_fBrakePedal > 0.0f || m_fGasPedal < 0.0f){
			CVector lookVector = GetPosition() - TheCamera.GetPosition();
			lookVector.Normalise();
			float behindness = DotProduct(lookVector, GetForward());
			if(behindness > 0.0f){
				if(m_fGasPedal < 0.0f){
					// reversing
					// no lights in this case
				}else{
					// braking
					CCoronas::RegisterCorona((uintptr)this + 14, 120, 0, 0, 255,
						light, 1.2f, 50.0f*TheCamera.LODDistMultiplier,
						CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
						CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
					// Enter the C bright-light algorithm with the original arguments
					BrightLights_RegisterOneLegacy(light, GetUp(), GetRight(), GetForward(), pHandling->RearLights + BRIGHTLIGHT_REAR);
				}
			}else{
				CCoronas::UpdateCoronaCoors((uintptr)this + 14, light, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
			}
		}else{
			CCoronas::UpdateCoronaCoors((uintptr)this + 14, light, 50.0f*TheCamera.LODDistMultiplier, 0.0f);
		}
	}


	// Wheel particles

	float fwdSpeed = DotProduct(m_vecMoveSpeed, GetForward())*180.0f;
	int drawParticles = General_AbsBelow(fwdSpeed, 90.0f);
	int susp = BIKESUSP_F1;
	for(i = 0; i < 2; i++){
		if(i == BIKEWHEEL_REAR)
			susp = BIKESUSP_R1;

		// Sparks for friction of burst wheels
		// Delegate the number of sparks to the C implementation
		int sparkCount = Bike_BurstWheelSparkCount(m_vecMoveSpeed.MagnitudeSqr(), m_aWheelColPoints[susp].surfaceB);
		if(m_wheelStatus[i] == WHEEL_STATUS_BURST && m_aSuspensionSpringRatioPrev[susp] < 1.0f && sparkCount > 0){
			CVector normalSpeed = CollisionVector_ToNative(m_aWheelColPoints[susp].normal) * DotProduct(CollisionVector_ToNative(m_aWheelColPoints[susp].normal), m_vecMoveSpeed);
			CVector frictionSpeed = m_vecMoveSpeed - normalSpeed;
			CVector sparkDir = 0.25f*frictionSpeed;
			for(int k = 0; k < sparkCount; k++)
				CParticle::AddParticle(PARTICLE_SPARK_SMALL, CollisionVector_ToNative(m_aWheelColPoints[susp].point), sparkDir);
		}else if(m_aSuspensionSpringRatioPrev[i] < 1.0f &&
		         (fwdSpeed > 0.2f || m_aWheelState[i] == WHEEL_STATE_SPINNING)){
			if(m_aWheelColPoints[susp].surfaceB == SURFACE_GRASS ||
			   m_aWheelColPoints[susp].surfaceB == SURFACE_MUD_DRY ||
			   m_aWheelColPoints[susp].surfaceB == SURFACE_SAND ||
			   m_aWheelColPoints[susp].surfaceB == SURFACE_SAND_BEACH)
				AddWheelDirtAndWater(&m_aWheelColPoints[susp], drawParticles);
		}
	}

	AddDamagedVehicleParticles();
	CShadows::StoreShadowForVehicle(this, VEH_SHD_TYPE_BIKE);

	CMatrix mat;
	CVector pos;
	CColModel *colModel = mi->GetColModel();

	// Wheel rotation
	// Delegate the direction of the wheel to the C implementation
	float steerXY_frontWheelFwd[2];
	Automobile_SteerForward(m_fSteerAngle, steerXY_frontWheelFwd);
	CVector frontWheelFwd = Multiply3x3(GetMatrix(), CVector(steerXY_frontWheelFwd[0], steerXY_frontWheelFwd[1], 0.0f));
	CVector rearWheelFwd = GetForward();
	if(m_aWheelTimer[BIKESUSP_F1] > 0.0f || m_aWheelTimer[BIKESUSP_F2] > 0.0f){
		// Delegate the lower ratio to the C implementation
		float springRatio = General_MinFloat(m_aSuspensionSpringRatioPrev[BIKESUSP_F1], m_aSuspensionSpringRatioPrev[BIKESUSP_F2]);
		CVector contactPoint(0.0f,
			(colModel->lines[BIKESUSP_F1].p0.y - colModel->lines[BIKESUSP_F2].p0.y)/2.0f,
			Bike_WheelContactZ(colModel->lines[BIKESUSP_F1].p0.z, m_aSuspensionSpringLength[BIKESUSP_F1], springRatio, mi->m_wheelScale));
		CVector contactSpeed = GetSpeed(contactPoint);
		// Why is wheel state always normal?
		m_aWheelSpeed[BIKEWHEEL_FRONT] = ProcessWheelRotation(WHEEL_STATE_NORMAL, frontWheelFwd, contactSpeed, 0.5f*mi->m_wheelScale);
		m_aWheelRotation[BIKEWHEEL_FRONT] += m_aWheelSpeed[BIKEWHEEL_FRONT];
	}
	if(m_aWheelTimer[BIKESUSP_R1] > 0.0f || m_aWheelTimer[BIKESUSP_R2] > 0.0f){
		// Delegate the lower ratio to the C implementation
		float springRatio = General_MinFloat(m_aSuspensionSpringRatioPrev[BIKESUSP_R1], m_aSuspensionSpringRatioPrev[BIKESUSP_R2]);
		CVector contactPoint(0.0f,
			(colModel->lines[BIKESUSP_R1].p0.y - colModel->lines[BIKESUSP_R2].p0.y)/2.0f,
			Bike_WheelContactZ(colModel->lines[BIKESUSP_R1].p0.z, m_aSuspensionSpringLength[BIKESUSP_R1], springRatio, mi->m_wheelScale));
		CVector contactSpeed = GetSpeed(contactPoint);
		m_aWheelSpeed[BIKEWHEEL_REAR] = ProcessWheelRotation(m_aWheelState[BIKEWHEEL_REAR], rearWheelFwd, contactSpeed, 0.5f*mi->m_wheelScale);
		m_aWheelRotation[BIKEWHEEL_REAR] += m_aWheelSpeed[BIKEWHEEL_REAR];
	}

	// Front fork
	if(m_aBikeNodes[BIKE_FORKS_FRONT]){
		mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_FORKS_FRONT]));
		pos = mat.GetPosition();

		RwMatrix rwrot;
		// TODO: this looks like some weird ctor we don't have
		CMatrix rot;
		rot.m_attachment = &rwrot;
		rot.SetUnity();
		rot.UpdateRW();

		// Make rotation matrix with front fork as axis
		// Delegate the direction of the fork to the C implementation
		float forkY, forkZ;
		Bike_ForkAxis(mi->m_bikeSteerAngle, &forkY, &forkZ);
		CVector forkAxis(0.0f, forkY, forkZ);
		forkAxis.Normalise();	// as if that's not already the case
		CQuaternion quat;
		// Use the C quaternion operation at the original animation boundary
		Quaternion_SetAxisAngle(&quat,Quaternion_VectorRef(forkAxis),-m_fWheelAngle);
		// Use the C quaternion operation at the original animation boundary
		Quaternion_GetMatrix(&quat,Quaternion_MatrixOut(*rot.m_attachment));
		rot.Update();

		// Transform fork
		mat.SetUnity();
		mat = mat * rot;
		mat.Translate(pos);
		mat.UpdateRW();

		if(m_aBikeNodes[BIKE_HANDLEBARS]){
			// Transform handle
			mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_HANDLEBARS]));
			pos = mat.GetPosition();
			if(GetStatus() == STATUS_ABANDONED || GetStatus() == STATUS_WRECKED){
				mat.SetUnity();
				mat = mat * rot;
				mat.Translate(pos);
			}else
				mat.SetTranslate(mat.GetPosition());
			mat.UpdateRW();
		}
	}

	// Rear fork
	if(m_aBikeNodes[BIKE_FORKS_REAR]){
		mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_FORKS_REAR]));
		pos = mat.GetPosition();
		// Delegate the angle of the rear fork to the C implementation
		mat.SetRotate(Bike_RearForkAngle(m_aWheelPosition[BIKEWHEEL_REAR], m_aWheelBasePosition[BIKEWHEEL_REAR], m_fRearForkLength), 0.0f, 0.0f);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Front wheel
	mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_WHEEL_FRONT]));
	pos.x = mat.GetPosition().x;
	// Delegate the position of the front wheel to the C implementation
	Bike_FrontWheelPos(colModel->lines[BIKESUSP_F1].p0.y, colModel->lines[BIKESUSP_F2].p0.y, m_aWheelPosition[BIKEWHEEL_FRONT], m_aWheelBasePosition[BIKEWHEEL_FRONT],
		m_fFrontForkZ, m_fFrontForkY, m_fFrontForkSlope, &pos.y, &pos.z);
	if(m_wheelStatus[BIKEWHEEL_FRONT] == WHEEL_STATUS_BURST)
		// Delegate the wobble of the burst wheel to the C implementation
		mat.SetRotate(m_aWheelRotation[BIKEWHEEL_FRONT], 0.0f, Bike_BurstWheelWobble(m_aWheelRotation[BIKEWHEEL_FRONT], 0.05f));
	else
		mat.SetRotateX(m_aWheelRotation[BIKEWHEEL_FRONT]);
	mat.Translate(pos);
	mat.UpdateRW();
	// and mudguard
	mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_MUDGUARD]));
	mat.SetTranslateOnly(pos);
	mat.UpdateRW();

	// Rear wheel
	mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_WHEEL_REAR]));
	pos = mat.GetPosition();
	if(m_wheelStatus[BIKEWHEEL_REAR] == WHEEL_STATUS_BURST)
		// Delegate the wobble of the burst wheel to the C implementation
		mat.SetRotate(m_aWheelRotation[BIKEWHEEL_REAR], 0.0f, Bike_BurstWheelWobble(m_aWheelRotation[BIKEWHEEL_REAR], 0.07f));
	else
		mat.SetRotateX(m_aWheelRotation[BIKEWHEEL_REAR]);
	mat.Translate(pos);
	mat.UpdateRW();

	// Chassis
	if(m_aBikeNodes[BIKE_CHASSIS]){
		mat.Attach(RwFrameGetMatrix(m_aBikeNodes[BIKE_CHASSIS]));
		pos = mat.GetPosition();
		// Delegate the height of the chassis to the C implementation
		pos.z = Bike_ChassisOffsetZ(m_fLeanLRAngle, colModel->boundingBox.min.z);
		// Delegate the roll of the chassis to the C implementation
		mat.SetRotateX(Bike_ChassisRoll(m_fLeanLRAngle));
		mat.RotateY(m_fLeanLRAngle);
		mat.Translate(pos);
		mat.UpdateRW();
	}

	// Exhaust smoke
	if(bEngineOn && !(pHandling->Flags & HANDLING_NO_EXHAUST) && fwdSpeed < 130.0f){
		CVector exhaustPos = mi->m_positions[CAR_POS_EXHAUST];
		CVector pos1, pos2, dir;

		if(exhaustPos != CVector(0.0f, 0.0f, 0.0f)){
			dir.z = 0.0f;
			if(fwdSpeed < 10.0f){
				// Delegate the direction of the wheel to the C implementation
				float steerXY[2];
				Automobile_SteerForward(m_fSteerAngle, steerXY);
				CVector steerFwd(steerXY[0], steerXY[1], 0.0f);
				steerFwd = Multiply3x3(GetMatrix(), steerFwd);
				float r = General_GetRandomNumberInRangeFloat(-0.06f, -0.03f);
				dir.x = steerFwd.x * r;
				dir.y = steerFwd.y * r;
			}else{
				dir.x = m_vecMoveSpeed.x;
				dir.y = m_vecMoveSpeed.y;
			}

			bool dblExhaust = false;
			pos1 = GetMatrix() * exhaustPos;
			if(pHandling->Flags & HANDLING_DBL_EXHAUST){
				dblExhaust = true;
				pos2 = exhaustPos;
				pos2.x = -pos2.x;
				pos2 = GetMatrix() * pos2;
			}

			static float fumesLimit = 2.0f;
			if(General_GetRandomNumberInRangeFloat(1.0f, 3.0f)*(m_fGasPedal+1.1f) > fumesLimit){
				CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos1, dir);
				if(dblExhaust)
					CParticle::AddParticle(PARTICLE_EXHAUST_FUMES, pos2, dir);

				if(GetStatus() == STATUS_PLAYER && ((*Timer_GetFrameCounter())&3) == 0 &&
				   Weather_Rain == 0.0f){
					CVector camDist = GetPosition() - TheCamera.GetPosition();
					if(DotProduct(GetForward(), camDist) > 0.0f ||
					   TheCamera.GetLookDirection() == LOOKING_LEFT ||
					   TheCamera.GetLookDirection() == LOOKING_RIGHT){
						if(dblExhaust)
							pos1 = 0.5f*pos1 + 0.5f*pos2;

						if(TheCamera.GetLookDirection() == LOOKING_LEFT ||
						   TheCamera.GetLookDirection() == LOOKING_RIGHT)
							pos1 -= 0.2f*GetForward();

						CParticle::AddParticle(PARTICLE_HEATHAZE, pos1, CVector(0.0f, 0.0f, 0.0f));
					}
				}
			}
		}
	}
}

void
CBike::Render(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)

	m_nSetPieceExtendedRangeTime = (*Timer_GetTimeInMilliseconds()) + 3000;
	mi->SetVehicleColour(m_currentColour1, m_currentColour2);
	CEntity::Render();
}

int32
CBike::ProcessEntityCollision(CEntity *ent, CColPoint *colpoints)
{
	int i;
	CColModel *colModel;

	if(GetStatus() != STATUS_SIMPLE)
		bVehicleColProcessed = true;

	colModel = GetColModel();

	int numWheelCollisions = 0;
	float prevRatios[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
	for(i = 0; i < 4; i++)
		prevRatios[i] = m_aSuspensionSpringRatio[i];

	if(m_bIsVehicleBeingShifted || bSkipLineCol || ent->IsPed() ||
	   GetModelIndex() == MI_DODO && ent->IsVehicle())
		colModel->numLines = 0;

	int numCollisions = CCollision::ProcessColModels(GetMatrix(), *colModel,
		ent->GetMatrix(), *ent->GetColModel(),
		colpoints,
		m_aWheelColPoints, m_aSuspensionSpringRatio);

	// m_aSuspensionSpringRatio are now set to the point where the tyre touches ground.
	// In ProcessControl these will be re-normalized to ignore the tyre radius.
	if(colModel->numLines){
		for(i = 0; i < 4; i++){
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

extern float get_horizon_angle_of_attack(void *p_ptr);

void
CBike::ProcessControlInputs(uint8 pad)
{
	float speed = DotProduct(m_vecMoveSpeed, GetForward());

	if(CPad::GetPad(pad)->GetExitVehicle())
		bIsHandbrakeOn = true;
	else
		bIsHandbrakeOn = !!CPad::GetPad(pad)->GetHandBrake();

	// Steer left/right
#ifdef FIX_BUGS
	if(CCamera::m_bUseMouse3rdPerson && !CVehicle::m_bDisableMouseSteering){
		if(Input_GetMouseX() != 0.0f){ // rouz edit (ChatGPT)
			// Delegate the steering with the mouse to the C implementation shared with cars
			m_fSteerInput = AutomobileLogic_SteerMouse(m_fSteerInput, Input_GetMouseX(), fMouseSteerSens, fMouseCentreRange, fMouseCentreMult, (*Timer_GetTimeStep()));
			nLastControlInput = 2;
		}else if(CPad::GetPad(pad)->GetSteeringLeftRight() || nLastControlInput != 2){
			// mouse hasn't move, steer with pad like below
			// Delegate the steering step to the C implementation shared with cars
			m_fSteerInput = AutomobileLogic_SteerPad(m_fSteerInput, CPad::GetPad(pad)->GetSteeringLeftRight(), (*Timer_GetTimeStep()));
			nLastControlInput = 0;
		}
	}else
#endif
	{
		// Delegate the steering step to the C implementation shared with cars
		m_fSteerInput = AutomobileLogic_SteerPad(m_fSteerInput, CPad::GetPad(pad)->GetSteeringLeftRight(), (*Timer_GetTimeStep()));
		nLastControlInput = 0;
	}
	// Delegate the limit to the C implementation
	m_fSteerInput = VehicleLogic_ClampValue(m_fSteerInput, -1.0f, 1.0f);

	// Lean forward/backward
	float updown;
#ifdef FREE_CAM
	if (CCamera::bFreeCam) updown = Input_affectedByController ? -CPad::GetPad(pad)->GetSteeringUpDown()/128.0f : CPad::GetPad(pad)->GetCarGunUpDown()/128.0f; // rouz edit (ChatGPT)
	else
#endif
	updown = -CPad::GetPad(pad)->GetSteeringUpDown()/128.0f + CPad::GetPad(pad)->GetCarGunUpDown()/128.0f;
	float lean_mul = 1.f;
	if (rouz.flight && m_nWheelsOnGround==0)
	{
		CVector normal_vec = this->GetUp();
		lean_mul = rouz.bike_midair_lean_mul + sinf(fabs(get_horizon_angle_of_attack((void *) this)));
	}

	// Delegate the lean step to the C implementation
	m_fLeanInput = BikeLogic_LeanInput(m_fLeanInput, updown, lean_mul, (*Timer_GetTimeStep()));

	// Accelerate/Brake
	// Delegate the pedals to the C implementation
	bool burnout;
	BikeLogic_Pedals(speed, CPad::GetPad(pad)->GetAccelerate(), CPad::GetPad(pad)->GetBrake(), GetModelIndex() == MI_DODO, &m_fGasPedal, &m_fBrakePedal, &burnout);
	if(burnout)
		m_doingBurnout = 1;

	// Actually turn wheels
	// Delegate the angle of the wheels to the C implementation shared with cars
	m_fSteerAngle = AutomobileLogic_WheelAngle(m_fSteerInput, pHandling->fSteeringLock);

	if(bComedyControls){
		// Delegate the pedals and steering of the comedy controls to the C implementation
		BikeLogic_ComedyControls((*Timer_GetTimeInMilliseconds()), &m_fGasPedal, &m_fBrakePedal, &m_fSteerAngle);
		bIsHandbrakeOn = false;
	}

	// Brake if player isn't in control
	// BUG: game always uses pad 0 here
	if(CPad::GetPad(pad)->ArePlayerControlsDisabled()){
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
CBike::ProcessBuoyancy(void)
{
	int i;
	CVector impulse, point;

	if(mod_Buoyancy.ProcessBuoyancy(this, m_fBuoyancy, &point, &impulse)){
		bTouchingWater = true;
		ApplyMoveForce(impulse);
		ApplyTurnForce(impulse, point);

		// Delegate the ratio of the impulse and the water resistance to the C implementation shared with cars
		float impulseRatio = Automobile_WaterImpulseRatio(impulse.z, m_fMass, (*Timer_GetTimeStep()));
		float waterResistance = Automobile_WaterResistance(impulseRatio, (*Timer_GetTimeStep()));
		m_vecMoveSpeed *= waterResistance;
		m_vecTurnSpeed *= waterResistance;

		// Delegate the test for the drowning to the C implementation
		if(Bike_IsDrowning(impulseRatio, m_aSuspensionSpringRatio[0] == 1.0f || m_aSuspensionSpringRatio[1] == 1.0f ||
			m_aSuspensionSpringRatio[2] == 1.0f || m_aSuspensionSpringRatio[3] == 1.0f)){
			bIsInWater = true;
			bIsDrowning = true;
			if(m_vecMoveSpeed.z < -0.1f)
				m_vecMoveSpeed.z = -0.1f;

			if(pDriver){
				pDriver->bIsInWater = true;
				if(pDriver->IsPlayer() || !bWaterTight){
					if(m_aSuspensionSpringRatio[0] < 1.0f ||
					   m_aSuspensionSpringRatio[1] < 1.0f ||
					   m_aSuspensionSpringRatio[2] < 1.0f ||
					   m_aSuspensionSpringRatio[3] < 1.0f)
						pDriver->InflictDamage(nil, WEAPONTYPE_DROWNING, (*Timer_GetTimeStep()), PEDPIECE_TORSO, 0);
					else
						KnockOffRider(WEAPONTYPE_DROWNING, 0, pDriver, false);
				}
			}
			for(i = 0; i < m_nNumMaxPassengers; i++)
				if(pPassengers[i]){
					pPassengers[i]->bIsInWater = true;
					if(pPassengers[i]->IsPlayer() || !bWaterTight){
						if(m_aSuspensionSpringRatio[0] < 1.0f ||
						   m_aSuspensionSpringRatio[1] < 1.0f ||
						   m_aSuspensionSpringRatio[2] < 1.0f ||
						   m_aSuspensionSpringRatio[3] < 1.0f)
							pPassengers[i]->InflictDamage(nil, WEAPONTYPE_DROWNING, (*Timer_GetTimeStep()), PEDPIECE_TORSO, 0);
						else
							KnockOffRider(WEAPONTYPE_DROWNING, 0, pPassengers[i], false);
					}
				}
		}else{
			bIsInWater = false;
			bIsDrowning = false;
		}
	}else{
		bIsInWater = false;
		bIsDrowning = false;
		bTouchingWater = false;
	}
}

void
CBike::DoDriveByShootings(void)
{
	CAnimBlendAssociation *anim;
	CPlayerInfo* playerInfo = ((CPlayerPed*)pDriver)->GetPlayerInfoForThisPlayerPed();
	if (playerInfo && !playerInfo->m_bDriveByAllowed)
		return;

	// rouz edit, prevents problems when being a passenger
	if (pDriver==NULL)
		return;

	CWeapon *weapon = pDriver->GetWeapon();
	if(WeaponInfo_GetWeaponInfo(weapon->m_eWeaponType)->m_nWeaponSlot != 5)
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

	if(lookingLeft || lookingRight || CPad::GetPad(0)->GetCarGunFired()){
		if(lookingLeft){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_RHS);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_FORWARD);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_LHS);
			if(anim == nil || anim->blendDelta < 0.0f)
				// Use the complete C animation manager at the original call boundary
				anim = AnimManager_AddAnimation(pDriver->GetClump(), m_bikeAnimType, ANIM_BIKE_DRIVEBY_LHS);
		}else if(lookingRight){
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_LHS);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_FORWARD);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_RHS);
			if(anim == nil || anim->blendDelta < 0.0f)
				// Use the complete C animation manager at the original call boundary
				anim = AnimManager_AddAnimation(pDriver->GetClump(), m_bikeAnimType, ANIM_BIKE_DRIVEBY_RHS);
		}else{
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_RHS);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_LHS);
			if(anim)
				anim->blendDelta = -1000.0f;
			anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_FORWARD);
			if(anim == nil || anim->blendDelta < 0.0f)
				// Use the complete C animation manager at the original call boundary
				anim = AnimManager_AddAnimation(pDriver->GetClump(), m_bikeAnimType, ANIM_BIKE_DRIVEBY_FORWARD);
		}

		// Use the complete C association at the original native operation boundary
		if (!anim || !AnimAssociation_IsRunning(anim)) {
			if (CPad::GetPad(0)->GetCarGunFired() && (*Timer_GetTimeInMilliseconds()) > weapon->m_nTimer) {
				weapon->FireFromCar(this, lookingLeft, lookingRight);
				weapon->m_nTimer = (*Timer_GetTimeInMilliseconds()) + 70;
			}
		}
	}else{
		weapon->Reload();
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_LHS);
		if(anim)
			anim->blendDelta = -1000.0f;
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_RHS);
		if(anim)
			anim->blendDelta = -1000.0f;
		anim = RpAnimBlendClumpGetAssociation(pDriver->GetClump(), ANIM_BIKE_DRIVEBY_FORWARD);
		if(anim)
			anim->blendDelta = -1000.0f;
	}
}

void
CBike::VehicleDamage(void)
{
	float impulse = m_fDamageImpulse;
	// Delegate the speed of the collision to the C implementation
	float colSpeed = Bike_CollisionSpeed(impulse, m_fMass, GetStatus() == STATUS_PLAYER, VehicleCreatedBy == MISSION_VEHICLE);

	if(!bCanBeDamaged)
		return;

	if(GetStatus() == STATUS_PLAYER && Stats_GetPercentageProgress() >= 100.0f)
		impulse *= 0.5f;

	if(bIsStanding && impulse > 20.0f)
		bIsStanding = false;

	// Inflict damage on the driver and passenger
	if(pDriver && pDriver->GetPedState() == PED_DRIVING && colSpeed > 10.0f){
		// Delegate the damage of the rider to the C implementation
		float damage = Bike_RiderDamage(DotProduct(m_vecDamageNormal, GetForward()), DotProduct(m_vecDamageNormal, GetRight()), DotProduct(m_vecDamageNormal, GetUp()),
			m_vecDamageNormal.z, GetModelIndex() == MI_SANCHEZ, colSpeed);

		if(pDriver->IsPlayer() && CullZones_CamStairsForPlayer() && CullZones_FindZoneWithStairsAttributeForPlayer())
			damage = 0.0f;

		if(damage > 75.0f){
			int dir = -10;
			if(pDriver){
				dir = pDriver->GetLocalDirection(-m_vecDamageNormal);
				if(pDriver->m_fHealth > 0.0f)
					pDriver->InflictDamage(m_pDamageEntity, WEAPONTYPE_RAMMEDBYCAR, 0.05f*damage, PEDPIECE_TORSO, dir);
				if(pDriver && pDriver->GetPedState() == PED_DRIVING)
					KnockOffRider(WEAPONTYPE_RAMMEDBYCAR, dir, pDriver, false);
			}
			if(pPassengers[0]){
				dir = pPassengers[0]->GetLocalDirection(-m_vecDamageNormal);
				if(pPassengers[0]->m_fHealth > 0.0f)
					pPassengers[0]->InflictDamage(m_pDamageEntity, WEAPONTYPE_RAMMEDBYCAR, 0.05f*damage, PEDPIECE_TORSO, dir);
				if(pPassengers[0] && pPassengers[0]->GetPedState() == PED_DRIVING)
					KnockOffRider(WEAPONTYPE_RAMMEDBYCAR, dir, pPassengers[0], false);
			}
		}
	}

	if(impulse > 25.0f && GetStatus() != STATUS_WRECKED){
		float damage = (impulse-25.0f)*pHandling->fCollisionDamageMultiplier;
		if(damage > 0.0f){
			if(damage > 5.0f &&
			   pDriver &&
			   m_pDamageEntity && m_pDamageEntity->IsVehicle() &&
			   (this != FindPlayerVehicle() || ((CVehicle*)m_pDamageEntity)->VehicleCreatedBy == MISSION_VEHICLE) &&
			   ((CVehicle*)m_pDamageEntity)->pDriver)
					pDriver->Say(SOUND_PED_CRASH_VEHICLE);

			// Delegate the health left to the C implementation shared with cars
			m_fHealth = Automobile_HealthAfterDamage(m_fHealth, damage, this == FindPlayerVehicle(), bTakeLessDamage, m_pDamageEntity && m_pDamageEntity == FindPlayerVehicle());
		}
	}

	if(m_fHealth < 250.0f){
		// Car is on fire
		if(!bIsOnFire){
			// Set engine on fire and remember who did this
			bIsOnFire = true;
			m_fFireBlowUpTimer = 0.0f;
			m_pSetOnFireEntity = m_pDamageEntity;
			if(m_pSetOnFireEntity)
				m_pSetOnFireEntity->RegisterReference(&m_pSetOnFireEntity);
		}
	}
}

void
CBike::AddDamagedVehicleParticles(void)
{
	if(this == FindPlayerVehicle() && TheCamera.GetLookingForwardFirstPerson())
		return;
	if(this != FindPlayerVehicle() && ((*Timer_GetFrameCounter()) + m_randomSeed) & 1)
		return;
	if(m_fHealth >= 650.0f)
		return;

	CVector direction = 0.5f*m_vecMoveSpeed;
	CVector damagePos = ((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())))->GetFrontSeatPosn(); // rouz edit (ChatGPT)

	damagePos.z -= 0.4f;
	damagePos = GetMatrix()*damagePos;

	CalculateLeanMatrix();

	if(m_fHealth < 250.0f){
		// fire, done in processControl
	}else if(m_fHealth < 320.0f){
		direction *= 0.2f;
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE2, damagePos, direction + 0.02f*m_leanMatrix.GetRight());
	}else if(m_fHealth < 390.0f){
		// Delegate the test for the frame to the C implementation
		if(Bike_SteamFrame((*Timer_GetFrameCounter()) + m_randomSeed))
			CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, direction + 0.05f*m_leanMatrix.GetRight());
		direction *= 0.3f;
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE, damagePos, direction + 0.04f*m_leanMatrix.GetRight());
	}else if(m_fHealth < 460.0f){
		int rnd = (*Timer_GetFrameCounter()) + m_randomSeed;
		// Delegate the test for the pause to the C implementation
		if(Bike_SteamSkipsFrame(rnd))
			return;
		direction.z += 0.05f;
		if(TheCamera.GetLookDirection() != LOOKING_FORWARD){
			CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, direction + 0.08f*m_leanMatrix.GetRight(), nil, 0.1f, 0, 0, 0, 1000);
		}else if((((*Timer_GetFrameCounter()) + m_randomSeed) & 1) == 0){
			direction = 0.8f*m_vecMoveSpeed;
			CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos, direction + 0.07f*m_leanMatrix.GetRight(), nil, 0.1f, 0, 0, 0, 1000);
		}
	}else if(Bike_SteamFrame((*Timer_GetFrameCounter()) + m_randomSeed)){
		CParticle::AddParticle(PARTICLE_ENGINE_STEAM, damagePos + 0.06f*m_leanMatrix.GetRight(), direction);
	}
}

int32
CBike::AddWheelDirtAndWater(CColPoint *colpoint, uint32 belowEffectSpeed)
{
	int i;
	CVector dir;
	static float minSize = 0.02f;
	static float maxSize = 0.04f;
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
			dir.z = General_GetRandomNumberInRangeFloat(0.03f, 0.04f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, CollisionVector_ToNative(colpoint->point), dir, nil,
				General_GetRandomNumberInRangeFloat(minSize, maxSize), grassCol);
		}
		return 0;
	case SURFACE_GRAVEL:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = General_GetRandomNumberInRangeFloat(0.03f, 0.04f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, CollisionVector_ToNative(colpoint->point), dir, nil,
				General_GetRandomNumberInRangeFloat(minSize, maxSize), gravelCol);
		}
		return 1;
	case SURFACE_MUD_DRY:
		dir.x = -0.05f*m_vecMoveSpeed.x;
		dir.y = -0.05f*m_vecMoveSpeed.y;
		for(i = 0; i < 4; i++){
			dir.z = General_GetRandomNumberInRangeFloat(0.03f, 0.04f);
			CParticle::AddParticle(PARTICLE_WHEEL_DIRT, CollisionVector_ToNative(colpoint->point), dir, nil,
				General_GetRandomNumberInRangeFloat(minSize, maxSize), mudCol);
		}
		return 0;
	case SURFACE_SAND:
	case SURFACE_SAND_BEACH:
		if((*Timer_GetFrameCounter()) & 2)
			return 0;
		dir.x = 0.75f*m_vecMoveSpeed.x;
		dir.y = 0.75f*m_vecMoveSpeed.y;
		for(i = 0; i < 1; i++){
			dir.z = General_GetRandomNumberInRangeFloat(0.02f, 0.055f);
			CParticle::AddParticle(PARTICLE_SAND, CollisionVector_ToNative(colpoint->point), dir, nil,
				0.8f*m_vecMoveSpeed.Magnitude(), sandCol);
		}
		return 0;
	default:
		if(Weather_WetRoads > 0.01f){
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

	return 0;
}

void
CBike::GetComponentWorldPosition(int32 component, CVector &pos)
{
	if(m_aBikeNodes[component] == nil){
		printf("BikeNode missing: %d %d\n", GetModelIndex(), component);
		return;
	}
	RwMatrix *ltm = RwFrameGetLTM(m_aBikeNodes[component]);
	pos = *RwMatrixGetPos(ltm);
}

bool
CBike::IsComponentPresent(int32 component)
{
	// Delegate to the C implementation
	return Bike_IsComponentPresent(this, component);
}

void
CBike::SetComponentRotation(int32 component, CVector rotation)
{
	CMatrix mat(RwFrameGetMatrix(m_aBikeNodes[component]));
	CVector pos = mat.GetPosition();
	// BUG: all these set the whole matrix
	mat.SetRotateX(DEGTORAD(rotation.x));
	mat.SetRotateY(DEGTORAD(rotation.y));
	mat.SetRotateZ(DEGTORAD(rotation.z));
	mat.Translate(pos);
	mat.UpdateRW();
}

bool
CBike::IsDoorReady(eDoors door)
{
	return true;
}

bool
CBike::IsDoorFullyOpen(eDoors door)
{
	return false;
}

bool
CBike::IsDoorClosed(eDoors door)
{
	return false;
}

bool
CBike::IsDoorMissing(eDoors door)
{
	return true;
}

void
CBike::RemoveRefsToVehicle(CEntity *ent)
{
	// Delegate to the C implementation
	Bike_RemoveRefsToVehicle(this, ent);
}

void
CBike::BlowUpCar(CEntity *culprit)
{
	if(!bCanBeDamaged)
		return;

#ifdef FIX_BUGS
	// taken from CAutomobile. maybe tweak values?
	if(culprit == FindPlayerPed() || culprit == FindPlayerVehicle()){
		CWorld::Players[CWorld::PlayerInFocus].m_nHavocLevel += 20;
		CWorld::Players[CWorld::PlayerInFocus].m_fMediaAttention += 10.0f;
		Stats_PropertyDestroyed += General_GetRandomNumber()%6000 + 4000;
	}
#endif

	// explosion pushes vehicle up
	m_vecMoveSpeed.z += 0.13f;
	SetStatus(STATUS_WRECKED);
	bRenderScorched = true;

	m_fHealth = 0.0f;
	m_nBombTimer = 0;

	TheCamera.CamShake(0.7f, GetPosition().x, GetPosition().y, GetPosition().z);

	KillPedsInVehicle();

	bEngineOn = false;
	bLightsOn = false;
	ChangeLawEnforcerState(false);

	Explosion_AddExplosion(this, culprit, EXPLOSION_CAR, GetPosition(), 0);
	Darkel_RegisterCarBlownUpByPlayer(this);
}

bool
CBike::SetUpWheelColModel(CColModel *colModel)
{
	RwMatrix *mat = RwMatrixCreate();
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)
	CColModel *vehColModel = mi->GetColModel();

	colModel->boundingSphere = vehColModel->boundingSphere;
	colModel->boundingBox = vehColModel->boundingBox;

	GetRelativeMatrix(mat, m_aBikeNodes[BIKE_WHEEL_FRONT], m_aBikeNodes[BIKE_CHASSIS]);
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter0=*RwMatrixGetPos(mat);
	    ColSphere_Set(&colModel->spheres[0],0.5f*mi->m_wheelScale,COLLISION_POINT_REF(sphereCenter0),SURFACE_RUBBER,CAR_PIECE_WHEEL_LF);
	}
	GetRelativeMatrix(mat, m_aBikeNodes[BIKE_WHEEL_REAR], m_aBikeNodes[BIKE_CHASSIS]);
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter1=*RwMatrixGetPos(mat);
	    ColSphere_Set(&colModel->spheres[1],0.5f*mi->m_wheelScale,COLLISION_POINT_REF(sphereCenter1),SURFACE_RUBBER,CAR_PIECE_WHEEL_LR);
	}
	colModel->numSpheres = 2;
#ifdef FIX_BUGS
	RwMatrixDestroy(mat);
#endif
	return true;
}

float fBikeBurstForceMult = 0.02f;
float fBikeBurstFallSpeed = 0.3f;
float fBikeBurstFallSpeedPlayer = 0.55f;

void
CBike::BurstTyre(uint8 wheel, bool applyForces)
{
	if(bTyresDontBurst)
		return;

	switch(wheel){
	case CAR_PIECE_WHEEL_LF: wheel = BIKEWHEEL_FRONT; break;
	case CAR_PIECE_WHEEL_LR: wheel = BIKEWHEEL_REAR; break;
	}

	if(m_wheelStatus[wheel] == WHEEL_STATUS_OK){
		m_wheelStatus[wheel] = WHEEL_STATUS_BURST;
#ifdef FIX_BUGS
		Stats_TyresPopped++;
#endif
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_CAR_TYRE_POP, 0.0f);

		if(GetStatus() == STATUS_SIMPLE){
			SetStatus(STATUS_PHYSICS);
			CCarCtrl::SwitchVehicleToRealPhysics(this);
		}

		if(applyForces){
			ApplyMoveForce(GetRight() * m_fMass * General_GetRandomNumberInRangeFloat(-0.02f, 0.02f));
			ApplyTurnForce(GetRight() * m_fTurnMass * General_GetRandomNumberInRangeFloat(-0.02f, 0.02f), GetForward());
		}

		// This code checks piece types originally so it is never triggered
		// as we have converted them to wheel indices above already.
		if(pDriver){
#ifdef FIX_SIGNIFICANT_BUGS
			if(wheel == BIKEWHEEL_FRONT && (m_aSuspensionSpringRatioPrev[BIKESUSP_F1] < 1.0f || m_aSuspensionSpringRatioPrev[BIKESUSP_F2] < 1.0f) ||
			   wheel == BIKEWHEEL_REAR && (m_aSuspensionSpringRatioPrev[BIKESUSP_R1] < 1.0f || m_aSuspensionSpringRatioPrev[BIKESUSP_R2] < 1.0f)){
#else
			if(wheel == CAR_PIECE_WHEEL_LF && (m_aSuspensionSpringRatioPrev[BIKESUSP_F1] < 1.0f || m_aSuspensionSpringRatioPrev[BIKESUSP_F2] < 1.0f) ||
			   wheel == CAR_PIECE_WHEEL_LR && (m_aSuspensionSpringRatioPrev[BIKESUSP_R1] < 1.0f || m_aSuspensionSpringRatioPrev[BIKESUSP_R2] < 1.0f)){
#endif
				float speedSq = m_vecMoveSpeed.MagnitudeSqr();
				// Delegate the test for the speed to the C implementation
				if(Bike_BurstThrowsRider(speedSq, GetStatus() == STATUS_PLAYER, fBikeBurstFallSpeed, fBikeBurstFallSpeedPlayer)){
#ifdef FIX_SIGNIFICANT_BUGS
					if(wheel == BIKEWHEEL_FRONT){
#else
					if(wheel == CAR_PIECE_WHEEL_LF){
#endif
						KnockOffRider(WEAPONTYPE_RAMMEDBYCAR, 0, pDriver, false);
						if(pPassengers[0])
							KnockOffRider(WEAPONTYPE_RAMMEDBYCAR, 0, pPassengers[0], false);
					}else
						ApplyTurnForce(2.0f*fBikeBurstForceMult*m_fTurnMass*GetRight(), GetForward());
				}
			}
		}
	}
}

bool
CBike::IsRoomForPedToLeaveCar(uint32 component, CVector *doorOffset)
{
	CColPoint colpoint;
	CEntity *ent;
	colpoint.point = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.0f));
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)

	CVector seatPos = mi->GetFrontSeatPosn();
	if(component == CAR_DOOR_RR || component == CAR_DOOR_LR)
		seatPos = mi->m_positions[CAR_POS_BACKSEAT];
	if(component == CAR_DOOR_LF || component == CAR_DOOR_LR)
		seatPos.x = -seatPos.x;
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

	// Removing that makes thiProcessEntityCollisions func. return false for van doors.
	doorPos.z += 0.5f;
	float length = dist.Magnitude();
	// Delegate the scale of the vector to the C implementation
	CVector pedPos = seatPos + dist*VehicleLogic_ExitCheckScale(length);

	if(!CWorld::GetIsLineOfSightClear(seatPos, pedPos, true, false, false, true, false, false))
		return false;
	if(CWorld::TestSphereAgainstWorld(doorPos, 0.6f, this, true, true, false, true, false, false))
		return false;
	if(CWorld::ProcessVerticalLine(doorPos, 1000.0f, colpoint, ent, true, false, false, true, false, false, nil))
		// Delegate the test for the ceiling to the C implementation
		if(VehicleLogic_ExitBlockedAbove(colpoint.point.z, doorPos.z))
			return false;
	float upperZ = colpoint.point.z;
	if(!CWorld::ProcessVerticalLine(doorPos, -1000.0f, colpoint, ent, true, false, false, true, false, false, nil))
		return false;
	// Delegate the test for the ground to the C implementation
	if(VehicleLogic_ExitBlockedBelow(upperZ, colpoint.point.z))
		return false;
	return true;
}

float
CBike::GetHeightAboveRoad(void)
{
	return m_fHeightAboveRoad;
}

void
CBike::PlayCarHorn(void)
{
	// The honking is decided in C, the driver shouts here
	if (VehicleLogic_PlayCarHorn(this, IsAlarmOn(), General_GetRandomNumber) && pDriver)
		pDriver->Say(SOUND_PED_ANNOYED_DRIVER);
}

void
CBike::KnockOffRider(eWeaponType weapon, uint8 direction, CPed *ped, bool bGetBackOn)
{
	AnimationId anim = ANIM_STD_KO_FRONT;
	if(ped == nil)
		return;

	if(!ped->IsPlayer()){
		if(bGetBackOn){
			// Delegate the choice of the objective to the C implementation
			bool temperAboveFear = ped->m_pedStats->m_temper > ped->m_pedStats->m_fear;
			bool pedNotMission = ped->CharCreatedBy != MISSION_CHAR;
			bool bikeNotMission = pedNotMission && ped->m_pMyVehicle->VehicleCreatedBy != MISSION_VEHICLE;
			bool playerGoesForBike = temperAboveFear && bikeNotMission && FindPlayerPed()->m_carInObjective == ped->m_pMyVehicle;
			switch(Bike_KnockOffRiderObjective(temperAboveFear, pedNotMission, bikeNotMission, playerGoesForBike, !CTheScripts::IsPlayerOnAMission())){
			case BIKE_KNOCKOFF_ENTER_AS_DRIVER:
				ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, ped->m_pMyVehicle);
				break;
			case BIKE_KNOCKOFF_WANDER:
				ped->SetObjective(OBJECTIVE_WANDER, ped->m_pMyVehicle);
				ped->m_nPathDir = General_GetRandomNumberInRangeInt(0, 8);
				break;
			default:
				break;
			}
		}else if(ped->m_leader == nil){
			if(pDriver == ped)
				ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, this);
			else
				ped->SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, this);
		}
	}

	if(ped->IsPed()){
		CAnimBlendAssociation *assoc;
		// Use the complete C animation plugin API with explicit overload and render arguments
		for(assoc = RpAnimBlendClumpGetFirstAssociationWithMask(ped->GetClump(), ASSOC_DRIVING);
		    assoc;
		    assoc = RpAnimBlendGetNextAssociation(assoc))
			assoc->flags |= ASSOC_DELETEFADEDOUT;
	}

	if (ped == FindPlayerPed() && rouz.cant_fall_off_bike)	// FIXME makes you stuck if you get jacked
		return;

	ped->SetPedState(PED_IDLE);
	// Use the complete C animation manager at the original call boundary
	AnimManager_BlendAnimation(ped->GetClump(), ped->m_animGroup, ANIM_STD_IDLE, 100.0f);
	ped->m_vehDoor = CAR_DOOR_LF;
	CPed::PedSetOutCarCB(nil, ped);
	ped->SetMoveState(PEDMOVE_STILL);
	// Delegate the heading of the ped to the C implementation
	ped->SetHeading(Bike_KnockOffHeading(GetUp().z, GetForward().Heading()));

	switch(weapon){
	case WEAPONTYPE_UNARMED:
	case WEAPONTYPE_UNIDENTIFIED:
		ped->m_vecMoveSpeed = m_vecMoveSpeed;
		ped->m_pCollidingEntity = this;
		anim = ANIM_STD_NUM;
		break;

	case WEAPONTYPE_BASEBALLBAT:
	default:
		switch(direction){
		case 0:
			anim = ANIM_STD_BIKE_FALLBACK;
			ped->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.1f);
			if(m_vecMoveSpeed.MagnitudeSqr() < SQR(0.3f))
				ped->ApplyMoveForce(5.0f*GetUp() - 6.0f*GetForward());
			ped->m_pCollidingEntity = this;
			break;
		case 1:
		case 2:
			if(m_vecMoveSpeed.MagnitudeSqr() > SQR(0.3f)){
				anim = ANIM_STD_HIGHIMPACT_LEFT;
				ped->m_vecMoveSpeed = 0.3f*m_vecMoveSpeed;
				ped->ApplyMoveForce(5.0f*GetUp() + 6.0f*GetRight());
			}else{
				anim = ANIM_STD_SPINFORWARD_LEFT;
				ped->m_vecMoveSpeed = m_vecMoveSpeed;
				ped->ApplyMoveForce(4.0f*GetUp() + 8.0f*GetRight());
			}
			// BUG or is it intentionally missing?
			//ped->m_pCollidingEntity = this;
			break;
		case 3:
			if(m_vecMoveSpeed.MagnitudeSqr() > SQR(0.3f)){
				anim = ANIM_STD_HIGHIMPACT_RIGHT;
				ped->m_vecMoveSpeed = 0.3f*m_vecMoveSpeed;
				ped->ApplyMoveForce(5.0f*GetUp() - 6.0f*GetRight());
			}else{
				anim = ANIM_STD_SPINFORWARD_RIGHT;
				ped->m_vecMoveSpeed = m_vecMoveSpeed;
				ped->ApplyMoveForce(4.0f*GetUp() - 8.0f*GetRight());
			}
			// BUG or is it intentionally missing?
			//ped->m_pCollidingEntity = this;
			break;
		}
		break;

	case WEAPONTYPE_DROWNING:{
		RwRGBA color;
		anim = ANIM_STD_FALL;
		ped->m_vecMoveSpeed = m_vecMoveSpeed*0.2f;
		ped->m_vecMoveSpeed.z = 0.0f;
		ped->m_pCollidingEntity = this;
		// Delegate the colour channels of the splash to the C implementation
		color.red = Automobile_WaterSplashChannel(TimeCycle_GetDirectionalRed(), TimeCycle_GetAmbientRed_Obj());
		color.green = Automobile_WaterSplashChannel(TimeCycle_GetDirectionalGreen(), TimeCycle_GetAmbientGreen_Obj());
		color.blue = Automobile_WaterSplashChannel(TimeCycle_GetDirectionalBlue(), TimeCycle_GetAmbientBlue_Obj());
		color.alpha = General_GetRandomNumberInRangeInt(48, 96);
		DMAudio_PlayOneShot(m_audioEntityId, SOUND_SPLASH, 0.0f);
		CVector splashPos = ped->GetPosition() + 2.2f*ped->m_vecMoveSpeed;
		float waterZ = 0.0f;
		if(CWaterLevel::GetWaterLevel(splashPos, &waterZ, false))
			splashPos.z = waterZ;
		CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, splashPos, CVector(0.0f, 0.0f, 0.1f),
			0.0f, 200, color, true);
		break;
	}

	case WEAPONTYPE_FALL: {
		ped->m_vecMoveSpeed = ped->m_pMyVehicle->m_vecMoveSpeed;
		// Delegate the force on the ground to the C implementation
		float forceXY = Bike_KnockOffForceXY(m_fDamageImpulse, ped->m_fMass, m_fMass);
		ped->ApplyMoveForce(m_vecDamageNormal.x*forceXY, m_vecDamageNormal.y*forceXY,
			General_GetRandomNumberInRangeFloat(3.0f, 7.0f));
		ped->m_pCollidingEntity = this;
		switch(direction){
		case 0: anim = ANIM_STD_HIGHIMPACT_BACK; break;
		case 1: anim = ANIM_STD_SPINFORWARD_RIGHT; break;
		case 2: anim = ANIM_STD_BIKE_FALLBACK; break;
		case 3: anim = ANIM_STD_SPINFORWARD_LEFT; break;
		}
		if(m_nWheelsOnGround == 0)
			ped->bKnockedOffBike = true;
		break;
	}

	case WEAPONTYPE_RAMMEDBYCAR: {
		ped->m_vecMoveSpeed = ped->m_pMyVehicle->m_vecMoveSpeed;
		static float minForceZ = 8.0f;
		static float maxForceZ = 15.0f;
		// Delegate the force on the ground to the C implementation
		float forceXY = Bike_KnockOffForceXY(m_fDamageImpulse, ped->m_fMass, m_fMass);
		ped->ApplyMoveForce(m_vecDamageNormal.x*forceXY, m_vecDamageNormal.y*forceXY,
			General_GetRandomNumberInRangeFloat(minForceZ, maxForceZ));
		ped->m_pCollidingEntity = this;
		switch(direction){
		case 0: anim = ANIM_STD_HIGHIMPACT_BACK; break;
		case 1: anim = ANIM_STD_SPINFORWARD_RIGHT; break;
		case 2: anim = ANIM_STD_HIGHIMPACT_FRONT; break;
		case 3: anim = ANIM_STD_SPINFORWARD_LEFT; break;
		}
		ped->bKnockedOffBike = true;
		if(ped->IsPlayer())
			ped->Say(SOUND_PED_DAMAGE);
		break;
	}
	}

	if(weapon == WEAPONTYPE_DROWNING){
		ped->bIsStanding = false;
		ped->bWasStanding = false;
		ped->bIsInTheAir = true;
		ped->bIsInWater = true;
		ped->bTouchingWater = true;
		// Use the complete C animation manager at the original call boundary
		AnimManager_BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_FALL, 4.0f);
	}else if(weapon != WEAPONTYPE_UNARMED){
		if(ped->m_fHealth > 0.0f)
			ped->SetFall(1000, anim, 0);
		else
			ped->SetDie(anim);
		ped->bIsStanding = false;
	}

	CEntity *ent = CWorld::TestSphereAgainstWorld(ped->GetPosition()+CVector(0.0f, 0.0, 0.5f), 0.4f, nil, true, false, false, false, false, false);
	if(ent == nil)
		ent = CWorld::TestSphereAgainstWorld(ped->GetPosition()+CVector(0.0f, 0.0, 0.8f), 0.4f, nil, true, false, false, false, false, false);
	if(ent == nil)
		ent = CWorld::TestSphereAgainstWorld(ped->GetPosition()+(*Timer_GetTimeStep())*ped->m_vecMoveSpeed+CVector(0.0f, 0.0, 0.5f), 0.4f, nil, true, false, false, false, false, false);
	if(ent == nil)
		ent = CWorld::TestSphereAgainstWorld(ped->GetPosition()+(*Timer_GetTimeStep())*ped->m_vecMoveSpeed+CVector(0.0f, 0.0, 0.8f), 0.4f, nil, true, false, false, false, false, false);
	if(ent){
		CColPoint point;
		ent = nil;
		if(CWorld::ProcessVerticalLine(ped->GetPosition(), ped->GetPosition().z-2.0f, point, ent, true, false, false, false, false, false, nil)){
			if(ped->m_pMyVehicle == nil){
				ped->m_pMyVehicle = this;
				ped->PositionPedOutOfCollision();
				ped->m_pMyVehicle = nil;
			}else
				ped->PositionPedOutOfCollision();
		}else
			ped->GetMatrix().Translate(CVector(0.0f, 0.0f, -2.0f));
		ped->m_pCollidingEntity = ped->m_pMyVehicle;
		ped->bKnockedOffBike = true;
		ped->bHeadStuckInCollision = true;
	}else if(weapon == WEAPONTYPE_RAMMEDBYCAR){
		if(CWorld::TestSphereAgainstWorld(ped->GetPosition()+CVector(0.0f, 0.0, 1.3f), 0.6f, nil, true, false, false, false, false, false) == nil)
			ped->GetMatrix().Translate(CVector(0.0f, 0.0f, 0.5f));
	}
	ped->m_pMyVehicle = nil;
}

void
CBike::PlayHornIfNecessary(void)
{
	if(AutoPilot.m_bSlowedDownBecauseOfPeds ||
	   AutoPilot.m_bSlowedDownBecauseOfCars)
		PlayCarHorn();
}

void
CBike::ResetSuspension(void)
{
	// Delegate to the C implementation
	Bike_ResetSuspension(this);
}

void
CBike::SetupSuspensionLines(void)
{
	int i;
	CVector posn;
	float suspOffset = 0.0f;
	RwFrame *node = nil;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)
	CColModel *colModel = mi->GetColModel();
	RwMatrix *mat = RwMatrixCreate();

	bool initialized = colModel->lines[0].p0.z != FAKESUSPENSION;

	for(i = 0; i < 4; i++){
		if(initialized){
			posn = CollisionVector_ToNative(colModel->lines[i].p0);
			if(i < 2)
				posn.z = m_aWheelBasePosition[0];
			else
				posn.z = m_aWheelBasePosition[1];
		}else{
			switch(i){
			case BIKESUSP_F1:
				node = m_aBikeNodes[BIKE_WHEEL_FRONT];
				suspOffset = 0.25f*mi->m_wheelScale;
				break;
			case BIKESUSP_F2:
				node = m_aBikeNodes[BIKE_WHEEL_FRONT];
				suspOffset = -0.25f*mi->m_wheelScale;
				break;
			case BIKESUSP_R1:
				node = m_aBikeNodes[BIKE_WHEEL_REAR];
				suspOffset = 0.25f*mi->m_wheelScale;
				break;
			case BIKESUSP_R2:
				node = m_aBikeNodes[BIKE_WHEEL_REAR];
				suspOffset = -0.25f*mi->m_wheelScale;
				break;
			}

			GetRelativeMatrix(mat, node, node);
			posn = *RwMatrixGetPos(mat);
			if(i == BIKESUSP_F1)
				m_aWheelBasePosition[BIKEWHEEL_FRONT] = posn.z;
			else if(i == BIKESUSP_R1){
				m_aWheelBasePosition[BIKEWHEEL_REAR] = posn.z;

				GetRelativeMatrix(mat, m_aBikeNodes[BIKE_FORKS_REAR], m_aBikeNodes[BIKE_FORKS_REAR]);
				float dz = posn.z - RwMatrixGetPos(mat)->z;
				float dy = posn.y - RwMatrixGetPos(mat)->y;
				// Delegate the length of the fork to the C implementation
				m_fRearForkLength = Bike_RearForkLength(dy, dz);
				assert(m_fRearForkLength != 0.0f);	// we want to divide by this
			}
			posn.y += suspOffset;
		}

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

	if(!initialized){
		GetRelativeMatrix(mat, m_aBikeNodes[BIKE_FORKS_FRONT], m_aBikeNodes[BIKE_FORKS_FRONT]);
		m_fFrontForkY = RwMatrixGetPos(mat)->y;
		m_fFrontForkZ = RwMatrixGetPos(mat)->z;
	}

	// Compress spring somewhat to get normal height on road
	// Delegate the height above the road to the C implementation
	m_fHeightAboveRoad = Bike_HeightAboveRoad(m_aSuspensionSpringLength[0], pHandling->fSuspensionForceLevel, colModel->lines[0].p0.z, mi->m_wheelScale);
	for(i = 0; i < 2; i++)
		m_aWheelPosition[i] = mi->m_wheelScale*0.5f - m_fHeightAboveRoad;

	// adjust col model to include suspension lines
	if(colModel->boundingBox.min.z > colModel->lines[0].p1.z)
		colModel->boundingBox.min.z = colModel->lines[0].p1.z;
	// Delegate the radius of the vehicle to the C implementation
	CVector radiusMin = CollisionVector_ToNative(colModel->boundingBox.min);
	CVector radiusMax = CollisionVector_ToNative(colModel->boundingBox.max);
	const float radiusMinC[3] = { radiusMin.x, radiusMin.y, radiusMin.z };
	const float radiusMaxC[3] = { radiusMax.x, radiusMax.y, radiusMax.z };
	float radius = VehicleLogic_BoundingRadius(radiusMinC, radiusMaxC);
	if(colModel->boundingSphere.radius < radius)
		colModel->boundingSphere.radius = radius;

#ifdef FIX_BUGS
	RwMatrixDestroy(mat);
#endif
}

void
CBike::CalculateLeanMatrix(void)
{
	if(bLeanMatrixClean)
		return;

	// The matrix starts as that of the bike and the rest is done in C
	m_leanMatrix = GetMatrix();
	Bike_CalculateLeanMatrix(&m_leanMatrix, m_fLeanLRAngle, &GetUp().x, GetColModel()->boundingBox.min.z);
	bLeanMatrixClean = true;
}

void
CBike::GetCorrectedWorldDoorPosition(CVector &pos, CVector p1, CVector p2)
{
	// Delegate to the C implementation
	CColModel *colModel = GetColModel();
	Bike_GetCorrectedWorldDoorPosition(&GetForward().x, &GetUp().x, &GetPosition().x,
		colModel->boundingBox.max.x, colModel->boundingBox.max.z, &p1.x, &p2.x, &pos.x);
}

void
CBike::Fix(void)
{
	bIsDamaged = false;
	bIsOnFire = false;
	m_wheelStatus[0] = WHEEL_STATUS_OK;
	m_wheelStatus[1] = WHEEL_STATUS_OK;
}

void
CBike::SetupModelNodes(void)
{
	// Clear the node array in C before filling it
	Bike_ClearNodes(this);
	CClumpModelInfo::FillFrameArray(GetClump(), m_aBikeNodes);
}

void
CBike::ReduceHornCounter(void)
{
	if(m_nCarHornTimer != 0)
		m_nCarHornTimer--;
}

#ifdef COMPATIBLE_SAVES
void
CBike::Save(uint8*& buf)
{
	CVehicle::Save(buf);
		// Transfer save data through the C buffer API with explicit sizes
	ZeroSaveBuf(&buf, 1260 - 672);
	}

void
CBike::Load(uint8*& buf)
{
	CVehicle::Load(buf);
		// Transfer save data through the C buffer API with explicit sizes
	SkipSaveBuf(&buf, 1260 - 672);
	}
#endif

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
