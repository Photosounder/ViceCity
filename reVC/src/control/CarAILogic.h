//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint16_t (*CarAIRandomFn)(void);
typedef int32_t (*CarAIRandomRangeFn)(int32_t low, int32_t high);

#ifdef __cplusplus
extern "C" {
#endif
// Distance at which a car switches to the next path node
float CarAI_FindSwitchDistanceFar(uint8_t switchDistance, bool isLawEnforcer);
// What police cars and boats do about the player at a wanted level, random is only called when the level needs it
uint8_t CarAI_FindPoliceCarMissionForWantedLevel(int wantedLevel, CarAIRandomFn random);
uint8_t CarAI_FindPoliceBoatMissionForWantedLevel(int wantedLevel);
int32_t CarAI_FindPoliceCarSpeedForWantedLevel(int wantedLevel, float maxVelocity, CarAIRandomRangeFn randomRange);
// New cruise speed of a chasing police car, distances and speed are those between the car and the player
int CarAI_MellowOutChaseSpeed(int wantedLevel, int cruiseSpeed, float distanceToPlayer, bool playerInVehicle, float playerSpeed);
// Cruise speed of a police boat at a wanted level, or the current one when the level is unknown
int CarAI_MellowOutChaseSpeedBoat(int wantedLevel, int cruiseSpeed);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
