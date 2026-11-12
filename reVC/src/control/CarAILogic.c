//+ rouz edit (Claude)
#include "CarAILogic.h"
#include "AutoPilot.h"

#define GAME_SPEED_TO_CARAI_SPEED 60.0f

float CarAI_FindSwitchDistanceFar(uint8_t switchDistance, bool isLawEnforcer)
{
	// Police cars switch far out, others a little beyond the close distance
	if (isLawEnforcer)
		return 50.0f;
	return (float)switchDistance + 5.0f;
}

uint8_t CarAI_FindPoliceCarMissionForWantedLevel(int wantedLevel, CarAIRandomFn random)
{
	// The more stars the more likely police ram instead of block
	switch (wantedLevel) {
	case 0:
	case 1: return MISSION_BLOCKPLAYER_FARAWAY;
	case 2: return (random() & 3) >= 3 ? MISSION_RAMPLAYER_FARAWAY : MISSION_BLOCKPLAYER_FARAWAY;
	case 3: return (random() & 3) >= 2 ? MISSION_RAMPLAYER_FARAWAY : MISSION_BLOCKPLAYER_FARAWAY;
	case 4:
	case 5:
	case 6: return (random() & 3) >= 1 ? MISSION_RAMPLAYER_FARAWAY : MISSION_BLOCKPLAYER_FARAWAY;
	default: return MISSION_BLOCKPLAYER_FARAWAY;
	}
}

uint8_t CarAI_FindPoliceBoatMissionForWantedLevel(int wantedLevel)
{
	// Boats block at low levels and attack above
	switch (wantedLevel) {
	case 0:
	case 1: return MISSION_BLOCKPLAYER_FARAWAY;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6: return MISSION_ATTACKPLAYER;
	default: return MISSION_BLOCKPLAYER_FARAWAY;
	}
}

int32_t CarAI_FindPoliceCarSpeedForWantedLevel(int wantedLevel, float maxVelocity, CarAIRandomRangeFn randomRange)
{
	// Fixed speeds at low levels, then a multiple of the top speed of the car
	switch (wantedLevel) {
	case 0: return randomRange(12, 16);
	case 1: return 25;
	case 2: return 34;
	case 3: return GAME_SPEED_TO_CARAI_SPEED * maxVelocity * 0.9f;
	case 4: return GAME_SPEED_TO_CARAI_SPEED * maxVelocity * 1.2f;
	case 5: return GAME_SPEED_TO_CARAI_SPEED * maxVelocity * 1.25f;
	case 6: return GAME_SPEED_TO_CARAI_SPEED * maxVelocity * 1.3f;
	default: return 0;
	}
}

int CarAI_MellowOutChaseSpeed(int wantedLevel, int cruiseSpeed, float distanceToPlayer, bool playerInVehicle, float playerSpeed)
{
	// Slow down when close to the player, with a different table for each of the first two levels
	if (wantedLevel == 1) {
		if (playerInVehicle) {
			if (distanceToPlayer < 10.0f)
				cruiseSpeed = 15;
			else if (distanceToPlayer < 20.0f)
				cruiseSpeed = 22;
			else
				cruiseSpeed = 25;
		} else {
			if (distanceToPlayer < 20.0f)
				cruiseSpeed = 5;
			else if (distanceToPlayer < 40.0f)
				cruiseSpeed = 13;
			else
				cruiseSpeed = 25;
		}
	} else if (wantedLevel == 2) {
		if (playerInVehicle) {
			if (distanceToPlayer < 10.0f)
				cruiseSpeed = 27;
			else if (distanceToPlayer < 20.0f)
				cruiseSpeed = 30;
			else
				cruiseSpeed = 34;
		} else {
			if (distanceToPlayer < 20.0f)
				cruiseSpeed = 5;
			else if (distanceToPlayer < 40.0f)
				cruiseSpeed = 18;
			else
				cruiseSpeed = 34;
		}
	}

	// A standing player on foot nearby is approached slowly
	if (!playerInVehicle && playerSpeed < 0.07f) {
		if (distanceToPlayer < 30.0f)
			cruiseSpeed = cruiseSpeed < 10 ? cruiseSpeed : 10;
	}
	return cruiseSpeed;
}

int CarAI_MellowOutChaseSpeedBoat(int wantedLevel, int cruiseSpeed)
{
	// One speed per level
	switch (wantedLevel) {
	case 0: return 8;
	case 1: return 10;
	case 2: return 15;
	case 3: return 20;
	case 4: return 25;
	case 5: return 30;
	case 6: return 40;
	}
	return cruiseSpeed;
}
//- rouz edit (Claude)
