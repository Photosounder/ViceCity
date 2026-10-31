//+ rouz edit (ChatGPT)
#pragma once
typedef struct CollisionVector3 { float x, y, z; } CollisionVector3;
typedef struct CollisionPointRef { const float *x,*y,*z; } CollisionPointRef;
static inline CollisionPointRef CollisionPoint_Ref(const float *x,const float *y,const float *z)
{
    // Retain live references to float subobjects without depending on a native vector type
    const CollisionPointRef value={x,y,z};return value;
}
#define COLLISION_POINT_REF(value) CollisionPoint_Ref(&(value).x,&(value).y,&(value).z)
//- rouz edit (ChatGPT)
