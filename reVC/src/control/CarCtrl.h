//+ rouz edit (ChatGPT)
#pragma once
#include "PopulationLimits.h" // rouz edit (ChatGPT)
#include "CarClasses.h"
#include "PathFind.h"
#include "Boat.h"
#include "Vehicle.h"

#define GAME_SPEED_TO_METERS_PER_SECOND 50.0f
#define METERS_PER_SECOND_TO_GAME_SPEED (1.0f / GAME_SPEED_TO_METERS_PER_SECOND)
#define GAME_SPEED_TO_CARAI_SPEED 60.0f
#define TIME_COPS_WAIT_TO_EXIT_AFTER_STOPPING 2500

struct CZoneInfo;
class CAutomobile;

enum{
	MAX_CARS_TO_KEEP = 2,
	MAX_CAR_MODELS_IN_ARRAY = 25,
};

#ifdef FIX_BUGS
#define FIX_PATHFIND_BUG
#endif

//+ rouz edit (Claude)
// Formerly the static class CCarCtrl, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
void CarCtrl_SwitchVehicleToRealPhysics(CVehicle*);
void CarCtrl_AddToCarArray(int32 id, int32 vehclass);
void CarCtrl_UpdateCarCount(CVehicle*, bool);
int32 CarCtrl_ChooseCarModel(int32 vehclass);
bool CarCtrl_JoinCarWithRoadSystemGotoCoors(CVehicle*, CVector, bool);
void CarCtrl_JoinCarWithRoadSystem(CVehicle*);
void CarCtrl_UpdateCarOnRails(CVehicle*);
bool CarCtrl_MapCouldMoveInThisArea(float x, float y);
void CarCtrl_ScanForPedDanger(CVehicle *veh);
void CarCtrl_RemoveFromInterestingVehicleList(CVehicle*);
void CarCtrl_GenerateRandomCars(void);
void CarCtrl_GenerateOneRandomCar(void);
void CarCtrl_GenerateEmergencyServicesCar(void);
int32 CarCtrl_ChooseModel(CZoneInfo*, int*);
int32 CarCtrl_ChoosePoliceCarModel(void);
int32 CarCtrl_ChooseGangCarModel(int32 gang);
void CarCtrl_RemoveDistantCars(void);
void CarCtrl_PossiblyRemoveVehicle(CVehicle*);
bool CarCtrl_IsThisVehicleInteresting(CVehicle*);
void CarCtrl_RegisterVehicleOfInterest(CVehicle*);
int32 CarCtrl_CountCarsOfType(int32 mi);
void CarCtrl_SlowCarOnRailsDownForTrafficAndLights(CVehicle*);
bool CarCtrl_PickNextNodeAccordingStrategy(CVehicle*);
void CarCtrl_DragCarToPoint(CVehicle*, CVector*);
float CarCtrl_FindMaximumSpeedForThisCarInTraffic(CVehicle*);
void CarCtrl_SlowCarDownForOtherCar(CEntity*, CVehicle*, float*, float);
float CarCtrl_TestCollisionBetween2MovingRects(CVehicle*, CVehicle*, float, float, CVector*, CVector*, uint8);
float CarCtrl_FindAngleToWeaveThroughTraffic(CVehicle*, CPhysical*, float, float);
void CarCtrl_WeaveForOtherCar(CEntity*, CVehicle*, float*, float*);
void CarCtrl_WeaveForPed(CEntity*, CVehicle*, float*, float*);
void CarCtrl_WeaveForObject(CEntity*, CVehicle*, float*, float*);
#ifdef FIX_PATHFIND_BUG
#else
#endif
bool CarCtrl_PickNextNodeToFollowPath(CVehicle*);
void CarCtrl_PickNextNodeRandomly(CVehicle*);
uint8 CarCtrl_FindPathDirection(int32, int32, int32);
void CarCtrl_Init(void);
void CarCtrl_ReInit(void);
float CarCtrl_FindSpeedMultiplier(float, float, float, float);
void CarCtrl_SteerAICarWithPhysics(CVehicle*);
void CarCtrl_SteerAICarWithPhysics_OnlyMission(CVehicle*, float*, float*, float*, bool*);
float CarCtrl_FindMaxSteerAngle(CVehicle*);
void CarCtrl_SteerAICarWithPhysicsFollowPath(CVehicle*, float*, float*, float*, bool*);
void CarCtrl_SteerAICarWithPhysicsHeadingForTarget(CVehicle*, CPhysical*, float, float, float*, float*, float*, bool*);
void CarCtrl_SteerAICarWithPhysicsTryingToBlockTarget(CVehicle*, float, float, float, float, float*, float*, float*, bool*);
void CarCtrl_SteerAICarWithPhysicsTryingToBlockTarget_Stop(CVehicle*, float, float, float, float, float*, float*, float*, bool*);
bool CarCtrl_ThisRoadObjectCouldMove(int16);
void CarCtrl_ClearInterestingVehicleList();
void CarCtrl_FindLinksToGoWithTheseNodes(CVehicle*);
bool CarCtrl_GenerateOneEmergencyServicesCar(uint32, CVector);
float CarCtrl_FindSpeedMultiplierWithSpeedFromNodes(int8);
int32 CarCtrl_ChooseBoatModel(int32);
int32 CarCtrl_ChooseBoatRating(CZoneInfo* pZoneInfo);
int32 CarCtrl_ChooseCarRating(CZoneInfo* pZoneInfo);
void CarCtrl_AddToLoadedVehicleArray(int32 mi, int32 rating, int32 freq);
void CarCtrl_RemoveFromLoadedVehicleArray(int32 mi, int32 rating);
int32 CarCtrl_ChooseCarModelToLoad(int32 rating);
bool CarCtrl_BoatWithTallMast(int32 mi);
void CarCtrl_RemoveCarsIfThePoolGetsFull(void);
void CarCtrl_SteerAIBoatWithPhysicsHeadingForTarget(CVehicle*, float, float, float*, float*, float*);
void CarCtrl_SteerAIHeliTowardsTargetCoors(CAutomobile*);
void CarCtrl_SteerAIPlaneTowardsTargetCoors(CAutomobile*);
void CarCtrl_SteerAIBoatWithPhysicsAttackingPlayer(CVehicle*, float*, float*, float*, bool*);
void CarCtrl_SteerAICarBlockingPlayerForwardAndBack(CVehicle*, float*, float*, float*, bool*);
extern bool CarCtrl_bMadDriversCheat;
extern int32 CarCtrl_NumLawEnforcerCars;
extern int32 CarCtrl_NumAmbulancesOnDuty;
extern int32 CarCtrl_NumFiretrucksOnDuty;
extern int32 CarCtrl_NumRandomCars;
extern int32 CarCtrl_NumMissionCars;
extern int32 CarCtrl_NumParkedCars;
extern int32 CarCtrl_NumPermanentCars;
extern bool CarCtrl_bCarsGeneratedAroundCamera;
extern float CarCtrl_CarDensityMultiplier;
extern int8 CarCtrl_CountDownToCarsAtStart;
extern uint32 CarCtrl_LastTimeLawEnforcerCreated;
extern uint32 CarCtrl_LastTimeFireTruckCreated;
extern uint32 CarCtrl_LastTimeAmbulanceCreated;
extern int32 CarCtrl_TotalNumOfCarsOfRating[CARCLASS_TOTAL_CUSTOM_CLASSES];
extern int32 CarCtrl_CarArrays[CARCLASS_TOTAL_CUSTOM_CLASSES][MAX_CAR_MODELS_IN_ARRAY];
extern int32 CarCtrl_MiamiViceCycle;
extern uint32 CarCtrl_LastTimeMiamiViceGenerated;
extern int32 CarCtrl_NumRequestsOfCarRating[CARCLASS_TOTAL_CUSTOM_CLASSES];
extern int32 CarCtrl_NumOfLoadedCarsOfRating[CARCLASS_TOTAL_CUSTOM_CLASSES];
extern int32 CarCtrl_CarFreqArrays[CARCLASS_TOTAL_CUSTOM_CLASSES][MAX_CAR_MODELS_IN_ARRAY];
extern int32 CarCtrl_LoadedCarsArray[CARCLASS_TOTAL_CUSTOM_CLASSES][MAX_CAR_MODELS_IN_ARRAY];
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
void CarCtrl_SlowCarDownForCarsSectorList(CPtrList&, CVehicle*, float, float, float, float, float*, float);
void CarCtrl_SlowCarDownForPedsSectorList(CPtrList&, CVehicle*, float, float, float, float, float*, float);
void CarCtrl_WeaveThroughCarsSectorList(CPtrList&, CVehicle*, CPhysical*, float, float, float, float, float*, float*);
void CarCtrl_WeaveThroughPedsSectorList(CPtrList&, CVehicle*, CPhysical*, float, float, float, float, float*, float*);
void CarCtrl_WeaveThroughObjectsSectorList(CPtrList&, CVehicle*, float, float, float, float, float*, float*);
#ifdef FIX_PATHFIND_BUG
void CarCtrl_PickNextNodeToChaseCar(CVehicle*, float, float, float, CVehicle*);
#else
void CarCtrl_PickNextNodeToChaseCar(CVehicle*, float, float, CVehicle*);
#endif
static inline float CarCtrl_GetPositionAlongCurrentCurve(CVehicle* pVehicle);
static inline float CarCtrl_LimitRadianAngle(float angle);
static inline float CarCtrl_GetPositionAlongCurrentCurve(CVehicle* pVehicle)
	{
		uint32 timeInCurve = (*Timer_GetTimeInMilliseconds()) - pVehicle->AutoPilot.m_nTimeEnteredCurve;
		return (float)timeInCurve / pVehicle->AutoPilot.m_nTimeToSpendOnCurrentCurve;
	}
static inline float CarCtrl_LimitRadianAngle(float angle)
	{
		while (angle < -PI)
			angle += TWOPI;
		while (angle > PI)
			angle -= TWOPI;
		return angle;
	}
#endif
//- rouz edit (Claude)

extern CVehicle* apCarsToKeep[MAX_CARS_TO_KEEP];
//- rouz edit (ChatGPT)
