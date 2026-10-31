//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "Timer.h"
#ifdef __cplusplus
class CVehicle;struct CPathNode;
#else
typedef struct CVehicle CVehicle;typedef struct CPathNode CPathNode;
#endif
typedef enum eCarMission
{
	MISSION_NONE,
	MISSION_CRUISE,
	MISSION_RAMPLAYER_FARAWAY,
	MISSION_RAMPLAYER_CLOSE,
	MISSION_BLOCKPLAYER_FARAWAY,
	MISSION_BLOCKPLAYER_CLOSE,
	MISSION_BLOCKPLAYER_HANDBRAKESTOP,
	MISSION_WAITFORDELETION,
	MISSION_GOTOCOORDS,
	MISSION_GOTOCOORDS_STRAIGHT,
	MISSION_EMERGENCYVEHICLE_STOP,
	MISSION_STOP_FOREVER,
	MISSION_GOTOCOORDS_ACCURATE,
	MISSION_GOTO_COORDS_STRAIGHT_ACCURATE,
	MISSION_GOTOCOORDS_ASTHECROWSWIMS,
	MISSION_RAMCAR_FARAWAY,
	MISSION_RAMCAR_CLOSE,
	MISSION_BLOCKCAR_FARAWAY,
	MISSION_BLOCKCAR_CLOSE,
	MISSION_BLOCKCAR_HANDBRAKESTOP,
	MISSION_HELI_FLYTOCOORS,
	MISSION_ATTACKPLAYER,
	MISSION_PLANE_FLYTOCOORS,
	MISSION_HELI_LAND,
	MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1,
	MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_2,
	MISSION_BLOCKPLAYER_FORWARDANDBACK
} eCarMission;

typedef enum eCarTempAction
{
	TEMPACT_NONE,
	TEMPACT_WAIT,
	TEMPACT_REVERSE,
	TEMPACT_HANDBRAKETURNLEFT,
	TEMPACT_HANDBRAKETURNRIGHT,
	TEMPACT_HANDBRAKESTRAIGHT,
	TEMPACT_TURNLEFT,
	TEMPACT_TURNRIGHT,
	TEMPACT_GOFORWARD,
	TEMPACT_SWERVELEFT,
	TEMPACT_SWERVERIGHT
} eCarTempAction;

typedef enum eCarDrivingStyle
{
	DRIVINGSTYLE_STOP_FOR_CARS,
	DRIVINGSTYLE_SLOW_DOWN_FOR_CARS,
	DRIVINGSTYLE_AVOID_CARS,
	DRIVINGSTYLE_PLOUGH_THROUGH,
	DRIVINGSTYLE_STOP_FOR_CARS_IGNORE_LIGHTS
} eCarDrivingStyle;

typedef struct AutoPilotVector {float x,y,z;} AutoPilotVector;
typedef struct CAutoPilot {
	int32_t m_nCurrentRouteNode;
	int32_t m_nNextRouteNode;
	int32_t m_nPrevRouteNode;
	int32_t m_nTimeEnteredCurve;
	int32_t m_nTimeToSpendOnCurrentCurve;
	uint32_t m_nCurrentPathNodeInfo;
	uint32_t m_nNextPathNodeInfo;
	uint32_t m_nPreviousPathNodeInfo;
	uint32_t m_nAntiReverseTimer;
	uint32_t m_nTimeToStartMission;
	int8_t m_nPreviousDirection;
	int8_t m_nCurrentDirection;
	int8_t m_nNextDirection;
	int8_t m_nCurrentLane;
	int8_t m_nNextLane;
	uint8_t m_nDrivingStyle;
	uint8_t m_nCarMission;
	uint8_t m_nTempAction;
	uint32_t m_nTimeTempAction;
	float m_fMaxTrafficSpeed;
	uint8_t m_nCruiseSpeed;
	uint8_t m_nCruiseSpeedMultiplierType;
	float m_fCruiseSpeedMultiplier;
	uint8_t m_bSlowedDownBecauseOfCars : 1;
	uint8_t m_bSlowedDownBecauseOfPeds : 1;
	uint8_t m_bStayInCurrentLevel : 1;
	uint8_t m_bStayInFastLane : 1;
	uint8_t m_bIgnorePathfinding : 1;
	uint8_t m_nSwitchDistance;
	AutoPilotVector m_vecDestinationCoors;
	CPathNode *m_aPathFindNodesInfo[NUM_PATH_NODES_IN_AUTOPILOT];
	int16_t m_nPathFindNodesCount;
	CVehicle *m_pTargetCar;

} CAutoPilot;
#ifdef __cplusplus
extern "C" {
#endif
void AutoPilot_Init(CAutoPilot *self);
void AutoPilot_ModifySpeed(CAutoPilot *self,float speed);
void AutoPilot_RemoveOnePathNode(CAutoPilot *self);
float AutoPilot_GetCruiseSpeed(const CAutoPilot *self);
#ifdef COMPATIBLE_SAVES
void AutoPilot_Save(CAutoPilot *self,uint8_t **buf);
void AutoPilot_Load(CAutoPilot *self,uint8_t **buf);
#endif
#ifdef __cplusplus
}
class CVector;
AutoPilotVector AutoPilot_FromVector(CVector value);
CVector AutoPilot_ToVector(AutoPilotVector value);
#ifdef VALIDATE_SIZE
VALIDATE_SIZE(CAutoPilot,0x70);
#endif
#endif
//- rouz edit (ChatGPT)
