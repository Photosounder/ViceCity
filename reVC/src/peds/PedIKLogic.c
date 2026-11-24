//+ rouz edit (Claude)
#include <math.h>
#include "PedIKLogic.h"

#define IK_CLAMP(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))

int PedIK_MoveLimb(LimbOrientation *limb, float targetYaw, float targetPitch, const LimbMovementInfo *moveInfo)
{
	int result = ONE_ANGLE_COULDNT_BE_SET_EXACTLY;

	// yaw

	if (fabsf(limb->yaw - targetYaw) < moveInfo->yawD) {
		limb->yaw = targetYaw;
		result = ANGLES_SET_EXACTLY;
	} else {
		if (limb->yaw > targetYaw) {
			limb->yaw -= moveInfo->yawD;
		} else if (limb->yaw < targetYaw) {
			limb->yaw += moveInfo->yawD;
		}
	}

	if (limb->yaw > moveInfo->maxYaw || limb->yaw < moveInfo->minYaw) {
		limb->yaw = IK_CLAMP(limb->yaw, moveInfo->minYaw, moveInfo->maxYaw);
		result = ANGLES_SET_TO_MAX;
	}

	// pitch

	if (fabsf(limb->pitch - targetPitch) < moveInfo->pitchD) {
		limb->pitch = targetPitch;
	} else {
		if (limb->pitch > targetPitch) {
			limb->pitch -= moveInfo->pitchD;
		} else if (limb->pitch < targetPitch) {
			limb->pitch += moveInfo->pitchD;
		}
		result = ONE_ANGLE_COULDNT_BE_SET_EXACTLY;
	}

	if (limb->pitch > moveInfo->maxPitch || limb->pitch < moveInfo->minPitch) {
		limb->pitch = IK_CLAMP(limb->pitch, moveInfo->minPitch, moveInfo->maxPitch);
		result = ANGLES_SET_TO_MAX;
	}
	return result;
}

void PedIK_ExtractYawAndPitchWorld(const float right[3], const float up[3], const float at[3], float *yaw, float *pitch)
{
	float f = up[0] * 0.0f + up[1] * 1.0f + up[2] * 0.0f;
	(void)at;
	f = IK_CLAMP(f, -1.0f, 1.0f);
	*yaw = acosf(f);
	if (up[0] > 0.0f) *yaw = -*yaw;

	f = right[0] * 0.0f + right[1] * 0.0f + right[2] * 1.0f;
	f = IK_CLAMP(f, -1.0f, 1.0f);
	*pitch = acosf(f);
	if (up[2] > 0.0f) *pitch = -*pitch;
}

void PedIK_ExtractYawAndPitchLocal(const float right[3], const float up[3], const float at[3], float *yaw, float *pitch)
{
	float f = at[0] * 0.0f + at[1] * 0.0f + at[2] * 1.0f;
	f = IK_CLAMP(f, -1.0f, 1.0f);
	*yaw = acosf(f);
	if (at[1] > 0.0f) *yaw = -*yaw;

	f = right[0] * 1.0f + right[1] * 0.0f + right[2] * 0.0f;
	f = IK_CLAMP(f, -1.0f, 1.0f);
	*pitch = acosf(f);
	if (up[0] > 0.0f) *pitch = -*pitch;
}
float PedIK_PitchFromUp(float upZ)
{
	return atan2f(upZ, sqrtf(1.0f - upZ * upZ));
}
float PedIK_ArmAimPitch(float targetPitch, float targetYaw)
{
	float v;

	if (targetPitch > 0.0f) {
		v = targetPitch - fabsf(targetYaw);
		return v > 0.0f ? v : 0.0f;
	}
	v = targetPitch + fabsf(targetYaw);
	return v < 0.0f ? v : 0.0f;
}

float PedIK_UpperArmPitch(float maxPitch, float lowerArmYaw, float upperArmYaw, float upperArmPitch)
{
	float uaPitchAmount = 1.0f - (lowerArmYaw + upperArmYaw) * 0.34f;
	float f1 = maxPitch * (uaPitchAmount > 0.0f ? uaPitchAmount : 0.0f);
	float f2 = 0.2f * lowerArmYaw + upperArmPitch;

	return f1 < f2 ? f1 : f2;
}
float PedIK_RollKept(float rollReduce)
{
	return 1.0f - (rollReduce < 1.0f ? rollReduce : 1.0f);
}
//- rouz edit (Claude)
