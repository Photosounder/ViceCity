//+ rouz edit (ChatGPT)
#include "Curves.h"
#include <math.h>

static CurvesVector Curves_Subtract(CurvesVector left, CurvesVector right)
{
    // Preserve all three components of the original vector subtraction
    CurvesVector result = { left.x - right.x, left.y - right.y, left.z - right.z };
    return result;
}

static CurvesVector Curves_Scale(CurvesVector left, float right)
{
    // Preserve the original component-wise vector multiplication
    CurvesVector result = { left.x * right, left.y * right, left.z * right };
    return result;
}

static float Curves_Magnitude2D(CurvesVector vector)
{
    // Use the original float sum and square root for planar distance
    return sqrtf(vector.x * vector.x + vector.y * vector.y);
}

static float Curves_Dot2D(CurvesVector left, CurvesVector right)
{
    // Use the original two-component dot product and addition order
    return left.x * right.x + left.y * right.y;
}

float Curves_CalcSpeedScaleFactor(CurvesVector pPoint1, CurvesVector pPoint2, float dir1X, float dir1Y, float dir2X, float dir2Y)
{
	// Retain the original curve arithmetic, branch boundaries and output order
	CurvesVector dir1 = { dir1X, dir1Y, 0.0f };
	CurvesVector dir2 = { dir2X, dir2Y, 0.0f };
	float distance = Curves_Magnitude2D(Curves_Subtract(pPoint1, pPoint2));
	float dp = Curves_Dot2D(dir1, dir2);
	if (dp > 0.9f)
		return distance + fabsf((pPoint1.x * dir1Y - pPoint1.y * dir1X) - (pPoint2.x * dir1Y - pPoint2.y * dir1X));
	else
		return ((1.0f - dp) * 0.25f + 1.0f) * distance;
}

void Curves_CalcCurvePoint(CurvesVector pPos1, CurvesVector pPos2, CurvesVector pDir1, CurvesVector pDir2, float between, int32_t timeOnCurve, CurvesVector* pOutPos, CurvesVector* pOutDir)
{
	// Retain the original curve arithmetic, branch boundaries and output order
	float actualFactor = Curves_CalcSpeedScaleFactor(pPos1, pPos2, pDir1.x, pDir1.y, pDir2.x, pDir2.y);
	CurvesVector dir1 = Curves_Scale(pDir1, actualFactor);
	CurvesVector dir2 = Curves_Scale(pDir2, actualFactor);
	float t1 = fabsf(Curves_Dot2D(Curves_Subtract(pPos2, pPos1), pDir1));
	float t2 = fabsf(Curves_Dot2D(Curves_Subtract(pPos1, pPos2), pDir2));
	float curveCoef;
	if (t1 > t2) {
		float coef = (t1 - t2) / (t1 + t2);
#ifdef FIX_BUGS
		if (between <= coef)
#else
		if (between < coef)
#endif
			curveCoef = 0.0f;
		else
			curveCoef = 0.5f - 0.5f * cosf(3.1415f * (between - coef) * (t1 + t2) / (2 * t2));
	}
	else {
		float coef = 2 * t1 / (t1 + t2);
#ifdef FIX_BUGS
		if (coef <= between)
#else
		if (coef < between)
#endif
			curveCoef = 1.0f;
		else
			curveCoef = 0.5f - 0.5f * cosf(3.1415f * between * (t1 + t2) / (2 * t1));
	}
	*pOutPos = (CurvesVector){
		(pPos1.x + between * dir1.x) * (1.0f - curveCoef) + (pPos2.x - (1 - between) * dir2.x) * curveCoef,
		(pPos1.y + between * dir1.y) * (1.0f - curveCoef) + (pPos2.y - (1 - between) * dir2.y) * curveCoef,
		0.0f};
	*pOutDir = (CurvesVector){
		(dir1.x * (1.0f - curveCoef) + dir2.x * curveCoef) / (timeOnCurve * 0.001f),
		(dir1.y * (1.0f - curveCoef) + dir2.y * curveCoef) / (timeOnCurve * 0.001f),
		0.0f};
}
//- rouz edit (ChatGPT)
