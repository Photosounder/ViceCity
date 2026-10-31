//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "AnimBlendList.h"
#include "AnimBlendFrameData.h"
#ifdef __cplusplus
#include "AnimBlendFrameData_native.h"
class CVector;
class CVector2D;
#else
typedef struct CVector CVector;
typedef struct CVector2D CVector2D;
#endif
typedef struct CAnimBlendClumpData {
    CAnimBlendLink link;
    int32_t numFrames;
    union {CVector2D *velocity2d;CVector *velocity3d;};
    AnimBlendFrameData *frames;
} CAnimBlendClumpData;
typedef void (*AnimBlendFrameCallback)(AnimBlendFrameData *frame,void *arg);
#ifdef __cplusplus
extern "C" {
#endif
void AnimBlendClump_Init(CAnimBlendClumpData *self);
void AnimBlendClump_Destroy(CAnimBlendClumpData *self);
void AnimBlendClump_SetNumberOfFrames(CAnimBlendClumpData *self,int32_t count);
void AnimBlendClump_SetNumberOfBones(CAnimBlendClumpData *self,int32_t count);
void AnimBlendClump_ForAllFrames(CAnimBlendClumpData *self,AnimBlendFrameCallback callback,void *arg);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
