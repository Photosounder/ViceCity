//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
typedef struct Range3DPoint { float x,y,z; } Range3DPoint;
typedef struct Range3D { Range3DPoint min,max; } Range3D;
#ifdef __cplusplus
extern "C" {
#endif
void Range3D_Init(Range3D *range,Range3DPoint min,Range3DPoint max);
bool Range3D_IsInRange(const Range3D *range,Range3DPoint point);
void Range3D_DebugShowRange(const Range3D *range,float value,int mode);
Range3DPoint Range3D_GetRandomPointInRange(const Range3D *range);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
