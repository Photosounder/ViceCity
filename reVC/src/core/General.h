//+ rouz edit (ChatGPT)
#pragma once
#include "VectorFwd.h" // rouz edit (Claude)
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif
float General_GetATanOfXY(float x, float y);
float General_LimitAngle(float angle);
float General_LimitRadianAngle(float angle);
float General_GetRadianAngleBetweenPoints(float x1, float y1, float x2, float y2);
float General_GetAngleBetweenPoints(float x1, float y1, float x2, float y2);
int General_GetNodeHeadingFromVector(float x, float y);
bool General_faststrcmp(const char *str1, const char *str2);
bool General_faststrncmp(const char *str1, const char *str2, uint32_t count);
bool General_faststricmp(const char *str1, const char *str2);
bool General_SolveQuadratic(float a, float b, float c, float *root1, float *root2);
uint16_t General_GetRandomNumber(void);
bool General_GetRandomTrueFalse(void);
float General_GetRandomNumberInRangeFloat(float low, float high);
int32_t General_GetRandomNumberInRangeInt(int32_t low, int32_t high);
void General_SetRandomSeed(int32_t seed);
//+ rouz edit (Claude)
// Subtracts step from value for as long as value is above limit
float General_ReduceAbove(float value, float limit, float step);
// Adds step to value for as long as value is below limit
float General_RaiseBelow(float value, float limit, float step);
// Subtracts step from value for as long as value is at least limit
float General_ReduceAtLeast(float value, float limit, float step);
// Rotation angle in degrees (negated) of a map object from the w component of its quaternion
float General_ObjectRotationDegrees(float w);
//- rouz edit (Claude)
// Arc sine of a value limited to -1..1
float General_AsinClamped(float value);
// Arc cosine of a value limited to -1..1
float General_AcosClamped(float value);
// Value limited between a low and a high bound
float General_ClampFloat(float value, float low, float high);
// The lower of two values
float General_MinFloat(float a, float b);
// The higher of two values
float General_MaxFloat(float a, float b);
// The higher of two integers
int General_MaxInt(int a, int b);
// The lower of two integers
int General_MinInt(int a, int b);
// Whether two values differ by less than a limit
bool General_AbsDiffBelow(float a, float b, float limit);
// Whether two values differ by more than a limit
bool General_AbsDiffAbove(float a, float b, float limit);
// Whether two values differ by at least a limit
bool General_AbsDiffAtLeast(float a, float b, float limit);
// Whether the absolute value is below a limit
bool General_AbsBelow(float value, float limit);
// Whether the absolute value is above a limit
bool General_AbsAbove(float value, float limit);
// Whether the absolute value of a is greater than the absolute value of b
bool General_AbsGreater(float a, float b);
// Whether the sum of the absolute values of two offsets is above a limit
bool General_ManhattanAbove(float dx, float dy, float limit);
// Whether both values are below a limit in absolute value
bool General_AbsBothBelow(float x, float y, float limit);
// Whether both values are above a limit in absolute value
bool General_AbsBothAbove(float x, float y, float limit);
// Whether either value is above a limit in absolute value
bool General_AbsEitherAbove(float x, float y, float limit);
// Mask of the axes whose component is above a threshold in absolute value: 1 for x, 2 for y, 4 for z
int General_AxisFlagMask(float x, float y, float z, float threshold);
// Whether any of three values is at least a limit in absolute value
bool General_AnyAbsAtLeast3(float a, float b, float c, float limit);
// Absolute value of the difference between two values
float General_AbsDiff(float a, float b);
// Normalised direction of a vector flattened onto the ground (its z set to zero)
void General_NormalisedHorizontal(const CVector *v, CVector *out);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
