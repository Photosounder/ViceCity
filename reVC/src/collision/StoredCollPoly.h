//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "ColPoint.h"
#ifdef VU_COLLISION
typedef struct StoredPolyVertex {
#ifdef __cplusplus
    alignas(16) float x;
#else
    _Alignas(16) float x;
#endif
    float y,z,w;
} StoredPolyVertex;
#else
typedef CollisionVector3 StoredPolyVertex;
#endif
typedef struct CStoredCollPoly {
    StoredPolyVertex verts[3];
    bool valid;
} CStoredCollPoly;
#ifdef __cplusplus
extern "C" {
#endif
void StoredPoly_SetVertex(CStoredCollPoly *self,uint32_t index,const StoredPolyVertex *value);
CollisionVector3 StoredPoly_GetVertex(const CStoredCollPoly *self,uint32_t index);
bool StoredPoly_IsValidVerticalLine(CollisionPointRef pos,float z,CColPoint *point,CStoredCollPoly *poly);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
