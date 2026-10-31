//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "SurfaceTypes.h"
#ifdef __cplusplus
struct CColPoint;
extern "C" {
#else
typedef struct CColPoint CColPoint;
#endif
void SurfaceTable_Initialise(const char *filename);
int SurfaceTable_GetAdhesionGroup(uint8_t surfaceType);
float SurfaceTable_GetWetMultiplier(uint8_t surfaceType);
float SurfaceTable_GetAdhesiveLimit(const CColPoint *colpoint);
bool SurfaceTable_IsSoftLanding(uint8_t surfaceType);
bool IsSeeThrough(uint8_t surfaceType);
bool IsSeeThroughVertical(uint8_t surfaceType);
bool IsShootThrough(uint8_t surfaceType);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
