//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Fade a corona towards its target alpha, blocked is the result of the line of sight and cloud test, returns true when the corona is visible and not fading out because of that
bool CoronaLogic_UpdateFade(uint8_t *alpha, uint8_t *fadeAlpha, bool registeredThisFrame, bool blocked, bool offScreen, bool changeBrightnessImmediately, float timeStep);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
