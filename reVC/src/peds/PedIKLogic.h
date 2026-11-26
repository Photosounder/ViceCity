//+ rouz edit (Claude)
#pragma once
#include "VectorFwd.h" // rouz edit (Claude)
#include "PedIKData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Move a limb one step towards a target orientation within its limits, returns a LimbMoveStatus
int PedIK_MoveLimb(LimbOrientation *limb, float targetYaw, float targetPitch, const LimbMovementInfo *moveInfo);
// Yaw and pitch of a frame given by the right, up and at axes
void PedIK_ExtractYawAndPitchWorld(const float right[3], const float up[3], const float at[3], float *yaw, float *pitch);
void PedIK_ExtractYawAndPitchLocal(const CVector *right, const CVector *up, const CVector *at, float *yaw, float *pitch);
// Pitch of a bone from the vertical part of its up vector (between -1 and 1)
float PedIK_PitchFromUp(float upZ);
// Pitch of the torso of a ped that aims with its arm: the pitch is brought closer to 0 by the yaw
float PedIK_ArmAimPitch(float targetPitch, float targetYaw);
// Pitch of the upper arm of a ped that aims with its arm, from the max pitch, the yaws and the pitch of the arm
float PedIK_UpperArmPitch(float maxPitch, float lowerArmYaw, float upperArmYaw, float upperArmPitch);
// Fraction of the roll of the upper arm and the hand that is kept when the lower arm yaw reduces it: one minus the reduction, limited to one
float PedIK_RollKept(float rollReduce);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
