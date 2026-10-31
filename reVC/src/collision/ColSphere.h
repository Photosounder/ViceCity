//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "CollisionVector.h"
#include "SurfaceTable.h"
typedef struct CSphere {
    CollisionVector3 center;
    float radius;
} CSphere;
typedef struct CColSphere {
    CSphere bounds;
    uint8_t surface,piece;
} CColSphere;
typedef struct CollisionPointOut { float *x,*y,*z; } CollisionPointOut;
static inline CollisionPointOut CollisionPoint_Output(float *x,float *y,float *z)
{
    // Retain the actual writable float subobjects without native vector type assumptions
    const CollisionPointOut value={x,y,z};return value;
}
#define COLLISION_POINT_OUT(value) CollisionPoint_Output(&(value).x,&(value).y,&(value).z)
#ifdef __cplusplus
extern "C" {
#endif
void Sphere_Set(CSphere *self,float radius,CollisionPointRef center);
void ColSphere_SetBounds(CColSphere *self,float radius,CollisionPointRef center);
void ColSphere_Set(CColSphere *self,float radius,CollisionPointRef center,uint8_t surface,uint8_t piece);
bool ColSphere_IntersectRay(CColSphere *self,CollisionPointRef from,CollisionPointRef dir,CollisionPointOut entry,CollisionPointOut exit);
CSphere Sphere_FromRaw16(const void *bytes);
CColSphere ColSphere_FromRaw16(const void *bytes);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
