//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include "PedPlacement.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void *PedPlacement_BackendSphere(float x,float y,float z,float radius);
bool PedPlacement_BackendVerticalLine(float x,float y,float z,float endZ,float *hitZ);
int16_t PedPlacement_BackendFindObjects(float x,float y,float z,float radius,int total,void *entities);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
