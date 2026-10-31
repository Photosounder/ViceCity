//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendFrameData.h"
#include <string.h>
static inline RwFrame *AnimBlendFrame_GetFrame(const AnimBlendFrameData *value)
{
    // Recover the original SDK object without introducing a synthetic frame view
    return static_cast<RwFrame*>(value->frame);
}
static inline RpHAnimStdInterpFrame *AnimBlendFrame_GetHAnim(const AnimBlendFrameData *value)
{
    // Keep writes through the actual live SDK pose object
    return static_cast<RpHAnimStdInterpFrame*>(value->hanimFrame);
}
static inline KeyFrameVector3 AnimBlendFrame_FromNativePosition(const RwV3d &value)
{
    // Transfer all native reset-position coordinate bits without arithmetic
    KeyFrameVector3 result;memcpy(&result.x,&value.x,4);memcpy(&result.y,&value.y,4);memcpy(&result.z,&value.z,4);return result;
}
//- rouz edit (ChatGPT)
