//+ rouz edit (ChatGPT)
#pragma once
#include "CollisionVector.h"
#include "Vector.h"
#include <string.h>
static inline CVector CollisionVector_ToNative(const CollisionVector3 &value)
{
    // Copy coordinate representations without arithmetic or struct type punning
    CVector result;
    memcpy(&result.x,&value.x,sizeof(float));memcpy(&result.y,&value.y,sizeof(float));memcpy(&result.z,&value.z,sizeof(float));
    return result;
}
static inline CollisionVector3 CollisionVector_FromNative(const CVector &value)
{
    // Preserve native coordinate bits in the plain C representation
    CollisionVector3 result;
    memcpy(&result.x,&value.x,sizeof(float));memcpy(&result.y,&value.y,sizeof(float));memcpy(&result.z,&value.z,sizeof(float));
    return result;
}
//- rouz edit (ChatGPT)
