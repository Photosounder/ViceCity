//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct CColPoint {
    CollisionVector3 point;
    int32_t pad1;
    CollisionVector3 normal;
    int32_t pad2;
    uint8_t surfaceA,pieceA,surfaceB,pieceB;
    float depth;
} CColPoint;
#ifdef __cplusplus
extern "C" {
#endif
const CollisionVector3 *ColPoint_GetNormal(const CColPoint *self);
float ColPoint_GetDepth(const CColPoint *self);
void ColPoint_Set(CColPoint *self,float depth,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB);
void ColPoint_SetSurfaces(CColPoint *self,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB);
CColPoint *ColPoint_Assign(CColPoint *self,const CColPoint *other);
CColPoint ColPoint_CopyValue(CColPoint value);
void ColPoint_NormaliseNormal(CColPoint *self);
void ColPoint_ReverseNormal(CColPoint *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
