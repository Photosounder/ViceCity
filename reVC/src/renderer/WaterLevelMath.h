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
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
