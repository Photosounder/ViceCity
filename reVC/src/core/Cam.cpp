//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "CollisionVector_native.h"

#include "main.h"
#include "Draw.h"
#include "World.h"
#include "Vehicle.h"
#include "Automobile.h"
#include "Boat.h"
#include "Bones.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "CopPed.h"
#include "RpAnimBlend.h"
#include "ControllerConfig.h"
#include "Pad.h"
#include "Frontend.h"
#include "General.h"
#include "Timecycle.h"
#include "Renderer.h"
#include "Shadows.h"
#include "Hud.h"
#include "ZoneCull.h"
#include "SurfaceTable.h"
#include "WaterLevel.h"
#include "MBlur.h"
#include "SceneEdit.h"
#include "Debug.h"
#include "Camera.h"
#include "CameraMath.h"
#include "DMAudio.h"
#include "Bike.h"
#include "Pickups.h"

bool PrintDebugCode = false;
int16 DebugCamMode;

extern float fRangePlayerRadius;
extern float fCloseNearClipLimit;

#ifdef FREE_CAM
bool CCamera::bFreeCam = false;
int nPreviousMode = -1;
#endif

static CVector
NormalisedDifference(const CVector &a, const CVector &b)
{
	CVector out;

	Camera_NormalisedDifference(&a, &b, &out);
	return CVector(out.x, out.y, out.z);
}

static CVector
NormalisedHorizontal(const CVector &v)
{
	CVector out;

	General_NormalisedHorizontal(&v, &out);
	return CVector(out.x, out.y, out.z);
}

void
CCam::Init(void)
{
	Mode = MODE_FOLLOWPED;
	Front = CVector(0.0f, 0.0f, -1.0f);
	Up = CVector(0.0f, 0.0f, 1.0f);
	Rotating = false;
	m_iDoCollisionChecksOnFrameNum = 1;
	m_iDoCollisionCheckEveryNumOfFrames = 9;
	m_iFrameNumWereAt = 0;
	m_bCollisionChecksOn = false;
	m_fRealGroundDist = 0.0f;
	BetaSpeed = 0.0f;
	AlphaSpeed = 0.0f;
	DistanceSpeed = 0.0f;
	f_max_role_angle = DEGTORAD(5.0f);
	Distance = 30.0f;
	DistanceSpeed = 0.0f;
	m_pLastCarEntered = nil;
	m_pLastPedLookedAt = nil;
	ResetStatics = true;
	Beta = 0.0f;
	m_fTilt = 0.0f;
	m_fTiltSpeed = 0.0f;
	m_bFixingBeta = false;
	CA_MIN_DISTANCE = 0.0f;
	CA_MAX_DISTANCE = 0.0f;
	LookingBehind = false;
	LookingLeft = false;
	LookingRight = false;
	m_fPlayerInFrontSyphonAngleOffSet = DEGTORAD(20.0f);
	m_fSyphonModeTargetZOffSet = 0.5f;
	m_fRadiusForDead = 1.5f;
	DirectionWasLooking = LOOKING_FORWARD;
	LookBehindCamWasInFront = false;
	f_Roll = 0.0f;
	f_rollSpeed = 0.0f;
	m_fCloseInPedHeightOffset = 0.0f;
	m_fCloseInPedHeightOffsetSpeed = 0.0f;
	m_fCloseInCarHeightOffset = 0.0f;
	m_fCloseInCarHeightOffsetSpeed = 0.0f;
	m_fPedBetweenCameraHeightOffset = 0.0f;
	m_fTargetBeta = 0.0f;
	m_fBufferedTargetBeta = 0.0f;
	m_fBufferedTargetOrientation = 0.0f;
	m_fBufferedTargetOrientationSpeed = 0.0f;
	m_fDimensionOfHighestNearCar = 0.0f;
}

float PLAYERPED_LEVEL_SMOOTHING_CONST_INV = 0.6f;
float PLAYERPED_TREND_SMOOTHING_CONST_INV = 0.8f;

void
CCam::Process(void)
{
	CVector CameraTarget;
	float TargetSpeedVar = 0.0f;
	float TargetOrientation = 0.0f;

	static CVector SmoothedPos(0.0f, 0.0f, 10000.0f);
	static CVector SmoothedSpeed(0.0f, 0.0f, 0.0f);

	if(CamTargetEntity == nil)
		CamTargetEntity = TheCamera.pTargetEntity;

	m_iFrameNumWereAt++;
	if(m_iFrameNumWereAt > m_iDoCollisionCheckEveryNumOfFrames)
		m_iFrameNumWereAt = 1;
	m_bCollisionChecksOn = m_iFrameNumWereAt == m_iDoCollisionChecksOnFrameNum;

	if(m_bCamLookingAtVector){
		CameraTarget = m_cvecCamFixedModeVector;
	}else if(CamTargetEntity->IsVehicle()){
		CameraTarget = CamTargetEntity->GetPosition();

		// Delegate the angle of the vector to the C implementation
		TargetOrientation = Camera_BetaOfVector(CamTargetEntity->GetForward().x, CamTargetEntity->GetForward().y);

		// Delegate the speed variables to the C implementation
		SpeedVar = Camera_VehicleSpeedVar(CamTargetEntity->GetForward().x, CamTargetEntity->GetForward().y,
			((CVehicle*)CamTargetEntity)->GetMoveSpeed().x, ((CVehicle*)CamTargetEntity)->GetMoveSpeed().y, SpeedVar, &TargetSpeedVar);
	}else{
		if(CamTargetEntity == FindPlayerPed()){
			// Some fancy smoothing of player position and speed
			// Delegate the smoothing factors to the C implementation
			float LevelSmoothing = Camera_SmoothingFactor(PLAYERPED_LEVEL_SMOOTHING_CONST_INV, (*Timer_GetTimeStep()));
			float TrendSmoothing = Camera_SmoothingFactor(PLAYERPED_TREND_SMOOTHING_CONST_INV, (*Timer_GetTimeStep()));

			CVector NewSmoothedPos, NewSmoothedSpeed;
			if((SmoothedPos - CamTargetEntity->GetPosition()).MagnitudeSqr() > SQR(3.0f) ||
			   (*Timer_GetTimeStep()) < 0.2f || Using3rdPersonMouseCam()){
				// Reset values
				NewSmoothedPos = CamTargetEntity->GetPosition();
				NewSmoothedSpeed = CVector(0.0f, 0.0f, 0.0f);
			}else{
				NewSmoothedPos = LevelSmoothing*CamTargetEntity->GetPosition() + (1.0f-LevelSmoothing)*(SmoothedPos + SmoothedSpeed*(*Timer_GetTimeStep()));
				NewSmoothedSpeed = TrendSmoothing*(NewSmoothedPos-SmoothedPos)/(*Timer_GetTimeStep()) + (1.0f-TrendSmoothing)*SmoothedSpeed;
			}
			   
			CameraTarget = NewSmoothedPos;
			SmoothedPos = NewSmoothedPos;
			SmoothedSpeed = NewSmoothedSpeed;
		}else
			CameraTarget = CamTargetEntity->GetPosition();

		// Delegate the angle of the vector to the C implementation
		TargetOrientation = Camera_BetaOfVector(CamTargetEntity->GetForward().x, CamTargetEntity->GetForward().y);
		TargetSpeedVar = 0.0f;
		SpeedVar = 0.0f;
	}

	switch(Mode){
	case MODE_TOPDOWN:
	case MODE_GTACLASSIC:
	//	Process_TopDown(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_BEHINDCAR:
		Process_BehindCar(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_FOLLOWPED:
#ifdef PC_PLAYER_CONTROLS
		if(CCamera::m_bUseMouse3rdPerson)
			Process_FollowPedWithMouse(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		else
#endif
#ifdef FREE_CAM
			if(CCamera::bFreeCam)
				Process_FollowPed_Rotation(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
			else
#endif
			Process_FollowPed(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
//	case MODE_AIMING:
	case MODE_DEBUG:
		Process_Debug(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_SNIPER:
	case MODE_CAMERA:
		Process_Sniper(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_ROCKETLAUNCHER:
		Process_Rocket(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_MODELVIEW:
		Process_ModelView(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
//	case MODE_BILL:
	case MODE_SYPHON:
		Process_Syphon(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_CIRCLE:
//		Process_Circle(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
//	case MODE_CHEESYZOOM:
	case MODE_WHEELCAM:
		Process_WheelCam(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_FIXED:
		Process_Fixed(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_1STPERSON:
		Process_1stPerson(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_FLYBY:
		Process_FlyBy(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_CAM_ON_A_STRING:
#ifdef FREE_CAM
		if(CCamera::bFreeCam && !CVehicle::bCheat5)
			Process_FollowCar_SA(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		else
#endif
			Process_Cam_On_A_String(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
//	case MODE_REACTION:
//	case MODE_FOLLOW_PED_WITH_BIND:
//	case MODE_CHRIS:
	case MODE_BEHINDBOAT:
#ifdef FREE_CAM
		if (CCamera::bFreeCam)
			Process_FollowCar_SA(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		else
#endif
			Process_BehindBoat(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_PLAYER_FALLEN_WATER:
		Process_Player_Fallen_Water(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
//	case MODE_CAM_ON_TRAIN_ROOF:
//	case MODE_CAM_RUNNING_SIDE_TRAIN:
//	case MODE_BLOOD_ON_THE_TRACKS:
//	case MODE_IM_THE_PASSENGER_WOOWOO:
	case MODE_SYPHON_CRIM_IN_FRONT:
		Process_Syphon_Crim_In_Front(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_PED_DEAD_BABY:
		ProcessPedsDeadBaby();
		break;
//	case MODE_PILLOWS_PAPS:
//	case MODE_LOOK_AT_CARS:
	case MODE_ARRESTCAM_ONE:
		ProcessArrestCamOne();
		break;
	case MODE_ARRESTCAM_TWO:
		ProcessArrestCamTwo();
		break;
	case MODE_M16_1STPERSON:
	case MODE_HELICANNON_1STPERSON:
		Process_M16_1stPerson(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_SPECIAL_FIXED_FOR_SYPHON:
		Process_SpecialFixedForSyphon(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_FIGHT_CAM:
		Process_Fight_Cam(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_LIGHTHOUSE:
		Process_LightHouse(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_TOP_DOWN_PED:
	//	Process_TopDownPed(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
	case MODE_SNIPER_RUNABOUT:
	case MODE_ROCKETLAUNCHER_RUNABOUT:
	case MODE_1STPERSON_RUNABOUT:
	case MODE_M16_1STPERSON_RUNABOUT:
	case MODE_FIGHT_CAM_RUNABOUT:
		Process_1rstPersonPedOnPC(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
#ifdef GTA_SCENE_EDIT
	case MODE_EDITOR:
		Process_Editor(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar);
		break;
#endif
	default:
		Source = CVector(0.0f, 0.0f, 0.0f);
		Front = CVector(0.0f, 1.0f, 0.0f);
		Up = CVector(0.0f, 0.0f, 1.0f);
	}

#ifdef FREE_CAM
	nPreviousMode = Mode;
#endif
	CVector TargetToCam = Source - m_cvecTargetCoorsForFudgeInter;
	float DistOnGround = TargetToCam.Magnitude2D();
	m_fTrueBeta = General_GetATanOfXY(TargetToCam.x, TargetToCam.y);
	m_fTrueAlpha = General_GetATanOfXY(DistOnGround, TargetToCam.z);
	if(TheCamera.m_uiTransitionState == 0)
		KeepTrackOfTheSpeed(Source, m_cvecTargetCoorsForFudgeInter, Up, m_fTrueAlpha, m_fTrueBeta, FOV);

	// Look Behind, Left, Right
	LookingBehind = false;
	LookingLeft = false;
	LookingRight = false;
	SourceBeforeLookBehind = Source;
	if(&TheCamera.Cams[TheCamera.ActiveCam] == this){
		if((Mode == MODE_CAM_ON_A_STRING || Mode == MODE_1STPERSON || Mode == MODE_BEHINDBOAT || Mode == MODE_BEHINDCAR) &&
		   CamTargetEntity->IsVehicle()){
			bool bDisableLR = CamTargetEntity &&
				(((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI || CamTargetEntity->GetModelIndex() == MI_RCBARON);
			if(CPad::GetPad(0)->GetLookBehindForCar()){
				LookBehind();
				if(DirectionWasLooking != LOOKING_BEHIND)
					TheCamera.m_bJust_Switched = true;
				DirectionWasLooking = LOOKING_BEHIND;
			}else if(bDisableLR){
				if(DirectionWasLooking != LOOKING_FORWARD)
					TheCamera.m_bJust_Switched = true;
				DirectionWasLooking = LOOKING_FORWARD;
			}else if(CPad::GetPad(0)->GetLookLeft()){
				LookLeft();
				if(DirectionWasLooking != LOOKING_LEFT)
					TheCamera.m_bJust_Switched = true;
				DirectionWasLooking = LOOKING_LEFT;
			}else if(CPad::GetPad(0)->GetLookRight()){
				LookRight();
				if(DirectionWasLooking != LOOKING_RIGHT)
					TheCamera.m_bJust_Switched = true;
				DirectionWasLooking = LOOKING_RIGHT;
			}else{
				if(DirectionWasLooking != LOOKING_FORWARD)
					TheCamera.m_bJust_Switched = true;
				DirectionWasLooking = LOOKING_FORWARD;
			}
		}
		if(Mode == MODE_FOLLOWPED && CamTargetEntity->IsPed()){
			if(CPad::GetPad(0)->GetLookBehindForPed()){
				LookBehind();
				if(DirectionWasLooking != LOOKING_BEHIND)
					TheCamera.m_bJust_Switched = true;
				DirectionWasLooking = LOOKING_BEHIND;
			}else
				DirectionWasLooking = LOOKING_FORWARD;
		}
	}

	if(Mode == MODE_SNIPER || Mode == MODE_ROCKETLAUNCHER || Mode == MODE_M16_1STPERSON ||
	   Mode == MODE_1STPERSON || Mode == MODE_HELICANNON_1STPERSON || Mode == MODE_CAMERA || GetWeaponFirstPersonOn())
		ClipIfPedInFrontOfPlayer();
}

// MaxSpeed is a limit of how fast the value is allowed to change. 1.0 = to Target in up to 1ms
// Acceleration is how fast the speed will change to MaxSpeed. 1.0 = to MaxSpeed in 1ms
void
WellBufferMe(float Target, float *CurrentValue, float *CurrentSpeed, float MaxSpeed, float Acceleration, bool IsAngle)
{
	float Delta = Target - *CurrentValue;

	if(IsAngle){
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&Delta);
	}

	float TargetSpeed = Delta * MaxSpeed;
	// Add or subtract absolute depending on sign, genius!
//	if(TargetSpeed - *CurrentSpeed > 0.0f)
//		*CurrentSpeed += Acceleration * Abs(TargetSpeed - *CurrentSpeed) * (*Timer_GetTimeStep());
//	else
//		*CurrentSpeed -= Acceleration * Abs(TargetSpeed - *CurrentSpeed) * (*Timer_GetTimeStep());
	// Delegate the step of the buffered value to the C implementation
	Camera_BufferStep(CurrentValue, CurrentSpeed, TargetSpeed, Acceleration, (*Timer_GetTimeStep()));
}

void
MakeAngleLessThan180(float &Angle)
{
	// Delegate to the C implementation
	CameraMath_MakeAngleLessThan180(&Angle);
}

void
CCam::ProcessSpecialHeightRoutines(void)
{
	int i;
	bool StandingOnBoat = false;
	static bool PreviouslyFailedRoadHeightCheck = false;
	CVector CamToTarget, CamToPed;
	float DistOnGround, BetaAngle;
	CPed *Player;
	float PedZDist;
	CColPoint colPoint;

	CamToTarget = TheCamera.pTargetEntity->GetPosition() - TheCamera.GetGameCamPosition();
	DistOnGround = CamToTarget.Magnitude2D();
	BetaAngle = General_GetATanOfXY(CamToTarget.x, CamToTarget.y);
	m_bTheHeightFixerVehicleIsATrain = false;
	// CGeneral::GetATanOfXY(TheCamera.GetForward().x, TheCamera.GetForward().y);
	Player = World_Players[World_PlayerInFocus].m_pPed;

	if(DistOnGround > 10.0f)
		DistOnGround = 10.0f;

	if(CamTargetEntity && CamTargetEntity->IsPed()){
		if(FindPlayerPed()->m_pCurSurface && FindPlayerPed()->m_pCurSurface->IsVehicle() &&
		   ((CVehicle*)FindPlayerPed()->m_pCurSurface)->IsBoat())
			StandingOnBoat = true;

		float FoundPedZ = -100.0f;

		// Move up the camera if there is a ped close to it
		if(Mode == MODE_FOLLOWPED || Mode == MODE_FIGHT_CAM || Mode == MODE_PILLOWS_PAPS){
			// Find highest ped close to camera
			for(i = 0; i < Player->m_numNearPeds; i++){
				CPed *nearPed = Player->m_nearPeds[i];
				if(nearPed && nearPed->GetPedState() != PED_DEAD){
					CamToPed = nearPed->GetPosition() - TheCamera.GetGameCamPosition();
					if(General_AbsBelow(CamToPed.z, 1.0f)){
						float DistSq = CamToPed.MagnitudeSqr();
						if(DistSq < SQR(2.1f)){
							if(nearPed->GetPosition().z > FoundPedZ)
								FoundPedZ = nearPed->GetPosition().z;
						}else{
							float Dist = Sqrt(DistSq);
							CamToPed /= Dist;
							// strange calculation
							CVector PlayerCamSpeed = DotProduct(Front, Player->m_vecMoveSpeed)*Front;
							float SpeedDiff = DotProduct(PlayerCamSpeed - nearPed->m_vecMoveSpeed, CamToPed);
							// Delegate the check that the ped will block the view to the C implementation
							if(Camera_PedWillBlockView(Dist, SpeedDiff, m_fPedBetweenCameraHeightOffset))
								if(nearPed->GetPosition().z > FoundPedZ)
									FoundPedZ = nearPed->GetPosition().z;
						}
					}
				}
			}

			if(FoundPedZ > -99.0f){
				float Offset = 0.0f;
				PedZDist = 0.0f;
				if(FoundPedZ > Player->GetPosition().z)
					PedZDist = FoundPedZ - Player->GetPosition().z;

				if(Mode == MODE_FOLLOWPED){
					if(TheCamera.PedZoomIndicator == CAM_ZOOM_1 &&
					   ((CPed*)CamTargetEntity)->GetPedState() != PED_ENTER_CAR &&
					   ((CPed*)CamTargetEntity)->GetPedState() != PED_CARJACK)
						Offset = 0.45f + PedZDist;
					// BUG: overrides this ^ case
					if(TheCamera.PedZoomIndicator == CAM_ZOOM_2 || TheCamera.PedZoomIndicator == CAM_ZOOM_1)
						Offset = 0.35f + PedZDist;
					if(TheCamera.PedZoomIndicator == CAM_ZOOM_3)
						Offset = 0.25f + PedZDist;
					m_fPedBetweenCameraHeightOffset = Offset + 1.3f;
				}else if(Mode == MODE_FIGHT_CAM)
					m_fPedBetweenCameraHeightOffset = PedZDist + 1.3f + 0.5f;
				else if(Mode == MODE_PILLOWS_PAPS)
					m_fPedBetweenCameraHeightOffset = PedZDist + 1.3f + 0.45f;
			}else{
				m_fPedBetweenCameraHeightOffset = 0.0f;
			}
		}


		// Move camera up for vehicles in the way
		if(m_bCollisionChecksOn && (Mode == MODE_FOLLOWPED || Mode == MODE_FIGHT_CAM)){
			bool FoundCar = false;
			CEntity *vehicle = nil;
			float TestDist = DistOnGround + 1.25f;
			float HighestCar = 0.0f;
			if(m_fDimensionOfHighestNearCar > 0.0f)
				TestDist += 0.3f;
			CVector TestBase = CamTargetEntity->GetPosition();
			CVector TestPoint;
			TestBase.z -= 0.15f;

			// Delegate the offset of the test point to the C implementation
			{
				float probeX, probeY;
				Camera_OrbitOffset(TestDist, BetaAngle, &probeX, &probeY);
				TestPoint = TestBase - CVector(probeX, probeY, TestDist * 0.0f);
			}
			if(World_ProcessLineOfSight(CamTargetEntity->GetPosition(), TestPoint, colPoint, vehicle, false, true, false, false, false, false) &&
			   vehicle->IsVehicle()){
				float height = CollisionVector_ToNative(Box_GetSize(&vehicle->GetColModel()->boundingBox)).z;
				FoundCar = true;
				HighestCar = height;
				if(((CVehicle*)vehicle)->IsTrain())
					m_bTheHeightFixerVehicleIsATrain = true;
			}

			// Delegate the offset of the test point to the C implementation
			{
				float probeX, probeY;
				Camera_OrbitOffset(TestDist, BetaAngle+DEGTORAD(28.0f), &probeX, &probeY);
				TestPoint = TestBase - CVector(probeX, probeY, TestDist * 0.0f);
			}
			if(World_ProcessLineOfSight(CamTargetEntity->GetPosition(), TestPoint, colPoint, vehicle, false, true, false, false, false, false) &&
			   vehicle->IsVehicle()){
				float height = CollisionVector_ToNative(Box_GetSize(&vehicle->GetColModel()->boundingBox)).z;
				if(FoundCar){
					HighestCar = Max(HighestCar, height);
				}else{
					FoundCar = true;
					HighestCar = height;
				}
				if(((CVehicle*)vehicle)->IsTrain())
					m_bTheHeightFixerVehicleIsATrain = true;
			}

			// Delegate the offset of the test point to the C implementation
			{
				float probeX, probeY;
				Camera_OrbitOffset(TestDist, BetaAngle-DEGTORAD(28.0f), &probeX, &probeY);
				TestPoint = TestBase - CVector(probeX, probeY, TestDist * 0.0f);
			}
			if(World_ProcessLineOfSight(CamTargetEntity->GetPosition(), TestPoint, colPoint, vehicle, false, true, false, false, false, false) &&
			   vehicle->IsVehicle()){
				float height = CollisionVector_ToNative(Box_GetSize(&vehicle->GetColModel()->boundingBox)).z;
				if(FoundCar){
					HighestCar = Max(HighestCar, height);
				}else{
					FoundCar = true;
					HighestCar = height;
				}
				if(((CVehicle*)vehicle)->IsTrain())
					m_bTheHeightFixerVehicleIsATrain = true;
			}

			if(FoundCar){
				m_fDimensionOfHighestNearCar = HighestCar + 0.1f;
				if(Mode == MODE_FIGHT_CAM)
					m_fDimensionOfHighestNearCar += 0.75f;
			}else
				m_fDimensionOfHighestNearCar = 0.0f;
		}
	}

	if(StandingOnBoat){
		m_fDimensionOfHighestNearCar = 1.0f;
		m_fPedBetweenCameraHeightOffset = 0.0f;
	}
}

void
CCam::GetVectorsReadyForRW(void)
{
	// Delegate to the C implementation
	CameraMath_GetVectorsReadyForRW((VectorMath3*)&Front, (VectorMath3*)&Up);
}

bool
CCam::GetBoatLook_L_R_HeightOffset(float &Offset)
{
	if(CamTargetEntity == nil)
		return false;
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(CamTargetEntity->GetModelIndex())); // rouz edit (ChatGPT)
	tBoatHandlingData *handling = mod_HandlingManager.GetBoatPointer(mi->m_handlingId);
	if(handling){
		Offset = handling->fLook_L_R_BehindCamHeight;
		return true;
	}
	return false;	// can't happen, we always get a boat pointer back
}

void
CCam::LookBehind(void)
{
	float Dist, DeltaBeta, TargetOrientation, Angle;
	CVector TargetCoors, TargetFwd, TestCoors;

	TargetCoors = CamTargetEntity->GetPosition();
	Front = CamTargetEntity->GetPosition() - Source;

	if((Mode == MODE_CAM_ON_A_STRING || Mode == MODE_BEHINDBOAT || Mode == MODE_BEHINDCAR) && CamTargetEntity->IsVehicle()){
		LookingBehind = true;
		Dist = Mode == MODE_CAM_ON_A_STRING ? CA_MAX_DISTANCE : 15.5f;
		// Delegate the orientation of the target to the C implementation
		{
			CVector tf = CamTargetEntity->GetForward();

			TargetOrientation = Camera_TargetForwardOrientation(&tf);
		}
		DeltaBeta = TargetOrientation - Beta;
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&DeltaBeta);
		if(DirectionWasLooking != LOOKING_BEHIND)
			LookBehindCamWasInFront = DeltaBeta <= -HALFPI || DeltaBeta >= HALFPI;
		if(LookBehindCamWasInFront)
			TargetOrientation += PI;
		// Delegate the offset on the orbit to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(Dist, TargetOrientation, &orbitX, &orbitY);
			Source.x = orbitX + TargetCoors.x;
			Source.y = orbitY + TargetCoors.y;
		}
		CVector OrigSource = Source;
		TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);
		Front = CamTargetEntity->GetPosition() - Source;
		GetVectorsReadyForRW();
	}
	if(Mode == MODE_1STPERSON && CamTargetEntity->IsVehicle()){
		LookingBehind = true;
		RwCameraSetNearClipPlane(Scene.camera, 0.25f);
		Front = CamTargetEntity->GetForward();
		Front.Normalise();
		if(((CVehicle*)CamTargetEntity)->IsBoat())
			Source.z -= 0.5f;
		if(((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE){
			float FrontDist = 1.1f;
			if(((CVehicle*)CamTargetEntity)->pDriver){
				CVector ExtraFwd(0.0f, 0.0f, 0.0f);
				((CVehicle*)CamTargetEntity)->pDriver->m_pedIK.GetComponentPosition(ExtraFwd, PED_HEAD);
				ExtraFwd += ((CVehicle*)CamTargetEntity)->m_vecMoveSpeed*(*Timer_GetTimeStep()) - CamTargetEntity->GetPosition();
				FrontDist += 0.2f + Max(DotProduct(ExtraFwd, CamTargetEntity->GetForward()), 0.0f);
			}
			Source += FrontDist*Front;
			Front = -Front;
		}else if(((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI){
			Front = -1.0f*CamTargetEntity->GetUp();
			Up = CamTargetEntity->GetForward();
			Source += 0.25f*Front;
		}else{
			Source += 0.25f*Front;
			Front = -Front;
		}
	}
	if(CamTargetEntity->IsPed()){
		Angle = General_GetATanOfXY(Source.x - TargetCoors.x, Source.y - TargetCoors.y) + PI;
		// Delegate the offset on the orbit to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(4.5f, Angle, &orbitX, &orbitY);
			Source.x = orbitX + TargetCoors.x;
			Source.y = orbitY + TargetCoors.y;
		}
		Source.z = 1.15f + TargetCoors.z;
		CVector OrigSource = Source;
		TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);
		Front = TargetCoors - Source;
		GetVectorsReadyForRW();
	}
}

float BOAT_1STPERSON_L_OFFSETX = 0.7f;
float BOAT_1STPERSON_R_OFFSETX = 0.3f;
float BOAT_1STPERSON_LR_OFFSETZ = 0.2f;

void
CCam::LookLeft(void)
{
	float Dist, TargetOrientation;
	CVector TargetCoors, TargetFwd;

	if((Mode == MODE_CAM_ON_A_STRING || Mode == MODE_BEHINDBOAT || Mode == MODE_BEHINDCAR) && CamTargetEntity->IsVehicle()){
		LookingLeft = true;
		TargetCoors = CamTargetEntity->GetPosition();
		Front = CamTargetEntity->GetPosition() - Source;
		if(Mode == MODE_CAM_ON_A_STRING)
			Dist = CA_MAX_DISTANCE;
		else if(Mode == MODE_BEHINDBOAT){
			Dist = 9.0f;
			float Offset = 0.0f;
			if(GetBoatLook_L_R_HeightOffset(Offset) && !CullZones_Cam1stPersonForPlayer())
				Source.z = TargetCoors.z + Offset;
		}else
			Dist = 9.0f;
		// Delegate the orientation of the target to the C implementation
		{
			CVector tf = CamTargetEntity->GetForward();

			TargetOrientation = Camera_TargetForwardOrientation(&tf);
		}
		// Delegate the offset on the orbit to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(Dist, TargetOrientation - HALFPI, &orbitX, &orbitY);
			Source.x = orbitX + TargetCoors.x;
			Source.y = orbitY + TargetCoors.y;
		}

		CColModel *colModel = CamTargetEntity->GetColModel();
		CVector OrigSource = Source;
		TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);

		CVector TopRight = CamTargetEntity->GetPosition() +
			CamTargetEntity->GetRight()*colModel->boundingBox.max.x +
			CamTargetEntity->GetUp()*colModel->boundingBox.max.z;
		// Delegate the cleared height to the C implementation
		Source.z = Camera_ClearedHeight(m_cvecTargetCoorsForFudgeInter.z, TopRight.z, OrigSource.z, Source.z);

		Front = CamTargetEntity->GetPosition() - Source;
		Front.z += 1.1f;
		if(Mode == MODE_BEHINDBOAT)
			Front.z += 1.2f;
		GetVectorsReadyForRW();
	}
	if(Mode == MODE_1STPERSON && CamTargetEntity->IsVehicle()){
		LookingLeft = true;
		RwCameraSetNearClipPlane(Scene.camera, 0.25f);
		if(((CVehicle*)CamTargetEntity)->IsBoat()){
			if(((CVehicle*)CamTargetEntity)->pDriver){
				CVector neck(0.0f, 0.0f, 0.0f);
				CPed *driver = ((CVehicle*)CamTargetEntity)->pDriver;
				driver->SetPedPositionInCar();
				driver->GetMatrix().UpdateRW();
				driver->UpdateRwFrame();
				driver->UpdateRpHAnim();
				driver->m_pedIK.GetComponentPosition(neck, PED_NECK);
				Source = neck +
					BOAT_1STPERSON_L_OFFSETX*CamTargetEntity->GetRight() +
					BOAT_1STPERSON_LR_OFFSETZ*CamTargetEntity->GetUp();
			}else
				Source.z -= 0.5f;
		}

		// Delegate the side direction of the camera to the C implementation
		{
			CVector u = CamTargetEntity->GetUp();
			CVector f = CamTargetEntity->GetForward();

			Camera_SideFront(&u, &f, true, &Up, &Front);
		}
		if(((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE)
			Source -= 1.45f*Front;
	}
}

void
CCam::LookRight(void)
{
	float Dist, TargetOrientation;
	CVector TargetCoors, TargetFwd;
	CColPoint colPoint;

	if((Mode == MODE_CAM_ON_A_STRING || Mode == MODE_BEHINDBOAT || Mode == MODE_BEHINDCAR) && CamTargetEntity->IsVehicle()){
		LookingRight = true;
		TargetCoors = CamTargetEntity->GetPosition();
		Front = CamTargetEntity->GetPosition() - Source;
		if(Mode == MODE_CAM_ON_A_STRING)
			Dist = CA_MAX_DISTANCE;
		else if(Mode == MODE_BEHINDBOAT){
			Dist = 9.0f;
			float Offset = 0.0f;
			if(GetBoatLook_L_R_HeightOffset(Offset) && !CullZones_Cam1stPersonForPlayer())
				Source.z = TargetCoors.z + Offset;
		}else
			Dist = 9.0f;
		// Delegate the orientation of the target to the C implementation
		{
			CVector tf = CamTargetEntity->GetForward();

			TargetOrientation = Camera_TargetForwardOrientation(&tf);
		}
		// Delegate the offset on the orbit to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(Dist, TargetOrientation + HALFPI, &orbitX, &orbitY);
			Source.x = orbitX + TargetCoors.x;
			Source.y = orbitY + TargetCoors.y;
		}

		CColModel *colModel = CamTargetEntity->GetColModel();
		CVector OrigSource = Source;
		TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);

		CVector TopLeft = CamTargetEntity->GetPosition() +
			CamTargetEntity->GetRight()*colModel->boundingBox.min.x +
			CamTargetEntity->GetUp()*colModel->boundingBox.max.z;
		// Delegate the cleared height to the C implementation
		Source.z = Camera_ClearedHeight(m_cvecTargetCoorsForFudgeInter.z, TopLeft.z, OrigSource.z, Source.z);

		Front = CamTargetEntity->GetPosition() - Source;
		Front.z += 1.1f;
		if(Mode == MODE_BEHINDBOAT)
			Front.z += 1.2f;
		GetVectorsReadyForRW();
	}
	if(Mode == MODE_1STPERSON && CamTargetEntity->IsVehicle()){
		LookingRight = true;
		RwCameraSetNearClipPlane(Scene.camera, 0.25f);
		if(((CVehicle*)CamTargetEntity)->IsBoat()){
			if(((CVehicle*)CamTargetEntity)->pDriver){
				CVector neck(0.0f, 0.0f, 0.0f);
				CPed *driver = ((CVehicle*)CamTargetEntity)->pDriver;
				driver->SetPedPositionInCar();
				driver->GetMatrix().UpdateRW();
				driver->UpdateRwFrame();
				driver->UpdateRpHAnim();
				driver->m_pedIK.GetComponentPosition(neck, PED_NECK);
				Source = neck +
					BOAT_1STPERSON_R_OFFSETX*CamTargetEntity->GetRight() +
					BOAT_1STPERSON_LR_OFFSETZ*CamTargetEntity->GetUp();
			}else
				Source.z -= 0.5f;
		}

		// Delegate the side direction of the camera to the C implementation
		{
			CVector u = CamTargetEntity->GetUp();
			CVector f = CamTargetEntity->GetForward();

			Camera_SideFront(&u, &f, false, &Up, &Front);
		}
		if(((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE)
			Source -= 1.45f*Front;
	}
}

void
CCam::ClipIfPedInFrontOfPlayer(void)
{
	float FwdAngle, PedAngle, Near;
	CVector vDist;
	CPed *Player;
	bool found = false;
	int ped = 0;

	// unused: TheCamera.pTargetEntity->GetPosition() - TheCamera.GetGameCamPosition();

	FwdAngle = General_GetATanOfXY(TheCamera.GetForward().x, TheCamera.GetForward().y);
	Player = World_Players[World_PlayerInFocus].m_pPed;
	while(ped < Player->m_numNearPeds && !found)
		if(Player->m_nearPeds[ped] && Player->m_nearPeds[ped]->GetPedState() != PED_DEAD)
			found = true;
		else
			ped++;
	if(found){
		vDist = Player->m_nearPeds[ped]->GetPosition() - TheCamera.GetGameCamPosition();
		PedAngle = General_GetATanOfXY(vDist.x, vDist.y);
		// Delegate to the C implementation
		if(CameraMath_NearClipForPedInFront(FwdAngle, PedAngle, vDist.x, vDist.y, DEFAULT_NEAR, &Near))
			RwCameraSetNearClipPlane(Scene.camera, Near);
	}
}

void
CCam::KeepTrackOfTheSpeed(const CVector &source, const CVector &target, const CVector &up, const float &alpha, const float &beta, const float &fov)
{
	static CamSpeedState state;
	static bool stateInitialised = false;

	// The previous values start as the first ones given
	if(!stateInitialised){
		state.source = *(const VectorMath3*)&source;
		state.target = *(const VectorMath3*)&target;
		state.up = *(const VectorMath3*)&up;
		state.beta = beta;
		state.alpha = alpha;
		state.fov = fov;
		stateInitialised = true;
	}

	// Delegate to the C implementation
	CameraMath_KeepTrackOfTheSpeed(&state, TheCamera.m_bJust_Switched, *(const VectorMath3*)&source, *(const VectorMath3*)&target, *(const VectorMath3*)&up, alpha, beta, fov,
		(VectorMath3*)&m_cvecSourceSpeedOverOneFrame, (VectorMath3*)&m_cvecTargetSpeedOverOneFrame, (VectorMath3*)&m_cvecUpOverOneFrame,
		&m_fFovSpeedOverOneFrame, &m_fBetaSpeedOverOneFrame, &m_fAlphaSpeedOverOneFrame);
}

bool
CCam::Using3rdPersonMouseCam(void) 
{
	return CCamera::m_bUseMouse3rdPerson && Mode == MODE_FOLLOWPED;
}

bool
CCam::GetWeaponFirstPersonOn(void)
{
	return CamTargetEntity && CamTargetEntity->IsPed() && ((CPed*)CamTargetEntity)->GetWeapon()->m_bAddRotOffset;
}

bool
CCam::IsTargetInWater(const CVector &CamCoors)
{
	if(CamTargetEntity){
		float WaterZ = -6000.0f;
		WaterLevel_GetWaterLevel(CamTargetEntity->GetPosition(), &WaterZ, false);
		// Delegate the test to the C implementation
		if(CamTargetEntity->IsPed()){
			if(CameraMath_IsTargetUnderWater(((CPed*)CamTargetEntity)->bIsDrowning, ((CPed*)CamTargetEntity)->bIsInWater, CamTargetEntity->GetPosition().z, WaterZ))
				return true;
		}else{
			assert(CamTargetEntity->IsVehicle());
			if(CameraMath_IsTargetUnderWater(((CVehicle*)CamTargetEntity)->bIsDrowning, ((CVehicle*)CamTargetEntity)->bIsInWater, CamTargetEntity->GetPosition().z, WaterZ))
				return true;
		}
	}
	m_vecLastAboveWaterCamPosition = Source;
	return false;
}

void
CCam::PrintMode(void)
{
	// Doesn't do anything
	char buf[256];

	if(PrintDebugCode){
		sprintf(buf, "                                                   ");
		sprintf(buf, "                                                   ");
		sprintf(buf, "                                                   ");

		static Const char *modes[] = { "None",
			"Top Down", "GTA Classic", "Behind Car", "Follow Ped",
			"Aiming", "Debug", "Sniper", "Rocket", "Model Viewer", "Bill",
			"Syphon", "Circle", "Cheesy Zoom", "Wheel", "Fixed",
			"1st Person", "Fly by", "on a String", "Reaction",
			"Follow Ped with Bind", "Chris", "Behind Boat",
			"Player fallen in Water", "Train Roof", "Train Side",
			"Blood on the tracks", "Passenger", "Syphon Crim in Front",
			"Dead Baby", "Pillow Paps", "Look at Cars", "Arrest One",
			"Arrest Two", "M16", "Special fixed for Syphon", "Fight",
			"Top Down Ped", "Lighthouse",
			"Sniper run about", "Rocket run about",
			"1st Person run about", "M16 run about", "Fight run about",
			"Editor", "Helicannon", "Camera"
		};
		sprintf(buf, "Cam: %s", modes[TheCamera.Cams[TheCamera.ActiveCam].Mode]);
		Debug_PrintAt(buf, 2, 5);
	}

	if(DebugCamMode != MODE_NONE){
		switch(Mode){
		case MODE_FOLLOWPED:
			sprintf(buf, "Debug:- Cam Choice1. No Locking, used as game default");
			break;
		case MODE_REACTION:
			sprintf(buf, "Debug:- Cam Choice2. Reaction Cam On A String ");
			sprintf(buf, "        Uses Locking Button LeftShoulder 1. ");	// lie
			break;
		case MODE_FOLLOW_PED_WITH_BIND:
			sprintf(buf, "Debug:- Cam Choice3. Game ReactionCam with Locking ");
			sprintf(buf, "        Uses Locking Button LeftShoulder 1. ");
			break;
		case MODE_CHRIS:
			sprintf(buf, "Debug:- Cam Choice4. Chris's idea.  ");
			sprintf(buf, "        Uses Locking Button LeftShoulder 1. ");
			sprintf(buf, "        Also control the camera using the right analogue stick.");
			break;
		}
	}
}

// This code is really bad. wtf R*?
CVector
CCam::DoAverageOnVector(const CVector &vec)
{
	// Delegate to the C implementation
	VectorMath3 average = CameraMath_DoAverageOnVector((VectorMath3*)m_arrPreviousVectors, &m_iRunningVectorArrayPos, &m_iRunningVectorCounter, ResetStatics, *(const VectorMath3*)&vec);
	return CVector(average.x, average.y, average.z);
}

float DefaultAcceleration = 0.045f;
float DefaultMaxStep = 0.15f;
float fDefaultSpeedStep = 0.025f;
float fDefaultSpeedMultiplier = 0.09f;
float fDefaultSpeedLimit = 0.15f;
float fDefaultSpeedStep4Avoid = 0.02f;
float fDefaultSpeedMultiplier4Avoid = 0.05f;
float fDefaultSpeedLimit4Avoid = 0.25f;
float fAvoidGeomThreshhold = 1.5f;
float fMiniGunBetaOffset = 0.3f;

void
CCam::Process_FollowPed(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	if(!CamTargetEntity->IsPed())
		return;

	CVector TargetCoors, Dist, IdealSource;
	float Length = 0.0f;
	static bool PickedASide;
	static float FixedTargetOrientation = 0.0f;
	float AngleToGoTo = 0.0f;
	bool StandingInTrain = false;
	float ZoomGroundTarget = 0.0f;
	float ZoomZTarget = 0.0f;
	static int TimeIndicatedWantedToGoDown = 0;
	static bool StartedCountingForGoDown = false;
	static float ZoomGround = 0.0f;
	static float ZoomGroundSpeed = 0.0f;
	static float ZoomZ = 0.0f;
	static float ZoomZSpeed = 0.0f;
	float DeltaBeta;

	m_bFixingBeta = false;
	bBelowMinDist = false;
	bBehindPlayerDesired = false;

	FOV = DefaultFOV;

	if(ResetStatics){
		Rotating = false;
		m_bCollisionChecksOn = true;
		FixedTargetOrientation = 0.0f;
		PickedASide = false;
		StartedCountingForGoDown = false;
		AngleToGoTo = 0.0f;
		ZoomGround = 0.0f;
		ZoomGroundSpeed = 0.0f;
		ZoomZ = 0.0f;
		ZoomZSpeed = 0.0f;
		Distance = 500.0f;
	}


	TargetCoors = CameraTarget;

	// Take speed of thing we're standing on into account
	CVector GroundMovement(0.0f, 0.0f, 0.0f);
	CPhysical *ground = (CPhysical*)((CPed*)CamTargetEntity)->m_pCurSurface;
	if(ground && (ground->IsVehicle() || ground->IsObject()))
		GroundMovement += ground->GetSpeed(CamTargetEntity->GetPosition() - ground->GetPosition()) * (*Timer_GetTimeStep());

	Source += GroundMovement;
	IdealSource = Source;
	TargetCoors.z += m_fSyphonModeTargetZOffSet;

	TargetCoors.z = DoAverageOnVector(TargetCoors).z;

	Dist.x = IdealSource.x - TargetCoors.x;
	Dist.y = IdealSource.y - TargetCoors.y;
	Length = Dist.Magnitude2D();

	// Cam on a string. With a fixed distance. Zoom in/out is done later.
	if(Length != 0.0f){
		IdealSource = TargetCoors + CVector(Dist.x, Dist.y, 0.0f)/Length * m_fMinRealGroundDist;
		IdealSource.z += GroundMovement.z;
	}else
		IdealSource = TargetCoors + CVector(1.0f, 1.0f, 0.0f);

	if(TheCamera.m_bUseTransitionBeta && ResetStatics){
		CVector VecDistance;
		// Delegate the offset on the orbit to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(m_fMinRealGroundDist, m_fTransitionBeta, &orbitX, &orbitY);
			IdealSource.x = TargetCoors.x + orbitX;
			IdealSource.y = TargetCoors.y + orbitY;
		}
		Beta = General_GetATanOfXY(IdealSource.x - TargetCoors.x, IdealSource.y - TargetCoors.y);
	}else
		Beta = General_GetATanOfXY(Source.x - TargetCoors.x, Source.y - TargetCoors.y);

	if(TheCamera.m_bCamDirectlyBehind){
		 m_bCollisionChecksOn = true;
		 Beta = TargetOrientation + PI;
	}

	if(FindPlayerVehicle())
		if(FindPlayerVehicle()->m_vehType == VEHICLE_TYPE_TRAIN)
			StandingInTrain = true;

	if(TheCamera.m_bCamDirectlyInFront){
		 m_bCollisionChecksOn = true;
		 Beta = TargetOrientation;
	}

	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);

	if(TheCamera.PedZoomIndicator == CAM_ZOOM_1 &&
	   ((CPed*)CamTargetEntity)->GetPedState() != PED_ENTER_CAR &&
	   ((CPed*)CamTargetEntity)->GetPedState() != PED_CARJACK){
		ZoomGroundTarget = m_fTargetZoomGroundOne;
		ZoomZTarget = m_fTargetZoomOneZExtra;
	}else if(TheCamera.PedZoomIndicator == CAM_ZOOM_2 || TheCamera.PedZoomIndicator == CAM_ZOOM_1){
		ZoomGroundTarget = m_fTargetZoomGroundTwo;
		ZoomZTarget = m_fTargetZoomTwoZExtra;
	}else if(TheCamera.PedZoomIndicator == CAM_ZOOM_3){
		ZoomGroundTarget = m_fTargetZoomGroundThree;
		ZoomZTarget = m_fTargetZoomThreeZExtra;
	}
	if(m_fCloseInPedHeightOffset >  0.00001f){
		ZoomGroundTarget = m_fTargetCloseInDist;
		ZoomZTarget = m_fTargetZoomZCloseIn;
	}
	if(ResetStatics){
		ZoomGround = ZoomGroundTarget;
		ZoomZ = ZoomZTarget;
	}

	float SpeedStep = fDefaultSpeedStep;
	float SpeedMultiplier = fDefaultSpeedMultiplier;
	float SpeedLimit = fDefaultSpeedLimit;
	bool Shooting = false;
	CPed *ped = (CPed*)CamTargetEntity;
	if(ped->GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED)
		if(CPad::GetPad(0)->GetWeapon())
			Shooting = true;
	if(ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_DETONATOR ||
	   ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT)
		Shooting = false;


	// Figure out if and where we want to rotate

	if(CPad::GetPad(0)->ForceCameraBehindPlayer() && !Pickups_PlayerOnWeaponPickup || Shooting){

		// Center cam behind player

		if(PickedASide){
			if(AngleToGoTo == 0.0f){
				FixedTargetOrientation = TargetOrientation + PI;
				if(Shooting && ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
					FixedTargetOrientation -= fMiniGunBetaOffset;
			}
			Rotating = true;
		}else{
			FixedTargetOrientation = TargetOrientation + PI;
			Rotating = true;
			PickedASide = true;
			if(Shooting && ped->GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
				FixedTargetOrientation -= fMiniGunBetaOffset;
		}
	}else if(Abs(TheCamera.m_fAvoidTheGeometryProbsTimer) > fAvoidGeomThreshhold && !Rotating ){

		if(TheCamera.m_fAvoidTheGeometryProbsTimer < 0.0f)
			FixedTargetOrientation = TargetOrientation;
		else
			FixedTargetOrientation = TargetOrientation + PI;
		float dist = (Source - TargetCoors).Magnitude();
		float mult = dist > 0.1f ? 1.0f/dist : 10.0f;
		SpeedStep = mult * fDefaultSpeedStep4Avoid;
		SpeedMultiplier = mult * fDefaultSpeedMultiplier4Avoid;
		SpeedLimit = mult * fDefaultSpeedLimit4Avoid;
	}

	int MoveState = ((CPed*)CamTargetEntity)->m_nMoveState;
	if(MoveState != PEDMOVE_NONE && MoveState != PEDMOVE_STILL &&
	   !(CPad::GetPad(0)->ForceCameraBehindPlayer() && !Pickups_PlayerOnWeaponPickup) && !Shooting){
		Rotating = false;
		if(TheCamera.m_fAvoidTheGeometryProbsTimer <= fAvoidGeomThreshhold)
			BetaSpeed = 0.0f;
	}

	// Now do the Beta rotation

	float RotDistance = m_fMinRealGroundDist;

	if(Rotating || TheCamera.m_fAvoidTheGeometryProbsTimer > fAvoidGeomThreshhold){
		m_bFixingBeta = true;

		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&FixedTargetOrientation);

		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&Beta);


		// This is inlined WellBufferMe - unfortunately modified so we can't just call it
		{
		DeltaBeta = FixedTargetOrientation - Beta;
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&DeltaBeta);

		// this is the added bit
		if(!Rotating){
			if(TheCamera.m_nAvoidTheGeometryProbsDirn == -1 && DeltaBeta > 0.0f ||
			   TheCamera.m_nAvoidTheGeometryProbsDirn == 1 && DeltaBeta < 0.0f)
				DeltaBeta *= -1.0f;
		}

		float ReqSpeed = DeltaBeta * SpeedMultiplier;
		// this is also added
		ReqSpeed = Clamp(ReqSpeed, -SpeedLimit, SpeedLimit);

		// Delegate the step of the angle of the camera to the C implementation
		Camera_FollowBetaStep(&Beta, &BetaSpeed, ReqSpeed, SpeedStep, (*Timer_GetTimeStep()));
		}

		if(ResetStatics){
			Beta = FixedTargetOrientation;
			BetaSpeed = 0.0f;
		}

		// Delegate the offset on the orbit to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(RotDistance, Beta, &orbitX, &orbitY);
			Source.x = TargetCoors.x + orbitX;
			Source.y = TargetCoors.y + orbitY;
		}

		// Check if we can stop rotating
		DeltaBeta = FixedTargetOrientation - Beta;
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&DeltaBeta);
		if(General_AbsBelow(DeltaBeta, DEGTORAD(1.0f)) && !bBehindPlayerDesired){
			// Stop rotation
			PickedASide = false;
			Rotating = false;
			BetaSpeed = 0.0f;
		}
	}


	if(TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront ||
	   StandingInTrain || Rotating ||
	   TheCamera.m_bUseTransitionBeta && ResetStatics ||
	   Abs(TheCamera.m_fAvoidTheGeometryProbsTimer) > fAvoidGeomThreshhold){
		if(TheCamera.m_bUseTransitionBeta){
			Beta = m_fTransitionBeta;
			// Delegate the offset on the orbit to the C implementation
			{
				float orbitX, orbitY;
				Camera_OrbitOffset(RotDistance, m_fTransitionBeta, &orbitX, &orbitY);
				Source.x = TargetCoors.x + orbitX;
				Source.y = TargetCoors.y + orbitY;
			}
		}
		if(TheCamera.m_bCamDirectlyBehind){
			Beta = TargetOrientation + PI;
			// Delegate the offset on the orbit to the C implementation
			{
				float orbitX, orbitY;
				Camera_OrbitOffset(RotDistance, Beta, &orbitX, &orbitY);
				Source.x = TargetCoors.x + orbitX;
				Source.y = TargetCoors.y + orbitY;
			}
		}
		if(TheCamera.m_bCamDirectlyInFront){
			Beta = TargetOrientation;
			// Delegate the offset on the orbit to the C implementation
			{
				float orbitX, orbitY;
				Camera_OrbitOffset(RotDistance, Beta, &orbitX, &orbitY);
				Source.x = TargetCoors.x + orbitX;
				Source.y = TargetCoors.y + orbitY;
			}
		}
		if(StandingInTrain){
			Beta = TargetOrientation + PI;
			// Delegate the offset on the orbit to the C implementation
			{
				float orbitX, orbitY;
				Camera_OrbitOffset(RotDistance, Beta, &orbitX, &orbitY);
				Source.x = TargetCoors.x + orbitX;
				Source.y = TargetCoors.y + orbitY;
			}
			m_fDimensionOfHighestNearCar = 0.0f;
			m_fCamBufferedHeight = 0.0f;
			m_fCamBufferedHeightSpeed = 0.0f;
		}
		if(StandingInTrain){
			Beta = TargetOrientation + PI;
			// Delegate the offset on the orbit to the C implementation
			{
				float orbitX, orbitY;
				Camera_OrbitOffset(RotDistance, Beta, &orbitX, &orbitY);
				Source.x = TargetCoors.x + orbitX;
				Source.y = TargetCoors.y + orbitY;
			}
			m_fDimensionOfHighestNearCar = 0.0f;
			m_fCamBufferedHeight = 0.0f;
			m_fCamBufferedHeightSpeed = 0.0f;
		}

		// Beta and Source already set in the rotation code
	}else{
		Source = IdealSource;
		BetaSpeed = 0.0f;
	}
	Source.z = IdealSource.z;

	// Zoom out camera
	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
	WellBufferMe(ZoomGroundTarget, &ZoomGround, &ZoomGroundSpeed, 0.2f, 0.07f, false);
	WellBufferMe(ZoomZTarget, &ZoomZ, &ZoomZSpeed, 0.2f, 0.07f, false);
	Source.x -= Front.x*ZoomGround;
	Source.y -= Front.y*ZoomGround;
	Source.z += ZoomZ;


	// Process height offset to avoid peds and cars

	float TargetZOffSet = Max(m_fDimensionOfHighestNearCar, m_fPedBetweenCameraHeightOffset);
	float TargetHeight = CameraTarget.z + TargetZOffSet - Source.z;

	if(TargetHeight > m_fCamBufferedHeight){
		// Have to go up
		if(TargetZOffSet == m_fPedBetweenCameraHeightOffset && TargetZOffSet > m_fCamBufferedHeight)
			WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.04f, false);
		else
			WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.025f, false);
		StartedCountingForGoDown = false;
	}else{
		// Have to go down
		if(StartedCountingForGoDown){
			if((*Timer_GetTimeInMilliseconds()) != TimeIndicatedWantedToGoDown){
				if(TargetHeight > 0.0f)
					WellBufferMe(TargetHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.01f, false);
				else
					WellBufferMe(0.0f, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.2f, 0.01f, false);
			}
		}else{
			StartedCountingForGoDown = true;
			TimeIndicatedWantedToGoDown = (*Timer_GetTimeInMilliseconds());
		}
	}

	Source.z += m_fCamBufferedHeight;
	// Delegate the raise of the target to the C implementation
	TargetCoors.z += Camera_BufferedHeightOffset(m_fCamBufferedHeight);
	m_cvecTargetCoorsForFudgeInter = TargetCoors;

	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);
	float TargetDist = (TargetCoors - Source).Magnitude();
	if(TargetDist < Distance)
		Distance = TargetDist;
	else{
		float f = Pow(0.97f, (*Timer_GetTimeStep()));
		Distance = (1.0f - f)*TargetDist + f*Distance;
		if(TargetDist > 0.05f)
			Source = TargetCoors + (Source-TargetCoors)*Distance/TargetDist;
		float clip = Distance-fRangePlayerRadius;
		if(clip < RwCameraGetNearClipPlane(Scene.camera))
			RwCameraSetNearClipPlane(Scene.camera, Max(clip, fCloseNearClipLimit));
	}

	Front = TargetCoors - Source;
	m_fRealGroundDist = Front.Magnitude2D();
	m_fMinDistAwayFromCamWhenInterPolating = m_fRealGroundDist;	
	Front.Normalise();
	GetVectorsReadyForRW();
	TheCamera.m_bCamDirectlyBehind = false;
	TheCamera.m_bCamDirectlyInFront = false;

	ResetStatics = false;
}

float fBaseDist = 1.7f;
float fAngleDist = 2.0f;
float fFalloff = 3.0f;
float fStickSens = 0.01f;
float fTweakFOV = 1.1f;
float fTranslateCamUp = 0.8f;
int16 nFadeControlThreshhold = 45;
float fDefaultAlphaOrient = -0.22f;
float fMouseAvoidGeomReturnRate = 0.92f;

void
CCam::Process_FollowPedWithMouse(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	FOV = DefaultFOV;

	if(!CamTargetEntity->IsPed())
		return;

	CVector TargetCoors;
	float CamDist;
	CColPoint colPoint;
	CEntity *entity;

	if(ResetStatics){
		Rotating = false;
		m_bCollisionChecksOn = true;
		Input_ClearMouseHistory(); // rouz edit (ChatGPT)
		ResetStatics = false;
	}

	bool OnTrain = FindPlayerVehicle() && FindPlayerVehicle()->IsTrain();

	TargetCoors = CameraTarget;
	if (rouz.ped_cam_change)
		TargetCoors.z += 0.2f;			// rouz edit, focus more on the ped instead of above his head
	else
		TargetCoors.z += fTranslateCamUp;

	float AlphaOffset, BetaOffset;
	if(CPad::GetPad(0)->IsPlayerControlsDisabledBy(PLAYERCONTROL_PLAYERINFO)){
		CVector ToCam = Source - TargetCoors;
		ToCam.Normalise();
		if(ToCam.z < -0.9f)
			BetaOffset = TargetOrientation + PI;
		else
			BetaOffset = Atan2(ToCam.y, ToCam.x);
		BetaOffset -= Beta;
		AlphaOffset = 0.0f;
	}else{
		// Look around
		bool UseMouse = false;
		float MouseX = Input_GetMouseX(); // rouz edit (ChatGPT)
		float MouseY = Input_GetMouseY(); // rouz edit (ChatGPT)
		float LookLeftRight, LookUpDown;
		if((MouseX != 0.0f || MouseY != 0.0f) && !CPad::GetPad(0)->ArePlayerControlsDisabled()){
			UseMouse = true;
			LookLeftRight = -2.5f*MouseX;
			LookUpDown = 4.0f*MouseY;
		}else{
			LookLeftRight = -CPad::GetPad(0)->LookAroundLeftRight();
			LookUpDown = CPad::GetPad(0)->LookAroundUpDown();
		}
		if(UseMouse){
			BetaOffset = LookLeftRight * TheCamera.m_fMouseAccelHorzntl * FOV/80.0f;
			AlphaOffset = LookUpDown * TheCamera.m_fMouseAccelVertical * FOV/80.0f;
		}else{
			// Delegate the offsets of the stick to the C implementation
			Camera_StickLookOffsets(LookLeftRight, LookUpDown, fStickSens, FOV, (*Timer_GetTimeStep()), &BetaOffset, &AlphaOffset);
		}
	}

	if(TheCamera.GetFading() && TheCamera.GetFadingDirection() == FADE_IN && nFadeControlThreshhold < Draw_FadeValue ||
	   Draw_FadeValue > 200 ||
	   CPad::GetPad(0)->IsPlayerControlsDisabledBy(PLAYERCONTROL_PLAYERINFO)){
		// Delegate the change of the angle to the C implementation
		AlphaOffset = Camera_AlphaRecentre(Alpha, fDefaultAlphaOrient);
	}

	Alpha += AlphaOffset;
	Beta += BetaOffset;
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);
	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 45.0f);

	// SA code
	// Delegate the distance of the camera to the C implementation
#ifdef FREE_CAM
	bool useFalloff = (CCamera::bFreeCam && Alpha > 0.0f) || (!CCamera::bFreeCam && Alpha > fBaseDist);
#else
	bool useFalloff = Alpha > fBaseDist;	// comparing an angle against a distance?
#endif
	CamDist = Camera_PedCamDistance(fBaseDist, Alpha, fFalloff, fAngleDist, useFalloff);

	if (rouz.ped_cam_change)
		CamDist += TheCamera.m_fPedZoomValueSmooth;

	if(TheCamera.m_bUseTransitionBeta)
		Beta = m_fTransitionBeta;

	if(TheCamera.m_bCamDirectlyBehind)
		Beta = TheCamera.m_PedOrientForBehindOrInFront + PI;
	if(TheCamera.m_bCamDirectlyInFront)
		Beta = TheCamera.m_PedOrientForBehindOrInFront;
	if(OnTrain)
		Beta = TargetOrientation;

	// Delegate the direction to the C implementation
	{
		CVector frontXYZ;
		Camera_FrontFromAngles(Alpha, Beta, &frontXYZ);
		Front.x = frontXYZ.x;
		Front.y = frontXYZ.y;
		Front.z = frontXYZ.z;
	}
	Source = TargetCoors - Front*CamDist;
	m_cvecTargetCoorsForFudgeInter = TargetCoors;

	// Clip Source and fix near clip
	World_pIgnoreEntity = CamTargetEntity;
	entity = nil;
	if(World_ProcessLineOfSight(TargetCoors, Source, colPoint, entity, true, true, true, true, false, false, true)){
		float PedColDist = (TargetCoors - CollisionVector_ToNative(colPoint.point)).Magnitude();
		float ColCamDist = CamDist - PedColDist;
		if(entity->IsPed() && ColCamDist > DEFAULT_NEAR + 0.1f){
			// Ped in the way but not clipping through
			if(World_ProcessLineOfSight(CollisionVector_ToNative(colPoint.point), Source, colPoint, entity, true, true, true, true, false, false, true)){
				PedColDist = (TargetCoors - CollisionVector_ToNative(colPoint.point)).Magnitude();
				Source = CollisionVector_ToNative(colPoint.point);
				if(PedColDist < DEFAULT_NEAR + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipNearPed(PedColDist));
			}else{
				RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipPedInWay(ColCamDist, DEFAULT_NEAR));
			}
		}else{
			Source = CollisionVector_ToNative(colPoint.point);
			if(PedColDist < DEFAULT_NEAR + 0.3f)
				RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipNearPed(PedColDist));
		}
	}
	World_pIgnoreEntity = nil;

	// Delegate the width of the view plane to the C implementation shared with the avoidance of the geometry
	float ViewPlaneWidth = Camera_AvoidViewPlaneWidth(FOV, Draw_CalculateAspectRatio(), fTweakFOV);
	float Near = RwCameraGetNearClipPlane(Scene.camera);
	float radius = ViewPlaneWidth*Near;
	entity = World_TestSphereAgainstWorld(Source + Front*Near, radius, nil, true, true, false, true, false, false);
	int i = 0;
	while(entity){
		CVector CamToCol = CollisionVector_ToNative(gaTempSphereColPoints[0].point) - Source;
		float frontDist = DotProduct(CamToCol, Front);
		float dist = (CamToCol - Front*frontDist).Magnitude() / ViewPlaneWidth;

		// Try to decrease near clip
		dist = Camera_NearClipFromCollision(Near, dist);
		if(dist < Near)
			RwCameraSetNearClipPlane(Scene.camera, dist);

		// Move forward a bit
		if(dist == 0.1f)
			Source += (TargetCoors - Source)*0.3f;

		Near = RwCameraGetNearClipPlane(Scene.camera);
#ifndef FIX_BUGS
		// this is wrong...DEGTORAD missing
		radius = Tan(FOV / 2.0f) * Draw_CalculateAspectRatio() * fTweakFOV * Near;
#else
		radius = ViewPlaneWidth*Near;
#endif
		// Keep testing
		entity = World_TestSphereAgainstWorld(Source + Front*Near, radius, nil, true, true, false, true, false, false);

		i++;
		if(i > 5)
			entity = nil;
	}

	float TargetDist = (TargetCoors - Source).Magnitude();
	// Delegate the distance to the C implementation
	if(Camera_MouseAvoidDistance(TargetDist, fMouseAvoidGeomReturnRate, (*Timer_GetTimeStep()), &Distance)){
		if(TargetDist > 0.05f)
			Source = TargetCoors + (Source-TargetCoors)*Distance/TargetDist;
		float clip = Distance-fRangePlayerRadius;
		if(clip < RwCameraGetNearClipPlane(Scene.camera))
			RwCameraSetNearClipPlane(Scene.camera, Max(clip, fCloseNearClipLimit));
	}

	TheCamera.m_bCamDirectlyInFront = false;
	TheCamera.m_bCamDirectlyBehind = false;

	GetVectorsReadyForRW();

	if(((CPed*)CamTargetEntity)->CanStrafeOrMouseControl() && Draw_FadeValue < 250 &&
	   (TheCamera.GetFadingDirection() != FADE_OUT || Draw_FadeValue <= 100) &&
	   !CPad::GetPad(0)->IsPlayerControlsDisabledBy(PLAYERCONTROL_PLAYERINFO)){
		float Heading = Front.Heading();
		((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Heading;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Heading;
		TheCamera.pTargetEntity->SetHeading(Heading);
		TheCamera.pTargetEntity->GetMatrix().UpdateRW();
	}
}

float fBillsBetaOffset;	// made up name, actually in CCam

void
CCam::Process_BehindCar(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	FOV = DefaultFOV;

	if(!CamTargetEntity->IsVehicle())
		return;

	CVector TargetCoors = CameraTarget;
	TargetCoors.z -= 0.2f;
	CA_MAX_DISTANCE = 9.95f;
	CA_MIN_DISTANCE = 8.5f;

	CVector Dist = Source - TargetCoors;
	float Length = Dist.Magnitude2D();
	m_fDistanceBeforeChanges = Length;
	if(Length < 0.002f)
		Length = 0.002f;
	Beta = General_GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y);
#ifdef TOGGLEABLE_BETA_FEATURES
	// This is completely made up but Bill's cam manipulates an angle before calling this
	// and otherwise calculating Beta doesn't make much sense.
	Beta += fBillsBetaOffset;
	fBillsBetaOffset = 0.0f;
	// Delegate the offset of the camera from its target to the C implementation
	{
		float orbitX, orbitY;
		Camera_OrbitOffset(Length, Beta, &orbitX, &orbitY);
		Dist.x = -orbitX;
		Dist.y = -orbitY;
	}
	Source = TargetCoors + Dist;
#endif
	if(Length > CA_MAX_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MAX_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MAX_DISTANCE;
	}else if(Length < CA_MIN_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MIN_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MIN_DISTANCE;
	}
	TargetCoors.z += 0.8f;

	Alpha = DEGTORAD(25.0f);
	Source.z = TargetCoors.z + CA_MAX_DISTANCE*Sin(Alpha);

	RotCamIfInFrontCar(TargetCoors, TargetOrientation);
	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, m_cvecTargetCoorsForFudgeInter, Source, FOV);

	Front = TargetCoors - Source;
	ResetStatics = false;
	GetVectorsReadyForRW();
}

float ZmOneAlphaOffset[] = { -0.01f, 0.1f, 0.125f, -0.1f, -0.06f };
float ZmTwoAlphaOffset[] = { 0.045f, 0.12f, 0.045f, 0.045f, -0.035f };
float ZmThreeAlphaOffset[] = { 0.005f, 0.005f, 0.15f, 0.005f, 0.12f };
float INIT_RC_HELI_HORI_EXTRA = 6.0f;
float INIT_RC_PLANE_HORI_EXTRA = 9.5f;
float INIT_RC_HELI_ALPHA_EXTRA = 0.2f;
float INIT_RC_PLANE_ALPHA_EXTRA = 0.295f;

void
CCam::WorkOutCamHeight(const CVector &TargetCoors, float TargetOrientation, float TargetHeight)
{
	if(!CamTargetEntity->IsVehicle())
		return;

	static float AlphaOffset = 0.0;
	static float AlphaOffsetSpeed = 0.0;
	static float AlphaDec = 0.0f;

	bool isHeli = false;
	bool isBike = false;
	int appearance = ((CVehicle*)CamTargetEntity)->GetVehicleAppearance();
	if(appearance == VEHICLE_APPEARANCE_BIKE)
		isBike = true;
	if(appearance == VEHICLE_APPEARANCE_HELI)
		isHeli = true;
	int index = 0;
	TheCamera.GetArrPosForVehicleType(appearance, index);

	float ExtraOffset = 0.0f;
	int id = CamTargetEntity->GetModelIndex();
	if(id == MI_RCRAIDER || id == MI_RCGOBLIN)
		ExtraOffset = INIT_RC_HELI_ALPHA_EXTRA;
	else if(id == MI_RCBARON)
		ExtraOffset = INIT_RC_PLANE_ALPHA_EXTRA;

	if(ResetStatics){
		AlphaOffset = 0.0f;
		AlphaOffsetSpeed = 0.0f;
		AlphaDec = 0.0f;

		if(TheCamera.CarZoomIndicator == CAM_ZOOM_1)
			AlphaOffset = ZmOneAlphaOffset[index] + ExtraOffset;
		else if(TheCamera.CarZoomIndicator == CAM_ZOOM_2)
			AlphaOffset = ZmTwoAlphaOffset[index] + ExtraOffset;
		else if(TheCamera.CarZoomIndicator == CAM_ZOOM_3)
			AlphaOffset = ZmThreeAlphaOffset[index] + ExtraOffset;
	}

	if(TheCamera.CarZoomIndicator == CAM_ZOOM_1)
		WellBufferMe(ZmOneAlphaOffset[index] + ExtraOffset, &AlphaOffset, &AlphaOffsetSpeed, 0.17f, 0.08f, false);
	else if(TheCamera.CarZoomIndicator == CAM_ZOOM_2)
		WellBufferMe(ZmTwoAlphaOffset[index] + ExtraOffset, &AlphaOffset, &AlphaOffsetSpeed, 0.17f, 0.08f, false);
	else if(TheCamera.CarZoomIndicator == CAM_ZOOM_3)
		WellBufferMe(ZmThreeAlphaOffset[index] + ExtraOffset, &AlphaOffset, &AlphaOffsetSpeed, 0.17f, 0.08f, false);

	float Length = (Source - TargetCoors).Magnitude2D();

	CVector Forward = CamTargetEntity->GetForward();
	float CarAlpha = General_GetATanOfXY(Forward.Magnitude2D(), Forward.z);
	// this shouldn't be necessary....
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&CarAlpha);

	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);

	float DeltaBeta = Beta - TargetOrientation;
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&DeltaBeta);

	float BehindCarNess = Cos(DeltaBeta);	// 1 if behind car, 0 if side, -1 if in front
	CarAlpha = -CarAlpha * BehindCarNess;

	float fwdSpeed = DotProduct(((CPhysical*)CamTargetEntity)->m_vecMoveSpeed, CamTargetEntity->GetForward())*180.0f;
	if(CamTargetEntity->GetModelIndex() == MI_FIRETRUCK && CPad::GetPad(0)->GetCarGunFired()){
		CarAlpha = DEGTORAD(10.0f);
	}else if(isHeli){
		// Delegate the angle behind the helicopter to the C implementation
		CarAlpha = Camera_HeliAlpha(CamTargetEntity->GetForward().Magnitude2D(), CamTargetEntity->GetForward().z, fwdSpeed);

		CColPoint point;
		CEntity *entity = nil;
		CVector Test = Source;
		// Delegate the height of the test point to the C implementation
		Test.z = Camera_HeliTestHeight(TargetCoors.z, Length, CarAlpha, AlphaOffset, m_fCloseInCarHeightOffset);
		if(World_ProcessVerticalLine(Test, CamTargetEntity->GetPosition().z, point, entity, true, false, false, false, false, false, nil)){
			// Delegate the angle above the ground to the C implementation
			CarAlpha = Camera_AngleAboveGround(point.point.z, TargetCoors.z, m_fCloseInCarHeightOffset, Length, AlphaOffset);
			if(CarAlpha < 0.0f)
				AlphaOffset += CarAlpha;
		} 
	}

	// Delegate the limits of the angle to the C implementation
	CarAlpha = Camera_ClampCarAlpha(CarAlpha);

	if(ResetStatics)
		Alpha = CarAlpha;

	float TargetAlpha = Alpha;
	float DeltaAlpha = CarAlpha - TargetAlpha;
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&DeltaAlpha);
	if(Abs(DeltaAlpha) > 0.0f && !TheCamera.m_bVehicleSuspenHigh)
		TargetAlpha = CarAlpha;

	if(isBike)
		WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.09f, 0.04f, true);
	else if(isHeli)
		WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.09f, 0.04f, true);
	else
		WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.15f, 0.07f, true);

	// Delegate the height of the source to the C implementation
	Source.z = Camera_CarSourceHeight(TargetCoors.z, Length, Alpha, AlphaOffset, m_fCloseInCarHeightOffset);
	AlphaOffset -= AlphaDec;
}

// Rotate cam behind the car when the car is moving forward
bool
CCam::RotCamIfInFrontCar(CVector &TargetCoors, float TargetOrientation)
{
	float BetaMaxSpeed = 0.15f;
	float BetaAcceleration = 0.007f;
	bool MovingForward = false;
	float MaxDiffBeta = DEGTORAD(160.0f);
	CPhysical *phys = (CPhysical*)CamTargetEntity;

	float ForwardSpeed = DotProduct(phys->GetForward(), phys->GetSpeed(CVector(0.0f, 0.0f, 0.0f)));
	if(ForwardSpeed > 0.02f)
		MovingForward = true;

	if(phys->IsVehicle() && (phys->GetModelIndex() == MI_SPARROW || phys->GetModelIndex() == MI_HUNTER)){
		MaxDiffBeta = DEGTORAD(160.0f);
		BetaMaxSpeed = 0.1f;
		BetaAcceleration = 0.003f;
		CVector speed = phys->GetSpeed(CVector(0.0f, 0.0f, 0.0f));
		speed.z = 0.0f;
		if(50.0f*speed.Magnitude() > 3.13f)
			TargetOrientation = General_GetATanOfXY(speed.x, speed.y);
	}

	float Dist = (Source - TargetCoors).Magnitude2D();

	float DeltaBeta = TargetOrientation - Beta;
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&DeltaBeta);

	// Delegate the check that the angle needs fixing to the C implementation
	if(Camera_NeedsBetaFix(DeltaBeta, MaxDiffBeta) && MovingForward && TheCamera.m_uiTransitionState == 0)
		m_bFixingBeta = true;

	CPad *pad = CPad::GetPad(0);
	if(!(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight()))
		if(DirectionWasLooking != LOOKING_FORWARD)
			TheCamera.m_bCamDirectlyBehind = true;

	if(!m_bFixingBeta && !TheCamera.m_bUseTransitionBeta && !TheCamera.m_bCamDirectlyBehind && !TheCamera.m_bCamDirectlyInFront)
		return false;

	bool SetBeta = false;
	if(TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront || TheCamera.m_bUseTransitionBeta)
		if(&TheCamera.Cams[TheCamera.ActiveCam] == this)
			SetBeta = true;

	if(m_bFixingBeta || SetBeta){
		WellBufferMe(TargetOrientation, &Beta, &BetaSpeed, BetaMaxSpeed, BetaAcceleration, true);

		if(TheCamera.m_bCamDirectlyBehind && &TheCamera.Cams[TheCamera.ActiveCam] == this)
			Beta = TargetOrientation;
		if(TheCamera.m_bCamDirectlyInFront && &TheCamera.Cams[TheCamera.ActiveCam] == this)
			Beta = TargetOrientation + PI;
		if(TheCamera.m_bUseTransitionBeta && &TheCamera.Cams[TheCamera.ActiveCam] == this)
			Beta = m_fTransitionBeta;

		// Delegate the offset of the camera from its target to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(Dist, Beta, &orbitX, &orbitY);
			Source.x = TargetCoors.x - orbitX;
			Source.y = TargetCoors.y - orbitY;
		}

		// Check if we're done
		DeltaBeta = TargetOrientation - Beta;
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&DeltaBeta);
		// Delegate the check that the angle is fixed to the C implementation
		if(Camera_BetaFixDone(DeltaBeta))
			m_bFixingBeta = false;
	}
	TheCamera.m_bCamDirectlyBehind = false;
	TheCamera.m_bCamDirectlyInFront = false;
	return true;
}

float FIRETRUCK_TRACKING_MULT = 0.1f;
float fTestShiftHeliCamTarget = 0.6f;
float TiltTopSpeed[] = { 0.035f, 0.035f, 0.001f, 0.005f, 0.035f };
float TiltSpeedStep[] = { 0.016f, 0.016f, 0.0002f, 0.0014f, 0.016f };
float TiltOverShoot[] = { 1.05f, 1.05f, 0.0f, 0.0f, 1.0f };

void
CCam::Process_Cam_On_A_String(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	if(!CamTargetEntity->IsVehicle())
		return;

	// unused
	// ((CVehicle*)CamTargetEntity)->GetVehicleAppearance();

	FOV = DefaultFOV;

	if(ResetStatics){
		AlphaSpeed = 0.0f;
		m_fTilt = 0.0f;
		m_fTiltSpeed = 0.0;
	}

	CBaseModelInfo *mi = ((CBaseModelInfo*)ModelRegistry_Get(CamTargetEntity->GetModelIndex())); // rouz edit (ChatGPT)
	CVector Dimensions = CollisionVector_ToNative(mi->GetColModel()->boundingBox.max) - CollisionVector_ToNative(mi->GetColModel()->boundingBox.min);
	CVector TargetCoors = CameraTarget;
	float BaseDist = Dimensions.Magnitude();

	if(((CVehicle*)CamTargetEntity)->IsBike())
		BaseDist *= 1.45f;
	if(((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI &&
	   CamTargetEntity->GetStatus() != STATUS_PLAYER_REMOTE)
		TargetCoors += fTestShiftHeliCamTarget * CamTargetEntity->GetUp() * Dimensions.z;
	else
		TargetCoors.z += 0.8f*Dimensions.z;

	Beta = General_GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y);
	Alpha = General_LimitRadianAngle(Alpha);
	Beta = General_LimitRadianAngle(Beta);

	if(CamTargetEntity->GetModelIndex() == MI_FIRETRUCK && CPad::GetPad(0)->GetCarGunFired() &&
	   ((CVehicle*)CamTargetEntity)->m_vecMoveSpeed.Magnitude2D() < 0.01f){
		float TargetBeta = CamTargetEntity->GetForward().Heading() - ((CAutomobile*)CamTargetEntity)->m_fCarGunLR + HALFPI;
		TargetBeta = General_LimitRadianAngle(TargetBeta);
		// Delegate the sideways movement to the C implementation
		float dist = Camera_FiretruckTracking(TargetBeta, Beta, (TargetCoors - Source).Magnitude(), FIRETRUCK_TRACKING_MULT);
		Source += dist*CrossProduct(Front, CVector(0.0f, 0.0f, 1.0f));
	}

	m_fDistanceBeforeChanges = (Source - TargetCoors).Magnitude2D();

	Cam_On_A_String_Unobscured(TargetCoors, BaseDist);
	WorkOutCamHeight(TargetCoors, TargetOrientation, Dimensions.z);
	RotCamIfInFrontCar(TargetCoors, TargetOrientation);
	FixCamWhenObscuredByVehicle(TargetCoors);

	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	CVector OrigSource = Source;
	if(World_GetIsLineOfSightClear(CamTargetEntity->GetPosition(), m_cvecTargetCoorsForFudgeInter, true, false, false, true, false, false, true))
		TheCamera.AvoidTheGeometry(OrigSource, m_cvecTargetCoorsForFudgeInter, Source, FOV);
	else
		TheCamera.AvoidTheGeometry(OrigSource, CamTargetEntity->GetPosition(), Source, FOV);

	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);

	int appearance = ((CVehicle*)CamTargetEntity)->GetVehicleAppearance();
	int index = 0;
	TheCamera.GetArrPosForVehicleType(appearance, index);

	if(appearance == VEHICLE_APPEARANCE_HELI){
		float TargetTilt = DotProduct(Front, ((CVehicle*)CamTargetEntity)->GetSpeed(CVector(0.0f, 0.0f, 0.0f)));
		CVector UpTarget = CamTargetEntity->GetUp();
		UpTarget.Normalise();
		int dir = TargetTilt < 0.0f ? -1 : 1;
		if(m_fTilt != 0.0f)
			TargetTilt += TiltOverShoot[index]*TargetTilt/m_fTilt * dir;
		WellBufferMe(TargetTilt, &m_fTilt, &m_fTiltSpeed, TiltTopSpeed[index], TiltSpeedStep[index], false);

		Up = CVector(0.0f, 0.0f, 1.0f) - (CVector(0.0f, 0.0f, 1.0f) - UpTarget)*m_fTilt;
		Up.Normalise();
		Front.Normalise();
		// Delegate the up vector of the camera to the C implementation
		{
			CVector newUpC;
			Camera_PerpendicularUp(&Front, &Up, false, &newUpC);
			Up = CVector(newUpC.x, newUpC.y, newUpC.z);
		}
	}else{
		// Delegate the roll to the C implementation
		float TargetRoll;
		{
			CVector FwdTarget = CamTargetEntity->GetForward();
			FwdTarget.Normalise();
			TargetRoll = Camera_CarTargetRoll(CPad::GetPad(0)->GetDPadLeft() || CPad::GetPad(0)->GetDPadRight(), CPad::GetPad(0)->GetDPadLeft(),
				DotProduct(((CVehicle*)CamTargetEntity)->m_vecMoveSpeed, CamTargetEntity->GetForward()), CPad::GetPad(0)->GetLeftStickX(),
				TiltOverShoot[index], f_max_role_angle, DotProduct(FwdTarget, Front));
		}

		WellBufferMe(TargetRoll, &f_Roll, &f_rollSpeed, 0.15f, 0.07f, false);
		// Delegate the up vector to the C implementation
		{
			Camera_UpFromRoll(f_Roll, &Up);
		}
		Up.Normalise();
		Front.Normalise();
		// Delegate the up vector of the camera to the C implementation
		{
			CVector newUpC;
			Camera_PerpendicularUp(&Front, &Up, true, &newUpC);
			Up = CVector(newUpC.x, newUpC.y, newUpC.z);
		}
	}

	ResetStatics = false;
}

// Basic Cam on a string algorithm
void
CCam::Cam_On_A_String_Unobscured(const CVector &TargetCoors, float BaseDist)
{
	int id = CamTargetEntity->GetModelIndex();
	float ExtraDist = 0.0f;
	if(id == MI_RCRAIDER || id == MI_RCGOBLIN)
		ExtraDist = INIT_RC_HELI_HORI_EXTRA;
	else if(id == MI_RCBARON)
		ExtraDist = INIT_RC_PLANE_HORI_EXTRA;

	CA_MAX_DISTANCE = BaseDist + 0.1f + TheCamera.CarZoomValueSmooth + ExtraDist;
	// Delegate the minimum distance to the C implementation
	CA_MIN_DISTANCE = Camera_MinCarDistance(BaseDist);
	if(CA_MIN_DISTANCE > CA_MAX_DISTANCE)
		CA_MIN_DISTANCE = CA_MAX_DISTANCE - 0.05f;

	CVector Dist = Source - TargetCoors;

	if(ResetStatics)
		Source = TargetCoors + Dist*(CA_MAX_DISTANCE + 1.0f);

	Dist = Source - TargetCoors;

	float Length = Dist.Magnitude2D();
	if(Length < 0.001f){
		// This probably shouldn't happen. reset view
		// Delegate the horizontal direction to the C implementation
		CVector Forward = NormalisedHorizontal(CamTargetEntity->GetForward());
		Source = TargetCoors - Forward*CA_MAX_DISTANCE;
		Dist = Source - TargetCoors;
		Length = Dist.Magnitude2D();
	}

	if(Length > CA_MAX_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MAX_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MAX_DISTANCE;
	}else if(Length < CA_MIN_DISTANCE){
		Source.x = TargetCoors.x + Dist.x/Length * CA_MIN_DISTANCE;
		Source.y = TargetCoors.y + Dist.y/Length * CA_MIN_DISTANCE;
	}
}

void
CCam::FixCamWhenObscuredByVehicle(const CVector &TargetCoors)
{
	// BUG? is this never reset
	static float HeightFixerCarsObscuring = 0.0f;
	static float HeightFixerCarsObscuringSpeed = 0.0f;
	CColPoint colPoint;
	CEntity *entity = nil;

	float HeightTarget = 0.0f;
	if(World_ProcessLineOfSight(TargetCoors, Source, colPoint, entity, false, true, false, false, false, false, false)){
		CBaseModelInfo *mi = ((CBaseModelInfo*)ModelRegistry_Get(entity->GetModelIndex())); // rouz edit (ChatGPT)
		HeightTarget = mi->GetColModel()->boundingBox.max.z + 1.0f + TargetCoors.z - Source.z;
		if(HeightTarget < 0.0f)
			HeightTarget = 0.0f;
	}
	WellBufferMe(HeightTarget, &HeightFixerCarsObscuring, &HeightFixerCarsObscuringSpeed, 0.2f, 0.025f, false);
	Source.z += HeightFixerCarsObscuring;
}

void
CCam::Process_TopDown(const CVector &CameraTarget, float TargetOrientation, float SpeedVar, float TargetSpeedVar)
{
	FOV = DefaultFOV;

	if(!CamTargetEntity->IsVehicle())
		return;

	float Dist;
	float HeightTarget = 0.0f;
	static float AdjustHeightTargetMoveBuffer = 0.0f;
	static float AdjustHeightTargetMoveSpeed = 0.0f;
	static float NearClipDistance = 1.5f;
	const float FarClipDistance = 200.0f;
	CVector TargetFront, Target;
	CVector TestSource, TestTarget;
	CColPoint colPoint;
	CEntity *entity;

	TargetFront = CameraTarget;
	TargetFront.x += 18.0f*CamTargetEntity->GetForward().x*SpeedVar;
	TargetFront.y += 18.0f*CamTargetEntity->GetForward().y*SpeedVar;

	if(ResetStatics){
		AdjustHeightTargetMoveBuffer = 0.0f;
		AdjustHeightTargetMoveSpeed = 0.0f;
	}

	float f = Pow(0.8f, 4.0f);
	Target = f*CameraTarget + (1.0f-f)*TargetFront;
	if(Mode == MODE_GTACLASSIC)
		SpeedVar = TargetSpeedVar;
	Source = Target + CVector(0.0f, 0.0f, (40.0f*SpeedVar + 30.0f)*0.8f);
	// What is this? looks horrible
	if(Mode == MODE_GTACLASSIC)
		Source.x += (uint8)(100.0f*CameraTarget.x)/500.0f;

	TestSource = Source;
	TestTarget = TestSource;
	TestTarget.z = Target.z;
	if(World_ProcessLineOfSight(TestTarget, TestSource, colPoint, entity, true, false, false, false, false, false, false)){
		if(Source.z < colPoint.point.z+3.0f)
			HeightTarget = colPoint.point.z+3.0f - Source.z;
	}else{
		TestSource = Source;
		TestTarget = TestSource;
		TestTarget.z += 10.0f;
		if(World_ProcessLineOfSight(TestTarget, TestSource, colPoint, entity, true, false, false, false, false, false, false))
			if(Source.z < colPoint.point.z+3.0f)
				HeightTarget = colPoint.point.z+3.0f - Source.z;
	}
	WellBufferMe(HeightTarget, &AdjustHeightTargetMoveBuffer, &AdjustHeightTargetMoveSpeed, 0.2f, 0.02f, false);
	Source.z += AdjustHeightTargetMoveBuffer;

	if(RwCameraGetFarClipPlane(Scene.camera) > FarClipDistance)
		RwCameraSetFarClipPlane(Scene.camera, FarClipDistance);
	RwCameraSetNearClipPlane(Scene.camera, NearClipDistance);

	Front = CVector(-0.01f, -0.01f, -1.0f);	// look down
	Front.Normalise();
	Dist = (Source - CameraTarget).Magnitude();
	m_cvecTargetCoorsForFudgeInter = Dist*Front + Source;
	Up = CVector(0.0f, 1.0f, 0.0f);

	ResetStatics = false;
}

void
CCam::AvoidWallsTopDownPed(const CVector &TargetCoors, const CVector &Offset, float *Adjuster, float *AdjusterSpeed, float yDistLimit)
{
	float Target = 0.0f;
	float MaxSpeed = 0.13f;
	float Acceleration = 0.015f;
	float dy;
	CVector TestPoint2;
	CVector TestPoint1;
	CColPoint colPoint;
	CEntity *entity;

	TestPoint2 = TargetCoors + Offset;
	TestPoint1 = TargetCoors;
	TestPoint1.z = TestPoint2.z;
	if(World_ProcessLineOfSight(TestPoint1, TestPoint2, colPoint, entity, true, false, false, false, false, false, false)){
		// What is this even?
		dy = TestPoint1.y - colPoint.point.y;
		// Delegate to the C implementation
		CameraMath_AvoidWallsHit(dy, yDistLimit, &Target, &MaxSpeed, &Acceleration);
	}
	WellBufferMe(Target, Adjuster, AdjusterSpeed, MaxSpeed, Acceleration, false);
}

void
CCam::Process_TopDownPed(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	if(!CamTargetEntity->IsPed())
		return;

	float Dist;
	float HeightTarget;
	static int NumPedPosCountsSoFar = 0;
	static float PedAverageSpeed = 0.0f;
	static float AdjustHeightTargetMoveBuffer = 0.0f;
	static float AdjustHeightTargetMoveSpeed = 0.0f;
	static float PedSpeedSoFar = 0.0f;
	static float FarClipDistance = 200.0f;
	static float NearClipDistance = 1.5f;
	static float TargetAdjusterForSouth = 0.0f;
	static float TargetAdjusterSpeedForSouth = 0.0f;
	static float TargetAdjusterForNorth = 0.0f;
	static float TargetAdjusterSpeedForNorth = 0.0f;
	static float TargetAdjusterForEast = 0.0f;
	static float TargetAdjusterSpeedForEast = 0.0f;
	static float TargetAdjusterForWest = 0.0f;
	static float TargetAdjusterSpeedForWest = 0.0f;
	static CVector PreviousPlayerMoveSpeedVec;
	CVector TargetCoors, PlayerMoveSpeed;
	CVector TestSource, TestTarget;
	CColPoint colPoint;
	CEntity *entity;

	FOV = DefaultFOV;
	TargetCoors = CameraTarget;
	PlayerMoveSpeed = ((CPed*)CamTargetEntity)->GetMoveSpeed();

	if(ResetStatics){
		PreviousPlayerMoveSpeedVec = PlayerMoveSpeed;
		AdjustHeightTargetMoveBuffer = 0.0f;
		AdjustHeightTargetMoveSpeed = 0.0f;
		NumPedPosCountsSoFar = 0;
		PedSpeedSoFar = 0.0f;
		PedAverageSpeed = 0.0f;
		TargetAdjusterForWest = 0.0f;
		TargetAdjusterSpeedForWest = 0.0f;
		TargetAdjusterForEast = 0.0f;
		TargetAdjusterSpeedForEast = 0.0f;
		TargetAdjusterForNorth = 0.0f;
		TargetAdjusterSpeedForNorth = 0.0f;
		TargetAdjusterForSouth = 0.0f;
		TargetAdjusterSpeedForSouth = 0.0f;
	}

	if(RwCameraGetFarClipPlane(Scene.camera) > FarClipDistance)
		RwCameraSetFarClipPlane(Scene.camera, FarClipDistance);
	RwCameraSetNearClipPlane(Scene.camera, NearClipDistance);

	// Average ped speed
	NumPedPosCountsSoFar++;
	PedSpeedSoFar += PlayerMoveSpeed.Magnitude();
	if(NumPedPosCountsSoFar == 5){
		// Delegate the average speed to the C implementation
		PedAverageSpeed = Camera_PedAverageSpeed(PedAverageSpeed, PedSpeedSoFar);
		NumPedPosCountsSoFar = 0;
		PedSpeedSoFar = 0.0f;
	}
	PreviousPlayerMoveSpeedVec = PlayerMoveSpeed;

	// Zoom out depending on speed
	// Delegate the height to the C implementation
	HeightTarget = Camera_TopDownHeightTarget(PedAverageSpeed);

	// Zoom out if locked on target is far away
	if(FindPlayerPed()->m_pPointGunAt){
		Dist = (FindPlayerPed()->m_pPointGunAt->GetPosition() - CameraTarget).Magnitude2D();
		if(Dist > 6.0f)
			HeightTarget = Max(HeightTarget, Dist/22.0f*37.0f);
	}

	Source = TargetCoors + CVector(0.0f, -1.0f, 9.0f);

	// Collision checks
	entity = nil;
	TestSource = TargetCoors + CVector(0.0f, -1.0f, 9.0f);
	TestTarget = TestSource;
	TestTarget.z = TargetCoors.z;
	if(World_ProcessLineOfSight(TestTarget, TestSource, colPoint, entity, true, false, false, false, false, false, false)){
		if(TargetCoors.z+9.0f+HeightTarget < colPoint.point.z+3.0f)
			HeightTarget = colPoint.point.z+3.0f - (TargetCoors.z+9.0f);
	}else{
		TestSource = TargetCoors + CVector(0.0f, -1.0f, 9.0f);
		TestTarget = TestSource;
		TestSource.z += HeightTarget;
		TestTarget.z = TestSource.z + 10.0f;
		if(World_ProcessLineOfSight(TestTarget, TestSource, colPoint, entity, true, false, false, false, false, false, false)){
			if(TargetCoors.z+9.0f+HeightTarget < colPoint.point.z+3.0f)
				HeightTarget = colPoint.point.z+3.0f - (TargetCoors.z+9.0f);
		}
	}

	WellBufferMe(HeightTarget, &AdjustHeightTargetMoveBuffer, &AdjustHeightTargetMoveSpeed, 0.3f, 0.03f, false);
	Source.z += AdjustHeightTargetMoveBuffer;

	// Wall checks
	AvoidWallsTopDownPed(TargetCoors, CVector(0.0f, -3.0f, 3.0f), &TargetAdjusterForSouth, &TargetAdjusterSpeedForSouth, 1.0f);
	Source.y += TargetAdjusterForSouth;
	AvoidWallsTopDownPed(TargetCoors, CVector(0.0f, 3.0f, 3.0f), &TargetAdjusterForNorth, &TargetAdjusterSpeedForNorth, 1.0f);
	Source.y -= TargetAdjusterForNorth;
	// BUG: east and west flipped
	AvoidWallsTopDownPed(TargetCoors, CVector(3.0f, 0.0f, 3.0f), &TargetAdjusterForWest, &TargetAdjusterSpeedForWest, 1.0f);
	Source.x -= TargetAdjusterForWest;
	AvoidWallsTopDownPed(TargetCoors, CVector(-3.0f, 0.0f, 3.0f), &TargetAdjusterForEast, &TargetAdjusterSpeedForEast, 1.0f);
	Source.x += TargetAdjusterForEast;

	TargetCoors.y = Source.y + 1.0f;
	TargetCoors.y += TargetAdjusterForSouth;
	TargetCoors.x += TargetAdjusterForEast;
	TargetCoors.x -= TargetAdjusterForWest;

	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
#ifdef FIX_BUGS
	if(Front.x == 0.0f && Front.y == 0.0f)
		Front.y = 0.0001f;
#else
	// someone used = instead of == in the above check by accident
	Front.x = 0.0f;
#endif
	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	Up = CrossProduct(Front, CVector(-1.0f, 0.0f, 0.0f));
	Up.Normalise();

	ResetStatics = false;
}

void
CCam::Process_Rocket(const CVector &CameraTarget, float, float, float)
{
	if(!CamTargetEntity->IsPed())
		return;

	float BackOffset = 0.19f;
	static bool FailedTestTwelveFramesAgo = false;
	RwV3d HeadPos;
	CVector TargetCoors;

	FOV = DefaultFOV;
	TargetCoors = CameraTarget;

	if(ResetStatics){
		Beta = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
		Alpha = 0.0f;
		m_fInitialPlayerOrientation = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
		FailedTestTwelveFramesAgo = false;
		// static DPadVertical unused
		// static DPadHorizontal unused
		m_bCollisionChecksOn = true;
		ResetStatics = false;
	}

	if(((CPed*)CamTargetEntity)->bIsDucking)
		BackOffset = 0.8f;
	CamTargetEntity->GetMatrix().UpdateRW();
	CamTargetEntity->UpdateRwFrame();
	CamTargetEntity->UpdateRpHAnim();
	((CPed*)CamTargetEntity)->m_pedIK.GetComponentPosition(HeadPos, PED_HEAD);
	Source = HeadPos;
	Source.z += 0.1f;
	// Delegate the offset on the orbit to the C implementation
	{
		float orbitX, orbitY;
		Camera_OrbitOffset(BackOffset, m_fInitialPlayerOrientation, &orbitX, &orbitY);
		Source.x -= orbitX;
		Source.y -= orbitY;
	}

	// Look around
	bool UseMouse = false;
	float MouseX = Input_GetMouseX(); // rouz edit (ChatGPT)
	float MouseY = Input_GetMouseY(); // rouz edit (ChatGPT)
	float LookLeftRight, LookUpDown;
	if(MouseX != 0.0f || MouseY != 0.0f){
		UseMouse = true;
		LookLeftRight = -3.0f*MouseX;
		LookUpDown = 4.0f*MouseY;
	}else{
		LookLeftRight = -CPad::GetPad(0)->SniperModeLookLeftRight();
		LookUpDown = CPad::GetPad(0)->SniperModeLookUpDown();
	}
	if(UseMouse){
		// Delegate the change of the angles to the C implementation
		Beta += Camera_MouseLookDelta(TheCamera.m_fMouseAccelHorzntl, LookLeftRight, FOV);
		Alpha += Camera_MouseLookDelta(TheCamera.m_fMouseAccelVertical, LookUpDown, FOV);
	}else{
		float xdir = LookLeftRight < 0.0f ? -1.0f : 1.0f;
		float ydir = LookUpDown < 0.0f ? -1.0f : 1.0f;
		// Delegate the change of the angles to the C implementation
		Beta += Camera_PadLookDelta(LookLeftRight, 100.0f, xdir, 0.8f, FOV, (*Timer_GetTimeStep()));
		Alpha += Camera_PadLookDelta(LookUpDown, 150.0f, ydir, 1.0f, FOV, (*Timer_GetTimeStep()));
	}
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);
	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 60.0f);

	// Delegate the point in the direction of the look to the C implementation
	{
		CVector lookPoint;
		Camera_LookPoint(Alpha, Beta, Source.x, Source.y, Source.z, &lookPoint);
		TargetCoors.x = lookPoint.x;
		TargetCoors.y = lookPoint.y;
		TargetCoors.z = lookPoint.z;
	}
	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
	Source += Front*0.4f;

	if(m_bCollisionChecksOn){
		if(!World_GetIsLineOfSightClear(TargetCoors, Source, true, true, false, true, false, true, true)){
			RwCameraSetNearClipPlane(Scene.camera, 0.4f);
			FailedTestTwelveFramesAgo = true;
		}else{
			CVector TestPoint;
			// Delegate the point in the direction of the look to the C implementation
			{
				CVector lookPoint;
				Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta + DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
				TestPoint.x = lookPoint.x;
				TestPoint.y = lookPoint.y;
				TestPoint.z = lookPoint.z;
			}
			if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
				RwCameraSetNearClipPlane(Scene.camera, 0.4f);
				FailedTestTwelveFramesAgo = true;
			}else{
				// Delegate the point in the direction of the look to the C implementation
				{
					CVector lookPoint;
					Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta - DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
					TestPoint.x = lookPoint.x;
					TestPoint.y = lookPoint.y;
					TestPoint.z = lookPoint.z;
				}
				if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
					RwCameraSetNearClipPlane(Scene.camera, 0.4f);
					FailedTestTwelveFramesAgo = true;
				}else
					FailedTestTwelveFramesAgo = false;
			}
		}
	}

	if(FailedTestTwelveFramesAgo)
		RwCameraSetNearClipPlane(Scene.camera, 0.4f);
	Source -= Front*0.4f;

	GetVectorsReadyForRW();
	float Rotation = General_GetATanOfXY(Front.x, Front.y) - HALFPI;
	((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Rotation;
	((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Rotation;
}

float fDuckingBackOffset = 0.5f;
float fDuckingRightOffset = 0.18f;

void
CCam::Process_M16_1stPerson(const CVector &CameraTarget, float, float, float)
{
	if(!CamTargetEntity->IsPed())
		return;

	float BackOffset = 0.3f;
	static bool FailedTestTwelveFramesAgo = false;
	RwV3d HeadPos;
	CVector TargetCoors;

	bool isAttached = ((CPed*)CamTargetEntity)->IsPlayer() && ((CPed*)CamTargetEntity)->m_attachedTo;

	FOV = DefaultFOV;
	TargetCoors = CameraTarget;

	if(ResetStatics){
		if(isAttached)
			Beta = 0.0f;
		else
			Beta = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
		Alpha = 0.0f;
		m_fInitialPlayerOrientation = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
		FailedTestTwelveFramesAgo = false;
		// static DPadVertical unused
		// static DPadHorizontal unused
		m_bCollisionChecksOn = true;
		ResetStatics = false;
	}

	// Look around
	bool UseMouse = false;
	float MouseX = Input_GetMouseX(); // rouz edit (ChatGPT)
	float MouseY = Input_GetMouseY(); // rouz edit (ChatGPT)
	float LookLeftRight, LookUpDown;
	if(MouseX != 0.0f || MouseY != 0.0f){
		UseMouse = true;
		LookLeftRight = -3.0f*MouseX;
		LookUpDown = 4.0f*MouseY;
	}else{
		LookLeftRight = -CPad::GetPad(0)->SniperModeLookLeftRight();
		LookUpDown = CPad::GetPad(0)->SniperModeLookUpDown();
	}
	if(UseMouse){
		// Delegate the change of the angles to the C implementation
		Beta += Camera_MouseLookDelta(TheCamera.m_fMouseAccelHorzntl, LookLeftRight, FOV);
		Alpha += Camera_MouseLookDelta(TheCamera.m_fMouseAccelVertical, LookUpDown, FOV);
	}else if(Mode == MODE_HELICANNON_1STPERSON){
		LookLeftRight /= 128.0f;
		LookUpDown /= 128.0f;
		// Delegate the change of the angles to the C implementation
		Beta += Camera_HeliCannonLookDelta(LookLeftRight, 0.56f, FOV, (*Timer_GetTimeStep()));
		Alpha += Camera_HeliCannonLookDelta(LookUpDown, 0.48f, FOV, (*Timer_GetTimeStep()));
	}else{
		float xdir = LookLeftRight < 0.0f ? -1.0f : 1.0f;
		float ydir = LookUpDown < 0.0f ? -1.0f : 1.0f;
		// Delegate the change of the angles to the C implementation
		Beta += Camera_PadLookDelta(LookLeftRight, 100.0f, xdir, 0.8f, FOV, (*Timer_GetTimeStep()));
		Alpha += Camera_PadLookDelta(LookUpDown, 150.0f, ydir, 1.0f, FOV, (*Timer_GetTimeStep()));
	}
	if (!isAttached) {
		// Delegate the angle wrapping to the C implementation
		Beta = General_ReduceAtLeast(Beta, TWOPI, TWOPI);
		Beta = General_RaiseBelow(Beta, 0.0f, TWOPI);
	}
	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 60.0f);

	if(((CPed*)CamTargetEntity)->bIsDucking)
		BackOffset = 0.8f;
	if(isAttached){
		CMatrix mat, rot;
		CPed *TargetPed = (CPed*)CamTargetEntity;
		TargetPed->PositionAttachedPed();
		CamTargetEntity->GetMatrix().UpdateRW();
		CamTargetEntity->UpdateRwFrame();
		CamTargetEntity->UpdateRpHAnim();

		HeadPos.x = 0.0f;
		HeadPos.y = 0.0f;
		HeadPos.z = 0.0f;
		TargetPed->m_pedIK.GetComponentPosition(HeadPos, PED_HEAD);
		Source = HeadPos;
		Source += 0.1f*CamTargetEntity->GetUp();
		Source -= BackOffset*CamTargetEntity->GetForward();

		if(TargetPed->m_attachRotStep < PI){
			if(Beta > TargetPed->m_attachRotStep){
				Beta = TargetPed->m_attachRotStep;
				CAutomobile *heli = (CAutomobile*)TargetPed->m_attachedTo;
				if(heli->IsVehicle() && heli->IsCar() && heli->IsRealHeli() && heli->m_fHeliOrientation > 0.0f){
					float heliOrient = heli->m_fHeliOrientation + (*Timer_GetTimeStep())*0.01f;
					if(heliOrient < 0.0f) heliOrient += TWOPI;
					else if(heliOrient > TWOPI) heliOrient -= TWOPI;
					heli->SetHeliOrientation(heliOrient);
				}
			}else if(Beta < -TargetPed->m_attachRotStep){
				Beta = -TargetPed->m_attachRotStep;
				CAutomobile *heli = (CAutomobile*)TargetPed->m_attachedTo;
				if(heli->IsVehicle() && heli->IsCar() && heli->IsRealHeli() && heli->m_fHeliOrientation > 0.0f){
					float heliOrient = heli->m_fHeliOrientation - (*Timer_GetTimeStep())*0.01f;
					if(heliOrient < 0.0f) heliOrient += TWOPI;
					else if(heliOrient > TWOPI) heliOrient -= TWOPI;
					heli->SetHeliOrientation(heliOrient);
				}
			}
		}else{
			// Delegate the angle wrapping to the C implementation
			Beta = General_RaiseBelow(Beta, -PI, TWOPI);
			Beta = General_ReduceAtLeast(Beta, PI, TWOPI);
		}

		mat = TargetPed->m_attachedTo->GetMatrix();
		rot.SetRotateX(Alpha);
		switch(TargetPed->m_attachType){
		case 0: rot.RotateZ(Beta); break;
		case 1: rot.RotateZ(Beta + HALFPI); break;
		case 2: rot.RotateZ(Beta + PI); break;
		case 3: rot.RotateZ(Beta - HALFPI); break;
		}
		mat = mat * rot;
		Front = mat.GetForward();
		Up = mat.GetUp();
		TargetCoors = Source + 3.0f*Front;
		RwCameraSetNearClipPlane(Scene.camera, 0.4f);

		float Rotation = General_GetATanOfXY(Front.x, Front.y) - HALFPI;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Rotation;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Rotation;
	}else{
		CamTargetEntity->GetMatrix().UpdateRW();
		CamTargetEntity->UpdateRwFrame();
		CamTargetEntity->UpdateRpHAnim();
		HeadPos.x = 0.0f;
		HeadPos.y = 0.0f;
		HeadPos.z = 0.0f;
		((CPed*)CamTargetEntity)->m_pedIK.GetComponentPosition(HeadPos, PED_HEAD);
		Source = HeadPos;
		Source.z += 0.1f;
		if(((CPed*)CamTargetEntity)->bIsDucking){
			Source.x -= fDuckingBackOffset*CamTargetEntity->GetForward().x;
			Source.y -= fDuckingBackOffset*CamTargetEntity->GetForward().y;
			Source.x -= fDuckingRightOffset*CamTargetEntity->GetRight().x;
			Source.y -= fDuckingRightOffset*CamTargetEntity->GetRight().y;
		}else{
			Source.x -= BackOffset*CamTargetEntity->GetForward().x;
			Source.y -= BackOffset*CamTargetEntity->GetForward().y;
		}

		// Delegate the point in the direction of the look to the C implementation
		{
			CVector lookPoint;
			Camera_LookPoint(Alpha, Beta, Source.x, Source.y, Source.z, &lookPoint);
			TargetCoors.x = lookPoint.x;
			TargetCoors.y = lookPoint.y;
			TargetCoors.z = lookPoint.z;
		}
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(TargetCoors, Source);
		Source += Front*0.4f;

		if(m_bCollisionChecksOn){
			if(!World_GetIsLineOfSightClear(TargetCoors, Source, true, true, false, true, false, true, true)){
				RwCameraSetNearClipPlane(Scene.camera, 0.4f);
				FailedTestTwelveFramesAgo = true;
			}else{
				CVector TestPoint;
				// Delegate the point in the direction of the look to the C implementation
				{
					CVector lookPoint;
					Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta + DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
					TestPoint.x = lookPoint.x;
					TestPoint.y = lookPoint.y;
					TestPoint.z = lookPoint.z;
				}
				if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
					RwCameraSetNearClipPlane(Scene.camera, 0.4f);
					FailedTestTwelveFramesAgo = true;
				}else{
					// Delegate the point in the direction of the look to the C implementation
					{
						CVector lookPoint;
						Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta - DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
						TestPoint.x = lookPoint.x;
						TestPoint.y = lookPoint.y;
						TestPoint.z = lookPoint.z;
					}
					if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
						RwCameraSetNearClipPlane(Scene.camera, 0.4f);
						FailedTestTwelveFramesAgo = true;
					}else
						FailedTestTwelveFramesAgo = false;
				}
			}
		}

		if(FailedTestTwelveFramesAgo)
			RwCameraSetNearClipPlane(Scene.camera, 0.4f);
		Source -= Front*0.4f;

		GetVectorsReadyForRW();
		float Rotation = General_GetATanOfXY(Front.x, Front.y) - HALFPI;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Rotation;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Rotation;
	}
}

float fBike1stPersonOffsetZ = 0.15f;

void
CCam::Process_1stPerson(const CVector &CameraTarget, float TargetOrientation, float SpeedVar, float TargetSpeedVar)
{
	float BackOffset = 0.3f;
	static float DontLookThroughWorldFixer = 0.0f;
	CVector TargetCoors;

	FOV = DefaultFOV;
	TargetCoors = CameraTarget;
	if(CamTargetEntity->m_rwObject == nil)
		return;

	if(ResetStatics){
		Beta = TargetOrientation;
		Alpha = 0.0f;
		m_fInitialPlayerOrientation = TargetOrientation;
		if(CamTargetEntity->IsPed()){
			Beta = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
			Alpha = 0.0f;
			m_fInitialPlayerOrientation = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
		}
		TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
		DontLookThroughWorldFixer = 0.0f;
	}

	if(CamTargetEntity->IsPed()){
		static bool FailedTestTwelveFramesAgo = false;
		RwV3d HeadPos;

		TargetCoors = CameraTarget;

		if(ResetStatics){
			Beta = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
			Alpha = 0.0f;
			m_fInitialPlayerOrientation = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
			FailedTestTwelveFramesAgo = false;
			// static DPadVertical unused
			// static DPadHorizontal unused
			m_bCollisionChecksOn = true;
			ResetStatics = false;
		}

		CamTargetEntity->GetMatrix().UpdateRW();
		CamTargetEntity->UpdateRwFrame();
		CamTargetEntity->UpdateRpHAnim();

		((CPed*)CamTargetEntity)->m_pedIK.GetComponentPosition(HeadPos, PED_HEAD);
		Source = HeadPos;
		Source.z += 0.1f;
		if(((CPed*)CamTargetEntity)->bIsDucking){
			Source.x -= fDuckingBackOffset*CamTargetEntity->GetForward().x;
			Source.y -= fDuckingBackOffset*CamTargetEntity->GetForward().y;
			Source.x -= fDuckingRightOffset*CamTargetEntity->GetRight().x;
			Source.y -= fDuckingRightOffset*CamTargetEntity->GetRight().y;
		}else{
			Source.x -= BackOffset*CamTargetEntity->GetForward().x;
			Source.y -= BackOffset*CamTargetEntity->GetForward().y;
		}

		float LookLeftRight, LookUpDown;
		LookLeftRight = -CPad::GetPad(0)->LookAroundLeftRight();
		LookUpDown = CPad::GetPad(0)->LookAroundUpDown();
		float xdir = LookLeftRight < 0.0f ? -1.0f : 1.0f;
		float ydir = LookUpDown < 0.0f ? -1.0f : 1.0f;
		// Delegate the change of the angles to the C implementation
		Beta += Camera_PadLookDelta(LookLeftRight, 100.0f, xdir, 0.8f, FOV, (*Timer_GetTimeStep()));
		Alpha += Camera_PadLookDelta(LookUpDown, 150.0f, ydir, 1.0f, FOV, (*Timer_GetTimeStep()));
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&Beta);
		// Delegate the limits of the angle to the C implementation
		Alpha = Camera_ClampAlpha(Alpha, 60.0f);

		// Delegate the point in the direction of the look to the C implementation
		{
			CVector lookPoint;
			Camera_LookPoint(Alpha, Beta, Source.x, Source.y, Source.z, &lookPoint);
			TargetCoors.x = lookPoint.x;
			TargetCoors.y = lookPoint.y;
			TargetCoors.z = lookPoint.z;
		}
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(TargetCoors, Source);
		Source += Front*0.4f;

		if(m_bCollisionChecksOn){
			if(!World_GetIsLineOfSightClear(TargetCoors, Source, true, true, false, true, false, true, true)){
				RwCameraSetNearClipPlane(Scene.camera, 0.4f);
				FailedTestTwelveFramesAgo = true;
			}else{
				CVector TestPoint;
				// Delegate the point in the direction of the look to the C implementation
				{
					CVector lookPoint;
					Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta + DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
					TestPoint.x = lookPoint.x;
					TestPoint.y = lookPoint.y;
					TestPoint.z = lookPoint.z;
				}
				if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
					RwCameraSetNearClipPlane(Scene.camera, 0.4f);
					FailedTestTwelveFramesAgo = true;
				}else{
					// Delegate the point in the direction of the look to the C implementation
					{
						CVector lookPoint;
						Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta - DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
						TestPoint.x = lookPoint.x;
						TestPoint.y = lookPoint.y;
						TestPoint.z = lookPoint.z;
					}
					if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
						RwCameraSetNearClipPlane(Scene.camera, 0.4f);
						FailedTestTwelveFramesAgo = true;
					}else
						FailedTestTwelveFramesAgo = false;
				}
			}
		}

		if(FailedTestTwelveFramesAgo)
			RwCameraSetNearClipPlane(Scene.camera, 0.4f);
		Source -= Front*0.4f;

		GetVectorsReadyForRW();
		float Rotation = General_GetATanOfXY(Front.x, Front.y) - HALFPI;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Rotation;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Rotation;
	}else{
		assert(CamTargetEntity->IsVehicle());

		if(((CVehicle*)CamTargetEntity)->IsBike() &&
		   (((CBike*)CamTargetEntity)->bWheelieCam || TheCamera.m_fAvoidTheGeometryProbsTimer > 0.0f)){
			if(CPad::GetPad(0)->GetLeftShoulder2() || CPad::GetPad(0)->GetRightShoulder2()){
				TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
				((CBike*)CamTargetEntity)->bWheelieCam = false;
			}else if(Process_WheelCam(CameraTarget, TargetOrientation, SpeedVar, TargetSpeedVar)){
				if(((CBike*)CamTargetEntity)->bWheelieCam)
					TheCamera.m_fAvoidTheGeometryProbsTimer = 50.0f;
				else{
					TheCamera.m_fAvoidTheGeometryProbsTimer -= (*Timer_GetTimeStep());
					((CBike*)CamTargetEntity)->bWheelieCam = true;
				}
				return;
			}else{
				TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
				((CBike*)CamTargetEntity)->bWheelieCam = false;
			}
		}

		CMatrix *matrix = &CamTargetEntity->GetMatrix();
		if(((CVehicle*)CamTargetEntity)->IsBike()){
			((CBike*)CamTargetEntity)->CalculateLeanMatrix();
			matrix = &((CBike*)CamTargetEntity)->m_leanMatrix;
		}

		CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(CamTargetEntity->GetModelIndex())); // rouz edit (ChatGPT)
		CVector CamPos = mi->GetFrontSeatPosn();

		// rouz edit
		if (rouz.car_first_person)
		{
			// POV is driver
			/*CamPos.x = -CamPos.x;
			CamPos.y -= 0.3f;
			CamPos.z += 0.7f;*/

			// POV is middle back
			CamPos.x = 0.f;
			CamPos.y -= 1.f;
			CamPos.z += 0.75f;

			if (mi->m_positions[CAR_POS_BACKSEAT].y < mi->m_positions[CAR_POS_FRONTSEAT].y)
			{
				CamPos = mi->m_positions[CAR_POS_BACKSEAT];
				CamPos.x = 0.f;
				CamPos.z += 0.75f;
			}

			CamPos.x += rouz.cam_offset_x;
			CamPos.y += rouz.cam_offset_y;
			CamPos.z += rouz.cam_offset_z;

			FOV = rouz.cam_fov;
			RwCameraSetNearClipPlane(Scene.camera, rouz.cam_near_z);
		}
		else
		{
			CamPos.x = 0.0f;
			CamPos.y += 0.08f;
			CamPos.z += 0.62f;
			FOV = 60.0f;
		}

		Source = Multiply3x3(*matrix, CamPos);
		Source += CamTargetEntity->GetPosition();

		// rouz edit
		/*if (rouz.car_first_person && FindPlayerPed())
		{
			CamTargetEntity->GetMatrix().UpdateRW();
			CamTargetEntity->UpdateRwFrame();
			CamTargetEntity->UpdateRpHAnim();
			FindPlayerPed()->GetMatrix().UpdateRW();
			FindPlayerPed()->UpdateRwFrame();
			FindPlayerPed()->UpdateRpHAnim();
			RwV3d HeadPos;
			FindPlayerPed()->m_pedIK.GetComponentPosition(HeadPos, PED_HEAD);
			Source = HeadPos + 0.1f * FindPlayerPed()->GetUp();

			// Look around
			bool UseMouse = false;
			float MouseX = Input_GetMouseX(); // rouz edit (ChatGPT)
			float MouseY = Input_GetMouseY(); // rouz edit (ChatGPT)
			float LookLeftRight, LookUpDown;
			if(MouseX != 0.0f || MouseY != 0.0f){
				UseMouse = true;
				LookLeftRight = -3.0f*MouseX;
				LookUpDown = 4.0f*MouseY;
			}else{
				LookLeftRight = -CPad::GetPad(0)->SniperModeLookLeftRight();
				LookUpDown = CPad::GetPad(0)->SniperModeLookUpDown();
			}
			if(UseMouse){
				// Delegate the change of the angles to the C implementation
				Beta += Camera_MouseLookDelta(TheCamera.m_fMouseAccelHorzntl, LookLeftRight, FOV);
				Alpha += Camera_MouseLookDelta(TheCamera.m_fMouseAccelVertical, LookUpDown, FOV);
			}else{
				float xdir = LookLeftRight < 0.0f ? -1.0f : 1.0f;
				float ydir = LookUpDown < 0.0f ? -1.0f : 1.0f;
				// Delegate the change of the angles to the C implementation
				Beta += Camera_PadLookDelta(LookLeftRight, 100.0f, xdir, 0.8f, FOV, (*Timer_GetTimeStep()));
				Alpha += Camera_PadLookDelta(LookUpDown, 150.0f, ydir, 1.0f, FOV, (*Timer_GetTimeStep()));
			}
			// Delegate the wrap of the angle to the C implementation
			CameraMath_MakeAngleLessThan180(&Beta);
			// Delegate the limits of the angle to the C implementation
			Alpha = Camera_ClampAlpha(Alpha, 60.0f);

			// Delegate the point in the direction of the look to the C implementation
			{
				CVector lookPoint;
				Camera_LookPoint(Alpha, Beta, Source.x, Source.y, Source.z, &lookPoint);
				TargetCoors.x = lookPoint.x;
				TargetCoors.y = lookPoint.y;
				TargetCoors.z = lookPoint.z;
			}
			// Delegate the direction to the C implementation
			Front = NormalisedDifference(TargetCoors, Source);
			Source += Front*0.4f;

			GetVectorsReadyForRW();
			float Rotation = CGeneral::GetATanOfXY(Front.x, Front.y) - HALFPI;
			FindPlayerPed()->m_fRotationCur = Rotation;
			FindPlayerPed()->m_fRotationDest = Rotation;
		}*/

		if(((CVehicle*)CamTargetEntity)->IsBoat())
			Source.z += 0.5f;
		else if(((CVehicle*)CamTargetEntity)->IsBike() && ((CVehicle*)CamTargetEntity)->pDriver){
			CVector Neck(0.0f, 0.0f, 0.0f);
			((CVehicle*)CamTargetEntity)->pDriver->m_pedIK.GetComponentPosition(Neck, PED_NECK);
			Neck += ((CVehicle*)CamTargetEntity)->m_vecMoveSpeed * (*Timer_GetTimeStep());
			Source.z = Neck.z + fBike1stPersonOffsetZ;
		}

		// rouz edit (ChatGPT removed moving the camera down when upside down)
		DontLookThroughWorldFixer = 0.0f;
		Source.z += DontLookThroughWorldFixer;
		Front = matrix->GetForward();
		Front.Normalise();
		Up = matrix->GetUp();
		Up.Normalise();
		// Delegate the right and up vectors of the camera to the C implementation
		CVector rightC;
		CVector UpC;
		{
			Camera_RightAndUp(&Front, &Up, true, &rightC, &UpC);
		}
		CVector Right = CVector(rightC.x, rightC.y, rightC.z);
		Up = CVector(UpC.x, UpC.y, UpC.z);
	}

	ResetStatics = false;
}

static CVector vecHeadCamOffset(0.06f, 0.05f, 0.0f);

void
CCam::Process_1rstPersonPedOnPC(const CVector&, float TargetOrientation, float, float)
{
	// static int DontLookThroughWorldFixer = 0;	// unused
	static CVector InitialHeadPos;

	if(Mode != MODE_SNIPER_RUNABOUT)
		FOV = DefaultFOV;
	TheCamera.m_1rstPersonRunCloseToAWall = false;
	if(CamTargetEntity->m_rwObject == nil)
		return;

	if(CamTargetEntity->IsPed()){
		// static bool FailedTestTwelveFramesAgo = false;	// unused
		CVector HeadPos = vecHeadCamOffset;
		CVector TargetCoors;

		((CPed*)CamTargetEntity)->TransformToNode(HeadPos, PED_HEAD);
		// Use the C helper and retain the original typed SDK result
		RpHAnimHierarchy *hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(CamTargetEntity->GetClump()));
		int32 idx = RpHAnimIDGetIndex(hier, ConvertPedNode2BoneTag(PED_HEAD));
		RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
		RwV3dTransformPoints(&HeadPos, &HeadPos, 1, &mats[idx]);
		RwV3d scl = { 0.0f, 0.0f, 0.0f };
		RwMatrixScale(&mats[idx], &scl, rwCOMBINEPRECONCAT);

		if(ResetStatics){
			Beta = TargetOrientation;
			Alpha = 0.0f;
			m_fInitialPlayerOrientation = TargetOrientation;
			if(CamTargetEntity->IsPed()){	// useless check
				Beta = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
				Alpha = 0.0f;
				m_fInitialPlayerOrientation = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
				// FailedTestTwelveFramesAgo = false;
				m_bCollisionChecksOn = true;
			}
			// DontLookThroughWorldFixer = false;
			m_vecBufferedPlayerBodyOffset = HeadPos;
			InitialHeadPos = HeadPos;
		}

		m_vecBufferedPlayerBodyOffset.y = HeadPos.y;

		if(TheCamera.m_bHeadBob){
			m_vecBufferedPlayerBodyOffset.x =
				TheCamera.m_fGaitSwayBuffer * m_vecBufferedPlayerBodyOffset.x +
				(1.0f-TheCamera.m_fGaitSwayBuffer) * HeadPos.x;
			m_vecBufferedPlayerBodyOffset.z =
				TheCamera.m_fGaitSwayBuffer * m_vecBufferedPlayerBodyOffset.z +
				(1.0f-TheCamera.m_fGaitSwayBuffer) * HeadPos.z;
			HeadPos = (CamTargetEntity->GetMatrix() * m_vecBufferedPlayerBodyOffset);
		}else{
			float HeadDelta = (HeadPos - InitialHeadPos).Magnitude2D();
			// Delegate the horizontal direction to the C implementation
			CVector Fwd = NormalisedHorizontal(CamTargetEntity->GetForward());
			HeadPos = HeadDelta*1.23f*Fwd + CamTargetEntity->GetPosition();
			HeadPos.z += 0.59f;
		}
		Source = HeadPos;

		// unused:
		// ((CPed*)CamTargetEntity)->m_pedIK.GetComponentPosition(MidPos, PED_MID);
		// Source - MidPos;

		// Look around
		bool UseMouse = false;
		float MouseX = Input_GetMouseX(); // rouz edit (ChatGPT)
		float MouseY = Input_GetMouseY(); // rouz edit (ChatGPT)
		float LookLeftRight, LookUpDown;
		if(MouseX != 0.0f || MouseY != 0.0f){
			UseMouse = true;
			LookLeftRight = -3.0f*MouseX;
			LookUpDown = 4.0f*MouseY;
		}else{
			LookLeftRight = -CPad::GetPad(0)->LookAroundLeftRight();
			LookUpDown = CPad::GetPad(0)->LookAroundUpDown();
		}
		if(UseMouse){
			// Delegate the change of the angles to the C implementation
			Beta += Camera_MouseLookDelta(TheCamera.m_fMouseAccelHorzntl, LookLeftRight, FOV);
			Alpha += Camera_MouseLookDelta(TheCamera.m_fMouseAccelVertical, LookUpDown, FOV);
		}else{
			float xdir = LookLeftRight < 0.0f ? -1.0f : 1.0f;
			float ydir = LookUpDown < 0.0f ? -1.0f : 1.0f;
			// Delegate the change of the angles to the C implementation
			Beta += Camera_PadLookDelta(LookLeftRight, 100.0f, xdir, 0.8f, FOV, (*Timer_GetTimeStep()));
			Alpha += Camera_PadLookDelta(LookUpDown, 150.0f, ydir, 1.0f, FOV, (*Timer_GetTimeStep()));
		}
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&Beta);
		// Delegate the limits of the angle to the C implementation
		Alpha = Camera_ClampAlpha(Alpha, 60.0f);

		if(((CPed*)CamTargetEntity)->IsPlayer() && ((CPed*)CamTargetEntity)->m_attachedTo){
			CPed *pedTarget = ((CPed*)CamTargetEntity);
			float NewBeta;
			switch(pedTarget->m_attachType){
			case 0:
				NewBeta = pedTarget->GetForward().Heading() + HALFPI;
				break;
			case 1:
				NewBeta = pedTarget->GetForward().Heading() + PI;
				break;
			case 2:
				NewBeta = pedTarget->GetForward().Heading() - HALFPI;
				break;
			case 3:
				NewBeta = pedTarget->GetForward().Heading();
				break;
			}

			float BetaOffset = Beta - NewBeta;
			if(BetaOffset > PI) BetaOffset -= TWOPI;
			else if(BetaOffset < PI) BetaOffset += TWOPI;

			BetaOffset = Clamp(BetaOffset, -pedTarget->m_attachRotStep, pedTarget->m_attachRotStep);
			Beta = NewBeta + BetaOffset;
		}

		// Delegate the point in the direction of the look to the C implementation
		{
			CVector lookPoint;
			Camera_LookPoint(Alpha, Beta, Source.x, Source.y, Source.z, &lookPoint);
			TargetCoors.x = lookPoint.x;
			TargetCoors.y = lookPoint.y;
			TargetCoors.z = lookPoint.z;
		}
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(TargetCoors, Source);
		Source += Front*0.4f;

		TheCamera.m_AlphaForPlayerAnim1rstPerson = Alpha;

		GetVectorsReadyForRW();

		float Heading = Front.Heading();
		((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Heading;
		((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Heading;
		TheCamera.pTargetEntity->SetHeading(Heading);
		TheCamera.pTargetEntity->GetMatrix().UpdateRW();

		if(Mode == MODE_SNIPER_RUNABOUT){
			// no mouse wheel FOV buffering here like in normal sniper mode
			if(CPad::GetPad(0)->SniperZoomIn() || CPad::GetPad(0)->SniperZoomOut()){
				if(CPad::GetPad(0)->SniperZoomOut())
					FOV *= (255.0f*(*Timer_GetTimeStep()) + 10000.0f) / 10000.0f;
				else
					FOV /= (255.0f*(*Timer_GetTimeStep()) + 10000.0f) / 10000.0f;
			}

			TheCamera.SetMotionBlur(180, 255, 180, 120, MOTION_BLUR_SNIPER);

			if(FOV > DefaultFOV)
				FOV = DefaultFOV;
			if(FOV < 15.0f)
				FOV = 15.0f;
		}
	}

	ResetStatics = false;
	RwCameraSetNearClipPlane(Scene.camera, 0.05f);
}

float fCameraNearClipMult = 0.15f;

void
CCam::Process_Sniper(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	if(!CamTargetEntity->IsPed())
		return;

	float BackOffset = 0.19f;
	static bool FailedTestTwelveFramesAgo = false;
	RwV3d HeadPos;
	CVector TargetCoors;
	TargetCoors = CameraTarget;

	static float TargetFOV = 0.0f;

	if(ResetStatics){
		Beta = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
		Alpha = 0.0f;
		m_fInitialPlayerOrientation = ((CPed*)CamTargetEntity)->m_fRotationCur + HALFPI;
		FailedTestTwelveFramesAgo = false;
		// static DPadVertical unused
		// static DPadHorizontal unused
		m_bCollisionChecksOn = true;
		FOVSpeed = 0.0f;
		TargetFOV = FOV;
		ResetStatics = false;
	}

	if(((CPed*)CamTargetEntity)->bIsDucking)
		BackOffset = 0.8f;
	CamTargetEntity->GetMatrix().UpdateRW();
	CamTargetEntity->UpdateRwFrame();
	CamTargetEntity->UpdateRpHAnim();
	((CPed*)CamTargetEntity)->m_pedIK.GetComponentPosition(HeadPos, PED_HEAD);
	Source = HeadPos;
	Source.z += 0.1f;
	if(((CPed*)CamTargetEntity)->bIsDucking){
		Source.x -= fDuckingBackOffset*CamTargetEntity->GetForward().x;
		Source.y -= fDuckingBackOffset*CamTargetEntity->GetForward().y;
		Source.x -= fDuckingRightOffset*CamTargetEntity->GetRight().x;
		Source.y -= fDuckingRightOffset*CamTargetEntity->GetRight().y;
	}else{
		Source.x -= BackOffset*CamTargetEntity->GetForward().x;
		Source.y -= BackOffset*CamTargetEntity->GetForward().y;
	}

	// Look around
	bool UseMouse = false;
	float MouseX = Input_GetMouseX(); // rouz edit (ChatGPT)
	float MouseY = Input_GetMouseY(); // rouz edit (ChatGPT)
	float LookLeftRight, LookUpDown;
	if(MouseX != 0.0f || MouseY != 0.0f){
		UseMouse = true;
		LookLeftRight = -3.0f*MouseX;
		LookUpDown = 4.0f*MouseY;
	}else{
		LookLeftRight = -CPad::GetPad(0)->SniperModeLookLeftRight();
		LookUpDown = CPad::GetPad(0)->SniperModeLookUpDown();
	}
	if(UseMouse){
		// Delegate the change of the angles to the C implementation
		Beta += Camera_MouseLookDelta(TheCamera.m_fMouseAccelHorzntl, LookLeftRight, FOV);
		Alpha += Camera_MouseLookDelta(TheCamera.m_fMouseAccelVertical, LookUpDown, FOV);
	}else{
		float xdir = LookLeftRight < 0.0f ? -1.0f : 1.0f;
		float ydir = LookUpDown < 0.0f ? -1.0f : 1.0f;
		// Delegate the change of the angles to the C implementation
		Beta += Camera_PadLookDelta(LookLeftRight, 100.0f, xdir, 0.8f, FOV, (*Timer_GetTimeStep()));
		Alpha += Camera_PadLookDelta(LookUpDown, 150.0f, ydir, 1.0f, FOV, (*Timer_GetTimeStep()));
	}
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);
	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 60.0f);

	// Delegate the point in the direction of the look to the C implementation
	{
		CVector lookPoint;
		Camera_LookPoint(Alpha, Beta, Source.x, Source.y, Source.z, &lookPoint);
		TargetCoors.x = lookPoint.x;
		TargetCoors.y = lookPoint.y;
		TargetCoors.z = lookPoint.z;
	}

	UseMouse = false;
	int ZoomInButton = Input_GetMouseButtonAssociatedWithAction(ControlsManager.m_aSettings, PED_SNIPER_ZOOM_IN); // rouz edit (ChatGPT)
	int ZoomOutButton = Input_GetMouseButtonAssociatedWithAction(ControlsManager.m_aSettings, PED_SNIPER_ZOOM_OUT); // rouz edit (ChatGPT)
	if(ZoomInButton == rsMOUSEWHEELUPBUTTON || ZoomInButton == rsMOUSEWHEELDOWNBUTTON || ZoomOutButton == rsMOUSEWHEELUPBUTTON || ZoomOutButton == rsMOUSEWHEELDOWNBUTTON){
		if(Input_GetMouseWheelUp() || Input_GetMouseWheelDown()){ // rouz edit (ChatGPT)
			if(CPad::GetPad(0)->SniperZoomIn()){
				TargetFOV = FOV - 10.0f;
				UseMouse = true;
			}
			if(CPad::GetPad(0)->SniperZoomOut()){
				TargetFOV = FOV + 10.0f;
				UseMouse = true;
			}
		}
	}
	if((CPad::GetPad(0)->SniperZoomIn() || CPad::GetPad(0)->SniperZoomOut()) && !UseMouse){
		if(CPad::GetPad(0)->SniperZoomOut()){
			FOV *= (255.0f*(*Timer_GetTimeStep()) + 10000.0f) / 10000.0f;
			TargetFOV = FOV;
			FOVSpeed = 0.0f;
		}else{
			FOV /= (255.0f*(*Timer_GetTimeStep()) + 10000.0f) / 10000.0f;
			TargetFOV = FOV;
			FOVSpeed = 0.0f;
		}
	}else{
		if(General_AbsDiffAbove(TargetFOV, FOV, 0.5f))
			WellBufferMe(TargetFOV, &FOV, &FOVSpeed, 0.5f, 0.25f, false);
		else
			FOVSpeed = 0.0f;
	}

	TheCamera.SetMotionBlur(180, 255, 180, 120, MOTION_BLUR_SNIPER);

	// Delegate the limits of the field of view to the C implementation
	FOV = Camera_LimitSniperFOV(FOV, DefaultFOV, Mode == MODE_CAMERA);

	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
	Source += Front*0.4f;

	if(m_bCollisionChecksOn){
		if(!World_GetIsLineOfSightClear(TargetCoors, Source, true, true, false, true, false, true, true)){
			RwCameraSetNearClipPlane(Scene.camera, 0.4f);
			FailedTestTwelveFramesAgo = true;
		}else{
			CVector TestPoint;
			// Delegate the point in the direction of the look to the C implementation
			{
				CVector lookPoint;
				Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta + DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
				TestPoint.x = lookPoint.x;
				TestPoint.y = lookPoint.y;
				TestPoint.z = lookPoint.z;
			}
			if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
				RwCameraSetNearClipPlane(Scene.camera, 0.4f);
				FailedTestTwelveFramesAgo = true;
			}else{
				// Delegate the point in the direction of the look to the C implementation
				{
					CVector lookPoint;
					Camera_LookPoint(Alpha - DEGTORAD(20.0f), Beta - DEGTORAD(35.0f), Source.x, Source.y, Source.z, &lookPoint);
					TestPoint.x = lookPoint.x;
					TestPoint.y = lookPoint.y;
					TestPoint.z = lookPoint.z;
				}
				if(!World_GetIsLineOfSightClear(TestPoint, Source, true, true, false, true, false, true, true)){
					RwCameraSetNearClipPlane(Scene.camera, 0.4f);
					FailedTestTwelveFramesAgo = true;
				}else
					FailedTestTwelveFramesAgo = false;
			}
		}
	}

	if(FailedTestTwelveFramesAgo)
		RwCameraSetNearClipPlane(Scene.camera, 0.4f);
	else if(Mode == MODE_CAMERA)
		// Delegate the near clip plane to the C implementation
		RwCameraSetNearClipPlane(Scene.camera, Camera_SniperNearClip(FOV, fCameraNearClipMult, DEFAULT_NEAR));
	Source -= Front*0.4f;

	GetVectorsReadyForRW();
	float Rotation = General_GetATanOfXY(Front.x, Front.y) - HALFPI;
	((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Rotation;
	((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Rotation;
}

float INIT_SYPHON_GROUND_DIST = 2.419f;
float INIT_SYPHON_ALPHA_OFFSET = -DEGTORAD(3.0f);
float INIT_SYPHON_DEGREE_OFFSET = -DEGTORAD(30.0f);
float FrontOffsetSyphon = -DEGTORAD(25.5f);	// unused
float INIT_SYPHON_Z_OFFSET = -0.5f;

void
CCam::Process_Syphon(const CVector &CameraTarget, float, float, float)
{
	FOV = DefaultFOV;

	if(!CamTargetEntity->IsPed())
		return;

	static bool CameraObscured = false;
	// unused FailedClippingTestPrevously
	static float BetaOffset = INIT_SYPHON_DEGREE_OFFSET;
	// unused AngleToGoTo
	// unused AngleToGoToSpeed
	// unused DistBetweenPedAndPlayerPreviouslyOn
	static float HeightDown = INIT_SYPHON_Z_OFFSET;
	static float AlphaOffset = INIT_SYPHON_ALPHA_OFFSET;
	static bool NegateBetaOffset = true;
	CVector TargetCoors;
	float fAimingDist;
	float TargetAlpha;

	bool StandingOnMovingThing = false;
	TargetCoors = CameraTarget;
	AlphaOffset = INIT_SYPHON_ALPHA_OFFSET;
	float GroundDist = INIT_SYPHON_GROUND_DIST;

	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);

	float NewBeta = General_GetATanOfXY(TheCamera.m_cvecAimingTargetCoors.x - TargetCoors.x, TheCamera.m_cvecAimingTargetCoors.y - TargetCoors.y) + PI;
	if(ResetStatics){
		BetaOffset = INIT_SYPHON_DEGREE_OFFSET;
		Beta = General_GetATanOfXY(Source.x - TargetCoors.x, Source.y - TargetCoors.y);
		// some unuseds
		ResetStatics = false;
	}
	if(NegateBetaOffset)
		BetaOffset = -INIT_SYPHON_DEGREE_OFFSET;
	Beta = NewBeta + BetaOffset;
	Source = TargetCoors;
	// Delegate the offset on the orbit to the C implementation
	{
		float orbitX, orbitY;
		Camera_OrbitOffset(GroundDist, Beta, &orbitX, &orbitY);
		Source.x += orbitX;
		Source.y += orbitY;
	}
	CPhysical *ground = (CPhysical*)((CPed*)CamTargetEntity)->m_pCurSurface;
	if(ground && (ground->IsVehicle() || ground->IsObject()))
		StandingOnMovingThing = true;
	TargetCoors.z += m_fSyphonModeTargetZOffSet;

	bool PlayerTooClose = false;
	fAimingDist = (TheCamera.m_cvecAimingTargetCoors - TargetCoors).Magnitude2D();
	if(fAimingDist < 6.5f){
		fAimingDist = 6.5f;
		PlayerTooClose = true;
	}
	TargetAlpha = General_GetATanOfXY(fAimingDist, TheCamera.m_cvecAimingTargetCoors.z - TargetCoors.z);
	if(ResetStatics)	// BUG: can never happen
		Alpha = -TargetAlpha;
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&TargetAlpha);
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Alpha);

	// inlined
	if(StandingOnMovingThing)
		WellBufferMe(-TargetAlpha, &Alpha, &AlphaSpeed, 0.07f/2.0f, 0.015f/2.0f, true);
	else
		WellBufferMe(-TargetAlpha, &Alpha, &AlphaSpeed, 0.07f, 0.015f, true);

	// Delegate the rise of the camera to the C implementation
	Source.z += Camera_GroundHeightGain(GroundDist, Alpha, AlphaOffset);
	if(Source.z < TargetCoors.z + HeightDown)
		Source.z = TargetCoors.z + HeightDown;

	if(!PlayerTooClose){
		CColPoint point;
		CEntity *entity = nil;
		World_pIgnoreEntity = CamTargetEntity;
		if(World_ProcessLineOfSight(TheCamera.m_cvecAimingTargetCoors, Source, point, entity, true, false, false, true, false, false, true)){
			CVector TestFront = TheCamera.m_cvecAimingTargetCoors - Source;
			TestFront.Normalise();
			CVector CamToPlayer = CameraTarget - Source;
			CVector CamToCol = CollisionVector_ToNative(point.point) - Source;
			if(DotProduct(TestFront, CamToCol) > DotProduct(TestFront, CamToPlayer)){
				// collision is beyond player
				float ColDist = (TheCamera.m_cvecAimingTargetCoors - CollisionVector_ToNative(point.point)).Magnitude();
				CVector PlayerToTarget = TheCamera.m_cvecAimingTargetCoors - CameraTarget;
				float PlayerToTargetDist = PlayerToTarget.Magnitude();
				PlayerToTarget.Normalise();
				CVector Center = TheCamera.m_cvecAimingTargetCoors - ColDist*PlayerToTarget;
				float Radius = (CollisionVector_ToNative(point.point) - Center).Magnitude();
				if(World_TestSphereAgainstWorld(Center, Radius, nil, true, false, false, true, false, true)){
					CVector LineToCol = CollisionVector_ToNative(gaTempSphereColPoints[0].point) - Center;
					LineToCol -= DotProduct(LineToCol, PlayerToTarget)*PlayerToTarget;
					// unused
					CVector LineToPrevCol = CollisionVector_ToNative(point.point) - Center;
					LineToPrevCol -= DotProduct(LineToPrevCol, PlayerToTarget)*PlayerToTarget;
					float LineDist = LineToCol.Magnitude();
					// Delegate the angle to turn by to the C implementation
					float NewBetaOffset = Camera_SyphonBetaOffset(LineDist, ColDist, PlayerToTargetDist, GroundDist);
					if(NewBetaOffset < BetaOffset){
						float Ratio = NewBetaOffset / BetaOffset;
						BetaOffset = NewBetaOffset;
						Beta = NewBeta + NewBetaOffset;
						// Delegate the ratio limit to the C implementation
						GroundDist *= Camera_ClampedGroundRatio(Ratio);
						// Delegate the offset of the camera from its target to the C implementation
						float orbitX, orbitY;
						Camera_OrbitOffset(GroundDist, Beta, &orbitX, &orbitY);
						Source.x = TargetCoors.x + orbitX;
						Source.y = TargetCoors.y + orbitY;
						Source.z += (1.0f-Ratio)*0.5f;
					}
				}
			}
		}
		World_pIgnoreEntity = nil;
	}

	Front = TheCamera.m_cvecAimingTargetCoors - Source;
	float TargetDistGround = Front.Magnitude2D();
	Front.Normalise();
	m_cvecTargetCoorsForFudgeInter = Source + TargetDistGround*Front;
	m_cvecTargetCoorsForFudgeInter.z = TargetCoors.z;

	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, CameraTarget + CVector(0.0f, 0.0f, 0.75f), Source, FOV);
	Source.z = OrigSource.z;

	GetVectorsReadyForRW();
}

void
CCam::Process_Syphon_Crim_In_Front(const CVector &CameraTarget, float, float, float)
{
	FOV = DefaultFOV;

	if(!CamTargetEntity->IsPed())
		return;

	CVector TargetCoors = CameraTarget;
	CVector vDist;
	float fDist, TargetDist;
	float zOffset;
	float AimingAngle;

	TargetDist = TheCamera.m_fPedZoomValueSmooth * 0.5f + 4.0f;
	vDist = Source - TargetCoors;
	fDist = vDist.Magnitude2D();
	zOffset = TargetDist - 2.65f;
	if(zOffset < 0.0f)
		zOffset = 0.0f;
	if(zOffset == 0.0f)
		Source = TargetCoors + CVector(1.0f, 1.0f, zOffset);
	else
		Source = TargetCoors + CVector(vDist.x/fDist*TargetDist, vDist.y/fDist*TargetDist, zOffset);

	AimingAngle = General_GetATanOfXY(TheCamera.m_cvecAimingTargetCoors.x - TargetCoors.x, TheCamera.m_cvecAimingTargetCoors.y - TargetCoors.y);
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&AimingAngle);

	if(ResetStatics){
		if(AimingAngle > 0.0f)
			m_fPlayerInFrontSyphonAngleOffSet = -m_fPlayerInFrontSyphonAngleOffSet;
		ResetStatics = false;
	}

	if(TheCamera.PlayerWeaponMode.Mode == MODE_SYPHON)
		Beta = AimingAngle + m_fPlayerInFrontSyphonAngleOffSet;

	Source.x = TargetCoors.x;
	Source.y = TargetCoors.y;
	// Delegate the offset on the orbit to the C implementation
	{
		float orbitX, orbitY;
		Camera_OrbitOffset(TargetDist, Beta, &orbitX, &orbitY);
		Source.x += orbitX;
		Source.y += orbitY;
	}

	TargetCoors = CameraTarget;
	TargetCoors.z += m_fSyphonModeTargetZOffSet;
	m_cvecTargetCoorsForFudgeInter = TargetCoors;

	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);

	Front = TargetCoors - Source;
	GetVectorsReadyForRW();
}

float MAX_HEIGHT_UP = 15.0f;
float WATER_Z_ADDITION = 2.75f;
float WATER_Z_ADDITION_MIN = 1.5f;
float SMALLBOAT_CLOSE_ALPHA_MINUS = 0.2f;
float afBoatBetaDiffMult[3] = { 0.15f, 0.07f, 0.01f };
float afBoatBetaSpeedDiffMult[3] = { 0.02f, 0.015f, 0.005f };

void
CCam::Process_BehindBoat(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	if(!CamTargetEntity->IsVehicle()){
		ResetStatics = false;
		return;
	}

	CVector TargetCoors = CameraTarget;
	float DeltaBeta = 0.0f;
	static float TargetWhenChecksWereOn = 0.0f;
	static float CenterObscuredWhenChecksWereOn = 0.0f;
	static float WaterZAddition = 2.75f;
	float WaterLevel = 0.0f;
	float MaxHeightUp = MAX_HEIGHT_UP;
	static float WaterLevelBuffered = 0.0f;
	static float WaterLevelSpeed = 0.0f;
	float BetaDiffMult = 0.0f;
	float BetaSpeedDiffMult = 0.0f;

	Beta = General_GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y);
	FOV = DefaultFOV;
	float TargetAlpha = 0.0f;

	if(ResetStatics){
		CenterObscuredWhenChecksWereOn = 0.0f;
		TargetWhenChecksWereOn = 0.0f;
	}else if(DirectionWasLooking != LOOKING_FORWARD)
		Beta = TargetOrientation;

	if(!WaterLevel_GetWaterLevelNoWaves(TargetCoors.x, TargetCoors.y, TargetCoors.z, &WaterLevel))
		WaterLevel = TargetCoors.z - 0.5f;
	if(ResetStatics){
		WaterLevelBuffered = WaterLevel;
		WaterLevelSpeed = 0.0f;
	}
	WellBufferMe(WaterLevel, &WaterLevelBuffered, &WaterLevelSpeed, 0.2f, 0.07f, false);

	static float FixerForGoingBelowGround = 0.4f;
	if(-FixerForGoingBelowGround < TargetCoors.z-WaterLevelBuffered+WATER_Z_ADDITION)
		WaterLevelBuffered += TargetCoors.z-WaterLevelBuffered+WATER_Z_ADDITION - FixerForGoingBelowGround;

	CVector BoatDimensions = CollisionVector_ToNative(Box_GetSize(&CamTargetEntity->GetColModel()->boundingBox));
	float BoatSize = BoatDimensions.Magnitude2D();
	int index = 0;
	TheCamera.GetArrPosForVehicleType(((CVehicle*)CamTargetEntity)->GetVehicleAppearance(), index);
	if(TheCamera.CarZoomIndicator == CAM_ZOOM_1){
		TargetAlpha = ZmOneAlphaOffset[index];
		BetaDiffMult = afBoatBetaDiffMult[0];
		BetaSpeedDiffMult = afBoatBetaSpeedDiffMult[0];
	}else if(TheCamera.CarZoomIndicator == CAM_ZOOM_2){
		TargetAlpha = ZmTwoAlphaOffset[index];
		BetaDiffMult = afBoatBetaDiffMult[1];
		BetaSpeedDiffMult = afBoatBetaSpeedDiffMult[1];
	}else if(TheCamera.CarZoomIndicator == CAM_ZOOM_3){
		TargetAlpha = ZmThreeAlphaOffset[index];
		BetaDiffMult = afBoatBetaDiffMult[2];
		BetaSpeedDiffMult = afBoatBetaSpeedDiffMult[2];
	}
	if(TheCamera.CarZoomIndicator == CAM_ZOOM_1 && BoatSize < 10.0f){
		TargetAlpha -= SMALLBOAT_CLOSE_ALPHA_MINUS;
		BoatSize = 10.0f;
	}else if(CullZones_Cam1stPersonForPlayer()){
		float Water = 0.0f;
		// useless call
		//CWaterLevel::GetWaterLevelNoWaves(TargetCoors.x, TargetCoors.y, TargetCoors.z, &Water);
		Water = (WaterLevel + WATER_Z_ADDITION_MIN - WaterLevelBuffered - WATER_Z_ADDITION)/(BoatDimensions.z/2.0f + MaxHeightUp);
		TargetAlpha = General_AsinClamped(Water);
	}

	if(ResetStatics){
		Alpha = TargetAlpha;
		AlphaSpeed = 0.0f;
	}
	WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.15f, 0.07f, true);

	if(ResetStatics){
		Beta = TargetOrientation;
		DeltaBeta = 0.0f;
	}
	// inlined
	WellBufferMe(TargetOrientation, &Beta, &BetaSpeed, BetaDiffMult * ((CVehicle*)CamTargetEntity)->m_vecMoveSpeed.Magnitude(), BetaSpeedDiffMult, true);

	// Delegate the offset behind the boat to the C implementation
	{
		float boatDistance = TheCamera.CarZoomValueSmooth+BoatSize;
		float orbitX, orbitY;
		Camera_OrbitOffset(boatDistance, Beta, &orbitX, &orbitY);
		Source = CVector(-orbitX, -orbitY, boatDistance * 0.0f) + TargetCoors;
	}
	Source.z = WaterLevelBuffered + WATER_Z_ADDITION + (BoatDimensions.z/2.0f + MaxHeightUp) * Sin(Alpha);

	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);
	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);


	float TargetRoll;
	if(CPad::GetPad(0)->GetDPadLeft() || CPad::GetPad(0)->GetDPadRight()){
#ifdef FIX_BUGS
		float fwdSpeed = 180.0f*DotProduct(((CVehicle*)CamTargetEntity)->m_vecMoveSpeed, CamTargetEntity->GetForward());
		if(fwdSpeed > 210.0f) fwdSpeed = 210.0f;
#endif
		if(CPad::GetPad(0)->GetDPadLeft())
			TargetRoll = DEGTORAD(10.0f)*TiltOverShoot[index] + f_max_role_angle;
		else
			TargetRoll = -(DEGTORAD(10.0f)*TiltOverShoot[index] + f_max_role_angle);
		CVector FwdTarget = CamTargetEntity->GetForward();
		FwdTarget.Normalise();
		float AngleDiff = DotProduct(FwdTarget, Front);
		AngleDiff = Camera_DirectionAngle(AngleDiff);
#ifdef FIX_BUGS
		TargetRoll *= fwdSpeed/210.0f * Sin(AngleDiff);
#else
		TargetRoll *= Sin(AngleDiff);
#endif
	}else{
		float fwdSpeed = 180.0f*DotProduct(((CVehicle*)CamTargetEntity)->m_vecMoveSpeed, CamTargetEntity->GetForward());
		if(fwdSpeed > 210.0f) fwdSpeed = 210.0f;
		TargetRoll = CPad::GetPad(0)->GetLeftStickX()/128.0f * fwdSpeed/210.0f;
		CVector FwdTarget = CamTargetEntity->GetForward();
		FwdTarget.Normalise();
		float AngleDiff = DotProduct(FwdTarget, Front);
		AngleDiff = Camera_DirectionAngle(AngleDiff);
		TargetRoll *= (DEGTORAD(10.0f)*TiltOverShoot[index] + f_max_role_angle) * Sin(AngleDiff);
	}

	WellBufferMe(TargetRoll, &f_Roll, &f_rollSpeed, 0.15f, 0.07f, false);
	// Delegate the up vector to the C implementation
	{
		Camera_UpFromRoll(f_Roll, &Up);
	}
	Up.Normalise();
	Front.Normalise();
	// Delegate the up vector of the camera to the C implementation
	{
		CVector newUpC;
		Camera_PerpendicularUp(&Front, &Up, true, &newUpC);
		Up = CVector(newUpC.x, newUpC.y, newUpC.z);
	}

	ResetStatics = false;
}

float FIGHT_HORIZ_DIST = 3.0f;
float FIGHT_VERT_DIST = 1.0f;
float FIGHT_BETA_ANGLE = 125.0f;

void
CCam::Process_Fight_Cam(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	if(!CamTargetEntity->IsPed())
		return;

	FOV = DefaultFOV;
	float HorizDist = FIGHT_HORIZ_DIST;
	float VertDist = FIGHT_VERT_DIST;
	float BetaLeft, BetaRight, DeltaBetaLeft, DeltaBetaRight;
	static bool PreviouslyFailedBuildingChecks = false;
	float TargetCamHeight;
	CVector TargetCoors;

	m_fMinDistAwayFromCamWhenInterPolating = FIGHT_HORIZ_DIST;
	Front = Source - CameraTarget;
	if(ResetStatics)
		Beta = General_GetATanOfXY(Front.x, Front.y);
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&TargetOrientation);
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);

	// Figure out Beta
	BetaLeft = TargetOrientation - DEGTORAD(FIGHT_BETA_ANGLE);
	BetaRight = TargetOrientation + DEGTORAD(FIGHT_BETA_ANGLE);
	DeltaBetaLeft = Beta - BetaLeft;
	DeltaBetaRight = Beta - BetaRight;
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&DeltaBetaLeft);
	// Delegate the wrap of the angle to the C implementation
	CameraMath_MakeAngleLessThan180(&DeltaBetaRight);

	if(ResetStatics){
		// Delegate the choice of the smaller angle to the C implementation
		m_fTargetBeta = Camera_SmallerDelta(DeltaBetaLeft, DeltaBetaRight);
		m_fBufferedTargetOrientation = TargetOrientation;
		m_fBufferedTargetOrientationSpeed = 0.0f;
		m_bCollisionChecksOn = true;
		BetaSpeed = 0.0f;
	}else if(CPad::GetPad(0)->WeaponJustDown()){
		// Delegate the choice of the smaller angle to the C implementation
		m_fTargetBeta = Camera_SmallerDelta(DeltaBetaLeft, DeltaBetaRight);
	}

	WellBufferMe(m_fTargetBeta, &Beta, &BetaSpeed, 0.015f, 0.007f, true);

	Source = CameraTarget + HorizDist*CVector(Cos(Beta), Sin(Beta), 0.0f);
	Source.z += VertDist;

	WellBufferMe(TargetOrientation, &m_fBufferedTargetOrientation, &m_fBufferedTargetOrientationSpeed, 0.07f, 0.004f, true);
	TargetCoors = CameraTarget + 0.1f*CVector(Cos(m_fBufferedTargetOrientation), Sin(m_fBufferedTargetOrientation), 0.0f);

	TargetCamHeight = CameraTarget.z - Source.z + Max(m_fPedBetweenCameraHeightOffset, m_fDimensionOfHighestNearCar) + VertDist;
	if(TargetCamHeight > m_fCamBufferedHeight)
		WellBufferMe(TargetCamHeight, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.15f, 0.04f, false);
	else
		WellBufferMe(0.0f, &m_fCamBufferedHeight, &m_fCamBufferedHeightSpeed, 0.08f, 0.0175f, false);
	Source.z += m_fCamBufferedHeight;

	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);
	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
	GetVectorsReadyForRW();

	ResetStatics = false;
}

/*
// Spline format is this, but game doesn't seem to use any kind of struct:
struct Spline
{
	float numFrames;
	struct {
		float time;
		float f[3];	// CVector for Vector spline
	} frames[1];	// numFrames
};
*/

// These two functions are pretty ugly

#define MS(t) (uint32)((t)*1000.0f)

void
FindSplinePathPositionFloat(float *out, float *spline, uint32 time, uint32 &marker)
{
	// Delegate the position in the spline to the C implementation
	bool outOfRange;
	float a = Camera_SplineFraction(spline, time, &marker, 4, &outOfRange);
	*out = Camera_BezierBlend(a, spline[marker-3], spline[marker-1], spline[marker+2], spline[marker+1]);
}

void
FindSplinePathPositionVector(CVector *out, float *spline, uint32 time, uint32 &marker)
{
	// Delegate the position in the spline to the C implementation
	uint32 numFrames = spline[0];
	bool outOfRange;
	float a = Camera_SplineFraction(spline, time, &marker, 10, &outOfRange);
	if(outOfRange){
		printf("Arraymarker %i \n", marker);
		printf("Path zero %i \n", numFrames);
	}

	out->x = Camera_BezierBlend(a, spline[marker-9], spline[marker-3], spline[marker+4], spline[marker+1]);
	out->y = Camera_BezierBlend(a, spline[marker-8], spline[marker-2], spline[marker+5], spline[marker+2]);
	out->z = Camera_BezierBlend(a, spline[marker-7], spline[marker-1], spline[marker+6], spline[marker+3]);
	*out += TheCamera.m_vecCutSceneOffset;
}

void
CCam::Process_FlyBy(const CVector&, float, float, float)
{
	float UpAngle = 0.0f;
	static float FirstFOVValue = 0.0f;
	static float PsuedoFOV;
	static uint32 ArrayMarkerFOV;
	static uint32 ArrayMarkerUp;
	static uint32 ArrayMarkerSource;
	static uint32 ArrayMarkerFront;

	if(TheCamera.m_bcutsceneFinished)
		return;
#ifdef FIX_BUGS
	// this would crash, not nice when cycling debug mode
	if(TheCamera.m_arrPathArray[0].m_arr_PathData == nil)
		return;
#endif

	Up = CVector(0.0f, 0.0f, 1.0f);
	if(TheCamera.m_bStartingSpline)
		m_fTimeElapsedFloat += Timer_GetTimeStepNonClippedInMilliseconds();
	else{
		m_fTimeElapsedFloat = 0.0f;
		m_uiFinishTime = MS(TheCamera.m_arrPathArray[2].m_arr_PathData[10*((int)TheCamera.m_arrPathArray[2].m_arr_PathData[0]-1) + 1]);
		TheCamera.m_bStartingSpline = true;
		FirstFOVValue = TheCamera.m_arrPathArray[0].m_arr_PathData[2];
		PsuedoFOV = TheCamera.m_arrPathArray[0].m_arr_PathData[2];
		ArrayMarkerFOV = 5;
		ArrayMarkerUp = 5;
		ArrayMarkerSource = 11;
		ArrayMarkerFront = 11;
	}

	float fTime = m_fTimeElapsedFloat;
	uint32 uiFinishTime = m_uiFinishTime;
	uint32 uiTime = fTime;
	if(uiTime < uiFinishTime){
		TheCamera.m_fPositionAlongSpline = (float) uiTime / uiFinishTime;

		// Advance to the spline segment of the time
		ArrayMarkerSource = CameraMath_AdvanceSplineMarker(TheCamera.m_arrPathArray[2].m_arr_PathData, ArrayMarkerSource, uiTime, 10);
		FindSplinePathPositionVector(&Source, TheCamera.m_arrPathArray[2].m_arr_PathData, uiTime, ArrayMarkerSource);

		// Advance to the spline segment of the time
		ArrayMarkerFront = CameraMath_AdvanceSplineMarker(TheCamera.m_arrPathArray[3].m_arr_PathData, ArrayMarkerFront, uiTime, 10);
		FindSplinePathPositionVector(&Front, TheCamera.m_arrPathArray[3].m_arr_PathData, uiTime, ArrayMarkerFront);

		// Advance to the spline segment of the time
		ArrayMarkerUp = CameraMath_AdvanceSplineMarker(TheCamera.m_arrPathArray[1].m_arr_PathData, ArrayMarkerUp, uiTime, 4);
		FindSplinePathPositionFloat(&UpAngle, TheCamera.m_arrPathArray[1].m_arr_PathData, uiTime, ArrayMarkerUp);
		// Delegate the up vector of the fly-by camera to the C implementation
		UpAngle = Camera_FlyByUp(UpAngle, &Up.x, &Up.z);

		// Advance to the spline segment of the time
		ArrayMarkerFOV = CameraMath_AdvanceSplineMarker(TheCamera.m_arrPathArray[0].m_arr_PathData, ArrayMarkerFOV, uiTime, 4);
		FindSplinePathPositionFloat(&PsuedoFOV, TheCamera.m_arrPathArray[0].m_arr_PathData, uiTime, ArrayMarkerFOV);

		m_cvecTargetCoorsForFudgeInter = Front;
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(Front, Source);
		// Delegate the up vector of the camera to the C implementation
		{
			CVector newUpC;
			Camera_PerpendicularUp(&Front, &Up, false, &newUpC);
			Up = CVector(newUpC.x, newUpC.y, newUpC.z);
		}
	}else if(uiTime >= uiFinishTime){
		// end
		ArrayMarkerSource = (TheCamera.m_arrPathArray[2].m_arr_PathData[0] - 1)*10 + 1;
		ArrayMarkerFront = (TheCamera.m_arrPathArray[3].m_arr_PathData[0] - 1)*10 + 1;
		ArrayMarkerUp = (TheCamera.m_arrPathArray[1].m_arr_PathData[0] - 1)*4 + 1;
		ArrayMarkerFOV = (TheCamera.m_arrPathArray[0].m_arr_PathData[0] - 1)*4 + 1;

		FindSplinePathPositionVector(&Source, TheCamera.m_arrPathArray[2].m_arr_PathData, uiTime, ArrayMarkerSource);
		FindSplinePathPositionVector(&Front, TheCamera.m_arrPathArray[3].m_arr_PathData, uiTime, ArrayMarkerFront);
		FindSplinePathPositionFloat(&UpAngle, TheCamera.m_arrPathArray[1].m_arr_PathData, uiTime, ArrayMarkerUp);
		// Delegate the up vector of the fly-by camera to the C implementation
		UpAngle = Camera_FlyByUp(UpAngle, &Up.x, &Up.z);
		FindSplinePathPositionFloat(&PsuedoFOV, TheCamera.m_arrPathArray[0].m_arr_PathData, uiTime, ArrayMarkerFOV);

		TheCamera.m_fPositionAlongSpline = 1.0f;
		ArrayMarkerFOV = 0;
		ArrayMarkerUp = 0;
		ArrayMarkerSource = 0;
		ArrayMarkerFront = 0;

		m_cvecTargetCoorsForFudgeInter = Front;
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(Front, Source);
		// Delegate the up vector of the camera to the C implementation
		{
			CVector newUpC;
			Camera_PerpendicularUp(&Front, &Up, false, &newUpC);
			Up = CVector(newUpC.x, newUpC.y, newUpC.z);
		}
	}
	FOV = PsuedoFOV;
}

CVector vecWheelCamBoatOffset(-0.5f, -0.8f, 0.3f);
CVector vecWheelCamBoatOffsetAlt(0.2f, -0.2f, -0.3f);
float fWheelCamCarXOffset = 0.33f;
float fWheelCamBikeXOffset = 0.2f;

bool
CCam::Process_WheelCam(const CVector&, float, float, float)
{
	FOV = DefaultFOV;

	CVector WheelPos;
	if(CamTargetEntity->IsPed()){
		// what? ped with wheels or what?
		Source = Multiply3x3(CamTargetEntity->GetMatrix(), CVector(-0.3f, -0.5f, 0.1f));
		Source += CamTargetEntity->GetPosition();
		Front = CVector(1.0f, 0.0f, 0.0f);
	}else{
		WheelPos = CollisionVector_ToNative(CamTargetEntity->GetColModel()->boundingBox.min);
		WheelPos.x -= 0.33f;
		WheelPos.y = -2.3f;
		WheelPos.z = 0.3f;
		Source = CamTargetEntity->GetMatrix() * WheelPos;
		Front = CamTargetEntity->GetForward();
	}

	CVector NewUp, Right;
	if(CamTargetEntity->IsVehicle() &&
	   (((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI ||
	    ((CVehicle*)CamTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE)){
		WheelPos.x = -1.55f;
		Right = CamTargetEntity->GetRight();
		NewUp = CamTargetEntity->GetUp();
		Source = CamTargetEntity->GetMatrix() * WheelPos;
	}else if(CamTargetEntity->IsVehicle() && ((CVehicle*)CamTargetEntity)->IsBoat()){
		NewUp = CVector(0.0f, 0.0f, 1.0f);
		// Delegate the right and up vectors of the camera to the C implementation
		CVector rightC;
		CVector NewUpC;
		{
			Camera_RightAndUp(&Front, &NewUp, true, &rightC, &NewUpC);
		}
		Right = CVector(rightC.x, rightC.y, rightC.z);
		NewUp = CVector(NewUpC.x, NewUpC.y, NewUpC.z);

		CVector BoatCamPos(0.0f, 0.0f, 0.0f);
		if(((CVehicle*)CamTargetEntity)->pDriver){
			((CVehicle*)CamTargetEntity)->pDriver->m_pedIK.GetComponentPosition(BoatCamPos, PED_HEAD);
			BoatCamPos += ((CVehicle*)CamTargetEntity)->m_vecMoveSpeed * (*Timer_GetTimeStep());
			BoatCamPos += vecWheelCamBoatOffset.x * Right;
			BoatCamPos += vecWheelCamBoatOffset.y * CamTargetEntity->GetForward();
			BoatCamPos.z += vecWheelCamBoatOffset.z;
			if(CamTargetEntity->GetModelIndex() == MI_PREDATOR){
				BoatCamPos += vecWheelCamBoatOffsetAlt.x * Right;
				BoatCamPos += vecWheelCamBoatOffsetAlt.y * CamTargetEntity->GetForward();
				BoatCamPos.z += vecWheelCamBoatOffsetAlt.z;
			}
			Source = BoatCamPos;
		}else
			Source.z += 2.0f*vecWheelCamBoatOffset.z;
	}else if(CamTargetEntity->IsVehicle() && ((CVehicle*)CamTargetEntity)->IsBike()){
		NewUp = CVector(0.0f, 0.0f, 1.0f);
		// Delegate the right and up vectors of the camera to the C implementation
		CVector rightC;
		CVector NewUpC;
		{
			Camera_RightAndUp(&Front, &NewUp, true, &rightC, &NewUpC);
		}
		Right = CVector(rightC.x, rightC.y, rightC.z);
		NewUp = CVector(NewUpC.x, NewUpC.y, NewUpC.z);

		WheelPos.z += fWheelCamCarXOffset - fWheelCamBikeXOffset;
		Source = CamTargetEntity->GetPosition();
		Source += WheelPos.x * CamTargetEntity->GetRight();
		Source += WheelPos.y * Front;
		Source += WheelPos.z * Up;
	}else{
		NewUp = CVector(0.0f, 0.0f, 1.0f);
		// Delegate the right and up vectors of the camera to the C implementation
		CVector NewUpC;
		{
			Camera_RightAndUp(&Front, &NewUp, true, &Right, &NewUpC);
		}
		NewUp = CVector(NewUpC.x, NewUpC.y, NewUpC.z);
	}

	// Delegate the roll of the wheel camera to the C implementation
	float RollCos, RollSin;
	Camera_WheelCamRoll((*Timer_GetTimeInMilliseconds()), &RollCos, &RollSin);
	Up = RollCos*NewUp + RollSin*Right;

	CEntity *entity = nil;
	CColPoint point;
	World_pIgnoreEntity = CamTargetEntity;
	bool blocked = World_ProcessLineOfSight(Source, CamTargetEntity->GetPosition(), point, entity, true, false, false, true, false, false, true);
	World_pIgnoreEntity = nil;
	return !blocked;
}

int BOAT_UNDERWATER_CAM_BLUR = 20;
float BOAT_UNDERWATER_CAM_COLORMAG_LIMIT = 10.0f;

//--MIAIM: done
void
CCam::Process_Fixed(const CVector &CameraTarget, float, float, float)
{
	if(DirectionWasLooking != LOOKING_FORWARD)
		DirectionWasLooking = LOOKING_FORWARD;

	Source = m_cvecCamFixedModeSource;
	// Delegate the direction to the C implementation
	Front = NormalisedDifference(CameraTarget, Source);
	m_cvecTargetCoorsForFudgeInter = CameraTarget;
	GetVectorsReadyForRW();

	// Delegate the up vector of the fixed camera to the C implementation
	{
		Camera_FixedModeUp(&Front, &m_cvecCamFixedModeUpOffSet, &Up);
	}

	FOV = DefaultFOV;
	if(TheCamera.m_bUseSpecialFovTrain)
		FOV = TheCamera.m_fFovForTrain;

	float WaterZ = 0.0f;
	if(WaterLevel_GetWaterLevel(Source, &WaterZ, true) && Source.z < WaterZ){
		// Delegate the luminance of the water to the C implementation
		float WaterLum = Camera_WaterLuminance(TimeCycle_GetWaterRed(), TimeCycle_GetWaterGreen(), TimeCycle_GetWaterBlue());
		if(WaterLum > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT){
			float f = BOAT_UNDERWATER_CAM_COLORMAG_LIMIT/WaterLum;
			TheCamera.SetMotionBlur(TimeCycle_GetWaterRed()*f,
				TimeCycle_GetWaterGreen()*f,
				TimeCycle_GetWaterBlue()*f, BOAT_UNDERWATER_CAM_BLUR, MOTION_BLUR_LIGHT_SCENE);
		}else{
			TheCamera.SetMotionBlur(TimeCycle_GetWaterRed(),
				TimeCycle_GetWaterGreen(),
				TimeCycle_GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MOTION_BLUR_LIGHT_SCENE);
		}
	}

#ifdef PC_PLAYER_CONTROLS
	if(FrontEndMenuManager.m_ControlMethod == CONTROL_STANDARD && Using3rdPersonMouseCam()){
		CPed *player = FindPlayerPed();
		if(player && player->CanStrafeOrMouseControl()){
			float Heading = Front.Heading();
			((CPed*)TheCamera.pTargetEntity)->m_fRotationCur = Heading;
			((CPed*)TheCamera.pTargetEntity)->m_fRotationDest = Heading;
			TheCamera.pTargetEntity->SetHeading(Heading);
			TheCamera.pTargetEntity->GetMatrix().UpdateRW();
		}
	}
#endif
}

void
CCam::Process_LightHouse(const CVector &CameraTarget, float, float, float)
{
	static float Timer;

	Source = CameraTarget;
	Source.x = 474.3f;
	Source.y = -1717.6f;

	// Delegate to the C implementation
	int CamMode = CameraMath_LightHouseMode(&Timer, CameraTarget.z, (CameraTarget-Source).Magnitude2D(), (*Timer_GetTimeStep()));

	if(CamMode == 2){
		Source.z = 57.5f;
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(Source, CameraTarget);
		Source.x = CameraTarget.x - 5.0f*Front.x;
		Source.y = CameraTarget.y - 5.0f*Front.y;
	}else if(CamMode == 1){
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(CameraTarget, Source);
		Source.x = CameraTarget.x - 2.0f*Front.x;
		Source.y = CameraTarget.y - 2.0f*Front.y;
	}else{
		Source.z += 4.0f;
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(CameraTarget, Source);
		Source -= 4.0f*Front;
		// Delegate the limit to the C implementation
		Source.z = General_MinFloat(Source.z, 55.0f);
		Front = CameraTarget - Source;
	}

	m_cvecTargetCoorsForFudgeInter = CameraTarget;
	GetVectorsReadyForRW();

	// Delegate the up vector of the fixed camera to the C implementation
	{
		Camera_FixedModeUp(&Front, &m_cvecCamFixedModeUpOffSet, &Up);
	}

	FOV = DefaultFOV;
	if(TheCamera.m_bUseSpecialFovTrain)	// uh, sure...
		FOV = TheCamera.m_fFovForTrain;
}

void
CCam::Process_Player_Fallen_Water(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	CColPoint colPoint;
	CEntity *entity = nil;

	FOV = DefaultFOV;
	Source = m_vecLastAboveWaterCamPosition;
	Source.z += 4.0f;

	m_cvecTargetCoorsForFudgeInter = CameraTarget;
	// Delegate the direction to the C implementation
	Front = NormalisedDifference(CameraTarget, Source);
	if(World_ProcessLineOfSight(CameraTarget, Source, colPoint, entity, true, false, false, true, false, true, true))
		Source = CollisionVector_ToNative(colPoint.point);
	GetVectorsReadyForRW();
	// Delegate the direction to the C implementation
	Front = NormalisedDifference(CameraTarget, Source);
}

void
CCam::Process_SpecialFixedForSyphon(const CVector &CameraTarget, float, float, float)
{
	Source = m_cvecCamFixedModeSource;
	m_cvecTargetCoorsForFudgeInter = CameraTarget;
	m_cvecTargetCoorsForFudgeInter.z += m_fSyphonModeTargetZOffSet;
	Front = CameraTarget - Source;
	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, m_cvecTargetCoorsForFudgeInter, Source, FOV);
	Front.z += m_fSyphonModeTargetZOffSet;

	GetVectorsReadyForRW();

	// Delegate the up and front vectors of the syphon camera to the C implementation
	{
		CVector upC(Up.x, Up.y, Up.z);
		CVector frontC(Front.x, Front.y, Front.z);
		Camera_SyphonBasis(&upC, &m_cvecCamFixedModeUpOffSet, &frontC);
		Up = CVector(upC.x, upC.y, upC.z);
		Front = CVector(frontC.x, frontC.y, frontC.z);
	}
	FOV = DefaultFOV;
}

#ifdef IMPROVED_CAMERA

#define KEYJUSTDOWN(k) Input_GetIsKeyboardKeyJustDown((RsKeyCodes)k) // rouz edit (ChatGPT)
#define KEYDOWN(k) Input_GetIsKeyboardKeyDown((RsKeyCodes)k) // rouz edit (ChatGPT)
#define CTRLJUSTDOWN(key) \
	       ((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYJUSTDOWN((RsKeyCodes)key) || \
	        (KEYJUSTDOWN(rsLCTRL) || KEYJUSTDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))
#define CTRLDOWN(key) ((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))


void
CCam::Process_Debug(const CVector&, float, float, float)
{
	static float Speed = 0.0f;
	static float PanSpeedX = 0.0f;
	static float PanSpeedY = 0.0f;
	CVector TargetCoors;

	RwCameraSetNearClipPlane(Scene.camera, DEFAULT_NEAR);
	FOV = DefaultFOV;
	Alpha += DEGTORAD(CPad::GetPad(1)->GetLeftStickY()) / 50.0f;
	Beta  += DEGTORAD(CPad::GetPad(1)->GetLeftStickX()*1.5f) / 19.0f;
	if(Input_GetLeftMouse()){ // rouz edit (ChatGPT)
		Alpha += DEGTORAD(Input_GetMouseY()/2.0f); // rouz edit (ChatGPT)
		Beta += DEGTORAD(Input_GetMouseX()/2.0f); // rouz edit (ChatGPT)
	}

	// Delegate the point to look at to the C implementation
	{
		CVector debugTarget;
		Camera_DebugTarget(Alpha, Beta, Source.x, Source.y, Source.z, 3.0f, &debugTarget);
		TargetCoors.x = debugTarget.x;
		TargetCoors.y = debugTarget.y;
		TargetCoors.z = debugTarget.z;
	}

	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 89.5f);

	// Delegate the speed to the C implementation
	Speed = Camera_DebugSpeed(Speed, CPad::GetPad(1)->GetSquare() || KEYDOWN('W'), CPad::GetPad(1)->GetCross() || KEYDOWN('S'));


	// Delegate the speed to the C implementation
	PanSpeedX = Camera_DebugSpeed(PanSpeedX, KEYDOWN(rsRIGHT) || KEYDOWN('D'), KEYDOWN(rsLEFT) || KEYDOWN('A'));


	// Delegate the speed to the C implementation
	PanSpeedY = Camera_DebugSpeed(PanSpeedY, KEYDOWN(rsUP), KEYDOWN(rsDOWN));


	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
	Source = Source + Front*Speed;

	Up = CVector{ 0.0f, 0.0f, 1.0f };
	CVector Right = CrossProduct(Front, Up);
	Up = CrossProduct(Right, Front);
	Source = Source + Up*PanSpeedY + Right*PanSpeedX;

	if(Source.z < -450.0f)
		Source.z = -450.0f;

	if(CPad::GetPad(1)->GetRightShoulder2JustDown() || KEYJUSTDOWN(rsENTER)){
		if(FindPlayerVehicle())
			FindPlayerVehicle()->Teleport(Source);
		else
			World_Players[World_PlayerInFocus].m_pPed->SetPosition(Source);
	}

	// stay inside sectors
	while(World_GetSectorX(Source.x) > NUMSECTORS_X-5.0f)
		Source.x -= 1.0f;
	while(World_GetSectorX(Source.x) < 5.0f)
		Source.x += 1.0f;
	while(World_GetSectorY(Source.y) > NUMSECTORS_X-5.0f)
		Source.y -= 1.0f;
	while(World_GetSectorY(Source.y) < 5.0f)
		Source.y += 1.0f;
	GetVectorsReadyForRW();

#ifdef FIX_BUGS
	CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_CAMERA);
#else
	CPad::GetPad(0)->DisablePlayerControls = PLAYERCONTROL_CAMERA;
#endif

	if(CPad::GetPad(1)->GetLeftShockJustDown() && gbBigWhiteDebugLightSwitchedOn)
		CShadows::StoreShadowToBeRendered(SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &Source,
			12.0f, 0.0f, 0.0f, -12.0f,
			128, 128, 128, 128, 1000.0f, false, 1.0f, nil, false);

	if(Hud_m_Wants_To_Draw_Hud){
		char str[256];
		sprintf(str, "CamX: %f CamY: %f  CamZ:  %f", Source.x, Source.y, Source.z);
		sprintf(str, "Frontx: %f, Fronty: %f, Frontz: %f ", Front.x, Front.y, Front.z);
		sprintf(str, "Look@: %f, Look@: %f, Look@: %f ", Front.x + Source.x, Front.y + Source.y, Front.z + Source.z);
	}
}
#else
void
CCam::Process_Debug(const CVector&, float, float, float)
{
	static float Speed = 0.0f;
	CVector TargetCoors;

	RwCameraSetNearClipPlane(Scene.camera, DEFAULT_NEAR);
	FOV = DefaultFOV;
	Alpha += DEGTORAD(CPad::GetPad(1)->GetLeftStickY()) / 50.0f;
	Beta  += DEGTORAD(CPad::GetPad(1)->GetLeftStickX()*1.5f) / 19.0f;

	// Delegate the point to look at to the C implementation
	{
		CVector debugTarget;
		Camera_DebugTarget(Alpha, Beta, Source.x, Source.y, Source.z, 3.0f, &debugTarget);
		TargetCoors.x = debugTarget.x;
		TargetCoors.y = debugTarget.y;
		TargetCoors.z = debugTarget.z;
	}

	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 89.5f);

	// Delegate the speed to the C implementation
	Speed = Camera_DebugSpeed(Speed, CPad::GetPad(1)->GetSquare() || Input_GetLeftMouse(), CPad::GetPad(1)->GetCross() || Input_GetRightMouse()); // rouz edit (ChatGPT)

	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
	Source = Source + Front*Speed;

	if(Source.z < -450.0f)
		Source.z = -450.0f;

	if(CPad::GetPad(1)->GetRightShoulder2JustDown()){
		if(FindPlayerVehicle())
			FindPlayerVehicle()->Teleport(Source);
		else
			World_Players[World_PlayerInFocus].m_pPed->SetPosition(Source);
	}

	// stay inside sectors
	while(World_GetSectorX(Source.x) > NUMSECTORS_X-5.0f)
		Source.x -= 1.0f;
	while(World_GetSectorX(Source.x) < 5.0f)
		Source.x += 1.0f;
	while(World_GetSectorY(Source.y) > NUMSECTORS_X-5.0f)
		Source.y -= 1.0f;
	while(World_GetSectorY(Source.y) < 5.0f)
		Source.y += 1.0f;
	GetVectorsReadyForRW();

	if(CPad::GetPad(1)->GetLeftShockJustDown() && gbBigWhiteDebugLightSwitchedOn)
		CShadows::StoreShadowToBeRendered(SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &Source,
			12.0f, 0.0f, 0.0f, -12.0f,
			128, 128, 128, 128, 1000.0f, false, 1.0f, nil, 1.0f);

	if(Hud_m_Wants_To_Draw_Hud){
		char str[256];
		sprintf(str, "CamX: %f CamY: %f  CamZ:  %f", Source.x, Source.y, Source.z);
		sprintf(str, "Frontx: %f, Fronty: %f, Frontz: %f ", Front.x, Front.y, Front.z);
		sprintf(str, "Look@: %f, Look@: %f, Look@: %f ", Front.x + Source.x, Front.y + Source.y, Front.z + Source.z);
	}
}
#endif

#ifdef GTA_SCENE_EDIT
void
CCam::Process_Editor(const CVector&, float, float, float)
{
	static float Speed = 0.0f;
	CVector TargetCoors;

	if(ResetStatics){
		Source = CVector(796.0f, -937.0, 40.0f);
		CamTargetEntity = nil;
	}
	ResetStatics = false;

	RwCameraSetNearClipPlane(Scene.camera, DEFAULT_NEAR);
	FOV = DefaultFOV;
	Alpha += DEGTORAD(CPad::GetPad(1)->GetLeftStickY()) / 50.0f;
	Beta  += DEGTORAD(CPad::GetPad(1)->GetLeftStickX()*1.5f) / 19.0f;

	if(CamTargetEntity && SceneEdit_m_bCameraFollowActor){
		TargetCoors = CamTargetEntity->GetPosition();
	}else if(SceneEdit_m_bRecording){
		// Delegate the point to look at to the C implementation
		{
			CVector debugTarget;
			Camera_DebugTarget(Alpha, Beta, Source.x, Source.y, Source.z, 7.0f, &debugTarget);
			TargetCoors.x = debugTarget.x;
			TargetCoors.y = debugTarget.y;
			TargetCoors.z = debugTarget.z;
		}
	}else
		TargetCoors = SceneEdit_m_vecCamHeading + Source;
	SceneEdit_m_vecCurrentPosition = TargetCoors;
	SceneEdit_m_vecCamHeading = TargetCoors - Source;

	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 89.5f);

	// Delegate the speed to the C implementation
	Speed = Camera_DebugSpeed(Speed, CPad::GetPad(1)->GetSquare() || Input_GetLeftMouse(), CPad::GetPad(1)->GetCross() || Input_GetRightMouse()); // rouz edit (ChatGPT)

	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);
	Source = Source + Front*Speed;

	if(Source.z < -450.0f)
		Source.z = -450.0f;

	if(CPad::GetPad(1)->GetRightShoulder2JustDown()){
		if(FindPlayerVehicle())
			FindPlayerVehicle()->Teleport(Source);
		else
			World_Players[World_PlayerInFocus].m_pPed->SetPosition(Source);
			
	}

	// stay inside sectors
	while(World_GetSectorX(Source.x) > NUMSECTORS_X-5.0f)
		Source.x -= 1.0f;
	while(World_GetSectorX(Source.x) < 5.0f)
		Source.x += 1.0f;
	while(World_GetSectorY(Source.y) > NUMSECTORS_X-5.0f)
		Source.y -= 1.0f;
	while(World_GetSectorY(Source.y) < 5.0f)
		Source.y += 1.0f;
	GetVectorsReadyForRW();

	if(CPad::GetPad(1)->GetLeftShockJustDown() && gbBigWhiteDebugLightSwitchedOn)
		CShadows::StoreShadowToBeRendered(SHADOWTYPE_ADDITIVE, gpShadowExplosionTex, &Source,
			12.0f, 0.0f, 0.0f, -12.0f,
			128, 128, 128, 128, 1000.0f, false, 1.0f, nil, false);

	if(Hud_m_Wants_To_Draw_Hud){
		char str[256];
		sprintf(str, "CamX: %f CamY: %f  CamZ:  %f", Source.x, Source.y, Source.z);
		sprintf(str, "Frontx: %f, Fronty: %f, Frontz: %f ", Front.x, Front.y, Front.z);
		sprintf(str, "Look@: %f, Look@: %f, Look@: %f ", Front.x + Source.x, Front.y + Source.y, Front.z + Source.z);
	}
}
#endif

void
CCam::Process_ModelView(const CVector &CameraTarget, float, float, float)
{
	CVector TargetCoors = CameraTarget;
	float Angle = Atan2(Front.x, Front.y);
	FOV = DefaultFOV;

	Angle += CPad::GetPad(0)->GetLeftStickX()/1280.0f;
	if(Distance < 10.0f)
		Distance += CPad::GetPad(0)->GetLeftStickY()/1000.0f;
	else
		Distance += CPad::GetPad(0)->GetLeftStickY() * ((Distance - 10.0f)/20.0f + 1.0f) / 1000.0f;
#ifdef IMPROVED_CAMERA
	if(Input_GetLeftMouse()){ // rouz edit (ChatGPT)
		Distance += DEGTORAD(Input_GetMouseY()/2.0f); // rouz edit (ChatGPT)
		Angle += DEGTORAD(Input_GetMouseX()/2.0f); // rouz edit (ChatGPT)
	}
#endif
	if(Distance < 1.5f)
		Distance = 1.5f;

	// Delegate the direction of the camera to the C implementation
	CVector frontXYZ;
	Camera_FixedPitchFront(Angle, &frontXYZ);
	Front.x = frontXYZ.x;
	Front.y = frontXYZ.y;
	Front.z = frontXYZ.z;
	Source = CameraTarget - Distance*Front;

	GetVectorsReadyForRW();
}

float DEADCAM_HEIGHT_START = 2.0f;
float DEADCAM_HEIGHT_RATE = 0.04f;
float DEADCAM_WAFT_AMPLITUDE = 2.0f;
float DEADCAM_WAFT_RATE = 600.0f;
float DEADCAM_WAFT_TILT_AMP = -0.35f;

void
CCam::ProcessPedsDeadBaby(void)
{
	CVector TargetCoors;
	CVector CamPos;

	if(TheCamera.pTargetEntity->IsPed())
		((CPed*)TheCamera.pTargetEntity)->m_pedIK.GetComponentPosition(TargetCoors, PED_MID);
	else if(TheCamera.pTargetEntity->IsVehicle()){
		TargetCoors = TheCamera.pTargetEntity->GetPosition();
		TargetCoors.z += TheCamera.pTargetEntity->GetColModel()->boundingBox.max.z;
	}else
		return;

	if(ResetStatics){
		TheCamera.m_uiTimeLastChange = (*Timer_GetTimeInMilliseconds());
		CamPos = TargetCoors;
		CamPos.z += DEADCAM_HEIGHT_START;
		float WaterZ = 0.0f;
		if(WaterLevel_GetWaterLevelNoWaves(TargetCoors.x, TargetCoors.y, TargetCoors.z, &WaterZ)){
			if(WaterZ + 1.5f > CamPos.z)
				CamPos.z = WaterZ + 1.5f;
		}
		// Delegate the right vector of the dead camera to the C implementation
		CVector Right;
		{
			Camera_DeadCamRight(&TheCamera.pTargetEntity->GetForward(), &Right);
		}
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(TargetCoors, CamPos);
		Up = CrossProduct(Right, Front);
		Up.Normalise();
		ResetStatics = false;
	}else{
		CamPos = Source;
		if(World_TestSphereAgainstWorld(CamPos+CVector(0.0f, 0.0f, 0.2f), 0.3f, TheCamera.pTargetEntity, true, true, false, true, false, true) == nil)
			CamPos.z += DEADCAM_HEIGHT_RATE*(*Timer_GetTimeStep());
		// Delegate the right vector of the dead camera to the C implementation
		CVector Right;
		{
			Camera_DeadCamRight(&TheCamera.pTargetEntity->GetForward(), &Right);
		}

		float Time = (*Timer_GetTimeInMilliseconds()) - TheCamera.m_uiTimeLastChange;
		// Delegate the offset of the wafting target to the C implementation
		CVector WaftOffset;
		{
			Camera_DeadCamWaftOffset(Time, DEADCAM_WAFT_AMPLITUDE, DEADCAM_WAFT_RATE, &Right, &WaftOffset);
		}
		CVector WaftPos = TargetCoors + WaftOffset;
		WaftPos.z = CamPos.z;
		CVector WaftFront = WaftPos - CamPos;
		WaftFront.Normalise();
		if(World_TestSphereAgainstWorld(CamPos+0.2f*WaftFront, 0.3f, TheCamera.pTargetEntity, true, true, false, true, false, true) == nil)
			CamPos = WaftPos;

		// Delegate the front vector of the wafting camera to the C implementation
		{
			Camera_DeadCamFront(Time, DEADCAM_WAFT_RATE, DEADCAM_WAFT_TILT_AMP, &Right, &Front);
		}
		Up = CrossProduct(Right, Front);
		Up.Normalise();
	}

	Source = CamPos;
	CVector OrigSource = Source;
	TheCamera.AvoidTheGeometry(OrigSource, TargetCoors, Source, FOV);
	TheCamera.m_bMoveCamToAvoidGeom = false;
}

float ARRESTDIST_BEHIND_COP = 5.0f;
float ARRESTDIST_RIGHTOF_COP = 3.0f;
float ARRESTDIST_ABOVE_COP = 1.4f;	// unused
float ARRESTDIST_MINFROM_PLAYER = 8.0f;
float ARRESTCAM_LAMP_BEST_DIST = 17.0f;
float ARRESTCAM_ROTATION_SPEED = 0.1f;
float ARRESTCAM_ROTATION_UP = 0.05f;
float ARRESTCAM_S_ROTATION_UP = 0.1f;
float ARRESTDIST_ALONG_GROUND = 5.0f;
float ARRESTDIST_SIDE_GROUND = 10.0f;
float ARRESTDIST_ABOVE_GROUND = 0.7f;
float ARRESTCAM_LAMPPOST_ROTATEDIST = 10.0f;
float ARRESTCAM_LAMPPOST_TRANSLATE = 0.1f;

bool
CCam::GetLookAlongGroundPos(CEntity *Target, CPed *Cop, CVector &TargetCoors, CVector &SourceOut)
{
	if(Target == nil || Cop == nil)
		return false;
	// Delegate the position along the ground to the C implementation
	{
		Camera_AlongGroundPos(&TargetCoors, &Cop->GetPosition(), ARRESTDIST_ALONG_GROUND, ARRESTDIST_SIDE_GROUND, &SourceOut);
	}
	bool found = false;
	float ground = World_FindGroundZFor3DCoord(SourceOut.x, SourceOut.y, SourceOut.z, &found);
	if(found)
		SourceOut.z = ground + ARRESTDIST_ABOVE_GROUND;
	return true;
}

bool
CCam::GetLookFromLampPostPos(CEntity *Target, CPed *Cop, CVector &TargetCoors, CVector &SourceOut)
{
	int i;
	int16 NumObjects;
	CEntity *Objects[16];
	CEntity *NearestLampPost = nil;
	World_FindObjectsInRange(TargetCoors, 30.0f, true, &NumObjects, 15, Objects, false, false, false, true, true);
	float NearestDist = 10000.0f;
	for(i = 0; i < NumObjects; i++){
		if(Objects[i]->GetIsStatic() && Objects[i]->GetUp().z > 0.9f && IsLampPost(Objects[i]->GetModelIndex())){
			float Dist = (Objects[i]->GetPosition() - TargetCoors).Magnitude2D();
			if(Abs(ARRESTCAM_LAMP_BEST_DIST - Dist) < NearestDist){
				CVector TestStart = CollisionVector_ToNative(Objects[i]->GetColModel()->boundingBox.max);
				TestStart = Objects[i]->GetMatrix() * TestStart;
				CVector TestEnd = TestStart - TargetCoors;
				TestEnd.Normalise();
				TestEnd += TargetCoors;
				if(World_GetIsLineOfSightClear(TestStart, TestEnd, true, false, false, false, false, true, true)){
					NearestDist = Abs(ARRESTCAM_LAMP_BEST_DIST - Dist);
					NearestLampPost = Objects[i];
					SourceOut = TestStart;
				}
			}
		}
	}
	return NearestLampPost != nil;
}

bool
CCam::GetLookOverShoulderPos(CEntity *Target, CPed *Cop, CVector &TargetCoors, CVector &SourceOut)
{
	if(Target == nil || Cop == nil)
		return false;
	// Delegate the position over the shoulder of the cop to the C implementation
	Camera_OverShoulderPos(&Cop->GetPosition(), &TargetCoors, ARRESTDIST_RIGHTOF_COP, ARRESTDIST_BEHIND_COP, ARRESTDIST_MINFROM_PLAYER, &SourceOut);
	return true;
}

enum {
	ARRESTCAM_OVERSHOULDER = 1,
	ARRESTCAM_ALONGGROUND,
	ARRESTCAM_ALONGGROUND_RIGHT,
	ARRESTCAM_ALONGGROUND_RIGHT_UP,
	ARRESTCAM_ALONGGROUND_LEFT,
	ARRESTCAM_ALONGGROUND_LEFT_UP,
	ARRESTCAM_LAMPPOST,
};

int nUsingWhichCamera;
CPed *pStoredCopPed;

bool
CCam::ProcessArrestCamOne(void)
{
	CVector TargetPos;
	CVector CamSource;
	CPed *cop = nil;
	FOV = 45.0f;
	bool foundPos = false;
	int ArrestModes[5] = { -1, -1, -1, -1, -1 };

	if(ResetStatics){
		CPed *targetPed = (CPed*)TheCamera.pTargetEntity;
		nUsingWhichCamera = 0;
		if(TheCamera.pTargetEntity->IsPed()){
			((CPed*)TheCamera.pTargetEntity)->m_pedIK.GetComponentPosition(TargetPos, PED_MID);
			if(FindPlayerPed() && FindPlayerPed()->m_pArrestingCop)
				cop = FindPlayerPed()->m_pArrestingCop;
			if(cop && General_GetRandomNumberInRangeFloat(0.0f, 1.0f) > 0.5f){
				ArrestModes[0] = ARRESTCAM_OVERSHOULDER;
				ArrestModes[1] = ARRESTCAM_ALONGGROUND;
				ArrestModes[2] = ARRESTCAM_OVERSHOULDER;
				ArrestModes[3] = ARRESTCAM_LAMPPOST;
			}else{
				ArrestModes[0] = ARRESTCAM_ALONGGROUND;
				ArrestModes[1] = ARRESTCAM_OVERSHOULDER;
				ArrestModes[2] = ARRESTCAM_LAMPPOST;
			}
		}else if(TheCamera.pTargetEntity->IsVehicle()){
			CVehicle *targetVehicle = (CVehicle*)TheCamera.pTargetEntity;
			if(targetVehicle->pDriver && targetVehicle->pDriver->IsPlayer()){
				targetPed = targetVehicle->pDriver;
				targetPed->m_pedIK.GetComponentPosition(TargetPos, PED_MID);
			}else{
				targetPed = nil;
				TargetPos = targetVehicle->GetPosition();
			}

			if(FindPlayerPed() && FindPlayerPed()->m_pArrestingCop)
				cop = FindPlayerPed()->m_pArrestingCop;
			if(cop && General_GetRandomNumberInRangeFloat(0.0f, 1.0f) > 0.65f){
				ArrestModes[0] = ARRESTCAM_OVERSHOULDER;
				ArrestModes[1] = ARRESTCAM_LAMPPOST;
				ArrestModes[2] = ARRESTCAM_ALONGGROUND;
				ArrestModes[3] = ARRESTCAM_OVERSHOULDER;
			}else{
				ArrestModes[0] = ARRESTCAM_LAMPPOST;
				ArrestModes[1] = ARRESTCAM_ALONGGROUND;
				ArrestModes[2] = ARRESTCAM_OVERSHOULDER;
			}
		}else
			return false;

		for(int i = 0; nUsingWhichCamera == 0 && i < ARRAY_SIZE(ArrestModes) && ArrestModes[i] > 0; i++){
			switch(ArrestModes[i]){
			case ARRESTCAM_OVERSHOULDER:
				if(cop){
					foundPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, cop, TargetPos, CamSource);
					pStoredCopPed = cop;
					cop = nil;
				}else if(targetPed){
					for(int j = 0; j < targetPed->m_numNearPeds; j++){
						CPed *nearPed = targetPed->m_nearPeds[j];
						if(nearPed->GetPedState() == PED_ARREST_PLAYER)
							foundPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, nearPed, TargetPos, CamSource);
						if(foundPos){
							pStoredCopPed = nearPed;
							break;
						}
					}
				}
				break;
			case ARRESTCAM_ALONGGROUND:
				if(cop){
					foundPos = GetLookAlongGroundPos(TheCamera.pTargetEntity, cop, TargetPos, CamSource);
					pStoredCopPed = cop;
					cop = nil;
				}else if(targetPed){
					for(int j = 0; j < targetPed->m_numNearPeds; j++){
						CPed *nearPed = targetPed->m_nearPeds[j];
						if(nearPed->GetPedState() == PED_ARREST_PLAYER)
							foundPos = GetLookAlongGroundPos(TheCamera.pTargetEntity, nearPed, TargetPos, CamSource);
						if(foundPos){
							pStoredCopPed = nearPed;
							break;
						}
					}
				}
				break;
			case ARRESTCAM_LAMPPOST:
				foundPos = GetLookFromLampPostPos(TheCamera.pTargetEntity, cop, TargetPos, CamSource);
				break;
			}

			if(foundPos){
				if(pStoredCopPed)
					pStoredCopPed->RegisterReference((CEntity**)&pStoredCopPed);
				nUsingWhichCamera = ArrestModes[i];
				if(ArrestModes[i] == ARRESTCAM_ALONGGROUND){
					float rnd = General_GetRandomNumberInRangeFloat(0.0f, 5.0f);
					// Delegate the choice of the variant to the C implementation
					static const int variants[5] = { ARRESTCAM_ALONGGROUND, ARRESTCAM_ALONGGROUND_RIGHT, ARRESTCAM_ALONGGROUND_RIGHT_UP, ARRESTCAM_ALONGGROUND_LEFT, ARRESTCAM_ALONGGROUND_LEFT_UP };
					nUsingWhichCamera = variants[Camera_ArrestVariantIndex(rnd)];
				}
			}else
				pStoredCopPed = nil;
		}

		Source = CamSource;
		CVector OrigSource = Source;
		TheCamera.AvoidTheGeometry(OrigSource, TargetPos, Source, FOV);
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(TargetPos, Source);
		Up = CVector(0.0f, 0.0f, 1.0f);
		// Delegate the right and up vectors of the camera to the C implementation
		CVector rightC;
		CVector UpC;
		{
			Camera_RightAndUp(&Front, &Up, false, &rightC, &UpC);
		}
		CVector Right = CVector(rightC.x, rightC.y, rightC.z);
		Up = CVector(UpC.x, UpC.y, UpC.z);
		if(nUsingWhichCamera != 0)
			ResetStatics = false;
		return true;
	}

	if(TheCamera.pTargetEntity->IsPed()){
		((CPed*)TheCamera.pTargetEntity)->m_pedIK.GetComponentPosition(TargetPos, PED_MID);
	}else if(TheCamera.pTargetEntity->IsVehicle()){
		CPed *driver = ((CVehicle*)TheCamera.pTargetEntity)->pDriver;
		if(driver && driver->IsPlayer())
			driver->m_pedIK.GetComponentPosition(TargetPos, PED_MID);
		else
			TargetPos = TheCamera.pTargetEntity->GetPosition();
	}else
		return false;

	if(nUsingWhichCamera == ARRESTCAM_OVERSHOULDER && pStoredCopPed){
		foundPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, pStoredCopPed, TargetPos, CamSource);
		float newZ = Source.z + ARRESTCAM_S_ROTATION_UP*(*Timer_GetTimeStep());
		if(CamSource.z > newZ)
			CamSource.z = newZ;
	}else if(nUsingWhichCamera >= ARRESTCAM_ALONGGROUND_RIGHT && nUsingWhichCamera <= ARRESTCAM_ALONGGROUND_LEFT_UP){
		CamSource = Source;
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(TargetPos, CamSource);
		Up = CVector(0.0f, 0.0f, 1.0f);
		CVector Right = CrossProduct(Front, Up);
		if(nUsingWhichCamera == ARRESTCAM_ALONGGROUND_LEFT || nUsingWhichCamera == ARRESTCAM_ALONGGROUND_LEFT_UP)
			Right *= -1.0f;
		if(World_TestSphereAgainstWorld(CamSource + 0.5f*Right, 0.4f, TheCamera.pTargetEntity, true, true, false, true, false, true) == nil){
			foundPos = true;
			CamSource += Right*ARRESTCAM_ROTATION_SPEED*(*Timer_GetTimeStep());
			if(nUsingWhichCamera == ARRESTCAM_ALONGGROUND_RIGHT_UP || nUsingWhichCamera == ARRESTCAM_ALONGGROUND_LEFT_UP){
				CamSource.z += ARRESTCAM_ROTATION_UP*(*Timer_GetTimeStep());
			}else{
				bool found = false;
				float ground = World_FindGroundZFor3DCoord(CamSource.x, CamSource.y, CamSource.z, &found);
				if(found)
					CamSource.z = ground + ARRESTDIST_ABOVE_GROUND;
			}
		}
	}else if(nUsingWhichCamera == ARRESTCAM_LAMPPOST){
		CamSource = Source;
		// Delegate the direction of the camera around the lamp post to the C implementation
		{
			Camera_LampPostFront(&TargetPos, &CamSource, ARRESTCAM_LAMPPOST_ROTATEDIST, &Front, &Up);
		}
		if(World_TestSphereAgainstWorld(CamSource + 0.5f*Front, 0.4f, TheCamera.pTargetEntity, true, true, false, true, false, true) == nil){
			foundPos = true;
			CamSource += Front*ARRESTCAM_LAMPPOST_TRANSLATE*(*Timer_GetTimeStep());
		}
	}

	if(foundPos){
		Source = CamSource;
		CVector OrigSource = Source;
		TheCamera.AvoidTheGeometry(OrigSource, TargetPos, Source, FOV);
		// Delegate the direction to the C implementation
		Front = NormalisedDifference(TargetPos, Source);
		Up = CVector(0.0f, 0.0f, 1.0f);
		// Delegate the right and up vectors of the camera to the C implementation
		CVector rightC;
		CVector UpC;
		{
			Camera_RightAndUp(&Front, &Up, false, &rightC, &UpC);
		}
		CVector Right = CVector(rightC.x, rightC.y, rightC.z);
		Up = CVector(UpC.x, UpC.y, UpC.z);
	}else{
		CVector OrigSource = Source;
		TheCamera.AvoidTheGeometry(OrigSource, TargetPos, Source, FOV);
	}

	return true;
}

bool
CCam::ProcessArrestCamTwo(void)
{
	CPed *player = World_Players[World_PlayerInFocus].m_pPed;
	if(!ResetStatics)
		return true;
	ResetStatics = false;

	CVector TargetCoors, ToCamera;
	float BetaOffset;
	float SourceX, SourceY;
	if(&TheCamera.Cams[TheCamera.ActiveCam] == this){
		SourceX = TheCamera.Cams[(TheCamera.ActiveCam + 1) % 2].Source.x;
		SourceY = TheCamera.Cams[(TheCamera.ActiveCam + 1) % 2].Source.y;
	}else{
		SourceX = TheCamera.Cams[TheCamera.ActiveCam].Source.x;
		SourceY = TheCamera.Cams[TheCamera.ActiveCam].Source.y;
	}

	for(int i = 0; i <= 1; i++){
		int Dir = i == 0 ? 1 : -1;

		FOV = 60.0f;
		TargetCoors = player->GetPosition();
		Beta = General_GetATanOfXY(TargetCoors.x-SourceX, TargetCoors.y-SourceY);
		BetaOffset = DEGTORAD(Dir*80);
		// Delegate the offset on the orbit to the C implementation
		{
			float orbitX, orbitY;
			Camera_OrbitOffset(11.5f, Beta+BetaOffset, &orbitX, &orbitY);
			Source = TargetCoors + CVector(orbitX, orbitY, 11.5f*0.0f);
		}

		// Delegate the direction to the C implementation
		ToCamera = NormalisedDifference(Source, TargetCoors);
		TargetCoors.x += 0.4f*ToCamera.x;
		TargetCoors.y += 0.4f*ToCamera.y;
		if(World_GetIsLineOfSightClear(Source, TargetCoors, true, true, false, true, false, true, true)){
			Source.z += 5.5f;
			TargetCoors += CVector(-0.8f*ToCamera.x, -0.8f*ToCamera.y, 2.2f);
			m_cvecTargetCoorsForFudgeInter = TargetCoors;
			Front = TargetCoors - Source;
			ResetStatics = false;
			GetVectorsReadyForRW();
			return true;
		}
	}
	return false;
}


#ifdef FREE_CAM
void
CCam::Process_FollowPed_Rotation(const CVector &CameraTarget, float TargetOrientation, float, float)
{
	FOV = DefaultFOV;

	const float MinDist = 2.0f;
	const float MaxDist = 2.0f + TheCamera.m_fPedZoomValueSmooth;
	const float BaseOffset = 0.75f;	// base height of camera above target

	CVector TargetCoors = CameraTarget;

	TargetCoors.z += m_fSyphonModeTargetZOffSet;
	TargetCoors = DoAverageOnVector(TargetCoors);
	TargetCoors.z += BaseOffset;	// add offset so alpha evens out to 0
//	TargetCoors.z += m_fRoadOffSet;

	CVector Dist = Source - TargetCoors;
	CVector ToCam;

	bool Shooting = false;
	if(((CPed*)CamTargetEntity)->GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED)
		if(CPad::GetPad(0)->GetWeapon())
			Shooting = true;
	if(((CPed*)CamTargetEntity)->GetWeapon()->m_eWeaponType == WEAPONTYPE_DETONATOR ||
	   ((CPed*)CamTargetEntity)->GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT)
		Shooting = false;


	if(ResetStatics){
		// Coming out of top down here probably
		// so keep Beta, reset alpha and calculate vectors
		Beta = General_GetATanOfXY(Dist.x, Dist.y);
		Alpha = 0.0f;

		// Delegate the direction to the C implementation
		{
			CVector sphericalDir;
			Camera_SphericalDirection(Alpha, Beta, &sphericalDir);
			Dist = MaxDist*CVector(sphericalDir.x, sphericalDir.y, sphericalDir.z);
		}
		Source = TargetCoors + Dist;

		ResetStatics = false;
	}

	// Drag the camera along at the look-down offset
	// Delegate the distance of the camera to the C implementation
	CVector distC(Dist.x, Dist.y, Dist.z);
	float CamDist = Camera_DragDistance(&distC, MinDist, MaxDist);
	Dist = CVector(distC.x, distC.y, distC.z);

	// Beta = 0 is looking east, HALFPI is north, &c.
	// Alpha positive is looking up
	float GroundDist = Dist.Magnitude2D();
	Beta = General_GetATanOfXY(-Dist.x, -Dist.y);
	Alpha = General_GetATanOfXY(GroundDist, -Dist.z);
	// Delegate the angle wrapping to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);
	CameraMath_MakeAngleLessThan180(&Alpha);

	// Look around
	bool UseMouse = false;
	float MouseX = Input_GetMouseX(); // rouz edit (ChatGPT)
	float MouseY = Input_GetMouseY(); // rouz edit (ChatGPT)
	float LookLeftRight, LookUpDown;
/*
	if((MouseX != 0.0f || MouseY != 0.0f) && !CPad::GetPad(0)->ArePlayerControlsDisabled()){
		UseMouse = true;
		LookLeftRight = -2.5f*MouseX;
		LookUpDown = 4.0f*MouseY;
	}else
*/
	{
		LookLeftRight = -CPad::GetPad(0)->LookAroundLeftRight();
		LookUpDown = CPad::GetPad(0)->LookAroundUpDown();
	}
	float AlphaOffset, BetaOffset;
	if(UseMouse){
		BetaOffset = LookLeftRight * TheCamera.m_fMouseAccelHorzntl * FOV/80.0f;
		AlphaOffset = LookUpDown * TheCamera.m_fMouseAccelVertical * FOV/80.0f;
	}else{
		BetaOffset = LookLeftRight * fStickSens * (1.0f/20.0f) * FOV/80.0f * (*Timer_GetTimeStep());
		AlphaOffset = LookUpDown * fStickSens * (0.6f/20.0f) * FOV/80.0f * (*Timer_GetTimeStep());
	}

	// Stop centering once stick has been touched
	if(BetaOffset)
		Rotating = false;

	Beta += BetaOffset;
	Alpha += AlphaOffset;
	// Delegate the angle wrapping to the C implementation
	CameraMath_MakeAngleLessThan180(&Beta);
	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlpha(Alpha, 45.0f);


	float BetaDiff = TargetOrientation+PI - Beta;
	// Delegate the angle wrapping to the C implementation
	CameraMath_MakeAngleLessThan180(&BetaDiff);
	float TargetAlpha = Alpha;
	// 12deg to account for our little height offset. we're not working on the true alpha here
	const float AlphaLimitUp = DEGTORAD(15.0f) + DEGTORAD(12.0f);
	const float AlphaLimitDown = -DEGTORAD(15.0f) + DEGTORAD(12.0f);
	if(General_AbsBelow(BetaDiff, DEGTORAD(25.0f)) && ((CPed*)CamTargetEntity)->GetMoveSpeed().Magnitude2D() > 0.01f){
		// Limit alpha when player is walking towards camera
		if(TargetAlpha > AlphaLimitUp) TargetAlpha = AlphaLimitUp;
		if(TargetAlpha < AlphaLimitDown) TargetAlpha = AlphaLimitDown;
	}

	WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.2f, 0.1f, true);

	if(CPad::GetPad(0)->ForceCameraBehindPlayer() || Shooting){
		m_fTargetBeta = TargetOrientation;
		Rotating = true;
	}

	if(Rotating){
		WellBufferMe(m_fTargetBeta, &Beta, &BetaSpeed, 0.1f, 0.06f, true);
		float DeltaBeta = m_fTargetBeta - Beta;
		// Delegate the wrap of the angle to the C implementation
		CameraMath_MakeAngleLessThan180(&DeltaBeta);
		if(General_AbsBelow(DeltaBeta, 0.06f))
			Rotating = false;
	}

	if(TheCamera.m_bUseTransitionBeta)
		Beta = Camera_TransitionBetaAngle(m_fTransitionBeta);

	if(TheCamera.m_bUseTransitionBeta)
		Beta = Camera_TransitionBetaAngle(m_fTransitionBeta);

	// Delegate the direction to the C implementation
	{
		Camera_SphericalDirection(Alpha, Beta, &Front);
	}
	Source = TargetCoors - Front*CamDist;
	TargetCoors.z -= BaseOffset;	// now get back to the real target coors again

	m_cvecTargetCoorsForFudgeInter = TargetCoors;


	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, Source);



	/*
	 * Handle collisions - taken from FollowPedWithMouse
	 */

	CEntity *entity;
	CColPoint colPoint;
	// Clip Source and fix near clip
	World_pIgnoreEntity = CamTargetEntity;
	entity = nil;
	if(World_ProcessLineOfSight(TargetCoors, Source, colPoint, entity, true, true, true, true, false, false, true)){
		float PedColDist = (TargetCoors - CollisionVector_ToNative(colPoint.point)).Magnitude();
		float ColCamDist = CamDist - PedColDist;
		if(entity->IsPed() && ColCamDist > DEFAULT_NEAR + 0.1f){
			// Ped in the way but not clipping through
			if(World_ProcessLineOfSight(CollisionVector_ToNative(colPoint.point), Source, colPoint, entity, true, true, true, true, false, false, true)){
				PedColDist = (TargetCoors - CollisionVector_ToNative(colPoint.point)).Magnitude();
				Source = CollisionVector_ToNative(colPoint.point);
				if(PedColDist < DEFAULT_NEAR + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipNearPed(PedColDist));
			}else{
				RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipPedInWay(ColCamDist, DEFAULT_NEAR));
			}
		}else{
			Source = CollisionVector_ToNative(colPoint.point);
			if(PedColDist < DEFAULT_NEAR + 0.3f)
				RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipNearPed(PedColDist));
		}
	}
	World_pIgnoreEntity = nil;

	// Delegate the width of the view plane to the C implementation shared with the avoidance of the geometry
	float ViewPlaneWidth = Camera_AvoidViewPlaneWidth(FOV, Draw_CalculateAspectRatio(), fTweakFOV);
	float Near = RwCameraGetNearClipPlane(Scene.camera);
	float radius = ViewPlaneWidth*Near;
	entity = World_TestSphereAgainstWorld(Source + Front*Near, radius, nil, true, true, false, true, false, false);
	int i = 0;
	while(entity){
		CVector CamToCol = CollisionVector_ToNative(gaTempSphereColPoints[0].point) - Source;
		float frontDist = DotProduct(CamToCol, Front);
		float dist = (CamToCol - Front*frontDist).Magnitude() / ViewPlaneWidth;

		// Try to decrease near clip
		dist = Camera_NearClipFromCollision(Near, dist);
		if(dist < Near)
			RwCameraSetNearClipPlane(Scene.camera, dist);

		// Move forward a bit
		if(dist == 0.1f)
			Source += (TargetCoors - Source)*0.3f;

		// Keep testing
		Near = RwCameraGetNearClipPlane(Scene.camera);
		radius = ViewPlaneWidth*Near;
		entity = World_TestSphereAgainstWorld(Source + Front*Near, radius, nil, true, true, false, true, false, false);

		i++;
		if(i > 5)
			entity = nil;
	}

	GetVectorsReadyForRW();
}

// LCS cam hehe
void
CCam::Process_FollowCar_SA(const CVector& CameraTarget, float TargetOrientation, float, float)
{
	// Missing things on III CCam
	static CVector m_aTargetHistoryPosOne;
	static CVector m_aTargetHistoryPosTwo;
	static CVector m_aTargetHistoryPosThree;
	static int m_nCurrentHistoryPoints = 0;
	static float lastBeta = -9999.0f;
	static float lastAlpha = -9999.0f;
	static float stepsLeftToChangeBetaByMouse;
	static float dontCollideWithCars;
	static bool alphaCorrected;
	static float heightIncreaseMult;

	if (!CamTargetEntity->IsVehicle())
		return;

	CVehicle* car = (CVehicle*)CamTargetEntity;
	CVector TargetCoors = CameraTarget;
	uint8 camSetArrPos = 0;

	// We may need those later
	bool isPlane = car->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE;
	bool isHeli = car->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI;
	bool isBike = car->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE;
	bool isCar = car->IsCar() && !isPlane && !isHeli && !isBike;

	CPad* pad = CPad::GetPad(0);

	// Next direction is non-existent in III
	uint8 nextDirectionIsForward = !(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight()) &&
		DirectionWasLooking == LOOKING_FORWARD;

	if (car->GetModelIndex() == MI_FIRETRUCK) {
		camSetArrPos = 7;
	} else if (car->GetModelIndex() == MI_RCBANDIT || car->GetModelIndex() == MI_RCBARON) {
		camSetArrPos = 5;
	} else if (car->GetModelIndex() == MI_RCGOBLIN || car->GetModelIndex() == MI_RCRAIDER) {
		camSetArrPos = 6;
	} else if (car->IsBoat()) {
		camSetArrPos = 4;
	} else if (isBike) {
		camSetArrPos = 1;
	} else if (isPlane) {
		camSetArrPos = 3;
	} else if (isHeli) {
		camSetArrPos = 2;
	}

	// LCS one but index 1(firetruck) moved to last
	float CARCAM_SET[][15] = {
		{1.3f, 1.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, DEGTORAD(45.0f), DEGTORAD(89.0f)}, // cars
		{1.1f, 1.0f, 0.1f, 10.0f, 11.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.75f, DEGTORAD(45.0f), DEGTORAD(89.0f)}, // bike
		{1.1f, 1.0f, 0.2f, 10.0f, 15.0f, 0.05f, 0.05f, 0.0f, 0.9f, 0.05f, 0.01f, 0.05f, 1.0f, DEGTORAD(10.0f), DEGTORAD(70.0f)}, // heli (SA values)
		{1.1f, 3.5f, 0.2f, 10.0f, 25.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, DEGTORAD(89.0f), DEGTORAD(89.0f)}, // plane (SA values)
		{0.9f, 1.0f, 0.1f, 10.0f, 15.0f, 0.5f, 1.0f, 0.0f, 0.9f, 0.05f, 0.005f, 0.05f, 1.0f, -0.2f, DEGTORAD(70.0f)}, // boat
		{1.1f, 1.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, DEGTORAD(45.0f), DEGTORAD(89.0f)}, // rc cars
		{1.1f, 1.0f, 0.2f, 10.0f, 5.0f, 0.5f, 1.0f, 1.0f, 0.75f, 0.1f, 0.005f, 0.2f, 1.0f, DEGTORAD(20.0f), DEGTORAD(70.0f)}, // rc heli/planes
		{1.3f, 1.0f, 0.4f, 10.0f, 15.0f, 0.5f, 1.0f, 1.0f, 0.85f, 0.2f, 0.075f, 0.05f, 0.8f, -0.18f, DEGTORAD(40.0f)}, // firetruck...
	};

	// RC Heli/planes use same alpha values with heli/planes (LCS firetruck will fallback to 0)
	uint8 alphaArrPos = (camSetArrPos > 4 ? (isPlane ? 3 : (isHeli ? 2 : 0)) : camSetArrPos);
	float zoomModeAlphaOffset = 0.0f;
	static float ZmOneAlphaOffsetLCS[] = { 0.12f, 0.08f, 0.15f, 0.08f, 0.08f };
	static float ZmTwoAlphaOffsetLCS[] = { 0.1f, 0.08f, 0.3f, 0.08f, 0.08f };
	static float ZmThreeAlphaOffsetLCS[] = { 0.065f, 0.05f, 0.15f, 0.06f, 0.08f };

	if (isHeli && car->GetStatus() == STATUS_PLAYER_REMOTE)
		zoomModeAlphaOffset = ZmTwoAlphaOffsetLCS[alphaArrPos];
	else {
		switch ((int)TheCamera.CarZoomIndicator) {
			// near
		case CAM_ZOOM_1:
			zoomModeAlphaOffset = ZmOneAlphaOffsetLCS[alphaArrPos];
			break;
			// mid
		case CAM_ZOOM_2:
			zoomModeAlphaOffset = ZmTwoAlphaOffsetLCS[alphaArrPos];
			break;
			// far
		case CAM_ZOOM_3:
			zoomModeAlphaOffset = ZmThreeAlphaOffsetLCS[alphaArrPos];
			break;
		default:
			break;
		}
	}

	CColModel* carCol = (CColModel*)car->GetColModel();
	float colMaxZ = carCol->boundingBox.max.z;  // As opposed to LCS and SA, VC does this: carCol->boundingBox.max.z - carCol->boundingBox.min.z;
	float approxCarLength = 2.0f * Abs(carCol->boundingBox.min.y); // SA taxi min.y = -2.95, max.z = 0.883502f

	// Delegate the distances of the car camera to the C implementation
	// (RC heli and plane: taken from VC CCam::Cam_On_A_String_Unobscured, otherwise we would see the world from the inside of the RC Goblin/Raider)
	float newDistance, minDistForThisCar, nextDistance, targetZAdd;
	{
		const float row[5] = { CARCAM_SET[camSetArrPos][0], CARCAM_SET[camSetArrPos][1], CARCAM_SET[camSetArrPos][2], CARCAM_SET[camSetArrPos][3], CARCAM_SET[camSetArrPos][4] };
		int rcKind = 0;
		if (car->m_modelIndex == MI_RCRAIDER || car->m_modelIndex == MI_RCGOBLIN)
			rcKind = 1;
		else if (car->m_modelIndex == MI_RCBARON)
			rcKind = 2;
		bool staysOutside = !isHeli || car->GetStatus() == STATUS_PLAYER_REMOTE;
		Camera_CarCamDistances(row, TheCamera.CarZoomValueSmooth, approxCarLength, colMaxZ, rcKind, INIT_RC_HELI_HORI_EXTRA, INIT_RC_PLANE_HORI_EXTRA,
			staysOutside, car->m_modelIndex == MI_RCGOBLIN, TheCamera.CarZoomIndicator == CAM_ZOOM_1 && (camSetArrPos < 2 || camSetArrPos == 7),
			&zoomModeAlphaOffset, &targetZAdd, &newDistance, &minDistForThisCar, &nextDistance);
		if (staysOutside) {
			if (targetZAdd > 0.0f)
				TargetCoors.z += targetZAdd;
		} else
			// 0.6f = fTestShiftHeliCamTarget
			TargetCoors += 0.6f * car->GetUp() * colMaxZ;
	}

	CA_MAX_DISTANCE = newDistance;
	CA_MIN_DISTANCE = 3.5f;

	if (ResetStatics) {
		FOV = DefaultFOV;
	} else {
		// Delegate the field of view to the C implementation (0.4f: CAR_FOV_START_SPEED, 0.98f: CAR_FOV_FADE_MULT)
		FOV = Camera_CarFOV(FOV, DefaultFOV, isCar || isBike, DotProduct(car->GetForward(), car->m_vecMoveSpeed), (*Timer_GetTimeStep()));
	}

	// WORKAROUND: I still don't know how looking behind works (m_bCamDirectlyInFront is unused in III, they seem to use m_bUseTransitionBeta)
	if (pad->GetLookBehindForCar())
		if (DirectionWasLooking == LOOKING_FORWARD || !LookingBehind)
			TheCamera.m_bCamDirectlyInFront = true;

	// Taken from RotCamIfInFrontCar, because we don't call it anymore
	if (!(pad->GetLookBehindForCar() || pad->GetLookBehindForPed() || pad->GetLookLeft() || pad->GetLookRight()))
		if (DirectionWasLooking != LOOKING_FORWARD)
			TheCamera.m_bCamDirectlyBehind = true;

	// Called when we just entered the car, just started to look behind or returned back from looking left, right or behind
	if (ResetStatics || TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront) {
		ResetStatics = false;
		Rotating = false;
		m_bCollisionChecksOn = true;

		if (!TheCamera.m_bJustCameOutOfGarage) {
			Alpha = 0.0f;
			Beta = car->GetForward().Heading() - HALFPI;
			if (TheCamera.m_bCamDirectlyInFront) {
				Beta += PI;
			}
		}

		BetaSpeed = 0.0;
		AlphaSpeed = 0.0;
		Distance = 1000.0;

		// Delegate the direction to the C implementation
		{
			CVector frontXYZ;
			Camera_FrontFromAngles(Alpha, Beta, &frontXYZ);
			Front.x = frontXYZ.x;
			Front.y = frontXYZ.y;
			Front.z = frontXYZ.z;
		}

		m_aTargetHistoryPosOne = TargetCoors - nextDistance * Front;

		m_aTargetHistoryPosTwo = TargetCoors - newDistance * Front;

		m_nCurrentHistoryPoints = 0;
		if (!TheCamera.m_bJustCameOutOfGarage)
			Alpha = -zoomModeAlphaOffset;
	}

	// Delegate the direction to the C implementation
	Front = NormalisedDifference(TargetCoors, m_aTargetHistoryPosOne);

	// Code that makes cam rotate around the car
	// Delegate the target angle of the camera to the C implementation
	float betaChangeSideSpeed = (car->m_vecMoveSpeed - DotProduct(car->m_vecMoveSpeed, Front) * Front).Magnitude();
	float targetBeta = Camera_FollowCarTargetBeta(Front.Heading(), car->m_vecMoveSpeed.Magnitude2D(), car->m_vecMoveSpeed.Heading(), Beta,
		(*Timer_GetTimeStep()), CARCAM_SET[camSetArrPos][10], CARCAM_SET[camSetArrPos][11], betaChangeSideSpeed);

	float carPosChange = (TargetCoors - m_aTargetHistoryPosTwo).Magnitude();
	if (carPosChange < newDistance && newDistance > minDistForThisCar) {
		newDistance = Max(minDistForThisCar, carPosChange);
	}
	float maxAlphaAllowed = CARCAM_SET[camSetArrPos][13];
	// rouz edit (removal of a block of code)

	// Delegate the target angle and the blend of the vertical angle to the C implementation
	float targetAlpha = Camera_FollowCarTargetAlpha(Front.z, zoomModeAlphaOffset, CARCAM_SET[camSetArrPos][14], maxAlphaAllowed);
	float targetAlphaBlendAmount = Camera_FollowCarAlphaBlend(targetAlpha, Alpha, CARCAM_SET[camSetArrPos][5], CARCAM_SET[camSetArrPos][6], (*Timer_GetTimeStep()));

	// Using GetCarGun(LR/UD) will give us same unprocessed RightStick value as SA
	float stickX = -(pad->GetCarGunLeftRight());
	float stickY = -pad->GetCarGunUpDown();

	// rouz edit, I always hated this feature
	if (rouz.car_no_cam_move_by_keyboard)
		stickX = stickY = 0.f;

	// In SA this checks for m_bUseMouse3rdPerson so num2 / num8 do not move camera
	// when Keyboard & Mouse controls are used. To make it work better with III/VC, check for actual pad state instead
	if (!Input_affectedByController && !isCar) // rouz edit (ChatGPT)
		stickY = 0.0f;
	else if (Input_invertPadLook) // rouz edit (ChatGPT)
		stickY = -stickY;

	// Delegate the movement from the stick to the C implementation
	float xMovement = Camera_StickMovement(stickX, FOV, 5.f);
	float yMovement = Camera_StickMovement(stickY, FOV, 3.f);

	bool correctAlpha = true;
	//	if (SA checks if we aren't in work car, why?) {
	if (!isCar || car->GetModelIndex() != MI_VOODOO) {
		correctAlpha = false;
	}
	else {
		xMovement = 0.0f;
		yMovement = 0.0f;
	}
	//	} else
	//		yMovement = 0.0;

	if (!nextDirectionIsForward) {
		yMovement = 0.0f;
		xMovement = 0.0f;
	}

	if (camSetArrPos == 0 || camSetArrPos == 7) {
		// This is not working on cars as SA
		// Because III/VC doesn't have any buttons tied to LeftStick if you're not in Classic Configuration, using Dodo or using GInput/Pad, so :shrug:
		if (General_AbsAbove(pad->GetSteeringUpDown(), 120.0f)) {
			if (car->pDriver && car->pDriver->m_objective != OBJECTIVE_LEAVE_CAR) {
				yMovement += Camera_StickMovement(pad->GetSteeringUpDown(), FOV, 3.f) * 0.5;
			}
		}
	}

	if (yMovement > 0.0)
		yMovement = yMovement * 0.5;

	bool mouseChangesBeta = false;

	// FIX: Disable mouse movement in drive-by, it's buggy. Original SA bug.
	if (/*bFreeMouseCam &&*/ CCamera::m_bUseMouse3rdPerson && !pad->ArePlayerControlsDisabled() && nextDirectionIsForward) {
		float mouseY = Input_GetMouseY() * 2.0f; // rouz edit (ChatGPT)
		float mouseX = Input_GetMouseX() * -2.0f; // rouz edit (ChatGPT)

		// If you want an ability to toggle free cam while steering with mouse, you can add an OR after DisableMouseSteering.
		// There was a pad->NewState.m_bVehicleMouseLook in SA, which doesn't exists in III.

		if ((mouseX != 0.0 || mouseY != 0.0) && (CVehicle::m_bDisableMouseSteering)) {
			yMovement = mouseY * FOV / 80.0f * TheCamera.m_fMouseAccelHorzntl; // Same as SA, horizontal sensitivity.
			BetaSpeed = 0.0;
			AlphaSpeed = 0.0;
			xMovement = mouseX * FOV / 80.0f * TheCamera.m_fMouseAccelHorzntl;
			targetAlpha = Alpha;
			stepsLeftToChangeBetaByMouse = 1.0f * 50.0f;
			mouseChangesBeta = true;
		} else if (stepsLeftToChangeBetaByMouse > 0.0f) {
			// Finish rotation by decreasing speed when we stopped moving mouse
			BetaSpeed = 0.0;
			AlphaSpeed = 0.0;
			yMovement = 0.0;
			xMovement = 0.0;
			targetAlpha = Alpha;
			// Delegate the steps left to the C implementation
			stepsLeftToChangeBetaByMouse = Camera_StepsLeftAfterTimeStep(stepsLeftToChangeBetaByMouse, (*Timer_GetTimeStep()));
			mouseChangesBeta = true;
		}
	}

	if (correctAlpha) {
		if (nPreviousMode != MODE_CAM_ON_A_STRING)
			alphaCorrected = false;

		if (!alphaCorrected && General_AbsAbove(zoomModeAlphaOffset + Alpha, 0.05f)) {
			yMovement = (-zoomModeAlphaOffset - Alpha) * 0.05f;
		} else
			alphaCorrected = true;
	}
	float alphaSpeedFromStickY = yMovement * CARCAM_SET[camSetArrPos][12];
	float betaSpeedFromStickX = xMovement * CARCAM_SET[camSetArrPos][12];

	float newAngleSpeedMaxBlendAmount = CARCAM_SET[camSetArrPos][9];
	float angleChangeStep = Pow(CARCAM_SET[camSetArrPos][8], (*Timer_GetTimeStep()));
	float targetBetaWithStickBlendAmount = betaSpeedFromStickX + (targetBeta - Beta) / Camera_BetaStickBlendDivisor((*Timer_GetTimeStep()));

	// Delegate the limits of the blend amount to the C implementation, the limit is never negative
	targetBetaWithStickBlendAmount = Camera_ClampBetween(targetBetaWithStickBlendAmount, -newAngleSpeedMaxBlendAmount, newAngleSpeedMaxBlendAmount);

	float angleChangeStepLeft = 1.0f - angleChangeStep;
	// Delegate the blend of the speed to the C implementation
	BetaSpeed = Camera_BlendBetaSpeed(targetBetaWithStickBlendAmount, angleChangeStep, BetaSpeed);

	float betaChangePerFrame;
	if (mouseChangesBeta)
		betaChangePerFrame = betaSpeedFromStickX;
	else
		betaChangePerFrame = (*Timer_GetTimeStep()) * BetaSpeed;
	Beta = betaChangePerFrame + Beta;

	if (TheCamera.m_bJustCameOutOfGarage) {
		float invHeading = Atan2(Front.y, Front.x);
		if (invHeading < 0.0f)
			invHeading += TWOPI;

		Beta = invHeading + PI;
	}

	Beta = General_LimitRadianAngle(Beta);
	if (Beta < 0.0f)
		Beta += TWOPI;

	if ((camSetArrPos <= 1 || camSetArrPos == 7) && targetAlpha < Alpha && carPosChange >= newDistance) {
		if (isCar && ((CAutomobile*)car)->m_nWheelsOnGround > 1 ||
			isBike && ((CBike*)car)->m_nWheelsOnGround > 1)
			alphaSpeedFromStickY += (targetAlpha - Alpha) * 0.075f;
	}

	AlphaSpeed = angleChangeStepLeft * alphaSpeedFromStickY + angleChangeStep * AlphaSpeed;
	float maxAlphaSpeed = newAngleSpeedMaxBlendAmount;
	if (alphaSpeedFromStickY > 0.0f)
		maxAlphaSpeed = maxAlphaSpeed * 0.5;

	// Delegate the limits of the speed to the C implementation
	AlphaSpeed = Camera_ClampBetween(AlphaSpeed, -maxAlphaSpeed, maxAlphaSpeed);

	if (General_AbsBelow(AlphaSpeed, 0.0001f))
		AlphaSpeed = 0.0f;

		float alphaWithSpeedAccounted;
		if (mouseChangesBeta) {
			alphaWithSpeedAccounted = alphaSpeedFromStickY + targetAlpha;
				Alpha += alphaSpeedFromStickY;
		} else {
			alphaWithSpeedAccounted = (*Timer_GetTimeStep()) * AlphaSpeed + targetAlpha;
			Alpha += targetAlphaBlendAmount;
		}

	// Delegate the limits of the angle to the C implementation
	Alpha = Camera_ClampAlphaStop(Alpha, -CARCAM_SET[camSetArrPos][14], maxAlphaAllowed, &AlphaSpeed);

	// Prevent unsignificant angle changes
	if (General_AbsDiffBelow(lastAlpha, Alpha, 0.0001f))
		Alpha = lastAlpha;

	lastAlpha = Alpha;

	if (General_AbsDiffBelow(lastBeta, Beta, 0.0001f))
		Beta = lastBeta;

	lastBeta = Beta;

	// Delegate the direction to the C implementation
	{
		CVector frontXYZ;
		Camera_FrontFromAngles(Alpha, Beta, &frontXYZ);
		Front.x = frontXYZ.x;
		Front.y = frontXYZ.y;
		Front.z = frontXYZ.z;
	}
	GetVectorsReadyForRW();
	TheCamera.m_bCamDirectlyBehind = false;
	TheCamera.m_bCamDirectlyInFront = false;

	Source = TargetCoors - newDistance * Front;

	m_cvecTargetCoorsForFudgeInter = TargetCoors;
	m_aTargetHistoryPosThree = m_aTargetHistoryPosOne;
	float nextAlpha = alphaWithSpeedAccounted + zoomModeAlphaOffset;
	// Delegate the positions behind the target to the C implementation
	{
		CVector historyTarget(TargetCoors.x, TargetCoors.y, TargetCoors.z);
		CVector historyPos;
		Camera_TargetHistoryPos(&historyTarget, Beta, nextAlpha, nextDistance, &historyPos);
		m_aTargetHistoryPosOne.x = historyPos.x;
		m_aTargetHistoryPosOne.y = historyPos.y;
		m_aTargetHistoryPosOne.z = historyPos.z;
		Camera_TargetHistoryPos(&historyTarget, Beta, nextAlpha, newDistance, &historyPos);
		m_aTargetHistoryPosTwo.x = historyPos.x;
		m_aTargetHistoryPosTwo.y = historyPos.y;
		m_aTargetHistoryPosTwo.z = historyPos.z;
	}

	// SA calls SetColVarsVehicle in here
	if (nextDirectionIsForward) {

		// LCS uses exactly the same collision code as FollowPedWithMouse, so we will do so.

		// This is only in LCS!
		float timestepFactor = Pow(0.99f, (*Timer_GetTimeStep()));
		dontCollideWithCars = (timestepFactor * dontCollideWithCars) + ((1.0f - timestepFactor) * car->m_vecMoveSpeed.Magnitude());

		// Our addition
#define IS_TRAFFIC_LIGHT(ent) (ent->IsObject() && IsLightObject(ent->GetModelIndex()))

		// Clip Source and fix near clip
		CColPoint colPoint;
		CEntity* entity;
		World_pIgnoreEntity = CamTargetEntity;
		if(World_ProcessLineOfSight(TargetCoors, Source, colPoint, entity, true, dontCollideWithCars < 0.1f, false, true, false, true, true) && !IS_TRAFFIC_LIGHT(entity)){
			float PedColDist = (TargetCoors - CollisionVector_ToNative(colPoint.point)).Magnitude();
			float ColCamDist = newDistance - PedColDist;
			if(entity->IsPed() && ColCamDist > DEFAULT_NEAR + 0.1f){
				// Ped in the way but not clipping through
				if(World_ProcessLineOfSight(CollisionVector_ToNative(colPoint.point), Source, colPoint, entity, true, dontCollideWithCars < 0.1f, false, true, false, true, true) || IS_TRAFFIC_LIGHT(entity)){
					PedColDist = (TargetCoors - CollisionVector_ToNative(colPoint.point)).Magnitude();
					Source = CollisionVector_ToNative(colPoint.point);
					if(PedColDist < DEFAULT_NEAR + 0.3f)
						RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipNearPed(PedColDist));
				}else{
					RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipPedInWay(ColCamDist, DEFAULT_NEAR));
				}
			}else{
				Source = CollisionVector_ToNative(colPoint.point);
				if(PedColDist < DEFAULT_NEAR + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, Camera_NearClipNearPed(PedColDist));
			}
		}
		
		World_pIgnoreEntity = nil;

		// If we're seeing blue hell due to camera intersects some surface, fix it.
		// SA and LCS have this unrolled.

		// Delegate the width of the view plane to the C implementation shared with the avoidance of the geometry
		float ViewPlaneWidth = Camera_AvoidViewPlaneWidth(FOV, Draw_CalculateAspectRatio(), fTweakFOV);
		float Near = RwCameraGetNearClipPlane(Scene.camera);
		float radius = ViewPlaneWidth*Near;
		entity = World_TestSphereAgainstWorld(Source + Front*Near, radius, nil, true, true, false, true, false, true);
		int i = 0;
		while(entity){

			if (IS_TRAFFIC_LIGHT(entity))
				break;

			CVector CamToCol = CollisionVector_ToNative(gaTempSphereColPoints[0].point) - Source;
			float frontDist = DotProduct(CamToCol, Front);
			float dist = (CamToCol - Front*frontDist).Magnitude() / ViewPlaneWidth;

			// Try to decrease near clip
			dist = Camera_NearClipFromCollision(Near, dist);
			if(dist < Near)
				RwCameraSetNearClipPlane(Scene.camera, dist);

			// Move forward a bit
			if(dist == 0.1f)
				Source += (TargetCoors - Source)*0.3f;

			// Keep testing
			Near = RwCameraGetNearClipPlane(Scene.camera);
			radius = ViewPlaneWidth*Near;
			entity = World_TestSphereAgainstWorld(Source + Front*Near, radius, nil, true, true, false, true, false, true);

			i++;
			if(i > 5)
				entity = nil;
		}
#undef IS_TRAFFIC_LIGHT
	}
	TheCamera.m_bCamDirectlyBehind = false;
	TheCamera.m_bCamDirectlyInFront = false;

	// ------- LCS specific part starts

	if (camSetArrPos == 5 && Source.z < 1.0f) // RC Bandit and Baron
		Source.z = 1.0f;

	// CCam::FixSourceAboveWaterLevel
	if (CameraTarget.z >= -2.0f) {
		float level = -6000.0;

		if (WaterLevel_GetWaterLevelNoWaves(Source.x, Source.y, Source.z, &level)) {
			if (Source.z < level)
				Source.z = level;
		}
	}
	Front = TargetCoors - Source;

	// -------- LCS specific part ends

	GetVectorsReadyForRW();
	// SA
	// gTargetCoordsForLookingBehind = TargetCoors;

	// SA code from CAutomobile::TankControl/FireTruckControl.
	if (car->GetModelIndex() == MI_RHINO || car->GetModelIndex() == MI_FIRETRUCK) {

		float &carGunLR = ((CAutomobile*)car)->m_fCarGunLR;
		CVector hi = Multiply3x3(Front, car->GetMatrix());

		// III/VC's firetruck turret angle is reversed
		float angleToFace = (car->GetModelIndex() == MI_FIRETRUCK ? -hi.Heading() : hi.Heading());

		// Delegate the turn of the turret to the C implementation
		float turnPerFrame = (*Timer_GetTimeStep()) * (car->GetModelIndex() == MI_FIRETRUCK ? 0.05f : 0.015f);
		angleToFace = Camera_TurretFace(angleToFace, carGunLR, turnPerFrame);

		if (car->GetModelIndex() == MI_RHINO && carGunLR != angleToFace) {
			DMAudio_PlayOneShot(car->m_audioEntityId, SOUND_CAR_TANK_TURRET_ROTATE, Abs(angleToFace - carGunLR));
		}
		carGunLR = angleToFace;

		if (carGunLR < -PI) {
			carGunLR += TWOPI;
		} else if (carGunLR > PI) {
			carGunLR -= TWOPI;
		}

		// Because firetruk turret also has Y movement
		if (car->GetModelIndex() == MI_FIRETRUCK) {
			float &carGunUD = ((CAutomobile*)car)->m_fCarGunUD;

			float alphaToFace = Atan2(hi.z, hi.Magnitude2D()) + DEGTORAD(15.0f);
			// Delegate the vertical angle of the turret to the C implementation
			carGunUD = Camera_TurretAlpha(alphaToFace, carGunUD, Timer_GetTimeStepInSeconds());
		}
	}
}
#endif

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
