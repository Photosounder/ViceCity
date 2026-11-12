//+ rouz edit (Claude)
#pragma once
#include "PlaneNodes.h"

#ifdef __cplusplus
extern "C" {
#endif
// Hermite interpolation along a flight path between two nodes, f is 0 at node and 1 at next, the result goes to out[3]
void PlaneMath_InterpolatePathPosition(const CPlaneNode *nodes, int numNodes, float totalLength, bool loop, int node, int next, float f, float *out);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
