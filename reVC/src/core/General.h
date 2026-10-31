//+ rouz edit (ChatGPT)
#pragma once
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
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
