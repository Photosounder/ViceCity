//+ rouz edit (Claude)
#include <math.h>
#include "PlayerPedLogic.h"

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
//- rouz edit (Claude)
