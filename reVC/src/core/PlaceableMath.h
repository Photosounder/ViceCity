//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

bool PlaceableMath_IsWithinArea2D(float px, float py, float x1, float y1, float x2, float y2);
bool PlaceableMath_IsWithinArea3D(float px, float py, float pz, float x1, float y1, float z1, float x2, float y2, float z2);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
