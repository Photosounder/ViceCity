//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"
typedef struct CurvesVector { float x, y, z; } CurvesVector;
#ifdef __cplusplus
#include "Vector.h"
static inline CurvesVector CurvesVector_FromVector(const CVector &vector)
{
    // Copy coordinates into a C value without relying on the C++ vector layout
    CurvesVector result = { vector.x, vector.y, vector.z };
    return result;
}
extern "C" {
#endif
float Curves_CalcSpeedScaleFactor(CurvesVector pPoint1, CurvesVector pPoint2, float dir1X, float dir1Y, float dir2X, float dir2Y);
void Curves_CalcCurvePoint(CurvesVector pPos1, CurvesVector pPos2, CurvesVector pDir1, CurvesVector pDir2, float between, int32_t timeOnCurve, CurvesVector *pOutPos, CurvesVector *pOutDir);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
