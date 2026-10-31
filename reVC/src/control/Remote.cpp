//+ rouz edit (ChatGPT)
#include "common.h"
#include "Automobile.h"
#include "CarCtrl.h"
#include "Camera.h"
#include "World.h"
#include "PlayerInfo.h"
#include "Pools.h"
#include "Remote_backend.h"
static_assert(int(REMOTE_RANDOM_VEHICLE)==int(RANDOM_VEHICLE),"Original remote-car constant");
static_assert(int(REMOTE_MISSION_VEHICLE)==int(MISSION_VEHICLE),"Original remote-car constant");
static_assert(int(REMOTE_MISSION_NONE)==int(MISSION_NONE),"Original remote-car constant");
static_assert(int(REMOTE_TEMPACT_NONE)==int(TEMPACT_NONE),"Original remote-car constant");
static_assert(int(REMOTE_DRIVINGSTYLE_STOP_FOR_CARS)==int(DRIVINGSTYLE_STOP_FOR_CARS),"Original remote-car constant");
RemoteVehicle * Remote_BackendAllocate(uint16_t model)
{
    // Access the original native operation or named field with its actual type
    CAutomobile *car=(CAutomobile*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
    assert(car);
    std::allocator<CAutomobile>().construct(car,model,MISSION_VEHICLE);
    return car;
}
float Remote_BackendBaseDistance(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return car->GetDistanceFromCentreOfMassToBaseOfModel();
}
float Remote_BackendGroundZ(float x,float y,float z,bool *found)
{
    // Access the original native operation or named field with its actual type
    return CWorld::FindGroundZFor3DCoord(x,y,z,found);
}
void Remote_BackendRotate(RemoteVehicle *car,float rotation)
{
    // Access the original native operation or named field with its actual type
    car->GetMatrix().SetRotateZOnly(rotation);
}
void Remote_BackendPosition(RemoteVehicle *car,float x,float y,float z)
{
    // Access the original native operation or named field with its actual type
    car->SetPosition(x,y,z);
}
void Remote_BackendStatus(RemoteVehicle *car,uint8_t status)
{
    // Access the original native operation or named field with its actual type
    car->SetStatus(status);
}
void Remote_BackendLocked(RemoteVehicle *car,bool value)
{
    // Access the original native operation or named field with its actual type
    car->bIsLocked=value;
}
void Remote_BackendJoinRoad(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    CCarCtrl::JoinCarWithRoadSystem(car);
}
uint8_t * Remote_BackendCarMission(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->AutoPilot.m_nCarMission;
}
uint8_t * Remote_BackendTempAction(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->AutoPilot.m_nTempAction;
}
uint8_t * Remote_BackendDrivingStyle(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->AutoPilot.m_nDrivingStyle;
}
uint8_t * Remote_BackendCruiseSpeed(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->AutoPilot.m_nCruiseSpeed;
}
float * Remote_BackendMaxTrafficSpeed(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->AutoPilot.m_fMaxTrafficSpeed;
}
int8_t * Remote_BackendNextLane(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->AutoPilot.m_nNextLane;
}
int8_t * Remote_BackendCurrentLane(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->AutoPilot.m_nCurrentLane;
}
void Remote_BackendEngineOn(RemoteVehicle *car,bool value)
{
    // Access the original native operation or named field with its actual type
    car->bEngineOn=value;
}
void Remote_BackendAddWorld(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    CWorld::Add(car);
}
RemoteVehicle * Remote_BackendPlayerVehicle(void)
{
    // Access the original native operation or named field with its actual type
    return FindPlayerVehicle();
}
RemoteVehicle ** Remote_BackendRemoteSlot(void)
{
    // Access the original native operation or named field with its actual type
    return &CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle;
}
void Remote_BackendRegisterReference(RemoteVehicle *car,RemoteVehicle **slot)
{
    // Access the original native operation or named field with its actual type
    car->RegisterReference((CEntity**)slot);
}
int32_t Remote_BackendAppearance(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return car->GetVehicleAppearance();
}
void Remote_BackendTakeCamera(RemoteVehicle *car,int16_t mode)
{
    // Access the original native operation or named field with its actual type
    TheCamera.TakeControl(car,mode,INTERPOLATION,CAMCONTROL_SCRIPT);
}
void Remote_BackendZoom(int16_t value)
{
    // Access the original native operation or named field with its actual type
    TheCamera.SetZoomValueCamStringScript(value);
}
uint8_t * Remote_BackendCreatedBy(RemoteVehicle *car)
{
    // Access the original native operation or named field with its actual type
    return &car->VehicleCreatedBy;
}
int32_t * Remote_BackendMissionCars(void)
{
    // Access the original native operation or named field with its actual type
    return &CCarCtrl::NumMissionCars;
}
int32_t * Remote_BackendRandomCars(void)
{
    // Access the original native operation or named field with its actual type
    return &CCarCtrl::NumRandomCars;
}
uint32_t * Remote_BackendLostTime(void)
{
    // Access the original native operation or named field with its actual type
    return &CWorld::Players[CWorld::PlayerInFocus].m_nTimeLostRemoteCar;
}
bool * Remote_BackendRemoteMode(void)
{
    // Access the original native operation or named field with its actual type
    return &CWorld::Players[CWorld::PlayerInFocus].m_bInRemoteMode;
}
bool * Remote_BackendBlowUp(void)
{
    // Access the original native operation or named field with its actual type
    return &CWorld::Players[CWorld::PlayerInFocus].field_D5;
}
bool * Remote_BackendLostFlag(void)
{
    // Access the original native operation or named field with its actual type
    return &CWorld::Players[CWorld::PlayerInFocus].field_D6;
}
//- rouz edit (ChatGPT)
