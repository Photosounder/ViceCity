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
#include "CollisionTests.h"
#include "CollisionProcess.h"
#include "CollisionTriangle.h"
#include "CollisionModel.h"
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
	// Delegate to the C implementation
	return Collision_TestSphereSphere(&s1, &s2);
}

bool
CCollision::TestSphereBox(const CSphere &sph, const CBox &box)
{
	// Delegate to the C implementation
	return Collision_TestSphereBox(&sph, &box);
}

bool
CCollision::TestLineBox(const CColLine &line, const CBox &box)
{
	// Delegate to the C implementation
	return Collision_TestLineBox(&line, &box);
}

bool
CCollision::TestVerticalLineBox(const CColLine &line, const CBox &box)
{
	// Delegate to the C implementation
	return Collision_TestVerticalLineBox(&line, &box);
}

bool
CCollision::TestLineTriangle(const CColLine &line, const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
	// Delegate to the C implementation
	return Collision_TestLineTriangle(&line, verts, &tri, &plane);
}

// Test if line segment intersects with sphere.
// If the first point is inside the sphere this test does not register a collision!
// The code is reversed from the original code and rather ugly, see Process for a clear version.
// TODO: actually rewrite this mess
bool
CCollision::TestLineSphere(const CColLine &line, const CColSphere &sph)
{
	// Delegate to the C implementation
	return Collision_TestLineSphere(&line, &sph);
}

bool
CCollision::TestSphereTriangle(const CColSphere &sphere,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane)
{
	// Delegate to the C implementation
	return Collision_TestSphereTriangle(&sphere, verts, &tri, &plane);
}

bool
CCollision::TestLineOfSight(const CColLine &line, const CMatrix &matrix, CColModel &model, bool ignoreSeeThrough, bool ignoreShootThrough)
{
	// Delegate to the C implementation
	return Collision_TestLineOfSight(&line, matrix.f, &model, ignoreSeeThrough, ignoreShootThrough, &ms_colModelCache);
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
	// Delegate to the C implementation
	return Collision_ProcessSphereSphere(&s1, &s2, &point, &mindistsq);
}

bool
CCollision::ProcessSphereBox(const CColSphere &sph, const CColBox &box, CColPoint &point, float &mindistsq)
{
	// Delegate to the C implementation
	return Collision_ProcessSphereBox(&sph, &box, &point, &mindistsq);
}

bool
CCollision::ProcessLineBox(const CColLine &line, const CColBox &box, CColPoint &point, float &mindist)
{
	// Delegate to the C implementation
	return Collision_ProcessLineBox(&line, &box, &point, &mindist);
}

// If line.p0 lies inside sphere, no collision is registered.
bool
CCollision::ProcessLineSphere(const CColLine &line, const CColSphere &sphere, CColPoint &point, float &mindist)
{
	// Delegate to the C implementation
	return Collision_ProcessLineSphere(&line, &sphere, &point, &mindist);
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
	// Do the intersection in C and track the polygon here
	float v[9];
	if (!Collision_ProcessLineTriangle(&line, verts, &tri, &plane, &point, &mindist, v))
		return false;
	if (poly) {
		const StoredPolyVertex storedVertex0 = StoredPoly_FromNative(CVector(v[0], v[1], v[2]));
		const StoredPolyVertex storedVertex1 = StoredPoly_FromNative(CVector(v[3], v[4], v[5]));
		const StoredPolyVertex storedVertex2 = StoredPoly_FromNative(CVector(v[6], v[7], v[8]));
		StoredPoly_SetVertex(poly, 0, &storedVertex0);
		StoredPoly_SetVertex(poly, 1, &storedVertex1);
		StoredPoly_SetVertex(poly, 2, &storedVertex2);
		poly->valid = true;
	}
	return true;
}

bool
CCollision::ProcessSphereTriangle(const CColSphere &sphere,
	const CompressedVector *verts, const CColTriangle &tri, const CColTrianglePlane &plane,
	CColPoint &point, float &mindistsq)
{
	// Delegate to the C implementation
	return Collision_ProcessSphereTriangle(&sphere, verts, &tri, &plane, &point, &mindistsq);
}

bool
CCollision::ProcessLineOfSight(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough)
{
	// Delegate to the C implementation
	return Collision_ProcessLineOfSight(&line, matrix.f, &model, &point, &mindist, ignoreSeeThrough, ignoreShootThrough, &ms_colModelCache);
}

bool
CCollision::ProcessVerticalLine(const CColLine &line,
	const CMatrix &matrix, CColModel &model,
	CColPoint &point, float &mindist, bool ignoreSeeThrough, bool ignoreShootThrough, CStoredCollPoly *poly)
{
	// Delegate to the C implementation
	return Collision_ProcessVerticalLine(&line, matrix.f, &model, &point, &mindist, ignoreSeeThrough, poly, &ms_colModelCache);
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
	// Delegate to the C implementation
	return Collision_ProcessColModels(matrixA.f, &modelA, matrixB.f, &modelB, spherepoints, linepoints, linedists, &ms_colModelCache);
}


//
// Misc
//

float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point)
{
	// Delegate to the C implementation
	return Collision_DistToLine(&l0->x, &l1->x, &point->x);
}

// same as above but also return the point on the line
float
CCollision::DistToLine(const CVector *l0, const CVector *l1, const CVector *point, CVector &closest)
{
	// Delegate to the C implementation
	return Collision_DistToLineClosest(&l0->x, &l1->x, &point->x, &closest.x);
}

void
CCollision::CalculateTrianglePlanes(CColModel *model)
{
	// Delegate to the C implementation
	Collision_CalculateTrianglePlanes(&ms_colModelCache, model);
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
