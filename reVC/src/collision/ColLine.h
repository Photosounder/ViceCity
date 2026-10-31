//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct CColLine {
    CollisionVector3 p0;
    int32_t pad0;
    CollisionVector3 p1;
    int32_t pad1;
} CColLine;
#ifdef __cplusplus
extern "C" {
#endif
void ColLine_Init(CColLine *self,CollisionPointRef p0,CollisionPointRef p1);
void ColLine_Set(CColLine *self,CollisionPointRef p0,CollisionPointRef p1);
CColLine ColLine_Make(CollisionPointRef p0,CollisionPointRef p1);
CColLine ColLine_FromRaw32(const void *bytes);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
