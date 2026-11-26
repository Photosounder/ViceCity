//+ rouz edit (Claude)
#pragma once
#include "VectorFwd.h" // rouz edit (Claude)
#include <stdint.h>
#include <stdbool.h>

// Corner of a polygon in the sector grid, the layout of RwV2d
typedef struct RendererV2 { float x, y; } RendererV2;

#ifdef __cplusplus
extern "C" {
#endif
// Calls scan for every sector (x, y) a convex polygon of the sector grid touches, line by line from the top, only sectors from 0 to numSectorsX - 1 and 0 to numSectorsY - 1 are given
void RendererLogic_ScanSectorPoly(const RendererV2 *poly, int32_t numVertices, int32_t numSectorsX, int32_t numSectorsY, void (*scan)(int32_t x, int32_t y, void *context), void *context);
// Alpha of an object that fades in at its largest draw distance, from its alpha and how far the camera is inside the fade distance
uint32_t RendererLogic_DistanceFadeAlpha(uint8_t alpha, float largestDistance, float camDist, float fadeDistance);
// Whether an object that is not loaded is close enough to be requested: its distance minus the stream distance is inside its largest draw distance
bool RendererLogic_WithinStreamDistance(float dist, float largestDistance, float streamDistance);
// Corners of the box of an occluder and which of its six faces (x+, x-, y+, y-, z+, z-) face the camera: from the centre of the box and the camera, its length, width and height and its angle, corners[8][3] receives the corners in the order the occluder code expects
void Occluder_BuildBox(const float pos[3], const float cam[3], float length, float width, float height, float angle, float corners[8][3], bool frontFace[6]);
// Whether the segment between two points crosses an infinite line given by a point and a direction (the points lie on opposite sides of it)
bool Occlusion_CrossesLine(float p1X, float p1Y, float p2X, float p2Y, float lineX, float lineY, float lineDX, float lineDY);
// Whether an infinite line given by a point and a direction touches the screen of the given size
bool Occlusion_LineTouchesScreen(float lineX, float lineY, float lineDX, float lineDY, float screenWidth, float screenHeight);
// Whether a point is on the inner side of a line given by a point and a direction, at least by the given area
bool Occlusion_IsPointInsideLine(float lineX, float lineY, float lineDX, float lineDY, float pX, float pY, float area);
// Quarter of the diagonal of a box of the given size, the side used to test whether it is hidden by an occluder
float Occlusion_BoxSide(float width, float length, float height);
// Area on the screen a test of occlusion allows, from the size of one unit on the screen in x and y and the side of the tested object
float Occlusion_TestArea(float screenW, float screenH, float side);
// Whether an occluder is far enough in front of an object to hide it, from the depth of the object, its side and the radius of the occluder
bool Occlusion_OccluderInFront(float depth, float side, float radius);
// Whether a component of a vehicle may be hidden because it faces away from the camera: only far enough from the camera, for components that may be culled and when the camera does not look steeply down on the vehicle
bool Visibility_CompCullCheckNeeded(float distToCameraSq, float cullCompsDist, bool noCull, float pitchToCamera);
// Whether a component of a vehicle faces away from the camera (dot product above 0) and is hidden, always when it has the angle cull flag and otherwise only when it is far enough
bool Visibility_CompIsHidden(float dot, bool angleCull, float distToCameraSq);
// Dot product of the direction from the camera to a vehicle atomic with the forward (front and rear atomics) or right (left and right atomics) vector of the vehicle, negated for left and rear atomics and widened by the same product for doors (flags are the ATOMIC_FLAG_* values, forward and right are the vectors of the clump)
float Visibility_DotWithCameraVector(const CVector *atomicPos, const CVector *cameraPos, const CVector *clumpForward, const CVector *clumpRight, unsigned int flags);

// Pitch angle from a vehicle to the camera, from the vector between them (camera position minus vehicle position)
float Visibility_PitchToCamera(float dx, float dy, float dz);

// Alpha of an entity that fades out near its draw distance, from the largest LOD distance, the distance to the camera, the fade distance and the base alpha of the model
unsigned int Visibility_FadeAlpha(float largestDistance, float camDist, float fadeDistance, unsigned int modelAlpha);

// Extends the screen bounds of an occluder (minimum and maximum on x and y) with the two end points of one of its edges
void Occlusion_ExtendBounds(float *minX, float *maxX, float *minY, float *maxY, float p1x, float p1y, float p2x, float p2y);
// The larger of two sides of an occluder
float Occlusion_LargerSide(float a, float b);
// Distance used for a static map entity seen from high altitude beyond its largest LOD distance: just under that distance, never negative
float Visibility_HighAltitudeLodDistance(float largestLodDist);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
