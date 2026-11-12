//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "GarageData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Garage geometry and door state helpers, a position is a 3D point
bool Garage_IsPointInside(const CGarageData *garage, float x, float y, float z);
bool Garage_IsPointInsideWithMargin(const CGarageData *garage, float x, float y, float z, float margin);
float Garage_CalcDistToRectangleSquared(const CGarageData *garage, float x, float y);
float Garage_CalcSmallestDistToDoorSquared(const CGarageData *garage, float x, float y);
void Garage_OpenThis(CGarageData *garage);
void Garage_CloseThis(CGarageData *garage);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
