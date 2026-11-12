//+ rouz edit (Claude)
#include "CoronaLogic.h"

bool CoronaLogic_UpdateFade(uint8_t *alpha, uint8_t *fadeAlpha, bool registeredThisFrame, bool blocked, bool offScreen, bool changeBrightnessImmediately, float timeStep)
{
	float v;

	// A corona that was not registered again this frame fades to nothing
	if (!registeredThisFrame)
		*alpha = 0;

	if (blocked || offScreen) {
		// Blocked or off screen, fade out
		v = *fadeAlpha - 15.0f*timeStep;
		*fadeAlpha = v > 0.0f ? v : 0.0f;
		return false;
	}

	// Visible
	if (*alpha > *fadeAlpha) {
		// Fade in
		v = *fadeAlpha + 15.0f*timeStep;
		*fadeAlpha = v < *alpha ? v : *alpha;
		if (changeBrightnessImmediately)
			*fadeAlpha = *alpha;
	} else if (*alpha < *fadeAlpha) {
		// Too visible, decrease alpha but not below the target
		v = *fadeAlpha - 15.0f*timeStep;
		*fadeAlpha = v > *alpha ? v : *alpha;
	}
	return true;
}
//- rouz edit (Claude)
