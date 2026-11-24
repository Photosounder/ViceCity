//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
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
// Strength of an object or ped shadow that fades out with the distance to the camera: full strength up to a quarter of the draw distance, then fading with a multiplier that is not clamped
int32_t ShadowMath_FadedStrength(float distToCam, float drawDistance, int32_t shadowStrength);
// Strength of a vehicle shadow that fades out in the last quarter of the draw distance
int32_t ShadowMath_CarFadedStrength(float distToCam, float drawDistance, int32_t shadowStrength);
// Rectangle covered by a shadow on the ground, from its position and its front and side vectors: start and end on the x and y axes
void ShadowMath_ShadowExtent(float posX, float posY, float frontX, float frontY, float sideX, float sideY, float *startX, float *endX, float *startY, float *endY);
// Factor the shadow of a bike is widened by, from its lean angle and, when it is under physics, the z of its right vector (when above 0.6)
float ShadowMath_BikeWidthMultiplier(float leanAngle, bool isPhysics, float rightZ);
// Directions on the ground the shadow of a vehicle is stretched along (front and side), from its forward, right and up vectors: a bike on its side or a heli on its side or nose uses the up vector instead
void ShadowMath_VehicleAxes(bool isBike, bool isHeli, const float forward[3], const float right[3], const float up[3], float front[2], float side[2]);
// Kinds of vehicle models that adjust the size of their shadow
enum { SHADOWMODEL_NONE, SHADOWMODEL_MOPED, SHADOWMODEL_BIKE, SHADOWMODEL_HELI, SHADOWMODEL_RCCOPTER, SHADOWMODEL_DODO };
// Draw distance of the shadow of a vehicle: 144 for planes and helicopters, 18 for the others
float ShadowMath_VehicleDrawDistance(bool isFlyer);
// Adjusts the height, the width and the size of the shadow of a vehicle for the kind of its model
void ShadowMath_ModelAdjust(int kind, float *height, float *width, float *size);
// Fades out the intensity and the colour of a static shadow in the last quarter of its draw distance (in place), from the squared distance to the camera on the ground and the draw distance (nothing happens for a draw distance of 0)
void ShadowMath_FadeStaticShadow(float distToCamSqr, float drawDistance, int32_t *intensity, int32_t *red, int32_t *green, int32_t *blue);
// Fades out the colour of the additive shadow of the headlights of a car in the last quarter of its draw distance (in place), from the distance to the camera and the draw distance
void ShadowMath_FadeHeadlightShadow(float distToCam, float drawDistance, int32_t *red, int32_t *green, int32_t *blue);
// Whether a static shadow is still close enough to its stored position on both axes (within the up distance) to be refreshed
bool ShadowMath_StaticShadowIsNear(float posX, float posY, float storedX, float storedY, float upDistance);
// Whether a static shadow is at the same place as the stored one: within 0.05 on both axes and 2 on the height
bool ShadowMath_StaticShadowIsSamePlace(float posX, float posY, float posZ, float storedX, float storedY, float storedZ);
// Whether two stored shadows have the same geometry: the position (3 values), front (2 values) and side (2 values) all within 0.01 of each other
bool ShadowMath_SameGeometry(const float a[7], const float b[7]);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
