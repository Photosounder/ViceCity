//+ rouz edit (ChatGPT)
#pragma once
#include "Remote.h"
#ifdef __cplusplus
class CVehicle;
typedef CVehicle RemoteVehicle;
extern "C" {
#else
typedef struct CVehicle RemoteVehicle;
#endif
enum {REMOTE_RANDOM_VEHICLE=1,REMOTE_MISSION_VEHICLE=2,REMOTE_MISSION_NONE=0,REMOTE_TEMPACT_NONE=0,REMOTE_DRIVINGSTYLE_STOP_FOR_CARS=0};
RemoteVehicle * Remote_BackendAllocate(uint16_t model);
float Remote_BackendBaseDistance(RemoteVehicle *car);
float Remote_BackendGroundZ(float x,float y,float z,bool *found);
void Remote_BackendRotate(RemoteVehicle *car,float rotation);
void Remote_BackendPosition(RemoteVehicle *car,float x,float y,float z);
void Remote_BackendStatus(RemoteVehicle *car,uint8_t status);
void Remote_BackendLocked(RemoteVehicle *car,bool value);
void Remote_BackendJoinRoad(RemoteVehicle *car);
uint8_t * Remote_BackendCarMission(RemoteVehicle *car);
uint8_t * Remote_BackendTempAction(RemoteVehicle *car);
uint8_t * Remote_BackendDrivingStyle(RemoteVehicle *car);
uint8_t * Remote_BackendCruiseSpeed(RemoteVehicle *car);
float * Remote_BackendMaxTrafficSpeed(RemoteVehicle *car);
int8_t * Remote_BackendNextLane(RemoteVehicle *car);
int8_t * Remote_BackendCurrentLane(RemoteVehicle *car);
void Remote_BackendEngineOn(RemoteVehicle *car,bool value);
void Remote_BackendAddWorld(RemoteVehicle *car);
RemoteVehicle * Remote_BackendPlayerVehicle(void);
RemoteVehicle ** Remote_BackendRemoteSlot(void);
void Remote_BackendRegisterReference(RemoteVehicle *car,RemoteVehicle **slot);
int32_t Remote_BackendAppearance(RemoteVehicle *car);
void Remote_BackendTakeCamera(RemoteVehicle *car,int16_t mode);
void Remote_BackendZoom(int16_t value);
uint8_t * Remote_BackendCreatedBy(RemoteVehicle *car);
int32_t * Remote_BackendMissionCars(void);
int32_t * Remote_BackendRandomCars(void);
uint32_t * Remote_BackendLostTime(void);
bool * Remote_BackendRemoteMode(void);
bool * Remote_BackendBlowUp(void);
bool * Remote_BackendLostFlag(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
