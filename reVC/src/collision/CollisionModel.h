//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "ColModel.h"
#include "LinkList.h"
#include "StoredCollPoly.h"

#ifdef __cplusplus
extern "C" {
#endif
// Make sure a model has its triangle planes, models that were used recently keep them in the cache list
void CollisionModel_CalculateTrianglePlanes(CLinkList *cache, CColModel *model);
// Tests and contacts of a line given in world space against a model placed by a matrix (float[4][4], CMatrix layout)
bool CollisionModel_TestLineOfSight(const CColLine *line, const float matrix[4][4], CColModel *model, bool ignoreSeeThrough, bool ignoreShootThrough, CLinkList *cache);
bool CollisionModel_ProcessLineOfSight(const CColLine *line, const float matrix[4][4], CColModel *model, CColPoint *point, float *mindist, bool ignoreSeeThrough, bool ignoreShootThrough, CLinkList *cache);
// Same for a vertical line, which also reports the polygon that was hit when poly is not NULL
bool CollisionModel_ProcessVerticalLine(const CColLine *line, const float matrix[4][4], CColModel *model, CColPoint *point, float *mindist, bool ignoreSeeThrough, CStoredCollPoly *poly, CLinkList *cache);
// Collide the spheres and lines of model A with the volumes of model B, both placed by matrices (float[4][4], CMatrix layout)
// Sphere contacts go to spherepoints and the count is returned, line contacts go to linepoints and linedists by line index
int CollisionModel_ProcessColModels(const float matrixA[4][4], CColModel *modelA, const float matrixB[4][4], CColModel *modelB, CColPoint *spherepoints, CColPoint *linepoints, float *linedists, CLinkList *cache);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
