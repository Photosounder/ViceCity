//+ rouz edit (ChatGPT)
#pragma once
#include "CompressedVector.h"
enum Direction {
	DIR_X_POS,
	DIR_X_NEG,
	DIR_Y_POS,
	DIR_Y_NEG,
	DIR_Z_POS,
	DIR_Z_NEG,
};

typedef struct CColTriangle {
    uint16_t a,b,c;
    uint8_t surface;
} CColTriangle;
typedef struct CColTrianglePlane {
#ifdef VU_COLLISION
    CompressedVector normal;
    int16_t dist;
#else
    CollisionVector3 normal;
    float dist;
    uint8_t dir;
#endif
} CColTrianglePlane;
#ifdef __cplusplus
extern "C" {
#endif
void ColTriangle_Set(CColTriangle *self,int32_t a,int32_t b,int32_t c,uint8_t surface);
void ColTrianglePlane_Set(CColTrianglePlane *self,CollisionPointRef a,CollisionPointRef b,CollisionPointRef c);
void ColTrianglePlane_SetFromVertices(CColTrianglePlane *self,const CompressedVector *vertices,const CColTriangle *triangle);
void ColTrianglePlane_GetNormal(const CColTrianglePlane *self,float *x,float *y,float *z);
float ColTrianglePlane_CalcPoint(const CColTrianglePlane *self,CollisionPointRef point);
#ifdef VU_COLLISION
void ColTrianglePlane_Unpack(const CColTrianglePlane *self,void *quadword);
#else
float ColTrianglePlane_GetNormalX(const CColTrianglePlane *self);
float ColTrianglePlane_GetNormalY(const CColTrianglePlane *self);
float ColTrianglePlane_GetNormalZ(const CColTrianglePlane *self);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
