//+ rouz edit (Claude)
#pragma once
#include <stdint.h>

#ifdef __cplusplus
class CPed;
#else
typedef struct CPed CPed;
#endif

typedef struct LimbOrientation
{
	float yaw;
	float pitch;
} LimbOrientation;

typedef struct LimbMovementInfo {
	float maxYaw;
	float minYaw;
	float yawD;
	float maxPitch;
	float minPitch;
	float pitchD;
} LimbMovementInfo;

enum LimbMoveStatus {
	ANGLES_SET_TO_MAX, // because given angles were unreachable
	ONE_ANGLE_COULDNT_BE_SET_EXACTLY, // because it can't be reached in a jiffy
	ANGLES_SET_EXACTLY
};

// Flag values of CPedIK::m_flags, the same as the CPedIK enum
#define PEDIK_GUN_POINTED_SUCCESSFULLY 1
#define PEDIK_LOOKAROUND_HEAD_ONLY 2
#define PEDIK_AIMS_WITH_ARM 4

// The plain data of a ped's inverse kinematics state, usable from C
typedef struct CPedIKData {
	CPed *m_ped;
	LimbOrientation m_headOrient;
	LimbOrientation m_torsoOrient;
	LimbOrientation m_upperArmOrient;
	LimbOrientation m_lowerArmOrient;
	int32_t m_flags;
} CPedIKData;
//- rouz edit (Claude)
