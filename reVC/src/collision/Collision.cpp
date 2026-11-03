//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "StoredCollPoly_native.h"
#include "StoredCollPoly_backend.h"
#include "ColSphere_native.h"
#include "CollisionVector_native.h"
#include "ColLine_native.h"

#include "VuVector.h"
#include "main.h"
#include "Lists.h"
#include "Game.h"
#include "Zones.h"
#include "General.h"
#include "ZoneCull.h"
#include "World.h"
#include "Entity.h"
#include "Train.h"
#include "Streaming.h"
#include "Pad.h"
#include "DMAudio.h"
#include "Population.h"
#include "FileLoader.h"
#include "Replay.h"
#include "CutsceneMgr.h"
#include "RenderBuffer.h"
#include "SurfaceTable.h"
#include "Lines.h"
#include "Collision.h"
#include "Camera.h"
#include "ColStore.h"

#ifdef VU_COLLISION
#include "VuCollision.h"

inline int
GetVUresult(void)
{
#ifdef GTA_PS2
	int ret;
	__asm__ volatile (
		"cfc2.i	%0,vi01\n"	// .i important! wait for VU0 to finish
		: "=r" (ret)
	);
	return ret;
#else
	return vi01;
#endif
}

inline int
GetVUresult(CVuVector &point, CVuVector &normal, float &dist)
{
#ifdef GTA_PS2
	int ret;
	__asm__ volatile (
		"cfc2.i	%0,vi01\n"	// .i important! wait for VU0 to finish
		"sqc2	vf01,(%1)\n"
		"sqc2	vf02,(%2)\n"
		"qmfc2	$12,vf03\n"
		"sw	$12,(%3)\n"
		: "=r" (ret)
		: "r" (&point), "r" (&normal), "r" (&dist)
		: "$12"
	);
	return ret;
#else
	point = vf01;
	normal = vf02;
	dist = vf03.x;
	return vi01;
#endif
}

#endif

#ifdef VU_COLLISION
bool StoredPoly_BackendLine(const StoredPolyVertex *p0,const StoredPolyVertex *p1,const CStoredCollPoly *poly,const StoredPolyVertex *plane,CollisionVector3 *point)
{
    // Transfer aligned C quadwords into actual native VU vector objects
    CVuVector a,b,v0,v1,v2,n;
    memcpy(&a,p0,16);memcpy(&b,p1,16);memcpy(&v0,&poly->verts[0],16);memcpy(&v1,&poly->verts[1],16);memcpy(&v2,&poly->verts[2],16);memcpy(&n,plane,16);
    LineToTriangleCollision(a,b,v0,v1,v2,n);
    // Capture the original VU outputs even on failure and publish XYZ only for a successful result
    CVuVector result;float distance;const int hit=GetVUresult(result,n,distance);
    if(hit){memcpy(&point->x,&result.x,sizeof(float));memcpy(&point->y,&result.y,sizeof(float));memcpy(&point->z,&result.z,sizeof(float));}
    return hit!=0;
}
#endif

eLevelName CCollision::ms_collisionInMemory;
//+ rouz edit (ChatGPT)
CLinkList CCollision::ms_colModelCache;
//- rouz edit (ChatGPT)

void
CCollision::Init(void)
{
	// Zero-initialize list nodes here so CITA records the owning subsystem
	//+ rouz edit (ChatGPT)
	CLinkList_Init(&ms_colModelCache, (CLink*)calloc(NUMCOLCACHELINKS, sizeof(CLink)), NUMCOLCACHELINKS);
	//- rouz edit (ChatGPT)
	ms_collisionInMemory = LEVEL_GENERIC;
	CColStore::Initialise();
}

void
CCollision::Shutdown(void)
{
	// Free the subsystem node array and reset its list sentinels
	//+ rouz edit (ChatGPT)
	free(ms_colModelCache.links);
	CLinkList_Init(&ms_colModelCache, NULL, 0);
	//- rouz edit (ChatGPT)
	CColStore::Shutdown();
}

void
CCollision::Update(void)
{
}

// unused
eLevelName
GetCollisionInSectorList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *e;
	int level;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		level = ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(e->GetModelIndex())))->level; // rouz edit (ChatGPT)
		if(level != LEVEL_GENERIC)
			return (eLevelName)level;
	}
	return LEVEL_GENERIC;
}

// unused
// Get a level this sector is in based on collision models
eLevelName
GetCollisionInSector(CSector &sect)
{
	int level;

	level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_BUILDINGS]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_OBJECTS]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_DUMMIES]);
	if(level == LEVEL_GENERIC)
		level = GetCollisionInSectorList(sect.m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
	return (eLevelName)level;
}

void
CCollision::LoadCollisionWhenINeedIt(bool forceChange)
{
}

void
CCollision::SortOutCollisionAfterLoad(void)
{
	CColStore::LoadCollision(TheCamera.GetPosition());
	CStreaming::LoadAllRequestedModels(false);
}

void
CCollision::LoadCollisionScreen(eLevelName level)
{
	static Const char *levelNames[] = {
		"",
		"IND_ZON",
		"COM_ZON",
	};

	// Why twice?
	LoadingIslandScreen(levelNames[level]);
	LoadingIslandScreen(levelNames[level]);
}

//
// Test
//


bool
CCollision::TestSphereSphere(const CSphere &s1, const CSphere &s2)
{
	float d = s1.radius + s2.radius;
	return (CollisionVector_ToNative(s1.center) - CollisionVector_ToNative(s2.center)).MagnitudeSqr() < d*d;
}

bool
CCollision::TestSphereBox(const CSphere &sph, const CBox &box)
{
	if(sph.center.x + sph.radius < box.min.x) return false;
	if(sph.center.x - sph.radius > box.max.x) return false;
	if(sph.center.y + sph.radius < box.min.y) return false;
	if(sph.center.y - sph.radius > box.max.y) return false;
	if(sph.center.z + sph.radius < box.min.z) return false;
	if(sph.center.z - sph.radius > box.max.z) return false;
	return true;
}

bool
CCollision::TestLineBox(const CColLine &line, const CBox &box)
{
	float t, x, y, z;
	// If either line point is in the box, we have a collision
	if(line.p0.x > box.min.x && line.p0.x < box.max.x &&
	   line.p0.y > box.min.y && line.p0.y < box.max.y &&
	   line.p0.z > box.min.z && line.p0.z < box.max.z)
		return true;
	if(line.p1.x > box.min.x && line.p1.x < box.max.x &&
	   line.p1.y > box.min.y && line.p1.y < box.max.y &&
	   line.p1.z > box.min.z && line.p1.z < box.max.z)
		return true;

	// check if points are on opposite sides of min x plane
	if((box.min.x - line.p1.x) * (box.min.x - line.p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box.min.x - line.p0.x) / (line.p1.x - line.p0.x);
		// y of intersection
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			// z of intersection
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// same test with max x plane
	if((line.p1.x - box.max.x) * (line.p0.x - box.max.x) < 0.0f){
		t = (line.p0.x - box.max.x) / (line.p0.x - line.p1.x);
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.min.y && y < box.max.y){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// min y plne
	if((box.min.y - line.p0.y) * (box.min.y - line.p1.y) < 0.0f){
		t = (box.min.y - line.p0.y) / (line.p1.y - line.p0.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// max y plane
	if((line.p0.y - box.max.y) * (line.p1.y - box.max.y) < 0.0f){
		t = (line.p0.y - box.max.y) / (line.p0.y - line.p1.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.min.z && z < box.max.z)
				return true;
		}
	}

	// min z plne
	if((box.min.z - line.p0.z) * (box.min.z - line.p1.z) < 0.0f){
		t = (box.min.z - line.p0.z) / (line.p1.z - line.p0.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				return true;
		}
	}

	// max z plane
	if((line.p0.z - box.max.z) * (line.p1.z - box.max.z) < 0.0f){
		t = (line.p0.z - box.max.z) / (line.p0.z - line.p1.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.min.x && x < box.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.min.y && y < box.max.y)
				return true;
		}
	}
	return false;
}

bool
CCollision::TestVerticalLineBox(const CColLine &line, const CBox &box)
{
	if(line.p0.x <= box.min.x) return false;
	if(line.p0.y <= box.min.y) return false;
	if(line.p0.x >= box.max.x) return false;
	if(line.p0.y >= box.max.y) return false;
	if(line.p0.z < line.p1.z){
		if(line.p0.z > box.max.z) return false;
		if(line.p1.z < box.min.z) return false;
	}else{
		if(line.p1.z > box.max.z) return false;
		if(line.p0.z < box.min.z) return false;
	}
	return true;
}

bool
CCollision::TestLineTriangle(const CColLine &line, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	CompressedVector_Unpack(&verts[tri.a],&vutri.v0);
	CompressedVector_Unpack(&verts[tri.b],&vutri.v1);
	CompressedVector_Unpack(&verts[tri.c],&vutri.v2);
	ColTrianglePlane_Unpack(&plane,&vutri.plane);

	LineToTriangleCollisionCompressed(ColLine_ToVu(line,0), ColLine_ToVu(line,1), vutri);

	if(GetVUresult())
		return true;
	return false;
#else
	float t;
	CVector normal;
	ColTrianglePlane_GetNormal(&plane,&normal.x,&normal.y,&normal.z);

	// if points are on the same side, no collision
	if(ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(line.p0)) * ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(line.p1)) > 0.0f)
		return false;

	float p0dist = DotProduct(CollisionVector_ToNative(line.p1) - CollisionVector_ToNative(line.p0), normal);

#ifdef FIX_BUGS
	// line lines in the plane, assume no collision
	if (p0dist == 0.0f)
		return false;
#endif

	// intersection parameter on line
	t = -ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(line.p0)) / p0dist;
	// find point of intersection
	CVector p = CollisionVector_ToNative(line.p0) + (CollisionVector_ToNative(line.p1)-CollisionVector_ToNative(line.p0))*t;

	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue0_0=CompressedVector_Get(&verts[tri.a]);
	const CVector &va = CVector(collisionVertexValue0_0.x,collisionVertexValue0_0.y,collisionVertexValue0_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue1_0=CompressedVector_Get(&verts[tri.b]);
	const CVector &vb = CVector(collisionVertexValue1_0.x,collisionVertexValue1_0.y,collisionVertexValue1_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue2_0=CompressedVector_Get(&verts[tri.c]);
	const CVector &vc = CVector(collisionVertexValue2_0.x,collisionVertexValue2_0.y,collisionVertexValue2_0.z);
	CVector2D vec1, vec2, vec3, vect;

	// We do the test in 2D. With the plane direction we
	// can figure out how to project the vectors.
	// normal = (c-a) x (b-a)
	switch(plane.dir){
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
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	// This is our triangle:
	// 3-------2
	//  \  P  /
	//   \   /
	//    \ /
	//     1
	// We can use the "2d cross product" to check on which side
	// a vector is of another. Test is true if point is inside of all edges.
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	return true;
#endif
}

// Test if line segment intersects with sphere.
// If the first point is inside the sphere this test does not register a collision!
// The code is reversed from the original code and rather ugly, see Process for a clear version.
// TODO: actually rewrite this mess
bool
CCollision::TestLineSphere(const CColLine &line, const CColSphere &sph)
{
	CVector v01 = CollisionVector_ToNative(line.p1) - CollisionVector_ToNative(line.p0);	// vector from p0 to p1
	CVector v0c = CollisionVector_ToNative(sph.bounds.center) - CollisionVector_ToNative(line.p0);	// vector from p0 to center
	float linesq = v01.MagnitudeSqr();
	// I leave in the strange -2 factors even though they serve no real purpose
	float projline = -2.0f * DotProduct(v01, v0c);	// project v0c onto line
	// Square of tangent from p0 multiplied by line length so we can compare with projline.
	// The length of the tangent would be this: Sqrt((c-p0)^2 - r^2).
	// Negative if p0 is inside the sphere! This breaks the test!
	float tansq = 4.0f * linesq *
		(CollisionVector_ToNative(sph.bounds.center).MagnitudeSqr() - 2.0f*DotProduct(CollisionVector_ToNative(sph.bounds.center), CollisionVector_ToNative(line.p0)) + CollisionVector_ToNative(line.p0).MagnitudeSqr() - sph.bounds.radius*sph.bounds.radius);
	float diffsq = projline*projline - tansq;
	// if diffsq < 0 that means the line is a passant, so no intersection
	if(diffsq < 0.0f)
		return false;
	// projline (negative in GTA for some reason) is the point on the line
	// in the middle of the two intersection points (startin from p0).
	// Sqrt(diffsq) somehow works out to be the distance from that
	// midpoint to the intersection points.
	// So subtract that and get rid of the awkward scaling:
	float f = (-projline - Sqrt(diffsq)) / (2.0f*linesq);
	// f should now be in range [0, 1] for [p0, p1]
	return f >= 0.0f && f <= 1.0f;
}

bool
CCollision::TestSphereTriangle(const CColSphere &sphere,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	CompressedVector_Unpack(&verts[tri.a],&vutri.v0);
	CompressedVector_Unpack(&verts[tri.b],&vutri.v1);
	CompressedVector_Unpack(&verts[tri.c],&vutri.v2);
	ColTrianglePlane_Unpack(&plane,&vutri.plane);

	SphereToTriangleCollisionCompressed(Sphere_ToVu(sphere.bounds), vutri);

	if(GetVUresult())
		return true;
	return false;
#else
	// If sphere and plane don't intersect, no collision
	float planedist = ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(sphere.bounds.center));
	if(Abs(planedist) > sphere.bounds.radius)
		return false;

	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue3_0=CompressedVector_Get(&verts[tri.a]);
	const CVector &va = CVector(collisionVertexValue3_0.x,collisionVertexValue3_0.y,collisionVertexValue3_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue4_0=CompressedVector_Get(&verts[tri.b]);
	const CVector &vb = CVector(collisionVertexValue4_0.x,collisionVertexValue4_0.y,collisionVertexValue4_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue5_0=CompressedVector_Get(&verts[tri.c]);
	const CVector &vc = CVector(collisionVertexValue5_0.x,collisionVertexValue5_0.y,collisionVertexValue5_0.z);

	// calculate two orthogonal basis vectors for the triangle
	CVector vec2 = vb - va;
	float len = vec2.Magnitude();
	vec2 = vec2 * (1.0f/len);
	CVector normal;
	ColTrianglePlane_GetNormal(&plane,&normal.x,&normal.y,&normal.z);
	CVector vec1 = CrossProduct(vec2, normal);

	// We know A has local coordinate [0,0] and B has [0,len].
	// Now calculate coordinates on triangle for these two vectors:
	CVector vac = vc - va;
	CVector vas = CollisionVector_ToNative(sphere.bounds.center) - va;
	CVector2D b(0.0f, len);
	CVector2D c(DotProduct(vec1, vac), DotProduct(vec2, vac));
	CVector2D s(DotProduct(vec1, vas), DotProduct(vec2, vas));

	// The three triangle lines partition the space into 6 sectors,
	// find out in which the center lies.
	int insideAB = CrossProduct2D(s, b) >= 0.0f;
	int insideAC = CrossProduct2D(c, s) >= 0.0f;
	int insideBC = CrossProduct2D(s-b, c-b) >= 0.0f;

	int testcase = insideAB + insideAC + insideBC;
	float dist = 0.0f;
	switch(testcase){
	case 0:
		return false;	// shouldn't happen
	case 1:
		// closest to a vertex
		if(insideAB) dist = (CollisionVector_ToNative(sphere.bounds.center) - vc).Magnitude();
		else if(insideAC) dist = (CollisionVector_ToNative(sphere.bounds.center) - vb).Magnitude();
		else if(insideBC) dist = (CollisionVector_ToNative(sphere.bounds.center) - va).Magnitude();
		else assert(0);
		break;
	case 2: {
		// closest to an edge
		// looks like original game as DistToLine manually inlined
		// Keep a native point alive through the existing edge-distance query
		const CVector sphereNativeCenter=CollisionVector_ToNative(sphere.bounds.center);
		if(!insideAB) dist = DistToLine(&va, &vb, &sphereNativeCenter);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphereNativeCenter);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphereNativeCenter);
		else assert(0);
		break;
	}
	case 3:
		// center is in triangle
		dist = Abs(planedist);
		break;
	default:
		assert(0);
	}

	return dist < sphere.bounds.radius;
#endif
}

bool
CCollision::TestLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSeeThrough, bool ignoreShootThrough)
{
#ifdef VU_COLLISION
	CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CVuVector newline[2];
	TransformColLines(newline,1,matTransform,&line);

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(ColLine_FromRaw32(newline), model.boundingBox))
		return false;

	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.spheres[i].surface)) continue;
		if(TestLineSphere(ColLine_FromRaw32(newline), model.spheres[i]))
			return true;
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.boxes[i].surface)) continue;
		if(TestLineBox(ColLine_FromRaw32(newline), model.boxes[i].bounds))
			return true;
	}

	CalculateTrianglePlanes(&model);
	int lastTest = -1;
	VuTriangle vutri;
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		CompressedVector_Unpack(&model.vertices[tri->a],&vutri.v0);
		CompressedVector_Unpack(&model.vertices[tri->b],&vutri.v1);
		CompressedVector_Unpack(&model.vertices[tri->c],&vutri.v2);
		ColTrianglePlane_Unpack(&model.trianglePlanes[i],&vutri.plane);

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lastTest = i;
		break;
	}
#ifdef FIX_BUGS
	// no need to check first again
	i++;
#endif
	for(; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		CompressedVector_Unpack(&model.vertices[tri->a],&vutri.v0);
		CompressedVector_Unpack(&model.vertices[tri->b],&vutri.v1);
		CompressedVector_Unpack(&model.vertices[tri->c],&vutri.v2);
		ColTrianglePlane_Unpack(&model.trianglePlanes[i],&vutri.plane);

		if(GetVUresult())
			return true;

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lastTest = i;

	}
	if(lastTest != -1 && GetVUresult())
		return true;

	return false;
#else
	static CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	// Retain the line record at its original construction boundary
	CColLine newline;
	// Capture the complete native expression before initializing line endpoints
	const CVector &lineConstructorPoint0_0=matTransform * CollisionVector_ToNative(line.p0);
	// Capture the complete native expression before initializing line endpoints
	const CVector &lineConstructorPoint0_1=matTransform * CollisionVector_ToNative(line.p1);
	// Initialize the line at the former parameterized constructor boundary
	ColLine_Init(&newline,COLLISION_POINT_REF(lineConstructorPoint0_0),COLLISION_POINT_REF(lineConstructorPoint0_1));

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.spheres[i].surface)) continue;
		if(TestLineSphere(newline, model.spheres[i]))
			return true;
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.boxes[i].surface)) continue;
		if(TestLineBox(newline, model.boxes[i].bounds))
			return true;
	}

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.triangles[i].surface)) continue;
		if(TestLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i]))
			return true;
	}

	return false;
#endif
}

// TODO: TestPillWithSpheresInColModel, but only called from overloaded CWeapon::FireMelee which isn't used

//
// Process
//

// For Spheres mindist is the squared distance to its center
// For Lines mindist is between [0,1]

bool
CCollision::ProcessSphereSphere(const CColSphere &s1, const CColSphere &s2, CColPoint &point, float &mindistsq)
{
	CVector dist = CollisionVector_ToNative(s1.bounds.center) - CollisionVector_ToNative(s2.bounds.center);
	float d = dist.Magnitude() - s2.bounds.radius;	// distance from s1's center to s2
	float depth = s1.bounds.radius - d;	// sphere overlap
	if(d < 0.0f) d = 0.0f;		// clamp to zero, i.e. if s1's center is inside s2
	// no collision if sphere is not close enough
	if(d*d < mindistsq && d < s1.bounds.radius){
		dist.Normalise();
		point.point = CollisionVector_FromNative(CollisionVector_ToNative(s1.bounds.center) - dist*d);
		point.normal = CollisionVector_FromNative(dist);
#ifndef VU_COLLISION
		point.surfaceA = s1.surface;
		point.pieceA = s1.piece;
		point.surfaceB = s2.surface;
		point.pieceB = s2.piece;
#endif
		point.depth = depth;	
		mindistsq = d*d;		// collision radius
		return true;
	}
	return false;
}

bool
CCollision::ProcessSphereBox(const CColSphere &sph, const CColBox &box, CColPoint &point, float &mindistsq)
{
	CVector p;
	CVector dist;

	// GTA's code is too complicated, uses a huge 3x3x3 if statement
	// we can simplify the structure a lot

	// first make sure we have a collision at all
	if(sph.bounds.center.x + sph.bounds.radius < box.bounds.min.x) return false;
	if(sph.bounds.center.x - sph.bounds.radius > box.bounds.max.x) return false;
	if(sph.bounds.center.y + sph.bounds.radius < box.bounds.min.y) return false;
	if(sph.bounds.center.y - sph.bounds.radius > box.bounds.max.y) return false;
	if(sph.bounds.center.z + sph.bounds.radius < box.bounds.min.z) return false;
	if(sph.bounds.center.z - sph.bounds.radius > box.bounds.max.z) return false;

	// Now find out where the sphere center lies in relation to all the sides
	int xpos = sph.bounds.center.x < box.bounds.min.x ? 1 :
	           sph.bounds.center.x > box.bounds.max.x ? 2 :
	           0;
	int ypos = sph.bounds.center.y < box.bounds.min.y ? 1 :
	           sph.bounds.center.y > box.bounds.max.y ? 2 :
	           0;
	int zpos = sph.bounds.center.z < box.bounds.min.z ? 1 :
	           sph.bounds.center.z > box.bounds.max.z ? 2 :
	           0;

	if(xpos == 0 && ypos == 0 && zpos == 0){
		// sphere is inside the box
		p = (CollisionVector_ToNative(box.bounds.min) + CollisionVector_ToNative(box.bounds.max))*0.5f;

		dist = CollisionVector_ToNative(sph.bounds.center) - p;
		float lensq = dist.MagnitudeSqr();
		if(lensq < mindistsq){
			point.normal = CollisionVector_FromNative(dist * (1.0f/Sqrt(lensq)));
			point.point = CollisionVector_FromNative(CollisionVector_ToNative(sph.bounds.center) - CollisionVector_ToNative(point.normal));
#ifndef VU_COLLISION
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;
#endif

			// find absolute distance to the closer side in each dimension
			float dx = dist.x > 0.0f ?
				box.bounds.max.x - sph.bounds.center.x :
				sph.bounds.center.x - box.bounds.min.x;
			float dy = dist.y > 0.0f ?
				box.bounds.max.y - sph.bounds.center.y :
				sph.bounds.center.y - box.bounds.min.y;
			float dz = dist.z > 0.0f ?
				box.bounds.max.z - sph.bounds.center.z :
				sph.bounds.center.z - box.bounds.min.z;
			// collision depth is maximum of that:
			if(dx > dy && dx > dz)
				point.depth = dx;
			else if(dy > dz)
				point.depth = dy;
			else
				point.depth = dz;
			return true;
		}
	}else{
		// sphere is outside.
		// closest point on box:
		p.x = xpos == 1 ? box.bounds.min.x :
		      xpos == 2 ? box.bounds.max.x :
		      sph.bounds.center.x;
		p.y = ypos == 1 ? box.bounds.min.y :
		      ypos == 2 ? box.bounds.max.y :
		      sph.bounds.center.y;
		p.z = zpos == 1 ? box.bounds.min.z :
		      zpos == 2 ? box.bounds.max.z :
		      sph.bounds.center.z;

		dist = CollisionVector_ToNative(sph.bounds.center) - p;
		float lensq = dist.MagnitudeSqr();
		if(lensq < mindistsq){
			float len = Sqrt(lensq);
			point.point = CollisionVector_FromNative(p);
			point.normal = CollisionVector_FromNative(dist * (1.0f/len));
#ifndef VU_COLLISION
			point.surfaceA = sph.surface;
			point.pieceA = sph.piece;
			point.surfaceB = box.surface;
			point.pieceB = box.piece;
#endif
			point.depth = sph.bounds.radius - len;
			mindistsq = lensq;
			return true;
		}
	}
	return false;
}

bool
CCollision::ProcessLineBox(const CColLine &line, const CColBox &box, CColPoint &point, float &mindist)
{
	float mint, t, x, y, z;
	CVector normal;
	CVector p;

	mint = 1.0f;
	// check if points are on opposite sides of min x plane
	if((box.bounds.min.x - line.p1.x) * (box.bounds.min.x - line.p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box.bounds.min.x - line.p0.x) / (line.p1.x - line.p0.x);
		// y of intersection
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.bounds.min.y && y < box.bounds.max.y){
			// z of intersection
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.bounds.min.z && z < box.bounds.max.z)
				if(t < mint){
					mint = t;
					p = CVector(box.bounds.min.x, y, z);
					normal = CVector(-1.0f, 0.0f, 0.0f);
				}
		}
	}

	// max x plane
	if((line.p1.x - box.bounds.max.x) * (line.p0.x - box.bounds.max.x) < 0.0f){
		t = (line.p0.x - box.bounds.max.x) / (line.p0.x - line.p1.x);
		y = line.p0.y + (line.p1.y - line.p0.y)*t;
		if(y > box.bounds.min.y && y < box.bounds.max.y){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.bounds.min.z && z < box.bounds.max.z)
				if(t < mint){
					mint = t;
					p = CVector(box.bounds.max.x, y, z);
					normal = CVector(1.0f, 0.0f, 0.0f);
				}
		}
	}

	// min y plne
	if((box.bounds.min.y - line.p0.y) * (box.bounds.min.y - line.p1.y) < 0.0f){
		t = (box.bounds.min.y - line.p0.y) / (line.p1.y - line.p0.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.bounds.min.x && x < box.bounds.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.bounds.min.z && z < box.bounds.max.z)
				if(t < mint){
					mint = t;
					p = CVector(x, box.bounds.min.y, z);
					normal = CVector(0.0f, -1.0f, 0.0f);
				}
		}
	}

	// max y plane
	if((line.p0.y - box.bounds.max.y) * (line.p1.y - box.bounds.max.y) < 0.0f){
		t = (line.p0.y - box.bounds.max.y) / (line.p0.y - line.p1.y);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.bounds.min.x && x < box.bounds.max.x){
			z = line.p0.z + (line.p1.z - line.p0.z)*t;
			if(z > box.bounds.min.z && z < box.bounds.max.z)
				if(t < mint){
					mint = t;
					p = CVector(x, box.bounds.max.y, z);
					normal = CVector(0.0f, 1.0f, 0.0f);
				}
		}
	}

	// min z plne
	if((box.bounds.min.z - line.p0.z) * (box.bounds.min.z - line.p1.z) < 0.0f){
		t = (box.bounds.min.z - line.p0.z) / (line.p1.z - line.p0.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.bounds.min.x && x < box.bounds.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.bounds.min.y && y < box.bounds.max.y)
				if(t < mint){
					mint = t;
					p = CVector(x, y, box.bounds.min.z);
					normal = CVector(0.0f, 0.0f, -1.0f);
				}
		}
	}

	// max z plane
	if((line.p0.z - box.bounds.max.z) * (line.p1.z - box.bounds.max.z) < 0.0f){
		t = (line.p0.z - box.bounds.max.z) / (line.p0.z - line.p1.z);
		x = line.p0.x + (line.p1.x - line.p0.x)*t;
		if(x > box.bounds.min.x && x < box.bounds.max.x){
			y = line.p0.y + (line.p1.y - line.p0.y)*t;
			if(y > box.bounds.min.y && y < box.bounds.max.y)
				if(t < mint){
					mint = t;
					p = CVector(x, y, box.bounds.max.z);
					normal = CVector(0.0f, 0.0f, 1.0f);
				}
		}
	}

	if(mint >= mindist)
		return false;

	point.point = CollisionVector_FromNative(p);
	point.normal = CollisionVector_FromNative(normal);
#ifndef VU_COLLISION
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = box.surface;
	point.pieceB = box.piece;
#endif
	mindist = mint;

	return true;
}

// If line.p0 lies inside sphere, no collision is registered.
bool
CCollision::ProcessLineSphere(const CColLine &line, const CColSphere &sphere, CColPoint &point, float &mindist)
{
	CVector v01 = CollisionVector_ToNative(line.p1) - CollisionVector_ToNative(line.p0);
	CVector v0c = CollisionVector_ToNative(sphere.bounds.center) - CollisionVector_ToNative(line.p0);
	float linesq = v01.MagnitudeSqr();
	// project v0c onto v01, scaled by |v01| this is the midpoint of the two intersections
	float projline = DotProduct(v01, v0c);
	// tangent of p0 to sphere, scaled by linesq just like projline^2
	float tansq = (v0c.MagnitudeSqr() - sphere.bounds.radius*sphere.bounds.radius) * linesq;
	// this works out to be the square of the distance between the midpoint and the intersections
	float diffsq = projline*projline - tansq;
	// no intersection
	if(diffsq < 0.0f)
		return false;
	// point of first intersection, in range [0,1] between p0 and p1
	float t = (projline - Sqrt(diffsq)) / linesq;
	// if not on line or beyond mindist, no intersection
	if(t < 0.0f || t > 1.0f || t >= mindist)
		return false;
	point.point = CollisionVector_FromNative(CollisionVector_ToNative(line.p0) + v01*t);
	point.normal = CollisionVector_FromNative(CollisionVector_ToNative(point.point) - CollisionVector_ToNative(sphere.bounds.center));
	ColPoint_NormaliseNormal(&point);
#ifndef VU_COLLISION
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = sphere.surface;
	point.pieceB = sphere.piece;
#endif
	mindist = t;
	return true;
}

// unused
bool
CCollision::ProcessVerticalLineTriangle(const CColLine &line,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist, CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	bool res = ProcessLineTriangle(line, verts, tri, plane, point, mindist);
	if(res && poly){
		// Copy complete decoded values before the original native vector expression
		const CollisionVector3 collisionVertexValue6_0=CompressedVector_Get(&verts[tri.a]);
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue6_0.x,collisionVertexValue6_0.y,collisionVertexValue6_0.z));
		    StoredPoly_SetVertex(poly,0,&storedVertex);
		}
		// Copy complete decoded values before the original native vector expression
		const CollisionVector3 collisionVertexValue7_0=CompressedVector_Get(&verts[tri.b]);
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue7_0.x,collisionVertexValue7_0.y,collisionVertexValue7_0.z));
		    StoredPoly_SetVertex(poly,1,&storedVertex);
		}
		// Copy complete decoded values before the original native vector expression
		const CollisionVector3 collisionVertexValue8_0=CompressedVector_Get(&verts[tri.c]);
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue8_0.x,collisionVertexValue8_0.y,collisionVertexValue8_0.z));
		    StoredPoly_SetVertex(poly,2,&storedVertex);
		}
		poly->valid = true;
	}
	return res;
#else
	float t;
	CVector normal;

	const CollisionVector3 &p0 = line.p0;
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue9_0=CompressedVector_Get(&verts[tri.a]);
	const CVector &va = CVector(collisionVertexValue9_0.x,collisionVertexValue9_0.y,collisionVertexValue9_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue10_0=CompressedVector_Get(&verts[tri.b]);
	const CVector &vb = CVector(collisionVertexValue10_0.x,collisionVertexValue10_0.y,collisionVertexValue10_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue11_0=CompressedVector_Get(&verts[tri.c]);
	const CVector &vc = CVector(collisionVertexValue11_0.x,collisionVertexValue11_0.y,collisionVertexValue11_0.z);

	// early out bound rect test
	if(p0.x < va.x && p0.x < vb.x && p0.x < vc.x) return false;
	if(p0.x > va.x && p0.x > vb.x && p0.x > vc.x) return false;
	if(p0.y < va.y && p0.y < vb.y && p0.y < vc.y) return false;
	if(p0.y > va.y && p0.y > vb.y && p0.y > vc.y) return false;

	ColTrianglePlane_GetNormal(&plane,&normal.x,&normal.y,&normal.z);
	// if points are on the same side, no collision
	if(ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(p0)) * ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(line.p1)) > 0.0f)
		return false;

	// intersection parameter on line
	float h = (CollisionVector_ToNative(line.p1) - CollisionVector_ToNative(p0)).z;
	t = -ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(p0)) / (h * normal.z);
	// early out if we're beyond the mindist
	if(t >= mindist)
		return false;
	CVector p(p0.x, p0.y, p0.z + h*t);

	CVector2D vec1, vec2, vec3, vect;
	switch(plane.dir){
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
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	if(t >= mindist) return false;
	point.point = CollisionVector_FromNative(p);
	point.normal = CollisionVector_FromNative(normal);
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	if(poly){
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(va);
		    StoredPoly_SetVertex(poly,0,&storedVertex);
		}
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(vb);
		    StoredPoly_SetVertex(poly,1,&storedVertex);
		}
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(vc);
		    StoredPoly_SetVertex(poly,2,&storedVertex);
		}
		poly->valid = true;
	}
	mindist = t;
	return true;
#endif
}



bool
CCollision::ProcessLineTriangle(const CColLine &line,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindist, CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	CompressedVector_Unpack(&verts[tri.a],&vutri.v0);
	CompressedVector_Unpack(&verts[tri.b],&vutri.v1);
	CompressedVector_Unpack(&verts[tri.c],&vutri.v2);
	ColTrianglePlane_Unpack(&plane,&vutri.plane);

	LineToTriangleCollisionCompressed(ColLine_ToVu(line,0), ColLine_ToVu(line,1), vutri);

	CVuVector pnt, normal;
	float dist;
	if(GetVUresult(pnt, normal, dist)){
		if(dist < mindist){
			point.point = CollisionVector_FromNative(pnt);
			point.normal = CollisionVector_FromNative(normal);
			mindist = dist;
			return true;
		}
	}
	return false;
#else
	float t;
	CVector normal;
	ColTrianglePlane_GetNormal(&plane,&normal.x,&normal.y,&normal.z);

	// if points are on the same side, no collision
	if(ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(line.p0)) * ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(line.p1)) > 0.0f)
		return false;

	float p0dist = DotProduct(CollisionVector_ToNative(line.p1) - CollisionVector_ToNative(line.p0), normal);

#ifdef FIX_BUGS
	// line lines in the plane, assume no collision
	if (p0dist == 0.0f)
		return false;
#endif

	// intersection parameter on line
	t = -ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(line.p0)) / p0dist;

	// early out if we're beyond the mindist
	if(t >= mindist)
		return false;
	// find point of intersection
	CVector p = CollisionVector_ToNative(line.p0) + (CollisionVector_ToNative(line.p1)-CollisionVector_ToNative(line.p0))*t;

	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue12_0=CompressedVector_Get(&verts[tri.a]);
	const CVector &va = CVector(collisionVertexValue12_0.x,collisionVertexValue12_0.y,collisionVertexValue12_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue13_0=CompressedVector_Get(&verts[tri.b]);
	const CVector &vb = CVector(collisionVertexValue13_0.x,collisionVertexValue13_0.y,collisionVertexValue13_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue14_0=CompressedVector_Get(&verts[tri.c]);
	const CVector &vc = CVector(collisionVertexValue14_0.x,collisionVertexValue14_0.y,collisionVertexValue14_0.z);
	CVector2D vec1, vec2, vec3, vect;

	switch(plane.dir){
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
		vec1.x = va.x; vec1.y = va.y;
		vec2.x = vb.x; vec2.y = vb.y;
		vec3.x = vc.x; vec3.y = vc.y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(CrossProduct2D(vec2-vec1, vect-vec1) < 0.0f) return false;
	if(CrossProduct2D(vec3-vec1, vect-vec1) > 0.0f) return false;
	if(CrossProduct2D(vec3-vec2, vect-vec2) < 0.0f) return false;
	if(t >= mindist) return false;
	point.point = CollisionVector_FromNative(p);
	point.normal = CollisionVector_FromNative(normal);
	point.surfaceA = 0;
	point.pieceA = 0;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
	if(poly){
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(va);
		    StoredPoly_SetVertex(poly,0,&storedVertex);
		}
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(vb);
		    StoredPoly_SetVertex(poly,1,&storedVertex);
		}
		{
		    // Preserve the complete native conversion before the individual stored-vertex update
		    const StoredPolyVertex storedVertex=StoredPoly_FromNative(vc);
		    StoredPoly_SetVertex(poly,2,&storedVertex);
		}
		poly->valid = true;
	}
	mindist = t;
	return true;
#endif
}

bool
CCollision::ProcessSphereTriangle(const CColSphere &sphere,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindistsq)
{
#ifdef VU_COLLISION
	// not used in favour of optimized loops
	VuTriangle vutri;
	CompressedVector_Unpack(&verts[tri.a],&vutri.v0);
	CompressedVector_Unpack(&verts[tri.b],&vutri.v1);
	CompressedVector_Unpack(&verts[tri.c],&vutri.v2);
	ColTrianglePlane_Unpack(&plane,&vutri.plane);

	SphereToTriangleCollisionCompressed(Sphere_ToVu(sphere.bounds), vutri);

	CVuVector pnt, normal;
	float dist;
	if(GetVUresult(pnt, normal, dist) && dist*dist < mindistsq){
		float depth = sphere.bounds.radius - dist;
		if(depth > point.depth){
			point.point = CollisionVector_FromNative(pnt);
			point.normal = CollisionVector_FromNative(normal);
			point.depth = depth;
			mindistsq = dist*dist;
			return true;
		}
	}
	return false;
#else
	// If sphere and plane don't intersect, no collision
	float planedist = ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(sphere.bounds.center));
	float distsq = planedist*planedist;
	if(Abs(planedist) > sphere.bounds.radius || distsq > mindistsq)
		return false;

	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue15_0=CompressedVector_Get(&verts[tri.a]);
	const CVector &va = CVector(collisionVertexValue15_0.x,collisionVertexValue15_0.y,collisionVertexValue15_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue16_0=CompressedVector_Get(&verts[tri.b]);
	const CVector &vb = CVector(collisionVertexValue16_0.x,collisionVertexValue16_0.y,collisionVertexValue16_0.z);
	// Copy complete decoded values before the original native vector expression
	const CollisionVector3 collisionVertexValue17_0=CompressedVector_Get(&verts[tri.c]);
	const CVector &vc = CVector(collisionVertexValue17_0.x,collisionVertexValue17_0.y,collisionVertexValue17_0.z);

	// calculate two orthogonal basis vectors for the triangle
	CVector normal;
	ColTrianglePlane_GetNormal(&plane,&normal.x,&normal.y,&normal.z);
	CVector vec2 = vb - va;
	float len = vec2.Magnitude();
	vec2 = vec2 * (1.0f/len);
	CVector vec1 = CrossProduct(vec2, normal);

	// We know A has local coordinate [0,0] and B has [0,len].
	// Now calculate coordinates on triangle for these two vectors:
	CVector vac = vc - va;
	CVector vas = CollisionVector_ToNative(sphere.bounds.center) - va;
	CVector2D b(0.0f, len);
	CVector2D c(DotProduct(vec1, vac), DotProduct(vec2, vac));
	CVector2D s(DotProduct(vec1, vas), DotProduct(vec2, vas));

	// The three triangle lines partition the space into 6 sectors,
	// find out in which the center lies.
	int insideAB = CrossProduct2D(s, b) >= 0.0f;
	int insideAC = CrossProduct2D(c, s) >= 0.0f;
	int insideBC = CrossProduct2D(s-b, c-b) >= 0.0f;

	int testcase = insideAB + insideAC + insideBC;
	float dist = 0.0f;
	CVector p;
	switch(testcase){
	case 0:
		return false;	// shouldn't happen
	case 1:
		// closest to a vertex
		if(insideAB) p = vc;
		else if(insideAC) p = vb;
		else if(insideBC) p = va;
		else assert(0);
		dist = (CollisionVector_ToNative(sphere.bounds.center) - p).Magnitude();
		break;
	case 2: {
		// closest to an edge
		// looks like original game as DistToLine manually inlined
		// Keep a native point alive through the existing edge-distance query
		const CVector sphereNativeCenter=CollisionVector_ToNative(sphere.bounds.center);
		if(!insideAB) dist = DistToLine(&va, &vb, &sphereNativeCenter, p);
		else if(!insideAC) dist = DistToLine(&va, &vc, &sphereNativeCenter, p);
		else if(!insideBC) dist = DistToLine(&vb, &vc, &sphereNativeCenter, p);
		else assert(0);
		break;
	}
	case 3:
		// center is in triangle
		dist = Abs(planedist);
		p = CollisionVector_ToNative(sphere.bounds.center) - normal*planedist;
		break;
	default:
		assert(0);
	}

	if(dist >= sphere.bounds.radius || dist*dist >= mindistsq)
		return false;

	point.point = CollisionVector_FromNative(p);
	point.normal = CollisionVector_FromNative(CollisionVector_ToNative(sphere.bounds.center) - p);
	ColPoint_NormaliseNormal(&point);
#ifndef VU_COLLISION
	point.surfaceA = sphere.surface;
	point.pieceA = sphere.piece;
	point.surfaceB = tri.surface;
	point.pieceB = 0;
#endif
	point.depth = sphere.bounds.radius - dist;
	mindistsq = dist*dist;
	return true;
#endif
}

bool
CCollision::ProcessLineOfSight(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough)
{
#ifdef VU_COLLISION
	CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CVuVector newline[2];
	TransformColLines(newline,1,matTransform,&line);

	if(mindist < 1.0f)
		newline[1] = newline[0] + (newline[1] - newline[0])*mindist;

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(ColLine_FromRaw32(newline), model.boundingBox))
		return false;

	float coldist = 1.0f;
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.spheres[i].surface)) continue;
		if(ProcessLineSphere(ColLine_FromRaw32(newline), model.spheres[i], point, coldist))
			ColPoint_SetSurfaces(&point,0, 0, model.spheres[i].surface, model.spheres[i].piece);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(ProcessLineBox(ColLine_FromRaw32(newline), model.boxes[i], point, coldist))
			ColPoint_SetSurfaces(&point,0, 0, model.boxes[i].surface, model.boxes[i].piece);
	}

	CalculateTrianglePlanes(&model);
	VuTriangle vutri;
	CColTriangle *lasttri = nil;
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		CompressedVector_Unpack(&model.vertices[tri->a],&vutri.v0);
		CompressedVector_Unpack(&model.vertices[tri->b],&vutri.v1);
		CompressedVector_Unpack(&model.vertices[tri->c],&vutri.v2);
		ColTrianglePlane_Unpack(&model.trianglePlanes[i],&vutri.plane);

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lasttri = tri;
		break;
	}
#ifdef FIX_BUGS
	// no need to check first again
	i++;
#endif
	CVuVector pnt, normal;
	float dist;
	for(; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.triangles[i].surface)) continue;

		CColTriangle *tri = &model.triangles[i];
		CompressedVector_Unpack(&model.vertices[tri->a],&vutri.v0);
		CompressedVector_Unpack(&model.vertices[tri->b],&vutri.v1);
		CompressedVector_Unpack(&model.vertices[tri->c],&vutri.v2);
		ColTrianglePlane_Unpack(&model.trianglePlanes[i],&vutri.plane);

		if(GetVUresult(pnt, normal, dist))
			if(dist < coldist){
				point.point = CollisionVector_FromNative(pnt);
				point.normal = CollisionVector_FromNative(normal);
				ColPoint_SetSurfaces(&point,0, 0, lasttri->surface, 0);
				coldist = dist;
			}

		LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
		lasttri = tri;
	}
	if(lasttri && GetVUresult(pnt, normal, dist))
		if(dist < coldist){
			point.point = CollisionVector_FromNative(pnt);
			point.normal = CollisionVector_FromNative(normal);
			ColPoint_SetSurfaces(&point,0, 0, lasttri->surface, 0);
			coldist = dist;
		}


	if(coldist < 1.0f){
		point.point = CollisionVector_FromNative(matrix * CollisionVector_ToNative(point.point));
		point.normal = CollisionVector_FromNative(Multiply3x3(matrix, CollisionVector_ToNative(point.normal)));
		mindist *= coldist;
		return true;
	}
	return false;
#else
	static CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	// Retain the line record at its original construction boundary
	CColLine newline;
	// Capture the complete native expression before initializing line endpoints
	const CVector &lineConstructorPoint1_0=matTransform * CollisionVector_ToNative(line.p0);
	// Capture the complete native expression before initializing line endpoints
	const CVector &lineConstructorPoint1_1=matTransform * CollisionVector_ToNative(line.p1);
	// Initialize the line at the former parameterized constructor boundary
	ColLine_Init(&newline,COLLISION_POINT_REF(lineConstructorPoint1_0),COLLISION_POINT_REF(lineConstructorPoint1_1));

	// If we don't intersect with the bounding box, no chance on the rest
	if(!TestLineBox(newline, model.boundingBox))
		return false;

	float coldist = mindist;
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.spheres[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.spheres[i].surface)) continue;
		ProcessLineSphere(newline, model.spheres[i], point, coldist);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.boxes[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.boxes[i].surface)) continue;
		ProcessLineBox(newline, model.boxes[i], point, coldist);
	}

	CalculateTrianglePlanes(&model);
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThrough(model.triangles[i].surface)) continue;
		if(ignoreShootThrough && IsShootThrough(model.triangles[i].surface)) continue;
		ProcessLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist);
	}

	if(coldist < mindist){
		point.point = CollisionVector_FromNative(matrix * CollisionVector_ToNative(point.point));
		point.normal = CollisionVector_FromNative(Multiply3x3(matrix, CollisionVector_ToNative(point.normal)));
		mindist = coldist;
		return true;
	}
	return false;
#endif
}

bool
CCollision::ProcessVerticalLine(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough, CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
	static CStoredCollPoly TempStoredPoly;
	CMatrix matTransform;
	int i;

	// transform line to model space
	Invert(matrix, matTransform);
	CVuVector newline[2];
	TransformColLines(newline,1,matTransform,&line);

	if(mindist < 1.0f)
		newline[1] = newline[0] + (newline[1] - newline[0])*mindist;

	if(!TestLineBox(ColLine_FromRaw32(newline), model.boundingBox))
		return false;

	float coldist = 1.0f;
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThroughVertical(model.spheres[i].surface)) continue;
		if(ProcessLineSphere(ColLine_FromRaw32(newline), model.spheres[i], point, coldist))
			ColPoint_SetSurfaces(&point,0, 0, model.spheres[i].surface, model.spheres[i].piece);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThroughVertical(model.boxes[i].surface)) continue;
		if(ProcessLineBox(ColLine_FromRaw32(newline), model.boxes[i], point, coldist))
			ColPoint_SetSurfaces(&point,0, 0, model.boxes[i].surface, model.boxes[i].piece);
	}

	CalculateTrianglePlanes(&model);
	TempStoredPoly.valid = false;
	if(model.numTriangles){
		bool registeredCol;
		CColTriangle *lasttri = nil;
		VuTriangle vutri;
		for(i = 0; i < model.numTriangles; i++){
			if(ignoreSeeThrough && IsSeeThroughVertical(model.triangles[i].surface)) continue;

			CColTriangle *tri = &model.triangles[i];
			CompressedVector_Unpack(&model.vertices[tri->a],&vutri.v0);
			CompressedVector_Unpack(&model.vertices[tri->b],&vutri.v1);
			CompressedVector_Unpack(&model.vertices[tri->c],&vutri.v2);
			ColTrianglePlane_Unpack(&model.trianglePlanes[i],&vutri.plane);

			LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
			lasttri = tri;
			break;
		}
#ifdef FIX_BUGS
		// no need to check first again
		i++;
#endif
		CVuVector pnt, normal;
		float dist;
		for(; i < model.numTriangles; i++){
			if(ignoreSeeThrough && IsSeeThroughVertical(model.triangles[i].surface)) continue;

			CColTriangle *tri = &model.triangles[i];
			CompressedVector_Unpack(&model.vertices[tri->a],&vutri.v0);
			CompressedVector_Unpack(&model.vertices[tri->b],&vutri.v1);
			CompressedVector_Unpack(&model.vertices[tri->c],&vutri.v2);
			ColTrianglePlane_Unpack(&model.trianglePlanes[i],&vutri.plane);

			if(GetVUresult(pnt, normal, dist)){
				if(dist < coldist){
					point.point = CollisionVector_FromNative(pnt);
					point.normal = CollisionVector_FromNative(normal);
					ColPoint_SetSurfaces(&point,0, 0, lasttri->surface, 0);
					coldist = dist;
					registeredCol = true;
				}else
					registeredCol = false;
			}else
				registeredCol = false;

			if(registeredCol){
				// Copy complete decoded values before the original native vector expression
				const CollisionVector3 collisionVertexValue18_0=CompressedVector_Get(&model.vertices[lasttri->a]);
				{
				    // Preserve the complete native conversion before the individual stored-vertex update
				    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue18_0.x,collisionVertexValue18_0.y,collisionVertexValue18_0.z));
				    StoredPoly_SetVertex(&TempStoredPoly,0,&storedVertex);
				}
				// Copy complete decoded values before the original native vector expression
				const CollisionVector3 collisionVertexValue19_0=CompressedVector_Get(&model.vertices[lasttri->b]);
				{
				    // Preserve the complete native conversion before the individual stored-vertex update
				    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue19_0.x,collisionVertexValue19_0.y,collisionVertexValue19_0.z));
				    StoredPoly_SetVertex(&TempStoredPoly,1,&storedVertex);
				}
				// Copy complete decoded values before the original native vector expression
				const CollisionVector3 collisionVertexValue20_0=CompressedVector_Get(&model.vertices[lasttri->c]);
				{
				    // Preserve the complete native conversion before the individual stored-vertex update
				    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue20_0.x,collisionVertexValue20_0.y,collisionVertexValue20_0.z));
				    StoredPoly_SetVertex(&TempStoredPoly,2,&storedVertex);
				}
				TempStoredPoly.valid = true;
			}

			LineToTriangleCollisionCompressed(newline[0], newline[1], vutri);
			lasttri = tri;
		}
		if(lasttri && GetVUresult(pnt, normal, dist)){
			if(dist < coldist){
				point.point = CollisionVector_FromNative(pnt);
				point.normal = CollisionVector_FromNative(normal);
				ColPoint_SetSurfaces(&point,0, 0, lasttri->surface, 0);
				coldist = dist;
				registeredCol = true;
			}else
				registeredCol = false;
		}else
			registeredCol = false;

		if(registeredCol){
			// Copy complete decoded values before the original native vector expression
			const CollisionVector3 collisionVertexValue21_0=CompressedVector_Get(&model.vertices[lasttri->a]);
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue21_0.x,collisionVertexValue21_0.y,collisionVertexValue21_0.z));
			    StoredPoly_SetVertex(&TempStoredPoly,0,&storedVertex);
			}
			// Copy complete decoded values before the original native vector expression
			const CollisionVector3 collisionVertexValue22_0=CompressedVector_Get(&model.vertices[lasttri->b]);
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue22_0.x,collisionVertexValue22_0.y,collisionVertexValue22_0.z));
			    StoredPoly_SetVertex(&TempStoredPoly,1,&storedVertex);
			}
			// Copy complete decoded values before the original native vector expression
			const CollisionVector3 collisionVertexValue23_0=CompressedVector_Get(&model.vertices[lasttri->c]);
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(CVector(collisionVertexValue23_0.x,collisionVertexValue23_0.y,collisionVertexValue23_0.z));
			    StoredPoly_SetVertex(&TempStoredPoly,2,&storedVertex);
			}
			TempStoredPoly.valid = true;
		}
	}

	if(coldist < 1.0f){
		point.point = CollisionVector_FromNative(matrix * CollisionVector_ToNative(point.point));
		point.normal = CollisionVector_FromNative(Multiply3x3(matrix, CollisionVector_ToNative(point.normal)));
		if(TempStoredPoly.valid && poly){
			*poly = TempStoredPoly;
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(matrix * StoredPoly_ToNative(poly->verts[0]));
			    StoredPoly_SetVertex(poly,0,&storedVertex);
			}
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(matrix * StoredPoly_ToNative(poly->verts[1]));
			    StoredPoly_SetVertex(poly,1,&storedVertex);
			}
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(matrix * StoredPoly_ToNative(poly->verts[2]));
			    StoredPoly_SetVertex(poly,2,&storedVertex);
			}
		}
		mindist *= coldist;
		return true;
	}
	return false;
#else
	static CStoredCollPoly TempStoredPoly;
	int i;

	// transform line to model space
	// Why does the game seem to do this differently than above?
	// Retain the line record at its original construction boundary
	CColLine newline;
	// Capture the complete native expression before initializing line endpoints
	const CVector &lineConstructorPoint2_0=MultiplyInverse(matrix, CollisionVector_ToNative(line.p0));
	// Capture the complete native expression before initializing line endpoints
	const CVector &lineConstructorPoint2_1=MultiplyInverse(matrix, CollisionVector_ToNative(line.p1));
	// Initialize the line at the former parameterized constructor boundary
	ColLine_Init(&newline,COLLISION_POINT_REF(lineConstructorPoint2_0),COLLISION_POINT_REF(lineConstructorPoint2_1));

	if(!TestLineBox(newline, model.boundingBox))
		return false;

	// BUG? is IsSeeThroughVertical really the right thing? also not checking shoot through
	float coldist = mindist;
	for(i = 0; i < model.numSpheres; i++){
		if(ignoreSeeThrough && IsSeeThroughVertical(model.spheres[i].surface)) continue;
		ProcessLineSphere(newline, model.spheres[i], point, coldist);
	}

	for(i = 0; i < model.numBoxes; i++){
		if(ignoreSeeThrough && IsSeeThroughVertical(model.boxes[i].surface)) continue;
		ProcessLineBox(newline, model.boxes[i], point, coldist);
	}

	CalculateTrianglePlanes(&model);
	TempStoredPoly.valid = false;
	for(i = 0; i < model.numTriangles; i++){
		if(ignoreSeeThrough && IsSeeThroughVertical(model.triangles[i].surface)) continue;
		ProcessLineTriangle(newline, model.vertices, model.triangles[i], model.trianglePlanes[i], point, coldist, &TempStoredPoly);
	}

	if(coldist < mindist){
		point.point = CollisionVector_FromNative(matrix * CollisionVector_ToNative(point.point));
		point.normal = CollisionVector_FromNative(Multiply3x3(matrix, CollisionVector_ToNative(point.normal)));
		if(TempStoredPoly.valid && poly){
			*poly = TempStoredPoly;
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(matrix * StoredPoly_ToNative(poly->verts[0]));
			    StoredPoly_SetVertex(poly,0,&storedVertex);
			}
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(matrix * StoredPoly_ToNative(poly->verts[1]));
			    StoredPoly_SetVertex(poly,1,&storedVertex);
			}
			{
			    // Preserve the complete native conversion before the individual stored-vertex update
			    const StoredPolyVertex storedVertex=StoredPoly_FromNative(matrix * StoredPoly_ToNative(poly->verts[2]));
			    StoredPoly_SetVertex(poly,2,&storedVertex);
			}
		}
		mindist = coldist;
		return true;
	}
	return false;
#endif
}

enum {
	MAXNUMSPHERES = 256,	// rouz edit, was 128
	MAXNUMBOXES = 64,		// rouz edit, was 32
	MAXNUMLINES = 32,		// rouz edit, was 16
	MAXNUMTRIS = 1200		// rouz edit, was 600
};

#ifdef VU_COLLISION
#ifdef GTA_PS2
#define SPR(off) ((uint8*)(0x70000000 + (off)))
#else
static uint8 fakeSPR[16*1024];
#define SPR(off) ((uint8*)(fakeSPR + (off)))
#endif
#endif

// This checks model A's spheres and lines against model B's spheres, boxes and triangles.
// Returns the number of A's spheres that collide.
// Returned ColPoints are in world space.
// NB: only vehicles can have col models with lines, exactly 4, one for each wheel
int32
CCollision::ProcessColModels(const CMatrix &matrixA, CColModel &modelA,
	const CMatrix &matrixB, CColModel &modelB,
	CColPoint *spherepoints, CColPoint *linepoints, float *linedists)
{
#ifdef VU_COLLISION
	CVuVector *aSpheresA = (CVuVector*)SPR(0x0000);
	CVuVector *aSpheresB = (CVuVector*)SPR(0x0800);
	CVuVector *aLinesA = (CVuVector*)SPR(0x1000);
	int32 *aSphereIndicesA = (int32*)SPR(0x1200);
	int32 *aSphereIndicesB = (int32*)SPR(0x1400);
	int32 *aBoxIndicesB = (int32*)SPR(0x1600);
	int32 *aTriangleIndicesB = (int32*)SPR(0x1680);
	bool *aCollided = (bool*)SPR(0x1FE0);
	CMatrix &matAB = *(CMatrix*)SPR(0x1FF0);
	CMatrix &matBA = *(CMatrix*)SPR(0x2040);
	int i, j, k;

	// From model A space to model B space
	Invert(matrixB, matAB);
	matAB *= matrixA;

	CVuVector bsphereAB;	// bounding sphere of A in B space
	TransformPoint(bsphereAB, matAB, CollisionVector_ToNative(modelA.boundingSphere.center));	// inlined
	bsphereAB.w = modelA.boundingSphere.radius;
	if(!TestSphereBox(Sphere_FromRaw16(&bsphereAB), modelB.boundingBox))
		return 0;

	// transform modelA's spheres and lines to B space
	TransformColSpheres(aSpheresA,modelA.numSpheres,matAB,modelA.spheres);
	for(i = 0; i < modelA.numSpheres; i++)
		aSpheresA[i].w = modelA.spheres[i].bounds.radius;
	TransformColLines(aLinesA,modelA.numLines,matAB,modelA.lines);

	// Test them against model B's bounding volumes
	int numSpheresA = 0;
	for(i = 0; i < modelA.numSpheres; i++)
		if(TestSphereBox(Sphere_FromRaw16(&aSpheresA[i]), modelB.boundingBox))
			aSphereIndicesA[numSpheresA++] = i;
	// No collision
	if(numSpheresA == 0 && modelA.numLines == 0)
		return 0;


	// B to A space
	Invert(matrixA, matBA);
	matBA *= matrixB;

	// transform modelB's spheres to A space
	TransformColSpheres(aSpheresB,modelB.numSpheres,matBA,modelB.spheres);
	for(i = 0; i < modelB.numSpheres; i++)
		aSpheresB[i].w = modelB.spheres[i].bounds.radius;

	// Check model B against A's bounding volumes
	int numSpheresB = 0;
	int numBoxesB = 0;
	int numTrianglesB = 0;
	for(i = 0; i < modelB.numSpheres; i++)
		if(TestSphereBox(Sphere_FromRaw16(&aSpheresB[i]), modelA.boundingBox))
			aSphereIndicesB[numSpheresB++] = i;
	for(i = 0; i < modelB.numBoxes; i++)
		if(TestSphereBox(Sphere_FromRaw16(&bsphereAB), modelB.boxes[i].bounds))
			aBoxIndicesB[numBoxesB++] = i;
	CalculateTrianglePlanes(&modelB);
	if(modelB.numTriangles){
		VuTriangle vutri;
		// process the first triangle
		CColTriangle *tri = &modelB.triangles[0];
		CompressedVector_Unpack(&modelB.vertices[tri->a],&vutri.v0);
		CompressedVector_Unpack(&modelB.vertices[tri->b],&vutri.v1);
		CompressedVector_Unpack(&modelB.vertices[tri->c],&vutri.v2);
		ColTrianglePlane_Unpack(&modelB.trianglePlanes[0],&vutri.plane);

		SphereToTriangleCollisionCompressed(bsphereAB, vutri);

		for(i = 1; i < modelB.numTriangles; i++){
			// set up the next triangle while VU0 is running
			tri = &modelB.triangles[i];
			CompressedVector_Unpack(&modelB.vertices[tri->a],&vutri.v0);
			CompressedVector_Unpack(&modelB.vertices[tri->b],&vutri.v1);
			CompressedVector_Unpack(&modelB.vertices[tri->c],&vutri.v2);
			ColTrianglePlane_Unpack(&modelB.trianglePlanes[i],&vutri.plane);

			// check previous result
			if(GetVUresult())
				aTriangleIndicesB[numTrianglesB++] = i-1;

			// kick off this one
			SphereToTriangleCollisionCompressed(bsphereAB, vutri);
		}

		// check last result
		if(GetVUresult())
			aTriangleIndicesB[numTrianglesB++] = i-1;
	}
	// No collision
	if(numSpheresB == 0 && numBoxesB == 0 && numTrianglesB == 0)
		return 0;

	// We now have the collision volumes in A and B that are worth processing.

	// Process A's spheres against B's collision volumes
	int numCollisions = 0;
	spherepoints[numCollisions].depth = -1.0f;
	for(i = 0; i < numSpheresA; i++){
		float coldist = 1.0e24f;
		bool hasCollided = false;
		CColSphere *sphA = &modelA.spheres[aSphereIndicesA[i]];
		CVuVector *vusphA = &aSpheresA[aSphereIndicesA[i]];

		for(j = 0; j < numSpheresB; j++)
			// This actually looks like something was inlined here
			if(ProcessSphereSphere(ColSphere_FromRaw16(vusphA), modelB.spheres[aSphereIndicesB[j]],
					spherepoints[numCollisions], coldist)){
				ColPoint_SetSurfaces(&spherepoints[numCollisions],
					sphA->surface, sphA->piece,
					modelB.spheres[aSphereIndicesB[j]].surface, modelB.spheres[aSphereIndicesB[j]].piece);
				hasCollided = true;
			}
		for(j = 0; j < numBoxesB; j++)
			if(ProcessSphereBox(ColSphere_FromRaw16(vusphA), modelB.boxes[aBoxIndicesB[j]],
					spherepoints[numCollisions], coldist)){
				ColPoint_SetSurfaces(&spherepoints[numCollisions],
					sphA->surface, sphA->piece,
					modelB.boxes[aBoxIndicesB[j]].surface, modelB.boxes[aBoxIndicesB[j]].piece);
				hasCollided = true;
			}
		if(numTrianglesB){
			CVuVector point, normal;
			float depth;
			bool registeredCol;
			CColTriangle *lasttri;

			VuTriangle vutri;
			// process the first triangle
			k = aTriangleIndicesB[0];
			CColTriangle *tri = &modelB.triangles[k];
			CompressedVector_Unpack(&modelB.vertices[tri->a],&vutri.v0);
			CompressedVector_Unpack(&modelB.vertices[tri->b],&vutri.v1);
			CompressedVector_Unpack(&modelB.vertices[tri->c],&vutri.v2);
			ColTrianglePlane_Unpack(&modelB.trianglePlanes[k],&vutri.plane);

			SphereToTriangleCollisionCompressed(*vusphA, vutri);
			lasttri = tri;

			for(j = 1; j < numTrianglesB; j++){
				k = aTriangleIndicesB[j];
				// set up the next triangle while VU0 is running
				tri = &modelB.triangles[k];
				CompressedVector_Unpack(&modelB.vertices[tri->a],&vutri.v0);
				CompressedVector_Unpack(&modelB.vertices[tri->b],&vutri.v1);
				CompressedVector_Unpack(&modelB.vertices[tri->c],&vutri.v2);
				ColTrianglePlane_Unpack(&modelB.trianglePlanes[k],&vutri.plane);

				// check previous result
				// TODO: this looks inlined but spherepoints[numCollisions] does not...
				if(GetVUresult(point, normal, depth)){
					depth = sphA->bounds.radius - depth;
					if(depth > spherepoints[numCollisions].depth){
						spherepoints[numCollisions].point = CollisionVector_FromNative(point);
						spherepoints[numCollisions].normal = CollisionVector_FromNative(normal);
						ColPoint_Set(&spherepoints[numCollisions],depth,
							sphA->surface, sphA->piece, lasttri->surface, 0);
						registeredCol = true;
					}else
						registeredCol = false;
				}else
					registeredCol = false;

				if(registeredCol)
					hasCollided = true;

				// kick off this one
				SphereToTriangleCollisionCompressed(*vusphA, vutri);
				lasttri = tri;
			}

			// check last result
			// TODO: this looks inlined but spherepoints[numCollisions] does not...
			if(GetVUresult(point, normal, depth)){
				depth = sphA->bounds.radius - depth;
				if(depth > spherepoints[numCollisions].depth){
					spherepoints[numCollisions].point = CollisionVector_FromNative(point);
					spherepoints[numCollisions].normal = CollisionVector_FromNative(normal);
					ColPoint_Set(&spherepoints[numCollisions],depth,
						sphA->surface, sphA->piece, lasttri->surface, 0);
					registeredCol = true;
				}else
					registeredCol = false;
			}else
				registeredCol = false;

			if(registeredCol)
				hasCollided = true;
		}

		if(hasCollided){
			numCollisions++;
			if(numCollisions == MAX_COLLISION_POINTS)
				break;
			spherepoints[numCollisions].depth = -1.0f;
		}
	}
	for(i = 0; i < numCollisions; i++){
		// TODO: both VU0 macros
		spherepoints[i].point = CollisionVector_FromNative(matrixB * CollisionVector_ToNative(spherepoints[i].point));
		spherepoints[i].normal = CollisionVector_FromNative(Multiply3x3(matrixB, CollisionVector_ToNative(spherepoints[i].normal)));
	}

	// And the same thing for the lines in A
	for(i = 0; i < modelA.numLines; i++){
		aCollided[i] = false;
		CVuVector *lineA = &aLinesA[i*2];

		for(j = 0; j < numSpheresB; j++)
			if(ProcessLineSphere(ColLine_FromRaw32(lineA), modelB.spheres[aSphereIndicesB[j]],
					linepoints[i], linedists[i])){
				ColPoint_SetSurfaces(&linepoints[i],0, 0,
#ifdef FIX_BUGS
					modelB.spheres[aSphereIndicesB[j]].surface, modelB.spheres[aSphereIndicesB[j]].piece);
#else
					modelB.spheres[j].surface, modelB.spheres[j].piece);
#endif
				aCollided[i] = true;
			}
		for(j = 0; j < numBoxesB; j++)
			if(ProcessLineBox(ColLine_FromRaw32(lineA), modelB.boxes[aBoxIndicesB[j]],
					linepoints[i], linedists[i])){
				ColPoint_SetSurfaces(&linepoints[i],0, 0,
					modelB.boxes[aBoxIndicesB[j]].surface, modelB.boxes[aBoxIndicesB[j]].piece);
				aCollided[i] = true;
			}
		if(numTrianglesB){
			CVuVector point, normal;
			float dist;
			bool registeredCol;
			CColTriangle *lasttri;

			VuTriangle vutri;
			// process the first triangle
			k = aTriangleIndicesB[0];
			CColTriangle *tri = &modelB.triangles[k];
			CompressedVector_Unpack(&modelB.vertices[tri->a],&vutri.v0);
			CompressedVector_Unpack(&modelB.vertices[tri->b],&vutri.v1);
			CompressedVector_Unpack(&modelB.vertices[tri->c],&vutri.v2);
			ColTrianglePlane_Unpack(&modelB.trianglePlanes[k],&vutri.plane);

			LineToTriangleCollisionCompressed(lineA[0], lineA[1], vutri);
			lasttri = tri;

			for(j = 1; j < numTrianglesB; j++){
				k = aTriangleIndicesB[j];
				// set up the next triangle while VU0 is running
				CColTriangle *tri = &modelB.triangles[k];
				CompressedVector_Unpack(&modelB.vertices[tri->a],&vutri.v0);
				CompressedVector_Unpack(&modelB.vertices[tri->b],&vutri.v1);
				CompressedVector_Unpack(&modelB.vertices[tri->c],&vutri.v2);
				ColTrianglePlane_Unpack(&modelB.trianglePlanes[k],&vutri.plane);

				// check previous result
				// TODO: this again somewhat looks inlined
				if(GetVUresult(point, normal, dist)){
					if(dist < linedists[i]){
						linepoints[i].point = CollisionVector_FromNative(point);
						linepoints[i].normal = CollisionVector_FromNative(normal);
						linedists[i] = dist;
						ColPoint_SetSurfaces(&linepoints[i],0, 0, lasttri->surface, 0);
						registeredCol = true;
					}else
						registeredCol = false;
				}else
					registeredCol = false;

				if(registeredCol)
					aCollided[i] = true;

				// kick of this one
				LineToTriangleCollisionCompressed(lineA[0], lineA[1], vutri);
				lasttri = tri;
			}

			// check last result
			if(GetVUresult(point, normal, dist)){
				if(dist < linedists[i]){
					linepoints[i].point = CollisionVector_FromNative(point);
					linepoints[i].normal = CollisionVector_FromNative(normal);
					linedists[i] = dist;
					ColPoint_SetSurfaces(&linepoints[i],0, 0, lasttri->surface, 0);
					registeredCol = true;
				}else
					registeredCol = false;
			}else
				registeredCol = false;

			if(registeredCol)
				aCollided[i] = true;
		}

		if(aCollided[i]){
			// TODO: both VU0 macros
			linepoints[i].point = CollisionVector_FromNative(matrixB * CollisionVector_ToNative(linepoints[i].point));
			linepoints[i].normal = CollisionVector_FromNative(Multiply3x3(matrixB, CollisionVector_ToNative(linepoints[i].normal)));
		}
	}

	return numCollisions;	// sphere collisions
#else
	static int aSphereIndicesA[MAXNUMSPHERES];
	static int aLineIndicesA[MAXNUMLINES];
	static int aSphereIndicesB[MAXNUMSPHERES];
	static int aBoxIndicesB[MAXNUMBOXES];
	static int aTriangleIndicesB[MAXNUMTRIS];
	static bool aCollided[MAXNUMLINES];
	static CColSphere aSpheresA[MAXNUMSPHERES];
	static CColLine aLinesA[MAXNUMLINES];
	static CMatrix matAB, matBA;
	CColSphere s;
	int i, j;

	assert(modelA.numSpheres <= MAXNUMSPHERES);
	assert(modelA.numLines <= MAXNUMLINES);

	// From model A space to model B space
	matAB = Invert(matrixB, matAB);
	matAB *= matrixA;

	CColSphere bsphereAB;	// bounding sphere of A in B space
	bsphereAB.bounds.radius = modelA.boundingSphere.radius;
	bsphereAB.bounds.center = CollisionVector_FromNative(matAB * CollisionVector_ToNative(modelA.boundingSphere.center));
	if(!TestSphereBox(bsphereAB.bounds, modelB.boundingBox))
		return 0;
	// B to A space
	matBA = Invert(matrixA, matBA);
	matBA *= matrixB;

	// transform modelA's spheres and lines to B space
	for(i = 0; i < modelA.numSpheres; i++){
		CColSphere &s = modelA.spheres[i];
		{
		    // Keep the native center expression alive at the former member-call boundary
		    const CVector &sphereCenter0=matAB * CollisionVector_ToNative(s.bounds.center);
		    ColSphere_Set(&aSpheresA[i],s.bounds.radius,COLLISION_POINT_REF(sphereCenter0),s.surface,s.piece);
		}
	}
	for(i = 0; i < modelA.numLines; i++)
		{
        // Capture both transformed values before the ordered C line update
        const CVector transformedLineStart=matAB * CollisionVector_ToNative(modelA.lines[i].p0);
        const CVector transformedLineEnd=matAB * CollisionVector_ToNative(modelA.lines[i].p1);
        ColLine_Set(&aLinesA[i],COLLISION_POINT_REF(transformedLineStart),COLLISION_POINT_REF(transformedLineEnd));
    }

	// Test them against model B's bounding volumes
	int numSpheresA = 0;
	int numLinesA = 0;
	for(i = 0; i < modelA.numSpheres; i++)
		if(TestSphereBox(aSpheresA[i].bounds, modelB.boundingBox))
			aSphereIndicesA[numSpheresA++] = i;
	// no actual check???
	for(i = 0; i < modelA.numLines; i++)
		aLineIndicesA[numLinesA++] = i;
	// No collision
	if(numSpheresA == 0 && numLinesA == 0)
		return 0;

	// Check model B against A's bounding volumes
	int numSpheresB = 0;
	int numBoxesB = 0;
	int numTrianglesB = 0;
	for(i = 0; i < modelB.numSpheres; i++){
		s.bounds.radius = modelB.spheres[i].bounds.radius;
		s.bounds.center = CollisionVector_FromNative(matBA * CollisionVector_ToNative(modelB.spheres[i].bounds.center));
		if(TestSphereBox(s.bounds, modelA.boundingBox))
			aSphereIndicesB[numSpheresB++] = i;
	}
	for(i = 0; i < modelB.numBoxes; i++)
		if(TestSphereBox(bsphereAB.bounds, modelB.boxes[i].bounds))
			aBoxIndicesB[numBoxesB++] = i;
	CalculateTrianglePlanes(&modelB);
	for(i = 0; i < modelB.numTriangles; i++)
		if(TestSphereTriangle(bsphereAB, modelB.vertices, modelB.triangles[i], modelB.trianglePlanes[i]))
			aTriangleIndicesB[numTrianglesB++] = i;
	assert(numSpheresB <= MAXNUMSPHERES);
	assert(numBoxesB <= MAXNUMBOXES);
	assert(numTrianglesB <= MAXNUMTRIS);
	// No collision
	if(numSpheresB == 0 && numBoxesB == 0 && numTrianglesB == 0)
		return 0;

	// We now have the collision volumes in A and B that are worth processing.

	// Process A's spheres against B's collision volumes
	int numCollisions = 0;
	for(i = 0; i < numSpheresA; i++){
		float coldist = 1.0e24f;
		bool hasCollided = false;

		for(j = 0; j < numSpheresB; j++)
			hasCollided |= ProcessSphereSphere(
				aSpheresA[aSphereIndicesA[i]],
				modelB.spheres[aSphereIndicesB[j]],
				spherepoints[numCollisions], coldist);
		for(j = 0; j < numBoxesB; j++)
			hasCollided |= ProcessSphereBox(
				aSpheresA[aSphereIndicesA[i]],
				modelB.boxes[aBoxIndicesB[j]],
				spherepoints[numCollisions], coldist);
		for(j = 0; j < numTrianglesB; j++)
			hasCollided |= ProcessSphereTriangle(
				aSpheresA[aSphereIndicesA[i]],
				modelB.vertices,
				modelB.triangles[aTriangleIndicesB[j]],
				modelB.trianglePlanes[aTriangleIndicesB[j]],
				spherepoints[numCollisions], coldist);

		if(hasCollided)
			numCollisions++;
	}
	for(i = 0; i < numCollisions; i++){
		spherepoints[i].point = CollisionVector_FromNative(matrixB * CollisionVector_ToNative(spherepoints[i].point));
		spherepoints[i].normal = CollisionVector_FromNative(Multiply3x3(matrixB, CollisionVector_ToNative(spherepoints[i].normal)));
	}

	// And the same thing for the lines in A
	for(i = 0; i < numLinesA; i++){
		aCollided[i] = false;

		for(j = 0; j < numSpheresB; j++)
			aCollided[i] |= ProcessLineSphere(
				aLinesA[aLineIndicesA[i]],
				modelB.spheres[aSphereIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
		for(j = 0; j < numBoxesB; j++)
			aCollided[i] |= ProcessLineBox(
				aLinesA[aLineIndicesA[i]],
				modelB.boxes[aBoxIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
		for(j = 0; j < numTrianglesB; j++)
			aCollided[i] |= ProcessLineTriangle(
				aLinesA[aLineIndicesA[i]],
				modelB.vertices,
				modelB.triangles[aTriangleIndicesB[j]],
				modelB.trianglePlanes[aTriangleIndicesB[j]],
				linepoints[aLineIndicesA[i]],
				linedists[aLineIndicesA[i]]);
	}
	for(i = 0; i < numLinesA; i++)
		if(aCollided[i]){
			j = aLineIndicesA[i];
			linepoints[j].point = CollisionVector_FromNative(matrixB * CollisionVector_ToNative(linepoints[j].point));
			linepoints[j].normal = CollisionVector_FromNative(Multiply3x3(matrixB, CollisionVector_ToNative(linepoints[j].normal)));
		}

	return numCollisions;	// sphere collisions
#endif
}


//
// Misc
//

float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point)
{
	float lensq = (*l1 - *l0).MagnitudeSqr();
	float dot = DotProduct(*point - *l0, *l1 - *l0);
	// Between 0 and len we're above the line.
	// if not, calculate distance to endpoint
	if(dot <= 0.0f) return (*point - *l0).Magnitude();
	if(dot >= lensq) return (*point - *l1).Magnitude();
	// distance to line
	float distSqr = (*point - *l0).MagnitudeSqr() - dot * dot / lensq;
	if(distSqr <= 0.f) return 0.f;
	return Sqrt(distSqr);
}

// same as above but also return the point on the line
float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point, CVector &closest)
{
	float lensq = (*l1 - *l0).MagnitudeSqr();
	float dot = DotProduct(*point - *l0, *l1 - *l0);
	// find out which point we're closest to
	if(dot <= 0.0f)
		closest = *l0;
	else if(dot >= lensq)
		closest = *l1;
	else
		closest = *l0 + (*l1 - *l0)*(dot/lensq);
	// this is the distance
	return (*point - closest).Magnitude();
}

void
CCollision::CalculateTrianglePlanes(CColModel *model)
{
	assert(model);
	if(model->numTriangles == 0)
		return;

	// Use the C linked-list functions and node types
	//+ rouz edit (ChatGPT)
	CLink *lptr;
	//- rouz edit (ChatGPT)
	if(model->trianglePlanes){
		// re-insert at front so it's not removed again soon
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		lptr = ColModel_GetLinkPtr(model);
		// Use the C linked-list functions and node types
		//+ rouz edit (ChatGPT)
		CLink_Remove(lptr);
		CLink_Insert(&ms_colModelCache.head, lptr);
		//- rouz edit (ChatGPT)
	}else{
		// Use the C linked-list functions and node types
		//+ rouz edit (ChatGPT)
		lptr = CLinkList_Insert(&ms_colModelCache, model);
		//- rouz edit (ChatGPT)
		if(lptr == nil){
			// make room if we have to, remove last in list
			lptr = ms_colModelCache.tail.prev;
			assert(lptr);
			assert(lptr->item);
			// Access the object payload through the C list node
			//+ rouz edit (ChatGPT)
			// Invoke the complete C collision owner at the original native lifetime or operation boundary
			ColModel_RemoveTrianglePlanes(((CColModel*)lptr->item));
			CLinkList_Remove(&ms_colModelCache, lptr);
			//- rouz edit (ChatGPT)
			// now this cannot fail
			// Use the C linked-list functions and node types
			//+ rouz edit (ChatGPT)
			lptr = CLinkList_Insert(&ms_colModelCache, model);
			//- rouz edit (ChatGPT)
			assert(lptr);
		}
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_CalculateTrianglePlanes(model);
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_SetLinkPtr(model,lptr);
	}
}

void
CCollision::RemoveTrianglePlanes(CColModel *model)
{
	if(model->trianglePlanes){
		// Use the C linked-list functions and node types
		//+ rouz edit (ChatGPT)
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		CLinkList_Remove(&ms_colModelCache, ColModel_GetLinkPtr(model));
		//- rouz edit (ChatGPT)
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_RemoveTrianglePlanes(model);
	}
}

void
CCollision::DrawColModel(const CMatrix &mat, const CColModel &colModel)
{
	int i;
	CVector min, max;
	CVector verts[8];
	CVector c;
	float r;

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

	min = CollisionVector_ToNative(colModel.boundingBox.min);
	max = CollisionVector_ToNative(colModel.boundingBox.max);

	verts[0] = mat * CVector(min.x, min.y, min.z);
	verts[1] = mat * CVector(min.x, min.y, max.z);
	verts[2] = mat * CVector(min.x, max.y, min.z);
	verts[3] = mat * CVector(min.x, max.y, max.z);
	verts[4] = mat * CVector(max.x, min.y, min.z);
	verts[5] = mat * CVector(max.x, min.y, max.z);
	verts[6] = mat * CVector(max.x, max.y, min.z);
	verts[7] = mat * CVector(max.x, max.y, max.z);

	// Draw the original endpoints and colors through the C line API

	Lines_RenderLineWithClipping(
		verts[0].x, verts[0].y, verts[0].z,
		verts[1].x, verts[1].y, verts[1].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[1].x, verts[1].y, verts[1].z,
		verts[3].x, verts[3].y, verts[3].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[3].x, verts[3].y, verts[3].z,
		verts[2].x, verts[2].y, verts[2].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[2].x, verts[2].y, verts[2].z,
		verts[0].x, verts[0].y, verts[0].z,
		0xFF0000FF, 0xFF0000FF);

	// Draw the original endpoints and colors through the C line API

	Lines_RenderLineWithClipping(
		verts[4].x, verts[4].y, verts[4].z,
		verts[5].x, verts[5].y, verts[5].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[5].x, verts[5].y, verts[5].z,
		verts[7].x, verts[7].y, verts[7].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[7].x, verts[7].y, verts[7].z,
		verts[6].x, verts[6].y, verts[6].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[6].x, verts[6].y, verts[6].z,
		verts[4].x, verts[4].y, verts[4].z,
		0xFF0000FF, 0xFF0000FF);

	// Draw the original endpoints and colors through the C line API

	Lines_RenderLineWithClipping(
		verts[0].x, verts[0].y, verts[0].z,
		verts[4].x, verts[4].y, verts[4].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[1].x, verts[1].y, verts[1].z,
		verts[5].x, verts[5].y, verts[5].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[2].x, verts[2].y, verts[2].z,
		verts[6].x, verts[6].y, verts[6].z,
		0xFF0000FF, 0xFF0000FF);
	// Draw the original endpoints and colors through the C line API
	Lines_RenderLineWithClipping(
		verts[3].x, verts[3].y, verts[3].z,
		verts[7].x, verts[7].y, verts[7].z,
		0xFF0000FF, 0xFF0000FF);

	for(i = 0; i < colModel.numSpheres; i++){
		c = mat * CollisionVector_ToNative(colModel.spheres[i].bounds.center);
		r = colModel.spheres[i].bounds.radius;

		// Draw the original endpoints and colors through the C line API

		Lines_RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x-r, c.y-r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x-r, c.y+r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x+r, c.y-r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			c.x,   c.y,   c.z-r,
			c.x+r, c.y+r, c.z,
			0xFF00FFFF, 0xFF00FFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			c.x-r, c.y-r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			c.x-r, c.y+r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			c.x+r, c.y-r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			c.x+r, c.y+r, c.z,
			c.x,   c.y,   c.z+r,
			0xFF00FFFF, 0xFF00FFFF);
	}

	for(i = 0; i < colModel.numLines; i++){
		verts[0] = CollisionVector_ToNative(colModel.lines[i].p0);
		verts[1] = CollisionVector_ToNative(colModel.lines[i].p1);

		verts[0] = mat * verts[0];
		verts[1] = mat * verts[1];

		// Draw the original endpoints and colors through the C line API

		Lines_RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[1].x, verts[1].y, verts[1].z,
			0x00FFFFFF, 0x00FFFFFF);
	}

	for(i = 0; i < colModel.numBoxes; i++){
		min = CollisionVector_ToNative(colModel.boxes[i].bounds.min);
		max = CollisionVector_ToNative(colModel.boxes[i].bounds.max);

		verts[0] = mat * CVector(min.x, min.y, min.z);
		verts[1] = mat * CVector(min.x, min.y, max.z);
		verts[2] = mat * CVector(min.x, max.y, min.z);
		verts[3] = mat * CVector(min.x, max.y, max.z);
		verts[4] = mat * CVector(max.x, min.y, min.z);
		verts[5] = mat * CVector(max.x, min.y, max.z);
		verts[6] = mat * CVector(max.x, max.y, min.z);
		verts[7] = mat * CVector(max.x, max.y, max.z);

		// Draw the original endpoints and colors through the C line API

		Lines_RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[1].x, verts[1].y, verts[1].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[1].x, verts[1].y, verts[1].z,
			verts[3].x, verts[3].y, verts[3].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[3].x, verts[3].y, verts[3].z,
			verts[2].x, verts[2].y, verts[2].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[2].x, verts[2].y, verts[2].z,
			verts[0].x, verts[0].y, verts[0].z,
			0xFFFFFFFF, 0xFFFFFFFF);

		// Draw the original endpoints and colors through the C line API

		Lines_RenderLineWithClipping(
			verts[4].x, verts[4].y, verts[4].z,
			verts[5].x, verts[5].y, verts[5].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[5].x, verts[5].y, verts[5].z,
			verts[7].x, verts[7].y, verts[7].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[7].x, verts[7].y, verts[7].z,
			verts[6].x, verts[6].y, verts[6].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[6].x, verts[6].y, verts[6].z,
			verts[4].x, verts[4].y, verts[4].z,
			0xFFFFFFFF, 0xFFFFFFFF);

		// Draw the original endpoints and colors through the C line API

		Lines_RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[4].x, verts[4].y, verts[4].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[1].x, verts[1].y, verts[1].z,
			verts[5].x, verts[5].y, verts[5].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[2].x, verts[2].y, verts[2].z,
			verts[6].x, verts[6].y, verts[6].z,
			0xFFFFFFFF, 0xFFFFFFFF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[3].x, verts[3].y, verts[3].z,
			verts[7].x, verts[7].y, verts[7].z,
			0xFFFFFFFF, 0xFFFFFFFF);
	}

	for(i = 0; i < colModel.numTriangles; i++){
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		verts[0] = CollisionVector_ToNative(ColModel_GetTrianglePoint(&colModel,colModel.triangles[i].a));
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		verts[1] = CollisionVector_ToNative(ColModel_GetTrianglePoint(&colModel,colModel.triangles[i].b));
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		verts[2] = CollisionVector_ToNative(ColModel_GetTrianglePoint(&colModel,colModel.triangles[i].c));
		verts[0] = mat * verts[0];
		verts[1] = mat * verts[1];
		verts[2] = mat * verts[2];
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[1].x, verts[1].y, verts[1].z,
			0x00FF00FF, 0x00FF00FF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[0].x, verts[0].y, verts[0].z,
			verts[2].x, verts[2].y, verts[2].z,
			0x00FF00FF, 0x00FF00FF);
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			verts[1].x, verts[1].y, verts[1].z,
			verts[2].x, verts[2].y, verts[2].z,
			0x00FF00FF, 0x00FF00FF);
	}

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
}

static void
GetSurfaceColor(uint8 surf, uint8 &r, uint8 &g, uint8 &b)
{
	// game doesn't do this
	r = 255;
	g = 128;
	b = 0;

	switch(SurfaceTable_GetAdhesionGroup(surf)){ // rouz edit (ChatGPT)
	case ADHESIVE_RUBBER:
		r = 255;
		g = 0;
		b = 0;
		break;
	case ADHESIVE_HARD:
		r = 255;
		g = 255;
		b = 128;
		break;
	case ADHESIVE_ROAD:
		r = 128;
		g = 128;
		b = 128;
		break;
	case ADHESIVE_LOOSE:
		r = 0;
		g = 255;
		b = 0;
		break;
	case ADHESIVE_SAND:
		r = 255;
		g = 128;
		b = 128;
		break;
	case ADHESIVE_WET:
		r = 0;
		g = 0;
		b = 255;
		break;
	}

	if(surf == SURFACE_SAND || surf == SURFACE_SAND_BEACH){
		r = 255;
		g = 255;
		b = 0;
	}

	float f = (surf & 0xF)/32.0f + 0.5f;
	r *= f;
	g *= f;
	b *= f;

	if(surf == SURFACE_TRANSPARENT_CLOTH || surf == SURFACE_METAL_CHAIN_FENCE ||
	   surf == SURFACE_TRANSPARENT_STONE || surf == SURFACE_SCAFFOLD_POLE)
		if((*Timer_GetFrameCounter()) & 1){
			r = 0;
			g = 0;
			b = 0;
		}
}

void
CCollision::DrawColModel_Coloured(const CMatrix &mat, const CColModel &colModel, int32 id)
{
	int i;
	int s;
	CVector verts[8];
	CVector min, max;
	uint8 r, g, b;
	RwImVertexIndex *iptr;
	RwIm3DVertex *vptr;

	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_ClearRenderBuffer();
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

	for(i = 0; i < colModel.numTriangles; i++){
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		verts[0] = CollisionVector_ToNative(ColModel_GetTrianglePoint(&colModel,colModel.triangles[i].a));
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		verts[1] = CollisionVector_ToNative(ColModel_GetTrianglePoint(&colModel,colModel.triangles[i].b));
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		verts[2] = CollisionVector_ToNative(ColModel_GetTrianglePoint(&colModel,colModel.triangles[i].c));
		verts[0] = mat * verts[0];
		verts[1] = mat * verts[1];
		verts[2] = mat * verts[2];

		s = colModel.triangles[i].surface;
		GetSurfaceColor(s, r, g, b);

		if(s > SURFACE_METAL_GATE){
			r = General_GetRandomNumber();
			g = General_GetRandomNumber();
			b = General_GetRandomNumber();
			printf("Illegal surfacetype:%d on MI:%d\n", s, id);
		}

		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_StartStoringLegacy(6, 3, &iptr, &vptr);
		RwIm3DVertexSetRGBA(&vptr[0], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[1], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[2], r, g, b, 255);
		RwIm3DVertexSetU(&vptr[0], 0.0f);
		RwIm3DVertexSetV(&vptr[0], 0.0f);
		RwIm3DVertexSetU(&vptr[1], 0.0f);
		RwIm3DVertexSetV(&vptr[1], 1.0f);
		RwIm3DVertexSetU(&vptr[2], 1.0f);
		RwIm3DVertexSetV(&vptr[2], 1.0f);
		RwIm3DVertexSetPos(&vptr[0], verts[0].x, verts[0].y, verts[0].z);
		RwIm3DVertexSetPos(&vptr[1], verts[1].x, verts[1].y, verts[1].z);
		RwIm3DVertexSetPos(&vptr[2], verts[2].x, verts[2].y, verts[2].z);
		iptr[0] = 0; iptr[1] = 1; iptr[2] = 2;
		iptr[3] = 0; iptr[4] = 2; iptr[5] = 1;
		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_StopStoring();
	}

	for(i = 0; i < colModel.numBoxes; i++){
		min = CollisionVector_ToNative(colModel.boxes[i].bounds.min);
		max = CollisionVector_ToNative(colModel.boxes[i].bounds.max);

		verts[0] = mat * CVector(min.x, min.y, min.z);
		verts[1] = mat * CVector(min.x, min.y, max.z);
		verts[2] = mat * CVector(min.x, max.y, min.z);
		verts[3] = mat * CVector(min.x, max.y, max.z);
		verts[4] = mat * CVector(max.x, min.y, min.z);
		verts[5] = mat * CVector(max.x, min.y, max.z);
		verts[6] = mat * CVector(max.x, max.y, min.z);
		verts[7] = mat * CVector(max.x, max.y, max.z);

		s = colModel.boxes[i].surface;
		GetSurfaceColor(s, r, g, b);

		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_StartStoringLegacy(36, 8, &iptr, &vptr);
		RwIm3DVertexSetRGBA(&vptr[0], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[1], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[2], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[3], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[4], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[5], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[6], r, g, b, 255);
		RwIm3DVertexSetRGBA(&vptr[7], r, g, b, 255);
		RwIm3DVertexSetU(&vptr[0], 0.0f);
		RwIm3DVertexSetV(&vptr[0], 0.0f);
		RwIm3DVertexSetU(&vptr[1], 0.0f);
		RwIm3DVertexSetV(&vptr[1], 1.0f);
		RwIm3DVertexSetU(&vptr[2], 1.0f);
		RwIm3DVertexSetV(&vptr[2], 1.0f);
		RwIm3DVertexSetU(&vptr[3], 0.0f);
		RwIm3DVertexSetV(&vptr[3], 0.0f);
		RwIm3DVertexSetU(&vptr[4], 0.0f);
		RwIm3DVertexSetV(&vptr[4], 1.0f);
		RwIm3DVertexSetU(&vptr[5], 1.0f);
		RwIm3DVertexSetV(&vptr[5], 1.0f);
		RwIm3DVertexSetU(&vptr[6], 0.0f);
		RwIm3DVertexSetV(&vptr[6], 1.0f);
		RwIm3DVertexSetU(&vptr[7], 1.0f);
		RwIm3DVertexSetV(&vptr[7], 1.0f);
		RwIm3DVertexSetPos(&vptr[0], verts[0].x, verts[0].y, verts[0].z);
		RwIm3DVertexSetPos(&vptr[1], verts[1].x, verts[1].y, verts[1].z);
		RwIm3DVertexSetPos(&vptr[2], verts[2].x, verts[2].y, verts[2].z);
		RwIm3DVertexSetPos(&vptr[3], verts[3].x, verts[3].y, verts[3].z);
		RwIm3DVertexSetPos(&vptr[4], verts[4].x, verts[4].y, verts[4].z);
		RwIm3DVertexSetPos(&vptr[5], verts[5].x, verts[5].y, verts[5].z);
		RwIm3DVertexSetPos(&vptr[6], verts[6].x, verts[6].y, verts[6].z);
		RwIm3DVertexSetPos(&vptr[7], verts[7].x, verts[7].y, verts[7].z);
		iptr[0] = 0; iptr[1] = 1; iptr[2] = 2;
		iptr[3] = 1; iptr[4] = 3; iptr[5] = 2;
		iptr[6] = 1; iptr[7] = 5; iptr[8] = 7;
		iptr[9] = 1; iptr[10] = 7; iptr[11] = 3;
		iptr[12] = 2; iptr[13] = 3; iptr[14] = 7;
		iptr[15] = 2; iptr[16] = 7; iptr[17] = 6;
		iptr[18] = 0; iptr[19] = 5; iptr[20] = 1;
		iptr[21] = 0; iptr[22] = 4; iptr[23] = 5;
		iptr[24] = 0; iptr[25] = 2; iptr[26] = 4;
		iptr[27] = 2; iptr[28] = 6; iptr[29] = 4;
		iptr[30] = 4; iptr[31] = 6; iptr[32] = 7;
		iptr[33] = 4; iptr[34] = 7; iptr[35] = 5;
		// Enter the C shared-buffer algorithm with the original arguments
		RenderBuffer_StopStoring();
	}
 
	// Enter the C shared-buffer algorithm with the original arguments
	RenderBuffer_RenderStuffInBuffer();
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
