//+ rouz edit (ChatGPT)
#include "common.h"
#include "AnimBlendFrameData_native.h"
#include "AnimBlendClumpData.h"
#include "Quaternion_native.h"
#include "FrameUpdate_backend.h"
void *FrameUpdate_HostFrameMatrix(AnimBlendFrameData *frame)
{
    // Recover the same live SDK matrix at the original callback lookup boundary
    return RwFrameGetMatrix(AnimBlendFrame_GetFrame(frame));
}
void FrameUpdate_HostMatrixIdentity(void *matrix)
{
    // Invoke the original native identity operation on the actual SDK matrix
    RwMatrixSetIdentity(static_cast<RwMatrix*>(matrix));
}
void FrameUpdate_HostMatrixUpdate(void *matrix)
{
    // Preserve the final SDK matrix update at the original callback position
    RwMatrixUpdate(static_cast<RwMatrix*>(matrix));
}
QuaternionMatrixOut FrameUpdate_HostMatrixRotation(void *matrix)
{
    // Borrow actual SDK rotation components for the existing complete C quaternion operation
    return Quaternion_MatrixOut(*static_cast<RwMatrix*>(matrix));
}
KeyFrameVectorOut FrameUpdate_HostMatrixPosition(void *matrix)
{
    // Return references to the actual live matrix position without making a temporary view
    RwMatrix *value=static_cast<RwMatrix*>(matrix);KeyFrameVectorOut out={&value->pos.x,&value->pos.y,&value->pos.z};return out;
}
FrameUpdateQuaternionOut FrameUpdate_HostPoseRotation(void *pose)
{
    // Borrow the actual SDK pose quaternion fields while leaving all blending policy in C
    RpHAnimStdInterpFrame *value=static_cast<RpHAnimStdInterpFrame*>(pose);FrameUpdateQuaternionOut out={&value->q.imag.x,&value->q.imag.y,&value->q.imag.z,&value->q.real};return out;
}
KeyFrameVectorOut FrameUpdate_HostPoseTranslation(void *pose)
{
    // Borrow actual live SDK pose translation fields without copying their values
    RpHAnimStdInterpFrame *value=static_cast<RpHAnimStdInterpFrame*>(pose);KeyFrameVectorOut out={&value->t.x,&value->t.y,&value->t.z};return out;
}
FrameUpdateVector2Out FrameUpdate_HostVelocity2d(CAnimBlendClumpData *owner)
{
    // Borrow the real native velocity vector fields through the current owner pointer
    FrameUpdateVector2Out out={&owner->velocity2d->x,&owner->velocity2d->y};return out;
}
KeyFrameVectorOut FrameUpdate_HostVelocity3d(CAnimBlendClumpData *owner)
{
    // Preserve the original union member and live native velocity storage
    KeyFrameVectorOut out={&owner->velocity3d->x,&owner->velocity3d->y,&owner->velocity3d->z};return out;
}
//- rouz edit (ChatGPT)
