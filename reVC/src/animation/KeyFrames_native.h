//+ rouz edit (ChatGPT)
#pragma once
#include "KeyFrames.h"
#include <string.h>
static inline KeyFrameVectorRef KeyFrame_VectorRef(const CVector &value)
{
    // Borrow actual native coordinates for the complete surrounding packed conversion
    const KeyFrameVectorRef result={&value.x,&value.y,&value.z};return result;
}
static inline KeyFrameVectorOut KeyFrame_VectorOut(CVector &value)
{
    // Preserve actual native output fields without introducing a record view
    const KeyFrameVectorOut result={&value.x,&value.y,&value.z};return result;
}
static inline KeyFrameVector3 KeyFrame_FromNative(const CVector &value)
{
    // Transfer exact XYZ representations before storing the unpacked C translation
    KeyFrameVector3 result;memcpy(&result.x,&value.x,4);memcpy(&result.y,&value.y,4);memcpy(&result.z,&value.z,4);return result;
}
static inline CVector KeyFrame_ToNative(const KeyFrameVector3 &value)
{
    // Create an actual native value for the remaining native vector expressions
    CVector result;memcpy(&result.x,&value.x,4);memcpy(&result.y,&value.y,4);memcpy(&result.z,&value.z,4);return result;
}
//- rouz edit (ChatGPT)
