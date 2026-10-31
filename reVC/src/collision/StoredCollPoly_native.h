//+ rouz edit (ChatGPT)
#pragma once
#include "StoredCollPoly.h"
#include "CollisionVector_native.h"
#include "VuVector.h"
static inline StoredPolyVertex StoredPoly_FromNative(const CVector &value)
{
    // Keep the original native vertex conversion before transferring its complete representation
#ifdef VU_COLLISION
    const CVuVector converted(value);
    StoredPolyVertex result;memcpy(&result,&converted,sizeof(result));return result;
#else
    return CollisionVector_FromNative(value);
#endif
}
static inline CVector StoredPoly_ToNative(const StoredPolyVertex &value)
{
    // Copy only the XYZ coordinates used by the remaining native scalar expressions
    CVector result;memcpy(&result.x,&value.x,sizeof(float));memcpy(&result.y,&value.y,sizeof(float));memcpy(&result.z,&value.z,sizeof(float));return result;
}
static inline CollisionPointRef StoredPoly_PositionRef(const CVector &value)
{
    // Evaluate a native position once and retain its temporary through the surrounding C call
    return COLLISION_POINT_REF(value);
}
//- rouz edit (ChatGPT)
