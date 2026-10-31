//+ rouz edit (ChatGPT)
#pragma once
#include "SetPieces.h"
enum {SETPIECE_DRIVINGSTYLE_SLOW_DOWN_FOR_CARS=1,SETPIECE_DRIVINGSTYLE_PLOUGH_THROUGH=3,SETPIECE_DRIVINGSTYLE_AVOID_CARS=2,SETPIECE_MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1=24,SETPIECE_MISSION_BLOCKPLAYER_FORWARDANDBACK=26,SETPIECE_MISSION_RAMCAR_CLOSE=16,SETPIECE_TEMPACT_GOFORWARD=8};
#ifdef __cplusplus
extern "C" {
#endif
#ifndef MASTER
void SetPieces_BackendRegisterDebug(bool *flag);
#endif
SetPieceVector SetPieces_BackendPlayerCoors(void);
SetPiecePoint SetPieces_BackendPlayerSpeed(void);
int32_t SetPieces_BackendWanted(void);
CVehicle * SetPieces_BackendPlayerVehicle(void);
void SetPieces_BackendVehicleStatus(CVehicle *car,uint8_t status);
void SetPieces_BackendOccupants(CVehicle *car);
void SetPieces_BackendRemoveVehicle(CVehicle *car);
void SetPieces_BackendRemoveCop(CCopPed *cop);
void SetPieces_BackendDestroyVehicle(CVehicle *car);
void SetPieces_BackendDestroyCop(CCopPed *cop);
uint8_t * SetPieces_BackendCruiseSpeed(CVehicle *car);
float * SetPieces_BackendMaxSpeed(CVehicle *car);
uint8_t * SetPieces_BackendDrivingStyle(CVehicle *car);
uint8_t * SetPieces_BackendCarMission(CVehicle *car);
uint8_t * SetPieces_BackendTempAction(CVehicle *car);
uint32_t * SetPieces_BackendTempTime(CVehicle *car);
uint32_t * SetPieces_BackendVehicleRangeTime(CVehicle *car);
float * SetPieces_BackendDestinationX(CVehicle *car);
float * SetPieces_BackendDestinationY(CVehicle *car);
float * SetPieces_BackendDestinationZ(CVehicle *car);
SetPieceVector SetPieces_BackendForward(CVehicle *car);
void SetPieces_BackendMoveSpeed(CVehicle *car,SetPieceVector value);
float SetPieces_BackendGroundZ(float x,float y);
void SetPieces_BackendObjectiveCompleted(CCopPed *cop,bool value);
void SetPieces_BackendObjective(CCopPed *cop,SetPieceVector value);
uint32_t * SetPieces_BackendCopRangeTime(CCopPed *cop);
CVehicle * SetPieces_BackendConstructVehicle(void);
CCopPed * SetPieces_BackendConstructCop(void);
bool SetPieces_BackendVerticalLine(SetPieceVector value,float *height);
float SetPieces_BackendHeightAboveRoad(CVehicle *car);
void SetPieces_BackendSetForward(CVehicle *car,SetPieceVector value);
void SetPieces_BackendSetRight(CVehicle *car,SetPieceVector value);
void SetPieces_BackendSetUp(CVehicle *car,SetPieceVector value);
void SetPieces_BackendVehiclePosition(CVehicle *car,SetPieceVector value);
void SetPieces_BackendCopPosition(CCopPed *cop,SetPieceVector value);
float SetPieces_BackendVehicleRadius(CVehicle *car);
float SetPieces_BackendCopRadius(CCopPed *cop);
void SetPieces_BackendColliding(SetPieceVector value,float radius,int16_t *total);
void SetPieces_BackendLawEnforcer(CVehicle *car,uint8_t enabled);
void SetPieces_BackendAddVehicle(CVehicle *car);
void SetPieces_BackendAddCop(CCopPed *cop);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
