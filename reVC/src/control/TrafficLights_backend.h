//+ rouz edit (ChatGPT)
#pragma once
#include "TrafficLights.h"
typedef struct TrafficLightVector {float x,y,z;} TrafficLightVector;
#ifdef __cplusplus
class CBaseModelInfo;struct CPtrList;struct CPtrNode;
typedef CBaseModelInfo TrafficLightModel;
typedef CPtrList TrafficLightList;
typedef CPtrNode TrafficLightNode;
extern "C" {
#else
typedef struct CBaseModelInfo TrafficLightModel;
typedef struct CPtrList TrafficLightList;
typedef struct CPtrNode TrafficLightNode;
#endif
float TrafficLights_BackendUpZ(CEntity *entity);
bool TrafficLights_BackendDamaged(CEntity *entity);
int32_t TrafficLights_BackendModel(CEntity *entity);
TrafficLightModel * TrafficLights_BackendModelInfo(int32_t model);
bool TrafficLights_BackendHasEffect(TrafficLightModel *model,int32_t index);
TrafficLightVector TrafficLights_BackendEffectPosition(TrafficLightModel *model,int32_t index);
TrafficLightVector TrafficLights_BackendTransform(CEntity *entity,TrafficLightVector point);
TrafficLightVector TrafficLights_BackendCameraForward(void);
float TrafficLights_BackendGroundBrightness(void);
float TrafficLights_BackendSpriteBrightness(void);
float TrafficLights_BackendSpriteSize(void);
void TrafficLights_BackendShadow(uintptr_t id,TrafficLightVector position,uint8_t red,uint8_t green,uint8_t blue);
void TrafficLights_BackendCorona(uintptr_t id,uint8_t red,uint8_t green,uint8_t blue,TrafficLightVector position,float size);
int32_t TrafficLights_BackendNumSectorsX(void);
int32_t TrafficLights_BackendNumSectorsY(void);
TrafficLightList * TrafficLights_BackendSectorList(int32_t x,int32_t y);
TrafficLightNode * TrafficLights_BackendFirst(TrafficLightList *list);
TrafficLightNode * TrafficLights_BackendNext(TrafficLightNode *node);
CEntity * TrafficLights_BackendNodeEntity(TrafficLightNode *node);
int32_t TrafficLights_BackendCarNodeCount(void);
int32_t TrafficLights_BackendNodeCount(void);
TrafficLightVector TrafficLights_BackendPathPosition(int32_t index);
int32_t TrafficLights_BackendNumLinks(int32_t index);
int32_t TrafficLights_BackendFirstLink(int32_t index);
int32_t TrafficLights_BackendConnectedNode(int32_t index);
int32_t TrafficLights_BackendCarConnection(int32_t index);
uint8_t TrafficLights_BackendLightType(int32_t index);
void TrafficLights_BackendSetLightType(int32_t index,int32_t type);
bool TrafficLights_BackendLightDirection(int32_t index);
void TrafficLights_BackendSetLightDirection(int32_t index,bool direction);
int32_t TrafficLights_BackendLinkNode(int32_t index);
bool TrafficLights_BackendCrossesRoad(int32_t index);
void TrafficLights_BackendSetConnectionLight(int32_t index);
bool TrafficLights_BackendBridgeLight(int32_t index);
TrafficLightVector TrafficLights_BackendLinkPosition(int32_t index);
TrafficLightVector TrafficLights_BackendLinkDirection(int32_t index);
TrafficLightVector TrafficLights_BackendVehiclePosition(CVehicle *vehicle);
int32_t TrafficLights_BackendVehicleStatus(CVehicle *vehicle);
float TrafficLights_BackendEffectX(TrafficLightModel *model,int32_t index);
float TrafficLights_BackendEffectY(TrafficLightModel *model,int32_t index);
float TrafficLights_BackendEffectZ(TrafficLightModel *model,int32_t index);
TrafficLightVector TrafficLights_BackendUp(CEntity *entity);
TrafficLightVector TrafficLights_BackendRight(CEntity *entity);
TrafficLightVector TrafficLights_BackendForward(CEntity *entity);
float TrafficLights_BackendForwardX(CEntity *entity);
float TrafficLights_BackendForwardY(CEntity *entity);
float TrafficLights_BackendPositionX(CEntity *entity);
float TrafficLights_BackendPositionY(CEntity *entity);
float TrafficLights_BackendPathX(int32_t index);
float TrafficLights_BackendPathY(int32_t index);
int32_t TrafficLights_BackendNextPathNodeInfo(CVehicle *vehicle);
int32_t TrafficLights_BackendCurrentPathNodeInfo(CVehicle *vehicle);
int32_t TrafficLights_BackendPreviousPathNodeInfo(CVehicle *vehicle);
int32_t TrafficLights_BackendNextRouteNode(CVehicle *vehicle);
int32_t TrafficLights_BackendCurrentRouteNode(CVehicle *vehicle);
int32_t TrafficLights_BackendPrevRouteNode(CVehicle *vehicle);
int32_t TrafficLights_BackendNextDirection(CVehicle *vehicle);
int32_t TrafficLights_BackendCurrentDirection(CVehicle *vehicle);
int32_t TrafficLights_BackendPreviousDirection(CVehicle *vehicle);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
