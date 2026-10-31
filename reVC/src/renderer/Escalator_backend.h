//+ rouz edit (ChatGPT)
#pragma once
#include "Escalator.h"
#include "Pool.h"
enum { ESCALATOR_TEMP_OBJECT=3,ESCALATOR_CONTROLLED_SUB_OBJECT=5 };
#ifdef __cplusplus
extern "C" {
#endif
float Escalator_BackendStepHeight(void);
EscalatorVector Escalator_BackendCameraPosition(void);
bool Escalator_BackendSphereVisible(EscalatorVector center,float radius);
CPool * Escalator_BackendObjectPool(void);
int8_t Escalator_BackendCreatedBy(CObject *object);
void Escalator_BackendConstruct(CObject *object);
void Escalator_BackendDestroy(CObject *object);
void Escalator_BackendWorldAdd(CObject *object);
void Escalator_BackendWorldRemove(CObject *object);
EscalatorVector Escalator_BackendPosition(CObject *object);
void Escalator_BackendSetPosition(CObject *object,EscalatorVector value);
void Escalator_BackendSetCreatedBy(CObject *object,int8_t value);
void Escalator_BackendAssignMatrix(CObject *object,int32_t index);
void Escalator_BackendSetMoveSpeed(CObject *object,EscalatorVector value);
void Escalator_BackendUpdateRW(CObject *object);
void Escalator_BackendUpdateFrame(CObject *object);
void Escalator_BackendMatrixUp(int32_t index,EscalatorVector value);
void Escalator_BackendMatrixForward(int32_t index,EscalatorVector value);
void Escalator_BackendMatrixRight(int32_t index,EscalatorVector value);
void Escalator_BackendMatrixPosition(int32_t index,EscalatorVector value);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
