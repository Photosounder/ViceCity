//+ rouz edit (Claude)
#pragma once
#include <stdint.h>

enum eFadeOperation
{
	FADED_OUT = 0,
	START_FADE_OUT,
	FADING_IN,
	FADING_OUT,
	FADE_DISABLED = 5,
};

#ifdef __cplusplus
extern "C" {
#endif
// Advance a fading HUD element by one frame and return its alpha, stepMs is the frame time in milliseconds
float Hud_DrawFadeStep(int32_t *fadeTimer, uint32_t *operation, uint32_t *timer, int forceFadingIn, uint32_t stepMs);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
