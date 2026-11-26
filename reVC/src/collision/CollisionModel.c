//+ rouz edit (Claude)
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "CollisionModel.h"
#include "CollisionTests.h"
#include "CollisionProcess.h"
#include "CollisionTriangle.h"
#include "MatrixMath.h"
#include "VectorMath.h"
#include "SurfaceTable.h"

void CollisionModel_CalculateTrianglePlanes(CLinkList *cache, CColModel *model)
{
	CLink *lptr;

	assert(model);
	if (model->numTriangles == 0)
		return;

	if (model->trianglePlanes) {
		// Re-insert at the front so it is not removed again soon
		lptr = ColModel_GetLinkPtr(model);
		CLink_Remove(lptr);
		CLink_Insert(&cache->head, lptr);
	} else {
		lptr = CLinkList_Insert(cache, model);
		if (lptr == NULL) {
			// Make room by removing the last in the list
			lptr = cache->tail.prev;
			assert(lptr);
			assert(lptr->item);
			ColModel_RemoveTrianglePlanes(((CColModel*)lptr->item));
			CLinkList_Remove(cache, lptr);

			// Now this cannot fail
			lptr = CLinkList_Insert(cache, model);
			assert(lptr);
		}
		ColModel_CalculateTrianglePlanes(model);
		ColModel_SetLinkPtr(model, lptr);
	}
}

// Move a world space line into model space
static CColLine TransformLine(const CColLine *line, const float matrix[4][4])
{
	float inv[4][4];
	VectorMath3 a = {line->p0.x, line->p0.y, line->p0.z};
	VectorMath3 b = {line->p1.x, line->p1.y, line->p1.z};
	VectorMath3 ta, tb;
	CColLine newline;

	MatrixMath_Invert(inv, matrix);
	ta = VectorMath_MatrixTimesVector((const float (*)[4])inv, a);
	tb = VectorMath_MatrixTimesVector((const float (*)[4])inv, b);
	ColLine_Init(&newline, COLLISION_POINT_REF(ta), COLLISION_POINT_REF(tb));
	return newline;
}

bool CollisionModel_TestLineOfSight(const CColLine *line, const float matrix[4][4], CColModel *model, bool ignoreSeeThrough, bool ignoreShootThrough, CLinkList *cache)
{
	int i;

	// Transform the line to model space
	CColLine newline = TransformLine(line, matrix);

	// If we do not intersect with the bounding box there is no chance on the rest
	if (!CollisionTests_TestLineBox(&newline, &model->boundingBox))
		return false;

	for (i = 0; i < model->numSpheres; i++) {
		if (ignoreSeeThrough && IsSeeThrough(model->spheres[i].surface)) continue;
		if (ignoreShootThrough && IsShootThrough(model->spheres[i].surface)) continue;
		if (CollisionTests_TestLineSphere(&newline, &model->spheres[i]))
			return true;
	}

	for (i = 0; i < model->numBoxes; i++) {
		if (ignoreSeeThrough && IsSeeThrough(model->boxes[i].surface)) continue;
		if (ignoreShootThrough && IsShootThrough(model->boxes[i].surface)) continue;
		if (CollisionTests_TestLineBox(&newline, &model->boxes[i].bounds))
			return true;
	}

	CollisionModel_CalculateTrianglePlanes(cache, model);
	for (i = 0; i < model->numTriangles; i++) {
		if (ignoreSeeThrough && IsSeeThrough(model->triangles[i].surface)) continue;
		if (ignoreShootThrough && IsShootThrough(model->triangles[i].surface)) continue;
		if (CollisionTriangle_TestLineTriangle(&newline, model->vertices, &model->triangles[i], &model->trianglePlanes[i]))
			return true;
	}

	return false;
}

bool CollisionModel_ProcessLineOfSight(const CColLine *line, const float matrix[4][4], CColModel *model, CColPoint *point, float *mindist, bool ignoreSeeThrough, bool ignoreShootThrough, CLinkList *cache)
{
	int i;
	float coldist;
	float verts[9];

	// Transform the line to model space
	CColLine newline = TransformLine(line, matrix);

	// If we do not intersect with the bounding box there is no chance on the rest
	if (!CollisionTests_TestLineBox(&newline, &model->boundingBox))
		return false;

	coldist = *mindist;
	for (i = 0; i < model->numSpheres; i++) {
		if (ignoreSeeThrough && IsSeeThrough(model->spheres[i].surface)) continue;
		if (ignoreShootThrough && IsShootThrough(model->spheres[i].surface)) continue;
		CollisionProcess_ProcessLineSphere(&newline, &model->spheres[i], point, &coldist);
	}

	for (i = 0; i < model->numBoxes; i++) {
		if (ignoreSeeThrough && IsSeeThrough(model->boxes[i].surface)) continue;
		if (ignoreShootThrough && IsShootThrough(model->boxes[i].surface)) continue;
		CollisionProcess_ProcessLineBox(&newline, &model->boxes[i], point, &coldist);
	}

	CollisionModel_CalculateTrianglePlanes(cache, model);
	for (i = 0; i < model->numTriangles; i++) {
		if (ignoreSeeThrough && IsSeeThrough(model->triangles[i].surface)) continue;
		if (ignoreShootThrough && IsShootThrough(model->triangles[i].surface)) continue;
		CollisionTriangle_ProcessLineTriangle(&newline, model->vertices, &model->triangles[i], &model->trianglePlanes[i], point, &coldist, verts);
	}

	// A contact was found, bring it back to world space
	if (coldist < *mindist) {
		VectorMath3 pp = {point->point.x, point->point.y, point->point.z};
		VectorMath3 pn = {point->normal.x, point->normal.y, point->normal.z};
		VectorMath3 wp = VectorMath_MatrixTimesVector(matrix, pp);
		VectorMath3 wn = VectorMath_MatrixTimesVector3x3(matrix, pn);
		point->point.x = wp.x; point->point.y = wp.y; point->point.z = wp.z;
		point->normal.x = wn.x; point->normal.y = wn.y; point->normal.z = wn.z;
		*mindist = coldist;
		return true;
	}
	return false;
}
// Store the three vertices of the hit triangle in a polygon record
static void StoreTriangle(CStoredCollPoly *poly, const float *v)
{
	StoredPolyVertex s0, s1, s2;
	s0.x = v[0]; s0.y = v[1]; s0.z = v[2];
	s1.x = v[3]; s1.y = v[4]; s1.z = v[5];
	s2.x = v[6]; s2.y = v[7]; s2.z = v[8];
	StoredPoly_SetVertex(poly, 0, &s0);
	StoredPoly_SetVertex(poly, 1, &s1);
	StoredPoly_SetVertex(poly, 2, &s2);
	poly->valid = true;
}

// Rotate a point by the inverse of a matrix after removing its position
static VectorMath3 MultiplyInverse(const float m[4][4], float x, float y, float z)
{
	float vx = x - m[3][0];
	float vy = y - m[3][1];
	float vz = z - m[3][2];
	VectorMath3 r = {
		m[0][0] * vx + m[0][1] * vy + m[0][2] * vz,
		m[1][0] * vx + m[1][1] * vy + m[1][2] * vz,
		m[2][0] * vx + m[2][1] * vy + m[2][2] * vz};
	return r;
}

bool CollisionModel_ProcessVerticalLine(const CColLine *line, const float matrix[4][4], CColModel *model, CColPoint *point, float *mindist, bool ignoreSeeThrough, CStoredCollPoly *poly, CLinkList *cache)
{
	static CStoredCollPoly TempStoredPoly;
	int i;
	float coldist;
	float verts[9];
	VectorMath3 ta = MultiplyInverse(matrix, line->p0.x, line->p0.y, line->p0.z);
	VectorMath3 tb = MultiplyInverse(matrix, line->p1.x, line->p1.y, line->p1.z);
	CColLine newline;

	// Transform the line to model space
	ColLine_Init(&newline, COLLISION_POINT_REF(ta), COLLISION_POINT_REF(tb));

	if (!CollisionTests_TestLineBox(&newline, &model->boundingBox))
		return false;

	// Only the see through flag counts here, as in the original
	coldist = *mindist;
	for (i = 0; i < model->numSpheres; i++) {
		if (ignoreSeeThrough && IsSeeThroughVertical(model->spheres[i].surface)) continue;
		CollisionProcess_ProcessLineSphere(&newline, &model->spheres[i], point, &coldist);
	}

	for (i = 0; i < model->numBoxes; i++) {
		if (ignoreSeeThrough && IsSeeThroughVertical(model->boxes[i].surface)) continue;
		CollisionProcess_ProcessLineBox(&newline, &model->boxes[i], point, &coldist);
	}

	CollisionModel_CalculateTrianglePlanes(cache, model);
	TempStoredPoly.valid = false;
	for (i = 0; i < model->numTriangles; i++) {
		if (ignoreSeeThrough && IsSeeThroughVertical(model->triangles[i].surface)) continue;
		if (CollisionTriangle_ProcessLineTriangle(&newline, model->vertices, &model->triangles[i], &model->trianglePlanes[i], point, &coldist, verts))
			StoreTriangle(&TempStoredPoly, verts);
	}

	// A contact was found, bring it back to world space
	if (coldist < *mindist) {
		VectorMath3 pp = {point->point.x, point->point.y, point->point.z};
		VectorMath3 pn = {point->normal.x, point->normal.y, point->normal.z};
		VectorMath3 wp = VectorMath_MatrixTimesVector(matrix, pp);
		VectorMath3 wn = VectorMath_MatrixTimesVector3x3(matrix, pn);
		point->point.x = wp.x; point->point.y = wp.y; point->point.z = wp.z;
		point->normal.x = wn.x; point->normal.y = wn.y; point->normal.z = wn.z;
		if (TempStoredPoly.valid && poly) {
			int k;
			*poly = TempStoredPoly;
			for (k = 0; k < 3; k++) {
				VectorMath3 v = {poly->verts[k].x, poly->verts[k].y, poly->verts[k].z};
				VectorMath3 w = VectorMath_MatrixTimesVector(matrix, v);
				StoredPolyVertex s;
				s.x = w.x; s.y = w.y; s.z = w.z;
				StoredPoly_SetVertex(poly, k, &s);
			}
		}
		*mindist = coldist;
		return true;
	}
	return false;
}
#define MAXNUMSPHERES 256
#define MAXNUMBOXES 64
#define MAXNUMLINES 32
#define MAXNUMTRIS 1200

// Move a point by a matrix, a rotation and a translation
static CollisionVector3 TransformPoint(const float m[4][4], CollisionVector3 p)
{
	VectorMath3 v = {p.x, p.y, p.z};
	VectorMath3 r = VectorMath_MatrixTimesVector(m, v);
	CollisionVector3 out = {r.x, r.y, r.z};
	return out;
}

// Move a direction by the rotation of a matrix
static CollisionVector3 TransformDirection(const float m[4][4], CollisionVector3 p)
{
	VectorMath3 v = {p.x, p.y, p.z};
	VectorMath3 r = VectorMath_MatrixTimesVector3x3(m, v);
	CollisionVector3 out = {r.x, r.y, r.z};
	return out;
}

int CollisionModel_ProcessColModels(const float matrixA[4][4], CColModel *modelA, const float matrixB[4][4], CColModel *modelB, CColPoint *spherepoints, CColPoint *linepoints, float *linedists, CLinkList *cache)
{
	static int aSphereIndicesA[MAXNUMSPHERES];
	static int aLineIndicesA[MAXNUMLINES];
	static int aSphereIndicesB[MAXNUMSPHERES];
	static int aBoxIndicesB[MAXNUMBOXES];
	static int aTriangleIndicesB[MAXNUMTRIS];
	static bool aCollided[MAXNUMLINES];
	static CColSphere aSpheresA[MAXNUMSPHERES];
	static CColLine aLinesA[MAXNUMLINES];
	static float matAB[4][4], matBA[4][4];
	float tmp[4][4];
	CColSphere s;
	CColSphere bsphereAB;
	int i, j;
	int numSpheresA, numLinesA, numSpheresB, numBoxesB, numTrianglesB, numCollisions;
	float verts[9];

	assert(modelA->numSpheres <= MAXNUMSPHERES);
	assert(modelA->numLines <= MAXNUMLINES);

	// From model A space to model B space
	MatrixMath_Invert(matAB, matrixB);
	MatrixMath_Multiply(tmp, (const float (*)[4])matAB, matrixA);
	memcpy(matAB, tmp, sizeof(tmp));

	// Bounding sphere of A in B space
	bsphereAB.bounds.radius = modelA->boundingSphere.radius;
	bsphereAB.bounds.center = TransformPoint((const float (*)[4])matAB, modelA->boundingSphere.center);
	if (!CollisionTests_TestSphereBox(&bsphereAB.bounds, &modelB->boundingBox))
		return 0;

	// B to A space
	MatrixMath_Invert(matBA, matrixA);
	MatrixMath_Multiply(tmp, (const float (*)[4])matBA, matrixB);
	memcpy(matBA, tmp, sizeof(tmp));

	// Transform the spheres and lines of model A to B space
	for (i = 0; i < modelA->numSpheres; i++) {
		CColSphere *src = &modelA->spheres[i];
		CollisionVector3 center = TransformPoint((const float (*)[4])matAB, src->bounds.center);
		ColSphere_Set(&aSpheresA[i], src->bounds.radius, COLLISION_POINT_REF(center), src->surface, src->piece);
	}
	for (i = 0; i < modelA->numLines; i++) {
		CollisionVector3 start = TransformPoint((const float (*)[4])matAB, modelA->lines[i].p0);
		CollisionVector3 end = TransformPoint((const float (*)[4])matAB, modelA->lines[i].p1);
		ColLine_Set(&aLinesA[i], COLLISION_POINT_REF(start), COLLISION_POINT_REF(end));
	}

	// Test them against the bounding volumes of model B
	numSpheresA = 0;
	numLinesA = 0;
	for (i = 0; i < modelA->numSpheres; i++)
		if (CollisionTests_TestSphereBox(&aSpheresA[i].bounds, &modelB->boundingBox))
			aSphereIndicesA[numSpheresA++] = i;

	// There is no actual check for lines
	for (i = 0; i < modelA->numLines; i++)
		aLineIndicesA[numLinesA++] = i;

	// No collision
	if (numSpheresA == 0 && numLinesA == 0)
		return 0;

	// Check model B against the bounding volumes of A
	numSpheresB = 0;
	numBoxesB = 0;
	numTrianglesB = 0;
	for (i = 0; i < modelB->numSpheres; i++) {
		s.bounds.radius = modelB->spheres[i].bounds.radius;
		s.bounds.center = TransformPoint((const float (*)[4])matBA, modelB->spheres[i].bounds.center);
		if (CollisionTests_TestSphereBox(&s.bounds, &modelA->boundingBox))
			aSphereIndicesB[numSpheresB++] = i;
	}
	for (i = 0; i < modelB->numBoxes; i++)
		if (CollisionTests_TestSphereBox(&bsphereAB.bounds, &modelB->boxes[i].bounds))
			aBoxIndicesB[numBoxesB++] = i;
	CollisionModel_CalculateTrianglePlanes(cache, modelB);
	for (i = 0; i < modelB->numTriangles; i++)
		if (CollisionTriangle_TestSphereTriangle(&bsphereAB, modelB->vertices, &modelB->triangles[i], &modelB->trianglePlanes[i]))
			aTriangleIndicesB[numTrianglesB++] = i;
	assert(numSpheresB <= MAXNUMSPHERES);
	assert(numBoxesB <= MAXNUMBOXES);
	assert(numTrianglesB <= MAXNUMTRIS);

	// No collision
	if (numSpheresB == 0 && numBoxesB == 0 && numTrianglesB == 0)
		return 0;

	// Process the spheres of A against the collision volumes of B
	numCollisions = 0;
	for (i = 0; i < numSpheresA; i++) {
		float coldist = 1.0e24f;
		bool hasCollided = false;

		for (j = 0; j < numSpheresB; j++)
			hasCollided |= CollisionProcess_ProcessSphereSphere(&aSpheresA[aSphereIndicesA[i]], &modelB->spheres[aSphereIndicesB[j]], &spherepoints[numCollisions], &coldist);
		for (j = 0; j < numBoxesB; j++)
			hasCollided |= CollisionProcess_ProcessSphereBox(&aSpheresA[aSphereIndicesA[i]], &modelB->boxes[aBoxIndicesB[j]], &spherepoints[numCollisions], &coldist);
		for (j = 0; j < numTrianglesB; j++)
			hasCollided |= CollisionTriangle_ProcessSphereTriangle(&aSpheresA[aSphereIndicesA[i]], modelB->vertices, &modelB->triangles[aTriangleIndicesB[j]], &modelB->trianglePlanes[aTriangleIndicesB[j]], &spherepoints[numCollisions], &coldist);

		if (hasCollided)
			numCollisions++;
	}
	for (i = 0; i < numCollisions; i++) {
		spherepoints[i].point = TransformPoint(matrixB, spherepoints[i].point);
		spherepoints[i].normal = TransformDirection(matrixB, spherepoints[i].normal);
	}

	// And the same thing for the lines of A
	for (i = 0; i < numLinesA; i++) {
		aCollided[i] = false;

		for (j = 0; j < numSpheresB; j++)
			aCollided[i] |= CollisionProcess_ProcessLineSphere(&aLinesA[aLineIndicesA[i]], &modelB->spheres[aSphereIndicesB[j]], &linepoints[aLineIndicesA[i]], &linedists[aLineIndicesA[i]]);
		for (j = 0; j < numBoxesB; j++)
			aCollided[i] |= CollisionProcess_ProcessLineBox(&aLinesA[aLineIndicesA[i]], &modelB->boxes[aBoxIndicesB[j]], &linepoints[aLineIndicesA[i]], &linedists[aLineIndicesA[i]]);
		for (j = 0; j < numTrianglesB; j++)
			aCollided[i] |= CollisionTriangle_ProcessLineTriangle(&aLinesA[aLineIndicesA[i]], modelB->vertices, &modelB->triangles[aTriangleIndicesB[j]], &modelB->trianglePlanes[aTriangleIndicesB[j]], &linepoints[aLineIndicesA[i]], &linedists[aLineIndicesA[i]], verts);
	}
	for (i = 0; i < numLinesA; i++)
		if (aCollided[i]) {
			j = aLineIndicesA[i];
			linepoints[j].point = TransformPoint(matrixB, linepoints[j].point);
			linepoints[j].normal = TransformDirection(matrixB, linepoints[j].normal);
		}

	// Number of sphere collisions
	return numCollisions;
}
//- rouz edit (Claude)
