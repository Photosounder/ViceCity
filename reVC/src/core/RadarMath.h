//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "RadarTraceData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Opacity of a blip by its distance in radar units
uint8_t Radar_CalculateBlipAlpha(bool menuMapActive, float dist);
// Clip a quad of four xy points to the radar square, poly receives up to eight xy points and the number of points is returned
int Radar_ClipRadarPoly(float *poly, const float *rect);
// RGBA colour of a trace colour index or the colour itself
uint32_t Radar_GetRadarTraceColour(uint32_t color, bool bright);
// Pull a point inside the unit circle, returns the distance from the centre before that
float Radar_LimitRadarPoint(float *point, bool menuMapActive);
// Convert between world space and radar space, with the rotation as sine and cosine
void Radar_TransformRealWorldPointToRadarSpace(float *out, const float *in, float s, float c, const float *origin, float radarRange);
void Radar_TransformRadarPointToRealWorldSpace(float *out, const float *in, float s, float c, const float *origin, float radarRange);
// Blip handles hold the array index in the low 16 bits and a counter in the high 16 bits that tells a stale handle from the current one
int32_t Radar_GetActualBlipArrayIndex(const sRadarTraceData *traces, int32_t handle);
int32_t Radar_GetNewUniqueBlipIndex(sRadarTraceData *traces, int32_t index);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
