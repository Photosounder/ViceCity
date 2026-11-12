//+ rouz edit (Claude)
#pragma once
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
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
