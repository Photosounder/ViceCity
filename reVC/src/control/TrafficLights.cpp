//+ rouz edit (ChatGPT)
#include "common.h"
#include "TrafficLights_backend.h"
#include "ModelInfo.h"
#include "Camera.h"
#include "Coronas.h"
#include "PathFind.h"
#include "Shadows.h"
#include "Timecycle.h"
#include "Vehicle.h"
#include "World.h"
static TrafficLightVector TrafficLights_FromNative(const CVector &value)
{
    // Materialize the original native vector at its value boundary
    TrafficLightVector result={value.x,value.y,value.z};return result;
}
static CVector TrafficLights_ToNative(TrafficLightVector value)
{
    // Supply the same three original coordinates to the still-native engine operation
    return CVector(value.x,value.y,value.z);
}
float TrafficLights_BackendUpZ(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetUp().z;
}
bool TrafficLights_BackendDamaged(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->bRenderDamaged;
}
int32_t TrafficLights_BackendModel(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetModelIndex();
}
TrafficLightVector TrafficLights_BackendTransform(CEntity *entity,TrafficLightVector point)
{
    // Enter the original typed native field or action at the C decision point
    return TrafficLights_FromNative(entity->GetMatrix()*TrafficLights_ToNative(point));
}
TrafficLightVector TrafficLights_BackendCameraForward(void)
{
    // Enter the original typed native field or action at the C decision point
    return TrafficLights_FromNative(TheCamera.GetForward());
}
void TrafficLights_BackendShadow(uintptr_t id,TrafficLightVector position,uint8_t red,uint8_t green,uint8_t blue)
{
    // Enter the original typed native field or action at the C decision point
    CVector native=TrafficLights_ToNative(position);CShadows::StoreStaticShadow(id,SHADOWTYPE_ADDITIVE,gpShadowExplosionTex,&native,8.0f,0.0f,0.0f,-8.0f,128,red,green,blue,12.0f,1.0f,40.0f,false,0.0f);
}
void TrafficLights_BackendCorona(uintptr_t id,uint8_t red,uint8_t green,uint8_t blue,TrafficLightVector position,float size)
{
    // Enter the original typed native field or action at the C decision point
    CCoronas::RegisterCorona(id,red,green,blue,255,TrafficLights_ToNative(position),size,50.0f,CCoronas::TYPE_STAR,CCoronas::FLARE_NONE,CCoronas::REFLECTION_ON,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_OFF,0.0f);
}
int32_t TrafficLights_BackendNumSectorsX(void)
{
    // Enter the original typed native field or action at the C decision point
    return NUMSECTORS_X;
}
int32_t TrafficLights_BackendNumSectorsY(void)
{
    // Enter the original typed native field or action at the C decision point
    return NUMSECTORS_Y;
}
TrafficLightList * TrafficLights_BackendSectorList(int32_t x,int32_t y)
{
    // Enter the original typed native field or action at the C decision point
    return &CWorld::GetSector(x,y)->m_lists[ENTITYLIST_DUMMIES];
}
TrafficLightNode * TrafficLights_BackendFirst(TrafficLightList *list)
{
    // Enter the original typed native field or action at the C decision point
    return list->first;
}
TrafficLightNode * TrafficLights_BackendNext(TrafficLightNode *node)
{
    // Enter the original typed native field or action at the C decision point
    return node->next;
}
CEntity * TrafficLights_BackendNodeEntity(TrafficLightNode *node)
{
    // Enter the original typed native field or action at the C decision point
    return (CEntity *)node->item;
}
int32_t TrafficLights_BackendCarNodeCount(void)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_numCarPathNodes;
}
int32_t TrafficLights_BackendNodeCount(void)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_numPathNodes;
}
TrafficLightVector TrafficLights_BackendPathPosition(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return TrafficLights_FromNative(ThePaths.m_pathNodes[index].GetPosition());
}
int32_t TrafficLights_BackendNumLinks(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_pathNodes[index].numLinks;
}
int32_t TrafficLights_BackendFirstLink(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_pathNodes[index].firstLink;
}
int32_t TrafficLights_BackendConnectedNode(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.ConnectedNode(index);
}
int32_t TrafficLights_BackendCarConnection(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_carPathConnections[index];
}
uint8_t TrafficLights_BackendLightType(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_carPathLinks[index].trafficLightType;
}
void TrafficLights_BackendSetLightType(int32_t index,int32_t type)
{
    // Enter the original typed native field or action at the C decision point
    ThePaths.m_carPathLinks[index].trafficLightType=type;
}
bool TrafficLights_BackendLightDirection(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_carPathLinks[index].trafficLightDirection;
}
void TrafficLights_BackendSetLightDirection(int32_t index,bool direction)
{
    // Enter the original typed native field or action at the C decision point
    ThePaths.m_carPathLinks[index].trafficLightDirection=direction;
}
int32_t TrafficLights_BackendLinkNode(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_carPathLinks[index].pathNodeIndex;
}
bool TrafficLights_BackendCrossesRoad(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.ConnectionCrossesRoad(index);
}
void TrafficLights_BackendSetConnectionLight(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    ThePaths.ConnectionSetTrafficLight(index);
}
bool TrafficLights_BackendBridgeLight(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_carPathLinks[index].bBridgeLights;
}
TrafficLightVector TrafficLights_BackendLinkPosition(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    CVector2D v=ThePaths.m_carPathLinks[index].GetPosition();return TrafficLightVector{v.x,v.y,0.0f};
}
TrafficLightVector TrafficLights_BackendLinkDirection(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    CVector2D v=ThePaths.m_carPathLinks[index].GetDirection();return TrafficLightVector{v.x,v.y,0.0f};
}
TrafficLightVector TrafficLights_BackendVehiclePosition(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    const CVector &value=vehicle->GetPosition();return TrafficLightVector{value.x,value.y,0.0f};
}
int32_t TrafficLights_BackendVehicleStatus(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->GetStatus();
}
TrafficLightVector TrafficLights_BackendUp(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return TrafficLights_FromNative(entity->GetUp());
}
TrafficLightVector TrafficLights_BackendRight(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return TrafficLights_FromNative(entity->GetRight());
}
TrafficLightVector TrafficLights_BackendForward(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return TrafficLights_FromNative(entity->GetForward());
}
float TrafficLights_BackendForwardX(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetForward().x;
}
float TrafficLights_BackendForwardY(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetForward().y;
}
float TrafficLights_BackendPositionX(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetPosition().x;
}
float TrafficLights_BackendPositionY(CEntity *entity)
{
    // Enter the original typed native field or action at the C decision point
    return entity->GetPosition().y;
}
float TrafficLights_BackendPathX(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_pathNodes[index].GetX();
}
float TrafficLights_BackendPathY(int32_t index)
{
    // Enter the original typed native field or action at the C decision point
    return ThePaths.m_pathNodes[index].GetY();
}
int32_t TrafficLights_BackendNextPathNodeInfo(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nNextPathNodeInfo;
}
int32_t TrafficLights_BackendCurrentPathNodeInfo(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nCurrentPathNodeInfo;
}
int32_t TrafficLights_BackendPreviousPathNodeInfo(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nPreviousPathNodeInfo;
}
int32_t TrafficLights_BackendNextRouteNode(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nNextRouteNode;
}
int32_t TrafficLights_BackendCurrentRouteNode(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nCurrentRouteNode;
}
int32_t TrafficLights_BackendPrevRouteNode(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nPrevRouteNode;
}
int32_t TrafficLights_BackendNextDirection(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nNextDirection;
}
int32_t TrafficLights_BackendCurrentDirection(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nCurrentDirection;
}
int32_t TrafficLights_BackendPreviousDirection(CVehicle *vehicle)
{
    // Enter the original typed native field or action at the C decision point
    return vehicle->AutoPilot.m_nPreviousDirection;
}
//- rouz edit (ChatGPT)
