//+ rouz edit (ChatGPT)
#pragma once
#include "ColLine.h"
#include "CollisionVector_native.h"
#include "VuVector.h"
static inline CVuVector ColLine_ToVu(const CColLine &line,unsigned endpoint)
{
    // Preserve every quadword bit including the original fourth lane
    CVuVector value;
    memcpy(&value,(const unsigned char*)&line+endpoint*16,16);
    return value;
}
//- rouz edit (ChatGPT)
