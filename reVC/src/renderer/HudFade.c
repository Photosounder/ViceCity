//+ rouz edit (Claude)
#include "HudFade.h"

float Hud_DrawFadeStep(int32_t *fadeTimerPtr, uint32_t *operationPtr, uint32_t *timerPtr, int forceFadingIn, uint32_t stepMs)
{
	float alpha = 255.0f;
	int32_t fadeTimer = *fadeTimerPtr;
	uint32_t operation = *operationPtr;
	uint32_t timer = *timerPtr;

	// Forcing the element in restarts a faded out or fading out element
	if (forceFadingIn) {
		switch (operation) {
			case FADED_OUT:
				fadeTimer = 0;
			case START_FADE_OUT:
			case FADING_OUT:
				timer = 5;
				operation = FADING_IN;
				break;
			default:
				break;
		}
	}

	// Run the state machine unless the element is hidden or disabled
	if (operation != FADED_OUT && operation != FADE_DISABLED) {
		switch (operation) {
			case START_FADE_OUT:
				fadeTimer = 1000;
				alpha = 255.0f;
				if (timer > 10000) {
					fadeTimer = 3000;
					operation = FADING_OUT;
				}
				break;
			case FADING_IN:
				fadeTimer += stepMs;
				if (fadeTimer > 1000.0f) {
					operation = START_FADE_OUT;
					fadeTimer = 1000;
				}
				alpha = fadeTimer / 1000.0f * 255.0f;
				break;
			case FADING_OUT:
				fadeTimer -= stepMs;
				if (fadeTimer < 0.0f) {
					fadeTimer = 0;
					operation = FADED_OUT;
				}
				alpha = fadeTimer / 1000.0f * 255.0f;
				break;
			default:
				break;
		}
		timer += stepMs;
	}

	*fadeTimerPtr = fadeTimer;
	*operationPtr = operation;
	*timerPtr = timer;

	// Keep alpha in the byte range
	return alpha < 0.0f ? 0.0f : (alpha > 255.0f ? 255.0f : alpha);
}
//- rouz edit (Claude)
