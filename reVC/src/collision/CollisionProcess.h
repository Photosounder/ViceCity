//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "ColBox.h"
#include "ColLine.h"
#include "ColPoint.h"
#include "ColSphere.h"

#ifdef __cplusplus
extern "C" {
#endif
// Closest contact between collision primitives, spheres report the squared distance to the center in mindistsq and lines a position in [0,1] along the line in mindist
bool CollisionProcess_ProcessSphereSphere(const CColSphere *s1, const CColSphere *s2, CColPoint *point, float *mindistsq);
bool CollisionProcess_ProcessSphereBox(const CColSphere *sph, const CColBox *box, CColPoint *point, float *mindistsq);
bool CollisionProcess_ProcessLineBox(const CColLine *line, const CColBox *box, CColPoint *point, float *mindist);
bool CollisionProcess_ProcessLineSphere(const CColLine *line, const CColSphere *sphere, CColPoint *point, float *mindist);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
