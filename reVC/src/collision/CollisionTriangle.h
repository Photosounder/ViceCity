//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "ColLine.h"
#include "ColPoint.h"
#include "ColSphere.h"
#include "ColTriangle.h"
#include "CompressedVector.h"

#ifdef __cplusplus
extern "C" {
#endif
// Tests of lines and spheres against a collision triangle, verts is the vertex array of the model
bool Collision_TestLineTriangle(const CColLine *line, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane);
bool Collision_TestSphereTriangle(const CColSphere *sphere, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane);
// Closest contact, on a hit the three decoded vertices are written to outVerts as nine floats for the caller that tracks the polygon
bool Collision_ProcessLineTriangle(const CColLine *line, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane, CColPoint *point, float *mindist, float *outVerts);
bool Collision_ProcessSphereTriangle(const CColSphere *sphere, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane, CColPoint *point, float *mindistsq);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
