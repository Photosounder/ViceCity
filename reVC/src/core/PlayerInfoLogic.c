//+ rouz edit (Claude)
#include <math.h>
#include "PlayerInfoLogic.h"
#include "EntityStatus.h"
#include "General.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"

bool PlayerInfoLogic_IsCarCandidate(int status, float upZ, bool isBike, bool isCar, float pedZ, float carCentreZ)
{
	// Wrecks and trains on the move are out, cars have to be upright but bikes may lie
	if (status == STATUS_WRECKED || status == STATUS_TRAIN_MOVING)
		return false;
	if (!(upZ > 0.3f || isBike))
		return false;

	// At about the same height, or a car a little below
	return fabsf(pedZ - carCentreZ) < 2.0f || (isCar && carCentreZ < pedZ && pedZ - 4.f < carCentreZ);
}

bool PlayerInfoLogic_EvaluateCarPosition(float carX, float carY, float pedX, float pedY, float pedForwardX, float pedForwardY, float carBoundCentrePedDist, float *lastCloseness)
{
	// The angle to face is worked out from the car position and the distance score from its bounding centre
	float dx = carX - pedX;
	float dy = carY - pedY;
	float neededTurn = General_GetATanOfXY(pedForwardX, pedForwardY) - General_GetATanOfXY(dx, dy);
	float closeness;

	while (neededTurn >= PI)
		neededTurn -= 2 * PI;
	while (neededTurn < -PI)
		neededTurn += 2 * PI;

	// Accounts for the turn the long way round and the car distance
	closeness = (1.0f - fabsf(neededTurn) / TWOPI) * (10.0f - carBoundCentrePedDist);
	if (closeness > *lastCloseness) {
		*lastCloseness = closeness;
		return true;
	}
	return false;
}
int32_t PlayerInfo_CancelStuntTimer(int32_t timer, float timeStepMs, float rate)
{
	float v = timer - timeStepMs * rate;

	return (int32_t)(0.0f > v ? 0.0f : v);
}
//- rouz edit (Claude)
