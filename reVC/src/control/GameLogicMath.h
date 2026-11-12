//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Advance the game clock by a number of minutes, the days passed only change when midnight is crossed
void GameLogic_PassTime(uint32_t time, int32_t clockMinutes, int32_t clockHours, int32_t daysPassed, int32_t *outMinutes, int32_t *outHours, int32_t *outDays);
// Index of the point closest to a position or -1, points are xyz triples and the distance of the closest goes to outDistance
int GameLogic_FindClosestPoint(const float *points, int count, float x, float y, float z, float *outDistance);
// The short cut taxi is abandoned when its driver is gone or in trouble or the taxi is wrecked
bool GameLogic_TaxiHasBeenMessedWith(bool hasDriver, bool driverDyingOrDead, bool driverDraggedOut, bool driverOnFire, bool driverToLeaveAndDie, float taxiHealth, bool taxiScorched);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
