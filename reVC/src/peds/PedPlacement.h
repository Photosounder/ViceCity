//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool PedPlacement_CFindZ(float x,float y,float *z);
bool PedPlacement_CIsClear(float x,float y,float z,float radius,int total,void *entities);
void *PedPlacement_CFindCar(float x,float y,float z);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
