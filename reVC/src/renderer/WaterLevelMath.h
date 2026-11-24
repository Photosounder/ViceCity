//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "VectorMath.h"

#ifdef __cplusplus
extern "C" {
#endif
// Water level at a point of a water block: the level of the block plus the waves, 0 and false when the water is more than 3 units above the point (unless bDontCheckZ), fX is not yet offset and x and y are the small sector of the point
bool WaterMath_LevelWithWaves(float *level, uint32_t timeMs, float windClipped, float fX, float fY, float fZ, bool dontCheckZ, int32_t x, int32_t y);
// Height of the waves of a small sector without a base level
float WaterMath_WavesOnly(uint32_t timeMs, float windClipped, int16_t x, int16_t y);
// Normal of the water surface at a point whose small sector is x and y
VectorMath3 WaterMath_Normal(uint32_t timeMs, float windClipped, float fX, float fY, int32_t x, int32_t y);
// Distance up to which water is drawn from the height of the camera
float WaterMath_WaterDrawDist(float cameraZ);
// Distance up to which the waves are drawn, larger when the player drives a boat
float WaterMath_WavyDrawDist(bool playerInBoat);
// Which edges of the water sectors are clipped to the camera when it looks mostly sideways, the flags are only set
void WaterMath_CamBounds(float camForwardX, float camForwardY, float camForwardZ, bool *useCamStartY, bool *useCamEndY, bool *useCamStartX, bool *useCamEndX);
// Radius of a square water sector
float WaterMath_SectorRadius(float size);
// Finds the water block that is hit when a 3x3 area of fine sectors is tested, returns false when none is
bool WaterMath_CollidesWithBlock(const int8_t blockList[128][128], int32_t x, int32_t y, int32_t *block);
// Angles of the two waves at a vertex of a wavy water sector, from the halved morph offsets of the vertex, the wave angle and the inverse length of the second wave
void WaterMath_WaveAngles(float morphVertXHalf, float morphVertYHalf, float angle, float wave2InvLength, float *waveMulA, float *waveMulB);
// Height of a vertex of a wavy water sector and the heights of the two tangent vectors used for its normal
void WaterMath_WaveHeights(float waveMulA, float waveMulB, float wind, float waveWind, float waveA, float waveB, float *z, float *tangentAZ, float *tangentBZ);
// Scrolling of the water textures for a frame: adds to the u and v offsets of the mask and the wake from the angle, the wind and the normalised time step
void WaterMath_ScrollTextures(float angle, float windAddUV, float timeStep, float *maskU, float *maskV, float *wakeU, float *wakeV);
// Slopes of the two waves of wavy water, from the camera direction on the x and y axes, the wind, the normal direction scalars and offsets of both waves and the normal scale of the second wave
void WaterMath_WaveSlopes(float camX, float camY, float windClipped, float dirScalar1, float normMult, float dirScalar2, float normMultB, float wave2NormScale, float *waveA, float *waveB);
// Sector index of the water limited to the range 0 to the last sector
int32_t WaterMath_ClampSectorIndex(int32_t index, int32_t lastSector);
// Size of a point of the wake of a boat in intervals, from its index, the base size, the shrink factor per remaining lifetime, the lifetime left and the interval between wake points: 1 when it would be negative
float WaterMath_WakeSizeInIntervals(int index, float size, float lifeTimeMult, float timeLeft, float minInterval);
// Alpha of a point of the wake of a boat from the lifetime and the age of the point: less lifetime left means less alpha, limited to 0..255
float WaterMath_WakeAlpha(float wakeLifetime, float pointLifetime);
// Corners of a segment of the wake of a boat between two points, from the two points, the sizes and the directions at each: corners[0..1] is (AA), [2..3] (AB), [4..5] (BA) and [6..7] (BB)
void WaterMath_WakeSegmentCorners(const float pointA[2], const float pointB[2], float sizeA, float sizeB, const float distA[2], const float distB[2], float corners[8]);
// Alpha blend multiplier of a corner of flat water, from the squared distances to the camera on both axes, the blend range, the draw distance and the minimum multiplier (the distance is taken in double precision as in the original)
float WaterMath_FlatWaterBlend(float distX, float distY, float blendRange, float blendDrawDist, float minAlphaMult);
// Normalises the direction between two wake points when the squared distance is above 9 (in double precision as in the original) and reports whether the segment is short enough to be rendered
bool WaterMath_NormaliseWakeDirection(float dir[2], float squaredDistance);
// Alpha of a light mask cell: the alpha reduced by the alpha multiplier times the sum of the distances of the cell to the offset on both axes, never negative
int WaterLevel_MaskAlpha(int alpha, float alphaMul, float dx, float dy);
// Whether a water particle moved by more than a limit, as the sum of its movement on the three axes
bool WaterLevel_MovedFarther(float dx, float dy, float dz, float limit);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
