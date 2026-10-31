//+ rouz edit (ChatGPT)
#include "ColTriangle.h"
#include <math.h>

static CollisionVector3 Triangle_Cross(CollisionPointRef a,CollisionPointRef b,CollisionPointRef c)
{
    // Capture both complete differences before writing any destination normal component
    const CollisionVector3 first={*c.x-*a.x,*c.y-*a.y,*c.z-*a.z};
    const CollisionVector3 second={*b.x-*a.x,*b.y-*a.y,*b.z-*a.z};
    const CollisionVector3 result={first.y*second.z-first.z*second.y,first.z*second.x-first.x*second.z,first.x*second.y-first.y*second.x};
    return result;
}
static void Triangle_Normalise(CollisionVector3 *value)
{
    // Preserve CVector normalization including the fallback that writes only X
    const float square=value->x*value->x+value->y*value->y+value->z*value->z;
    if(square>0.0f){
        // Use the original reciprocal square root and ordered component updates
        const float inverse=1.0f/sqrtf(square);
        value->x*=inverse;value->y*=inverse;value->z*=inverse;
    }else value->x=1.0f;
}
void ColTriangle_Set(CColTriangle *self,int32_t a,int32_t b,int32_t c,uint8_t surface)
{
    // Preserve index narrowing, member order and untouched triangle padding
    self->a=a;self->b=b;self->c=c;self->surface=surface;
}
void ColTrianglePlane_Set(CColTrianglePlane *self,CollisionPointRef a,CollisionPointRef b,CollisionPointRef c)
{
#ifdef VU_COLLISION
    // Compute the complete normal and distance before narrowing the packed outputs
    CollisionVector3 normal=Triangle_Cross(a,b,c);
    Triangle_Normalise(&normal);
    const float distance=normal.x * *a.x+normal.y * *a.y+normal.z * *a.z;
    self->normal.x=normal.x*4096.0f;
    self->normal.y=normal.y*4096.0f;
    self->normal.z=normal.z*4096.0f;
    self->dist=distance*128.0f;
#else
    // Keep source references live after the destination normal has been overwritten and normalized
    self->normal=Triangle_Cross(a,b,c);
    Triangle_Normalise(&self->normal);
    self->dist=self->normal.x * *a.x+self->normal.y * *a.y+self->normal.z * *a.z;
    const CollisionVector3 absolute={fabsf(self->normal.x),fabsf(self->normal.y),fabsf(self->normal.z)};
    if(absolute.x>absolute.y&&absolute.x>absolute.z)
        self->dir=self->normal.x<0.0f?DIR_X_NEG:DIR_X_POS;
    else if(absolute.y>absolute.z)
        self->dir=self->normal.y<0.0f?DIR_Y_NEG:DIR_Y_POS;
    else
        self->dir=self->normal.z<0.0f?DIR_Z_NEG:DIR_Z_POS;
#endif
}
void ColTrianglePlane_SetFromVertices(CColTrianglePlane *self,const CompressedVector *vertices,const CColTriangle *triangle)
{
    // Decode all three original temporaries before invoking the complete plane operation
    const CollisionVector3 a=CompressedVector_Get(&vertices[triangle->a]);
    const CollisionVector3 b=CompressedVector_Get(&vertices[triangle->b]);
    const CollisionVector3 c=CompressedVector_Get(&vertices[triangle->c]);
    ColTrianglePlane_Set(self,COLLISION_POINT_REF(a),COLLISION_POINT_REF(b),COLLISION_POINT_REF(c));
}
void ColTrianglePlane_GetNormal(const CColTrianglePlane *self,float *x,float *y,float *z)
{
    // Preserve ordered output writes and the original normal scaling
#ifdef VU_COLLISION
    *x=self->normal.x/4096.0f;*y=self->normal.y/4096.0f;*z=self->normal.z/4096.0f;
#else
    *x=self->normal.x;*y=self->normal.y;*z=self->normal.z;
#endif
}
float ColTrianglePlane_CalcPoint(const CColTrianglePlane *self,CollisionPointRef point)
{
    // Retain the original dot-product grouping and representation-specific distance scale
#ifdef VU_COLLISION
    CollisionVector3 normal;
    ColTrianglePlane_GetNormal(self,&normal.x,&normal.y,&normal.z);
    return normal.x * *point.x+normal.y * *point.y+normal.z * *point.z-self->dist/128.0f;
#else
    return self->normal.x * *point.x+self->normal.y * *point.y+self->normal.z * *point.z-self->dist;
#endif
}
#ifdef VU_COLLISION
void ColTrianglePlane_Unpack(const CColTrianglePlane *self,void *quadword)
{
    // Preserve all four packed plane lanes and the native PS2 instruction sequence
#ifdef GTA_PS2
    typedef unsigned int TriangleQuadWord __attribute__((mode(TI)));
		__asm__ volatile (
			"lh      $8, 0(%1)\n"
			"lh      $9, 2(%1)\n"
			"lh      $10, 4(%1)\n"
			"lh      $11, 6(%1)\n"
			"pextlw  $10, $8\n"
			"pextlw  $11, $9\n"
			"pextlw  $2, $11, $10\n"
			"sq      $2, %0\n"
			: "=m" (*(TriangleQuadWord*)quadword)
			: "r" (self)
			: "$8", "$9", "$10", "$11", "$2"
		);
#else
    int32_t *qword=(int32_t*)quadword;
    qword[0]=self->normal.x;qword[1]=self->normal.y;qword[2]=self->normal.z;qword[3]=self->dist;
#endif
}
#else
float ColTrianglePlane_GetNormalX(const CColTrianglePlane *self)
{
    // Return the original X component
    return self->normal.x;
}
float ColTrianglePlane_GetNormalY(const CColTrianglePlane *self)
{
    // Return the original Y component
    return self->normal.y;
}
float ColTrianglePlane_GetNormalZ(const CColTrianglePlane *self)
{
    // Return the original Z component
    return self->normal.z;
}
#endif
//- rouz edit (ChatGPT)
