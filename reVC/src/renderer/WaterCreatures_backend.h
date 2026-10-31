//+ rouz edit (ChatGPT)
#pragma once
#include "WaterCreatures.h"
#include "Pool.h"
enum { WATER_CREATURE_TEMP_OBJECT=3,WATER_CREATURE_CONTROLLED_SUB_OBJECT=5 };
typedef void (*WaterCreatureSpawnCallback)(WaterCreatureVector,WaterCreaturePosition *,int32_t);
#ifdef __cplusplus
extern "C" {
#endif
int16_t * WaterCreatures_BackendTempCount(void);
CPool * WaterCreatures_BackendObjectPool(void);
int8_t WaterCreatures_BackendCreatedBy(CObject *object);
void WaterCreatures_BackendConstruct(CObject *object,int16_t modelID);
void WaterCreatures_BackendDestroy(CObject *object);
void WaterCreatures_BackendWorldRemove(CObject *object);
void WaterCreatures_BackendWorldAdd(CObject *object);
void WaterCreatures_BackendSetPosition(CObject *object,const WaterCreaturePosition *position);
void WaterCreatures_BackendUpdateRW(CObject *object);
void WaterCreatures_BackendRotateZ(CObject *object,float angle);
void WaterCreatures_BackendSetCreatedBy(CObject *object,int8_t value);
void WaterCreatures_BackendSetStatic(CObject *object,bool value);
void WaterCreatures_BackendSetTouchingWater(CObject *object,bool value);
void WaterCreatures_BackendSetUnderwater(CObject *object,bool value);
uint32_t * WaterCreatures_BackendEndTime(CObject *object);
void WaterCreatures_BackendSetAlpha(CObject *object,int32_t value);
bool WaterCreatures_BackendOnScreen(CObject *object);
WaterCreatureVector WaterCreatures_BackendPlayerPosition(void);
WaterCreatureVector WaterCreatures_BackendVector(const WaterCreaturePosition *position);
WaterCreatureVector WaterCreatures_BackendPosition(CObject *object);
WaterCreatureVector WaterCreatures_BackendRight(CObject *object);
const WaterCreaturePosition * WaterCreatures_BackendObjectPosition(CObject *object);
bool WaterCreatures_BackendVisible(WaterCreaturePosition *position,float radius);
bool WaterCreatures_BackendDepth(const WaterCreaturePosition *position,float *depth,float *level,float *ground);
void WaterCreatures_BackendSetZ(WaterCreaturePosition *position,float value);
void WaterCreatures_BackendSetMoveSpeed(CObject *object,WaterCreatureVector value);
void WaterCreatures_BackendSetTurnSpeed(CObject *object,WaterCreatureVector value);
void WaterCreatures_BackendWithFishPosition(const WaterCreaturePosition *position,WaterCreatureVector player,int32_t modelID,WaterCreatureSpawnCallback callback);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
