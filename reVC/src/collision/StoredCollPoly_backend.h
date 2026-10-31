//+ rouz edit (ChatGPT)
#pragma once
#include "StoredCollPoly.h"
#ifdef VU_COLLISION
#ifdef __cplusplus
extern "C" {
#endif
bool StoredPoly_BackendLine(const StoredPolyVertex *p0,const StoredPolyVertex *p1,const CStoredCollPoly *poly,const StoredPolyVertex *plane,CollisionVector3 *point);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
