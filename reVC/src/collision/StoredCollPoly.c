//+ rouz edit (ChatGPT)
#include "StoredCollPoly.h"
#include "StoredCollPoly_backend.h"
#include "ColTriangle.h"
#include <string.h>
#include <math.h>
#include <assert.h>
static CollisionVector3 StoredPoly_Sub(CollisionVector3 a,CollisionVector3 b)
{
    // Retain complete native vector subtraction before subsequent arithmetic
    const CollisionVector3 value={a.x-b.x,a.y-b.y,a.z-b.z};return value;
}
static float StoredPoly_Dot(CollisionVector3 a,CollisionVector3 b)
{
    // Preserve left-associated native dot-product grouping
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
#ifndef VU_COLLISION
typedef struct StoredPolyVector2 {float x,y;} StoredPolyVector2;
static CollisionVector3 StoredPoly_Add(CollisionVector3 a,CollisionVector3 b)
{
    // Retain complete native addition after the scaled line difference
    const CollisionVector3 value={a.x+b.x,a.y+b.y,a.z+b.z};return value;
}
static CollisionVector3 StoredPoly_Scale(CollisionVector3 a,float scale)
{
    // Keep the original component multiplication before adding the line origin
    const CollisionVector3 value={a.x*scale,a.y*scale,a.z*scale};return value;
}
static StoredPolyVector2 StoredPoly_Sub2(StoredPolyVector2 a,StoredPolyVector2 b)
{
    // Preserve the original two-dimensional difference temporaries
    const StoredPolyVector2 value={a.x-b.x,a.y-b.y};return value;
}
static float StoredPoly_Cross2(StoredPolyVector2 a,StoredPolyVector2 b)
{
    // Retain the original signed edge test
    return a.x*b.y-a.y*b.x;
}
#endif
void StoredPoly_SetVertex(CStoredCollPoly *self,uint32_t index,const StoredPolyVertex *value)
{
    // Capture the complete original vertex before writing its destination including any VU fourth lane
    StoredPolyVertex copy;memcpy(&copy,value,sizeof(copy));memcpy(&self->verts[index],&copy,sizeof(copy));
}
CollisionVector3 StoredPoly_GetVertex(const CStoredCollPoly *self,uint32_t index)
{
    // Return exact XYZ representations without a native class view or arithmetic
    CollisionVector3 value;memcpy(&value.x,&self->verts[index].x,sizeof(float));memcpy(&value.y,&self->verts[index].y,sizeof(float));memcpy(&value.z,&self->verts[index].z,sizeof(float));return value;
}
bool StoredPoly_IsValidVerticalLine(CollisionPointRef pos,float z,CColPoint *point,CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
    // Retain the original invalid-cache early exit before reading any position coordinates
    if(!poly->valid)return false;
    // Snapshot the original two line endpoints and fill unused scratch lanes consumed only as XYZ
    const StoredPolyVertex p0={*pos.x,*pos.y,*pos.z,0.0f};
    const StoredPolyVertex p1={*pos.x,*pos.y,z,0.0f};
    const CollisionVector3 a=StoredPoly_GetVertex(poly,0),b=StoredPoly_GetVertex(poly,1),c=StoredPoly_GetVertex(poly,2);
    const CollisionVector3 v01=StoredPoly_Sub(b,a),v02=StoredPoly_Sub(c,a);
    // Keep the original cross-product direction and native normalization including its X-only fallback
    CollisionVector3 normal={v02.y*v01.z-v02.z*v01.y,v02.z*v01.x-v02.x*v01.z,v02.x*v01.y-v02.y*v01.x};
    const float square=StoredPoly_Dot(normal,normal);
    if(square>0.0f){
        // Apply the same reciprocal square root and ordered component writes
        const float inverse=1.0f/sqrtf(square);normal.x*=inverse;normal.y*=inverse;normal.z*=inverse;
    }else normal.x=1.0f;
    // Execute the existing VU routine with the original uncompressed plane distance
    const StoredPolyVertex plane={normal.x,normal.y,normal.z,StoredPoly_Dot(normal,a)};
    CollisionVector3 result;
    if(!StoredPoly_BackendLine(&p0,&p1,poly,&plane,&result)){
#ifdef FIX_BUGS
        // Preserve the fixed build's cache invalidation on a failed VU result
        return poly->valid=false;
#else
        // Preserve the classic build's unchanged valid flag on failure
        return false;
#endif
    }
    // Publish only the point coordinates while leaving every other contact field and valid flag unchanged
    point->point=result;return true;
#else
    // Preserve the full original scalar cached-polygon policy and all six projection directions
	float t;

	if(!poly->valid)
		return false;

	// maybe inlined?
	CColTrianglePlane plane;
	ColTrianglePlane_Set(&plane,COLLISION_POINT_REF(poly->verts[0]),COLLISION_POINT_REF(poly->verts[1]),COLLISION_POINT_REF(poly->verts[2]));

	const StoredPolyVertex *va = &poly->verts[0];
	const StoredPolyVertex *vb = &poly->verts[1];
	const StoredPolyVertex *vc = &poly->verts[2];
	const CollisionVector3 p0={*pos.x,*pos.y,*pos.z};
	const CollisionVector3 p1={*pos.x,*pos.y,z};

	// The rest is pretty much CCollision::ProcessLineTriangle

	// if points are on the same side, no collision
	if(ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(p0)) * ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(p1)) > 0.0f)
		return poly->valid = false;

	// intersection parameter on line
	CollisionVector3 normal;
	ColTrianglePlane_GetNormal(&plane,&normal.x,&normal.y,&normal.z);
	t = -ColTrianglePlane_CalcPoint(&plane,COLLISION_POINT_REF(p0)) / StoredPoly_Dot(StoredPoly_Sub(p1,p0),normal);
	// find point of intersection
	const CollisionVector3 p=StoredPoly_Add(p0,StoredPoly_Scale(StoredPoly_Sub(p1,p0),t));

	StoredPolyVector2 vec1, vec2, vec3, vect;
	switch(plane.dir){
	case DIR_X_POS:
		vec1.x = va->y; vec1.y = va->z;
		vec2.x = vc->y; vec2.y = vc->z;
		vec3.x = vb->y; vec3.y = vb->z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_X_NEG:
		vec1.x = va->y; vec1.y = va->z;
		vec2.x = vb->y; vec2.y = vb->z;
		vec3.x = vc->y; vec3.y = vc->z;
		vect.x = p.y; vect.y = p.z;
		break;
	case DIR_Y_POS:
		vec1.x = va->z; vec1.y = va->x;
		vec2.x = vc->z; vec2.y = vc->x;
		vec3.x = vb->z; vec3.y = vb->x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Y_NEG:
		vec1.x = va->z; vec1.y = va->x;
		vec2.x = vb->z; vec2.y = vb->x;
		vec3.x = vc->z; vec3.y = vc->x;
		vect.x = p.z; vect.y = p.x;
		break;
	case DIR_Z_POS:
		vec1.x = va->x; vec1.y = va->y;
		vec2.x = vc->x; vec2.y = vc->y;
		vec3.x = vb->x; vec3.y = vb->y;
		vect.x = p.x; vect.y = p.y;
		break;
	case DIR_Z_NEG:
		vec1.x = va->x; vec1.y = va->y;
		vec2.x = vb->x; vec2.y = vb->y;
		vec3.x = vc->x; vec3.y = vc->y;
		vect.x = p.x; vect.y = p.y;
		break;
	default:
		assert(0);
	}
	if(StoredPoly_Cross2(StoredPoly_Sub2(vec2,vec1),StoredPoly_Sub2(vect,vec1)) < 0.0f) return poly->valid = false;
	if(StoredPoly_Cross2(StoredPoly_Sub2(vec3,vec1),StoredPoly_Sub2(vect,vec1)) > 0.0f) return poly->valid = false;
	if(StoredPoly_Cross2(StoredPoly_Sub2(vec3,vec2),StoredPoly_Sub2(vect,vec2)) < 0.0f) return poly->valid = false;
	point->point=p;
	return poly->valid = true;
#endif
}
//- rouz edit (ChatGPT)
