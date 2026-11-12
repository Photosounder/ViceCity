//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Score of a candidate for the neighbouring target selection, the higher the better, angle is the limited angle between the candidate and the camera direction
float PlayerPedLogic_NeighbourCloseness(float angleBetweenUs, bool lookToLeft);
// Score of a candidate for the target selection from its flat distance and angle, priority targets get a bonus
float PlayerPedLogic_TargetCloseness(float dist, float angleBetweenUs, bool priority);
// A target further than the weapon range has to be dropped
bool PlayerPedLogic_DoesTargetHaveToBeBroken(float distance, float weaponRange);
// Sprinting uses up stamina unless the player has infinite sprint or adrenaline, and long sprints raise the maximum
void PlayerPedLogic_UseSprintEnergy(float *currentStamina, float *staminaProgress, float *maxStamina, bool infiniteSprint, bool adrenalineActive, float timeStep);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
