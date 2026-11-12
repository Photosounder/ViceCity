//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include "VectorMath.h"

#ifdef __cplusplus
extern "C" {
#endif
// Directions of the shadow quad of a ped from the direction of the light, only x and y of the light are used
void ShadowMath_CalcPedShadowValues(float lightX, float lightY, float *frontX, float *frontY, float *sideX, float *sideY, float *displacementX, float *displacementY);
// Corners (x and y of points) of the shadow quad in the space of an entity and the box around it, from the direction the shadow is cast along (front and side) and the entity axes, rel is the shadow position relative to the entity
void ShadowMath_ProjectQuad(float frontX, float frontY, float sideX, float sideY, float rightX, float rightY, float forwardX, float forwardY, float relX, float relY,
	VectorMath3 points[4], float *minX, float *maxX, float *minY, float *maxY);
// Clips the shadow quad (the x and y of the four points) to the triangle of the collision model in the xy plane. list and texture hold 20 vertices, the clipped polygon is left at index 10 on and its vertex count is returned
int32_t ShadowMath_ClipQuadToTriangle(VectorMath3 *list, VectorMath3 *texture, const VectorMath3 *points, VectorMath3 pointA, VectorMath3 pointB, VectorMath3 pointC);
// Puts the clipped polygon (from index 10 on) on the plane of the triangle and moves it from the space of the entity to the world
void ShadowMath_ClippedToWorld(VectorMath3 *list, int32_t numVerts, VectorMath3 normal, VectorMath3 pointA, float forwardX, float forwardY, float rightX, float rightY, VectorMath3 entityPos);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
