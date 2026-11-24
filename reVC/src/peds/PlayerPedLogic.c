//+ rouz edit (Claude)
#include <math.h>
#include <stdlib.h>
#include "PlayerPedLogic.h"
#include "General.h"

float PlayerPedLogic_NeighbourCloseness(float angleBetweenUs, bool lookToLeft)
{
	// Only candidates on the wanted side count, the nearer to straight ahead the better
	if (lookToLeft)
		return angleBetweenUs > 0.0f ? -fabsf(angleBetweenUs) : -100000.0f;
	return angleBetweenUs > 0.0f ? -100000.0f : -fabsf(angleBetweenUs);
}

float PlayerPedLogic_TargetCloseness(float dist, float angleBetweenUs, bool priority)
{
	// Near and in front scores best
	float closeness = -dist - 5.0f * fabsf(angleBetweenUs);
	if (priority)
		closeness += 30.0f;
	return closeness;
}

bool PlayerPedLogic_DoesTargetHaveToBeBroken(float distance, float weaponRange)
{
	// Out of range
	return distance > weaponRange;
}

void PlayerPedLogic_UseSprintEnergy(float *currentStamina, float *staminaProgress, float *maxStamina, bool infiniteSprint, bool adrenalineActive, float timeStep)
{
	// Spend stamina and track how long the player sprinted
	if (*currentStamina > -150.0f && !infiniteSprint && !adrenalineActive) {
		*currentStamina = *currentStamina - timeStep;
		*staminaProgress = *staminaProgress + timeStep;
	}

	// Enough sprinting makes the player fitter
	if (*staminaProgress >= 500.0f) {
		*staminaProgress = 0;
		if (*maxStamina < 1000.0f)
			*maxStamina += 10.0f;
	}
}
void PlayerPedLogic_MeleeAttackOffset(int dir, float dist, float *dx, float *dy)
{
	*dx = 0.0f;
	*dy = 0.0f;
	switch (dir) {
	case 0:
		*dy += dist;
		break;
	case 1:
		*dx += sqrtf(3.f / 4.f) * dist;
		*dy += 0.5f * dist;
		break;
	case 2:
		*dx += sqrtf(3.f / 4.f) * dist;
		*dy -= 0.5f * dist;
		break;
	case 3:
		*dy -= dist;
		break;
	case 4:
		*dx -= sqrtf(3.f / 4.f) * dist;
		*dy -= 0.5f * dist;
		break;
	case 5:
		*dx -= sqrtf(3.f / 4.f) * dist;
		*dy += 0.5f * dist;
		break;
	default:
		break;
	}
}
bool PlayerPedLogic_LookBehindAngle(float camAngle, float rotationCur, float lookDirection, bool isDucking, float *lookAngle)
{
	const float pi = (float)3.14159265358979323846;
	float angleBetweenPlayerAndCam = fabsf(camAngle - rotationCur);

	*lookAngle = camAngle;
	if (angleBetweenPlayerAndCam > 30.0f * pi / 180.0f && angleBetweenPlayerAndCam < 330.0f * pi / 180.0f) {
		if (angleBetweenPlayerAndCam > 150.0f * pi / 180.0f && angleBetweenPlayerAndCam < 210.0f * pi / 180.0f) {
			float rightTurnAngle = General_LimitRadianAngle(rotationCur - 150.0f * pi / 180.0f);
			float leftTurnAngle = General_LimitRadianAngle(150.0f * pi / 180.0f + rotationCur);

			if (lookDirection == 999999.0f || isDucking)
				*lookAngle = rightTurnAngle;
			else if (fabsf(rightTurnAngle - lookDirection) < fabsf(leftTurnAngle - lookDirection))
				*lookAngle = rightTurnAngle;
			else
				*lookAngle = leftTurnAngle;
		}
		return true;
	}
	return false;
}
int PlayerPedLogic_MeleeWantedDir(float distX, float distY)
{
	const float pi = (float)3.14159265358979323846;
	float angle = atan2f(-distX, -distY);
	float adjustedAngle = angle + 30.0f * pi / 180.0f;

	if (adjustedAngle < 0.f)
		adjustedAngle += pi * 2;
	return (int)floorf(adjustedAngle / (60.0f * pi / 180.0f));
}

int PlayerPedLogic_ClosestFreeMeleeDir(const bool occupied[6], int preferredDir)
{
	int closest = -99;
	int i;

	for (i = 0; i < 6; i++) {
		if (!occupied[i]) {
			if (abs(i - preferredDir) < abs(closest - preferredDir))
				closest = i;
		}
	}
	return closest;
}
void PlayerPedLogic_JumpPush(float rotationCur, float *x, float *y)
{
	float angleSin = sinf(rotationCur);
	float angleCos = cosf(rotationCur);

	*x = -angleSin * 3.0f;
	*y = 3.0f * angleCos;
}

float PlayerPedLogic_MoveSpeedRamp(float padMoveInGameUnit, float moveSpeed, float timeStep)
{
	float v = 0.07f * timeStep + moveSpeed;

	return padMoveInGameUnit < v ? padMoveInGameUnit : v;
}
float PlayerPedLogic_GunSpinUp(float spinSpeed, float timeStep)
{
	float v = spinSpeed + timeStep * 0.013f;

	return 0.45f < v ? 0.45f : v;
}

float PlayerPedLogic_GunSpinDown(float spinSpeed, float timeStep)
{
	float v = spinSpeed - timeStep * 0.003f;

	return 0.0f > v ? 0.0f : v;
}

bool PlayerPedLogic_CanReachTarget(float angleDiff, bool zRotImportant, bool canAimWithArm, float dz, float dist2D)
{
	const float halfpi = (float)3.14159265358979323846 / 2;

	return (!zRotImportant || canAimWithArm || fabsf(angleDiff) <= halfpi) &&
		(canAimWithArm || fabsf(dz) <= dist2D);
}
float PlayerPedLogic_SeekMoveSpeed(float dist2D)
{
	float v = 2.0f * dist2D;
	return 2.0f < v ? 2.0f : v;
}
//- rouz edit (Claude)
