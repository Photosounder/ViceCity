//+ rouz edit (Claude)
#pragma once
#include "PedIKData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Move a limb one step towards a target orientation within its limits, returns a LimbMoveStatus
int PedIK_MoveLimb(LimbOrientation *limb, float targetYaw, float targetPitch, const LimbMovementInfo *moveInfo);
// Yaw and pitch of a frame given by the right, up and at axes
void PedIK_ExtractYawAndPitchWorld(const float right[3], const float up[3], const float at[3], float *yaw, float *pitch);
void PedIK_ExtractYawAndPitchLocal(const float right[3], const float up[3], const float at[3], float *yaw, float *pitch);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
