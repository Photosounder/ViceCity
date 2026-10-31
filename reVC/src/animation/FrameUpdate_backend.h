//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendFrameUpdate.h"
#include "Quaternion.h"
typedef struct FrameUpdateVector2Out {float *x,*y;} FrameUpdateVector2Out;
typedef struct FrameUpdateQuaternionOut {float *x,*y,*z,*w;} FrameUpdateQuaternionOut;
#ifdef __cplusplus
extern "C" {
#endif
void *FrameUpdate_HostFrameMatrix(AnimBlendFrameData *frame);
void FrameUpdate_HostMatrixIdentity(void *matrix);
void FrameUpdate_HostMatrixUpdate(void *matrix);
QuaternionMatrixOut FrameUpdate_HostMatrixRotation(void *matrix);
KeyFrameVectorOut FrameUpdate_HostMatrixPosition(void *matrix);
FrameUpdateQuaternionOut FrameUpdate_HostPoseRotation(void *pose);
KeyFrameVectorOut FrameUpdate_HostPoseTranslation(void *pose);
FrameUpdateVector2Out FrameUpdate_HostVelocity2d(struct CAnimBlendClumpData *owner);
KeyFrameVectorOut FrameUpdate_HostVelocity3d(struct CAnimBlendClumpData *owner);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
