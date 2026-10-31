//+ rouz edit (ChatGPT)
#pragma once
#include "Bridge.h"
#include "Pool.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef GTA_BRIDGE
CPool * Bridge_BackendBuildingPool(void);
CEntity * Bridge_BackendBuilding(void *slot);
int32_t Bridge_BackendModel(CEntity *entity);
int32_t Bridge_BackendLiftModel(void);
int32_t Bridge_BackendRoadModel(void);
int32_t Bridge_BackendWeightModel(void);
float Bridge_BackendPositionZ(CEntity *entity);
void Bridge_BackendSetPositionZ(CEntity *entity,float value);
void Bridge_BackendUpdateMatrix(CEntity *entity);
void Bridge_BackendUpdateFrame(CEntity *entity);
int32_t Bridge_BackendCommercialPassed(void);
void Bridge_BackendSetLights(float x1,float x2,float y1,float y2,bool enabled);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
