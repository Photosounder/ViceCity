//+ rouz edit (ChatGPT)
#pragma once
#include "MovingThings.h"
#ifdef __cplusplus
extern "C" {
#endif
int32_t MovingThings_BackendSectorsX(void);
int32_t MovingThings_BackendSectorsY(void);
void * MovingThings_BackendSectorFirst(int32_t x,int32_t y);
void * MovingThings_BackendBigFirst(int32_t level);
void * MovingThings_BackendNodeNext(void *node);
CEntity * MovingThings_BackendNodeEntity(void *node);
MovingThingVector MovingThings_BackendPosition(CEntity *entity);
MovingThingVector MovingThings_BackendCameraPosition(void);
float MovingThings_BackendPositionAxis(CEntity *entity,int32_t axis);
float MovingThings_BackendCameraAxis(int32_t axis);
void MovingThings_BackendSetRight(CEntity *entity,float x,float y,float z);
void MovingThings_BackendSetForward(CEntity *entity,float x,float y,float z);
void MovingThings_BackendSetUp(CEntity *entity,float x,float y,float z);
void MovingThings_BackendSetPosition(CEntity *entity,float x,float y,float z);
void MovingThings_BackendUpdateRW(CEntity *entity);
void MovingThings_BackendUpdateFrame(CEntity *entity);
int16_t MovingThings_BackendModelIndex(CEntity *entity);
void MovingThings_BackendCorona(MovingThingVector position);
void MovingThings_BackendPush(const char *name);
void MovingThings_BackendPop(void);
int16_t MovingThings_BackendModelLIGHTBEAM(void);
int16_t MovingThings_BackendModelAIRPORTRADAR(void);
int16_t MovingThings_BackendModelMALLFAN(void);
int16_t MovingThings_BackendModelHOTELFAN_NIGHT(void);
int16_t MovingThings_BackendModelHOTELFAN_DAY(void);
int16_t MovingThings_BackendModelHOTROOMFAN(void);
int16_t MovingThings_BackendModelBLIMP_NIGHT(void);
int16_t MovingThings_BackendModelBLIMP_DAY(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
