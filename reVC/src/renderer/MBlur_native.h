//+ rouz edit (ChatGPT)
#pragma once
#include "MBlur.h"
static inline MBlurRectRef MBlur_NativeRect(RwRect *rect)
{
    // Borrow the original SDK rectangle's actual integer subobjects without incompatible-record aliasing
    const MBlurRectRef value={&rect->x,&rect->y,&rect->w,&rect->h};return value;
}
//- rouz edit (ChatGPT)
