//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"
#include "SetPieces_backend.h"
#include "Automobile.h"
#include "CarAI.h"
#include "CopPed.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "World.h"
#include "Pools.h"
#include "VarConsole.h"
static_assert(int(SETPIECE_DRIVINGSTYLE_SLOW_DOWN_FOR_CARS)==int(DRIVINGSTYLE_SLOW_DOWN_FOR_CARS),"Original set-piece constant");
static_assert(int(SETPIECE_DRIVINGSTYLE_PLOUGH_THROUGH)==int(DRIVINGSTYLE_PLOUGH_THROUGH),"Original set-piece constant");
static_assert(int(SETPIECE_DRIVINGSTYLE_AVOID_CARS)==int(DRIVINGSTYLE_AVOID_CARS),"Original set-piece constant");
static_assert(int(SETPIECE_MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1)==int(MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1),"Original set-piece constant");
static_assert(int(SETPIECE_MISSION_BLOCKPLAYER_FORWARDANDBACK)==int(MISSION_BLOCKPLAYER_FORWARDANDBACK),"Original set-piece constant");
static_assert(int(SETPIECE_MISSION_RAMCAR_CLOSE)==int(MISSION_RAMCAR_CLOSE),"Original set-piece constant");
static_assert(int(SETPIECE_TEMPACT_GOFORWARD)==int(TEMPACT_GOFORWARD),"Original set-piece constant");
#ifndef MASTER
void SetPieces_BackendRegisterDebug(bool *flag)
{
    // Access the original typed native operation or canonical named field
    VarConsole_AddBool(&VarConsole,"Show set pieces",flag,true);
}
#endif
SetPieceVector SetPieces_BackendPlayerCoors(void)
{
    // Access the original typed native operation or canonical named field
    CVector value=FindPlayerCoors();return SetPieceVector{value.x,value.y,value.z};
}
SetPiecePoint SetPieces_BackendPlayerSpeed(void)
{
    // Access the original typed native operation or canonical named field
    CVector2D value=FindPlayerSpeed();return SetPiecePoint{value.x,value.y};
}
int32_t SetPieces_BackendWanted(void)
{
    // Access the original typed native operation or canonical named field
    return Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted);
}
CVehicle * SetPieces_BackendPlayerVehicle(void)
{
    // Access the original typed native operation or canonical named field
    return FindPlayerVehicle();
}
void SetPieces_BackendVehicleStatus(CVehicle *car,uint8_t status)
{
    // Access the original typed native operation or canonical named field
    car->SetStatus(status);
}
void SetPieces_BackendOccupants(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    CCarAI::AddPoliceCarOccupants(car);
}
void SetPieces_BackendRemoveVehicle(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    CWorld::Remove(car);
}
void SetPieces_BackendRemoveCop(CCopPed *cop)
{
    // Access the original typed native operation or canonical named field
    CWorld::Remove(cop);
}
void SetPieces_BackendDestroyVehicle(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    car->~CVehicle();CPool_Delete(CPools::GetVehiclePool(),car);
}
void SetPieces_BackendDestroyCop(CCopPed *cop)
{
    // Access the original typed native operation or canonical named field
    cop->~CCopPed();CPool_Delete(CPools::GetPedPool(),cop);
}
uint8_t * SetPieces_BackendCruiseSpeed(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_nCruiseSpeed;
}
float * SetPieces_BackendMaxSpeed(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_fMaxTrafficSpeed;
}
uint8_t * SetPieces_BackendDrivingStyle(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_nDrivingStyle;
}
uint8_t * SetPieces_BackendCarMission(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_nCarMission;
}
uint8_t * SetPieces_BackendTempAction(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_nTempAction;
}
uint32_t * SetPieces_BackendTempTime(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_nTimeTempAction;
}
uint32_t * SetPieces_BackendVehicleRangeTime(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->m_nSetPieceExtendedRangeTime;
}
float * SetPieces_BackendDestinationX(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_vecDestinationCoors.x;
}
float * SetPieces_BackendDestinationY(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_vecDestinationCoors.y;
}
float * SetPieces_BackendDestinationZ(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return &car->AutoPilot.m_vecDestinationCoors.z;
}
SetPieceVector SetPieces_BackendForward(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    const CVector &value=car->GetForward();return SetPieceVector{value.x,value.y,value.z};
}
void SetPieces_BackendMoveSpeed(CVehicle *car,SetPieceVector value)
{
    // Access the original typed native operation or canonical named field
    car->SetMoveSpeed(CVector(value.x,value.y,value.z));
}
float SetPieces_BackendGroundZ(float x,float y)
{
    // Access the original typed native operation or canonical named field
    return CWorld::FindGroundZForCoord(x,y);
}
void SetPieces_BackendObjectiveCompleted(CCopPed *cop,bool value)
{
    // Access the original typed native operation or canonical named field
    cop->bScriptObjectiveCompleted=value;
}
void SetPieces_BackendObjective(CCopPed *cop,SetPieceVector value)
{
    // Access the original typed native operation or canonical named field
    cop->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT,CVector(value.x,value.y,value.z));
}
uint32_t * SetPieces_BackendCopRangeTime(CCopPed *cop)
{
    // Access the original typed native operation or canonical named field
    return &cop->m_nExtendedRangeTimer;
}
CVehicle * SetPieces_BackendConstructVehicle(void)
{
    // Access the original typed native operation or canonical named field
    CVehicle *car=(CVehicle*)CPool_New(CPools::GetVehiclePool());assert(car);std::allocator<CAutomobile>().construct((CAutomobile*)car,MI_POLICE,RANDOM_VEHICLE);return car;
}
CCopPed * SetPieces_BackendConstructCop(void)
{
    // Access the original typed native operation or canonical named field
    CCopPed *cop=(CCopPed*)((CPed*)CPool_New(CPools::GetPedPool()));assert(cop);std::allocator<CCopPed>().construct(cop,COP_STREET);return cop;
}
bool SetPieces_BackendVerticalLine(SetPieceVector value,float *height)
{
    // Access the original typed native operation or canonical named field
    CVector position(value.x,value.y,value.z);CColPoint point;CEntity *entity;bool hit=CWorld::ProcessVerticalLine(position,-1000.0f,point,entity,true,false,false,false,true,false,nil);
    if(hit){
        // Copy only the original valid hit-height output before the C controller decides placement
        *height=point.point.z;
    }
    return hit;
}
float SetPieces_BackendHeightAboveRoad(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return car->GetHeightAboveRoad();
}
void SetPieces_BackendSetForward(CVehicle *car,SetPieceVector value)
{
    // Access the original typed native operation or canonical named field
    car->GetForward()=CVector(value.x,value.y,value.z);
}
void SetPieces_BackendSetRight(CVehicle *car,SetPieceVector value)
{
    // Access the original typed native operation or canonical named field
    car->GetRight()=CVector(value.x,value.y,value.z);
}
void SetPieces_BackendSetUp(CVehicle *car,SetPieceVector value)
{
    // Access the original typed native operation or canonical named field
    car->GetUp()=CVector(value.x,value.y,value.z);
}
void SetPieces_BackendVehiclePosition(CVehicle *car,SetPieceVector value)
{
    // Access the original typed native operation or canonical named field
    car->SetPosition(CVector(value.x,value.y,value.z));
}
void SetPieces_BackendCopPosition(CCopPed *cop,SetPieceVector value)
{
    // Access the original typed native operation or canonical named field
    cop->SetPosition(CVector(value.x,value.y,value.z));
}
float SetPieces_BackendVehicleRadius(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    return car->GetColModel()->spheres->bounds.radius;
}
float SetPieces_BackendCopRadius(CCopPed *cop)
{
    // Access the original typed native operation or canonical named field
    return cop->GetColModel()->spheres->bounds.radius;
}
void SetPieces_BackendColliding(SetPieceVector value,float radius,int16_t *total)
{
    // Access the original typed native operation or canonical named field
    CVector position(value.x,value.y,value.z);CWorld::FindObjectsKindaColliding(position,radius,false,total,16,nil,false,true,true,false,false);
}
void SetPieces_BackendLawEnforcer(CVehicle *car,uint8_t enabled)
{
    // Access the original typed native operation or canonical named field
    car->ChangeLawEnforcerState(enabled);
}
void SetPieces_BackendAddVehicle(CVehicle *car)
{
    // Access the original typed native operation or canonical named field
    CWorld::Add(car);
}
void SetPieces_BackendAddCop(CCopPed *cop)
{
    // Access the original typed native operation or canonical named field
    CWorld::Add(cop);
}
//- rouz edit (ChatGPT)
