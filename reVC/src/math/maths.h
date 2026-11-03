//+ rouz edit (ChatGPT)
#pragma once
#include <math.h>

// wrapper around float versions of functions
// in gta they are in CMaths but that makes the code rather noisy

static inline float Sin(float x) { return sinf(x); }
static inline float Asin(float x) { return asinf(x); }
static inline float Cos(float x) { return cosf(x); }
static inline float Acos(float x) { return acosf(x); }
static inline float Tan(float x) { return tanf(x); }
static inline float Atan(float x) { return atanf(x); }
static inline float Atan2(float y, float x) { return atan2f(y, x); }
static inline float Abs(float x) { return fabsf(x); }
static inline float Sqrt(float x) { return sqrtf(x); }
static inline float RecipSqrtScaled(float x, float y) { return x/Sqrt(y); }
static inline float RecipSqrt(float x) { return RecipSqrtScaled(1.0f, x); }
static inline float Pow(float x, float y) { return powf(x, y); }
static inline float Floor(float x) { return floorf(x); }
static inline float Ceil(float x) { return ceilf(x); }
//- rouz edit (ChatGPT)
