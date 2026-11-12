//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "ColBox.h"
#include "ColLine.h"
#include "ColSphere.h"

#ifdef __cplusplus
extern "C" {
#endif
// Overlap tests between collision primitives
bool Collision_TestSphereSphere(const CSphere *s1, const CSphere *s2);
bool Collision_TestSphereBox(const CSphere *sph, const CBox *box);
bool Collision_TestLineBox(const CColLine *line, const CBox *box);
bool Collision_TestVerticalLineBox(const CColLine *line, const CBox *box);
bool Collision_TestLineSphere(const CColLine *line, const CColSphere *sph);
// Distance of a point to the segment l0-l1 given as xyz triples, the second version also returns the closest point
float Collision_DistToLine(const float *l0, const float *l1, const float *point);
float Collision_DistToLineClosest(const float *l0, const float *l1, const float *point, float *closest);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
