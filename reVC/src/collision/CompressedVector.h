//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"
#include "CollisionVector.h"
typedef struct CompressedVector {
#ifdef COMPRESSED_COL_VECTORS
    int16_t x, y, z;
#else
    float x, y, z;
#endif
} CompressedVector;
#ifdef __cplusplus
extern "C" {
#endif
CollisionVector3 CompressedVector_Get(const CompressedVector *self);
void CompressedVector_Set(CompressedVector *self,float x,float y,float z);
#ifdef COMPRESSED_COL_VECTORS
void CompressedVector_Unpack(const CompressedVector *self,void *quadword);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
