//+ rouz edit (Claude)
#include <math.h>
#include "config.h"
#include "CollisionTriangle.h"
#include "CollisionTests.h"
#include "VectorMath.h"

typedef struct V3 { float x, y, z; } V3;
typedef struct V2 { float x, y; } V2;

static float Cross2D(V2 a, V2 b)
{
	return a.x*b.y - a.y*b.x;
}

static V3 Sub3(V3 a, V3 b)
{
	V3 r = {a.x - b.x, a.y - b.y, a.z - b.z};
	return r;
}

static float Dot3(V3 a, V3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

static float Mag3(V3 a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

static V3 Decode(const CompressedVector *verts, int index)
{
	CollisionVector3 v = CompressedVector_Get(&verts[index]);
	V3 r = {v.x, v.y, v.z};
	return r;
}

// Whether p lies inside the triangle projected along the dominant axis of the plane
static bool PointInTriangle(int dir, V3 va, V3 vb, V3 vc, V3 p)
{
	V2 vec1, vec2, vec3, vect;

	// Project the vertices to 2D, the winding follows the plane direction
	switch (dir) {
	case DIR_X_POS:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vc.y; vec2.y = vc.z;
		vec3.x = vb.y; vec3.y = vb.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va.y; vec1.y = va.z;
		vec2.x = vb.y; vec2.y = vb.z;
		vec3.x = vc.y; vec3.y = vc.z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vc.z; vec2.y = vc.x;
		vec3.x = vb.z; vec3.y = vb.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va.z; vec1.y = va.x;
		vec2.x = vb.z; vec2.y = vb.x;
		vec3.x = vc.z; vec3.y = vc.x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vc.x; vec2.y = vc.y;
		vec3.x = vb.x; vec3.y = vb.y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
	default:
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	}

	// The point must be on the inner side of all three edges
	{
		V2 e12 = {vec2.x - vec1.x, vec2.y - vec1.y};
		V2 t1 = {vect.x - vec1.x, vect.y - vec1.y};
		V2 e13 = {vec3.x - vec1.x, vec3.y - vec1.y};
		V2 e23 = {vec3.x - vec2.x, vec3.y - vec2.y};
		V2 t2 = {vect.x - vec2.x, vect.y - vec2.y};
		if (Cross2D(e12, t1) < 0.0f) return false;
		if (Cross2D(e13, t1) > 0.0f) return false;
		if (Cross2D(e23, t2) < 0.0f) return false;
	}
	return true;
}

bool Collision_TestLineTriangle(const CColLine *line, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane)
{
	float t, p0dist;
	V3 normal, p, va, vb, vc, p0, p1;
	CollisionVector3 n;
	ColTrianglePlane_GetNormal(plane, &n.x, &n.y, &n.z);
	normal.x = n.x; normal.y = n.y; normal.z = n.z;
	p0.x = line->p0.x; p0.y = line->p0.y; p0.z = line->p0.z;
	p1.x = line->p1.x; p1.y = line->p1.y; p1.z = line->p1.z;

	// If both points are on the same side there is no collision
	if (ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(line->p0)) * ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(line->p1)) > 0.0f)
		return false;

	p0dist = Dot3(Sub3(p1, p0), normal);

#ifdef FIX_BUGS
	// A line lying in the plane is assumed not to collide
	if (p0dist == 0.0f)
		return false;
#endif

	// Intersection parameter and point on the line
	t = -ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(line->p0)) / p0dist;
	{
		V3 d = Sub3(p1, p0);
		p.x = p0.x + d.x*t;
		p.y = p0.y + d.y*t;
		p.z = p0.z + d.z*t;
	}

	va = Decode(verts, tri->a);
	vb = Decode(verts, tri->b);
	vc = Decode(verts, tri->c);
	return PointInTriangle(plane->dir, va, vb, vc, p);
}

bool Collision_ProcessLineTriangle(const CColLine *line, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane, CColPoint *point, float *mindist, float *outVerts)
{
	float t, p0dist;
	V3 normal, p, va, vb, vc, p0, p1;
	CollisionVector3 n;
	ColTrianglePlane_GetNormal(plane, &n.x, &n.y, &n.z);
	normal.x = n.x; normal.y = n.y; normal.z = n.z;
	p0.x = line->p0.x; p0.y = line->p0.y; p0.z = line->p0.z;
	p1.x = line->p1.x; p1.y = line->p1.y; p1.z = line->p1.z;

	// If both points are on the same side there is no collision
	if (ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(line->p0)) * ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(line->p1)) > 0.0f)
		return false;

	p0dist = Dot3(Sub3(p1, p0), normal);

#ifdef FIX_BUGS
	// A line lying in the plane is assumed not to collide
	if (p0dist == 0.0f)
		return false;
#endif

	// Intersection parameter on the line
	t = -ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(line->p0)) / p0dist;

	// Early out beyond the minimum distance
	if (t >= *mindist)
		return false;

	// Point of intersection
	{
		V3 d = Sub3(p1, p0);
		p.x = p0.x + d.x*t;
		p.y = p0.y + d.y*t;
		p.z = p0.z + d.z*t;
	}

	va = Decode(verts, tri->a);
	vb = Decode(verts, tri->b);
	vc = Decode(verts, tri->c);
	if (!PointInTriangle(plane->dir, va, vb, vc, p))
		return false;
	if (t >= *mindist)
		return false;

	point->point.x = p.x; point->point.y = p.y; point->point.z = p.z;
	point->normal.x = normal.x; point->normal.y = normal.y; point->normal.z = normal.z;
	point->surfaceA = 0;
	point->pieceA = 0;
	point->surfaceB = tri->surface;
	point->pieceB = 0;
	outVerts[0] = va.x; outVerts[1] = va.y; outVerts[2] = va.z;
	outVerts[3] = vb.x; outVerts[4] = vb.y; outVerts[5] = vb.z;
	outVerts[6] = vc.x; outVerts[7] = vc.y; outVerts[8] = vc.z;
	*mindist = t;
	return true;
}

// Find which of the six sectors around the triangle the sphere center lies in and its closest point, returns 0 for no result
static int SphereTriangleClosest(const CColSphere *sphere, V3 va, V3 vb, V3 vc, V3 normal, float planedist, float *outDist, V3 *outP)
{
	V3 c = {sphere->bounds.center.x, sphere->bounds.center.y, sphere->bounds.center.z};
	V3 vec2 = Sub3(vb, va);
	float len = Mag3(vec2);
	V3 vec1;
	V3 vac, vas;
	V2 b, cc, s;
	int insideAB, insideAC, insideBC, testcase;
	float dist = 0.0f;
	V3 p = {0.0f, 0.0f, 0.0f};
	float inv = 1.0f/len;

	// Calculate two orthogonal basis vectors for the triangle
	vec2.x = vec2.x * inv; vec2.y = vec2.y * inv; vec2.z = vec2.z * inv;
	{
		VectorMath3 a = {vec2.x, vec2.y, vec2.z};
		VectorMath3 nn = {normal.x, normal.y, normal.z};
		VectorMath3 r = VectorMath_Cross(a, nn);
		vec1.x = r.x; vec1.y = r.y; vec1.z = r.z;
	}

	// A has local coordinate [0,0] and B has [0,len], find the coordinates of C and the sphere center
	vac = Sub3(vc, va);
	vas = Sub3(c, va);
	b.x = 0.0f; b.y = len;
	cc.x = Dot3(vec1, vac); cc.y = Dot3(vec2, vac);
	s.x = Dot3(vec1, vas); s.y = Dot3(vec2, vas);

	// The three triangle lines partition the space into 6 sectors
	{
		V2 sb = {s.x - b.x, s.y - b.y};
		V2 cb = {cc.x - b.x, cc.y - b.y};
		insideAB = Cross2D(s, b) >= 0.0f;
		insideAC = Cross2D(cc, s) >= 0.0f;
		insideBC = Cross2D(sb, cb) >= 0.0f;
	}

	testcase = insideAB + insideAC + insideBC;
	switch (testcase) {
	case 0:
		return 0;
	case 1:
		// Closest to a vertex
		if (insideAB) p = vc;
		else if (insideAC) p = vb;
		else if (insideBC) p = va;
		dist = Mag3(Sub3(c, p));
		break;
	case 2: {
		// Closest to an edge
		float lv[6];
		const float *l0, *l1;
		float cl[3];
		float cf[3] = {c.x, c.y, c.z};
		lv[0] = va.x; lv[1] = va.y; lv[2] = va.z;
		lv[3] = vb.x; lv[4] = vb.y; lv[5] = vb.z;
		if (!insideAB) {
			l0 = &lv[0]; l1 = &lv[3];
			dist = Collision_DistToLineClosest(l0, l1, cf, cl);
		} else if (!insideAC) {
			float lv2[6] = {va.x, va.y, va.z, vc.x, vc.y, vc.z};
			dist = Collision_DistToLineClosest(&lv2[0], &lv2[3], cf, cl);
		} else {
			float lv3[6] = {vb.x, vb.y, vb.z, vc.x, vc.y, vc.z};
			dist = Collision_DistToLineClosest(&lv3[0], &lv3[3], cf, cl);
		}
		p.x = cl[0]; p.y = cl[1]; p.z = cl[2];
		break;
	}
	case 3:
		// The center is inside the triangle
		dist = fabsf(planedist);
		p.x = c.x - normal.x*planedist;
		p.y = c.y - normal.y*planedist;
		p.z = c.z - normal.z*planedist;
		break;
	}
	*outDist = dist;
	*outP = p;
	return testcase;
}

bool Collision_TestSphereTriangle(const CColSphere *sphere, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane)
{
	float planedist = ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(sphere->bounds.center));
	V3 va, vb, vc, normal, p;
	CollisionVector3 n;
	float dist;

	// If the sphere and the plane do not intersect there is no collision
	if (fabsf(planedist) > sphere->bounds.radius)
		return false;

	va = Decode(verts, tri->a);
	vb = Decode(verts, tri->b);
	vc = Decode(verts, tri->c);
	ColTrianglePlane_GetNormal(plane, &n.x, &n.y, &n.z);
	normal.x = n.x; normal.y = n.y; normal.z = n.z;
	if (SphereTriangleClosest(sphere, va, vb, vc, normal, planedist, &dist, &p) == 0)
		return false;
	return dist < sphere->bounds.radius;
}

bool Collision_ProcessSphereTriangle(const CColSphere *sphere, const CompressedVector *verts, const CColTriangle *tri, const CColTrianglePlane *plane, CColPoint *point, float *mindistsq)
{
	float planedist = ColTrianglePlane_CalcPoint(plane, COLLISION_POINT_REF(sphere->bounds.center));
	float distsq = planedist*planedist;
	V3 va, vb, vc, normal, p;
	CollisionVector3 n;
	float dist;

	// If the sphere and the plane do not intersect there is no collision
	if (fabsf(planedist) > sphere->bounds.radius || distsq > *mindistsq)
		return false;

	va = Decode(verts, tri->a);
	vb = Decode(verts, tri->b);
	vc = Decode(verts, tri->c);
	ColTrianglePlane_GetNormal(plane, &n.x, &n.y, &n.z);
	normal.x = n.x; normal.y = n.y; normal.z = n.z;
	if (SphereTriangleClosest(sphere, va, vb, vc, normal, planedist, &dist, &p) == 0)
		return false;

	if (dist >= sphere->bounds.radius || dist*dist >= *mindistsq)
		return false;

	point->point.x = p.x; point->point.y = p.y; point->point.z = p.z;
	point->normal.x = sphere->bounds.center.x - p.x;
	point->normal.y = sphere->bounds.center.y - p.y;
	point->normal.z = sphere->bounds.center.z - p.z;
	ColPoint_NormaliseNormal(point);
	point->surfaceA = sphere->surface;
	point->pieceA = sphere->piece;
	point->surfaceB = tri->surface;
	point->pieceB = 0;
	point->depth = sphere->bounds.radius - dist;
	*mindistsq = dist*dist;
	return true;
}
//- rouz edit (Claude)
