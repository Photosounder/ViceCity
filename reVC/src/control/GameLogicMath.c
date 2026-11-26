//+ rouz edit (Claude)
#include <math.h>
#include "GameLogicMath.h"

void GameLogicMath_PassTime(uint32_t time, int32_t clockMinutes, int32_t clockHours, int32_t daysPassed, int32_t *outMinutes, int32_t *outHours, int32_t *outDays)
{
	int32_t minutes = time + clockMinutes;
	int32_t hours = clockHours;
	int32_t days = daysPassed;

	// Carry whole hours
	for (; minutes >= 60; minutes -= 60)
		hours++;

	// Carry whole days
	if (hours > 23) {
		for (; hours >= 24; hours -= 24)
			days++;
	}

	*outMinutes = minutes;
	*outHours = hours;
	*outDays = days;
}

int GameLogic_FindClosestPoint(const float *points, int count, float x, float y, float z, float *outDistance)
{
	int closest = -1;
	float best = 999999.9f;
	int i;

	// Keep the nearest point
	for (i = 0; i < count; i++) {
		float dx = points[i*3 + 0] - x;
		float dy = points[i*3 + 1] - y;
		float dz = points[i*3 + 2] - z;
		float dist = sqrtf(dx*dx + dy*dy + dz*dz);
		if (dist < best) {
			best = dist;
			closest = i;
		}
	}
	*outDistance = best;
	return closest;
}

bool GameLogic_TaxiHasBeenMessedWith(bool hasDriver, bool driverDyingOrDead, bool driverDraggedOut, bool driverOnFire, bool driverToLeaveAndDie, float taxiHealth, bool taxiScorched)
{
	// No driver or a driver in trouble, or a damaged taxi
	return !hasDriver || driverDyingOrDead || driverDraggedOut || driverOnFire || driverToLeaveAndDie ||
		taxiHealth < 250.0f || taxiScorched;
}
//- rouz edit (Claude)
