//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
typedef struct Range2DPoint { float x,y; } Range2DPoint;
typedef struct Range2D { Range2DPoint min,max; } Range2D;
#ifdef __cplusplus
extern "C" {
#endif
void Range2D_Init(Range2D *range,Range2DPoint min,Range2DPoint max);
bool Range2D_IsInRange(const Range2D *range,Range2DPoint point);
void Range2D_DebugShowRange(const Range2D *range,float value,int mode);
Range2DPoint Range2D_GetRandomPointInRange(const Range2D *range);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
