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
// Shape of a blip without a sprite from the heights of the blip and the player: 0 triangle up for blips above the player, 1 triangle down for blips well below, 2 square for the others
int Radar_BlipHeightMode(float blipZ, float playerZ);
// Radius of the debug marker of a blip after a step, it starts over at 5 when it gets below 1
float Radar_DebugRadiusStep(float radius);
// Position of a corner (0 to 3) of the rotated square marker drawn on the radar, from its centre, its size on both axes, the corrected angle and the angle step between corners
void Radar_MarkerCorner(int corner, float x, float y, float sizeX, float sizeY, float correctedAngle, float *outX, float *outY);
// Point of a quarter circle of the rounded corner of the radar, from the corner and the step (0 to 6)
void Radar_QuarterCirclePoint(float cornerX, float cornerY, int step, float *outX, float *outY);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
