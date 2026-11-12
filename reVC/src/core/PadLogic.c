//+ rouz edit (Claude)
#include <math.h>
#include "PadLogic.h"

bool PadLogic_AnalogueEdge(int16_t stick, int16_t *old, bool negative)
{
	bool pressed;

	// Pushed past 15 after being back under 5 in the same direction
	if (negative)
		pressed = stick < -15 && *old >= -5;
	else
		pressed = stick > 15 && *old <= 5;
	*old = stick;
	return pressed;
}

int16_t PadLogic_SniperAxis(int16_t axis, int16_t dpad)
{
	// The stick wins when it is pushed further than the pad, with a dead zone of 35
	if (fabsf(axis) > fabsf(dpad)) {
		if (fabsf(axis) > 35.0f)
			return (axis > 0.f ? axis - 35.f : axis + 35.f) * (128.f / (128 - 35));
		else
			return 0;
	}
	return dpad;
}

int16_t PadLogic_LookAroundAxis(float axis, bool lookBehind, bool mouseCam, float mouseDeadZone)
{
	// Past 85 the stick looks around unless the player looks behind
	if (fabsf(axis) > 85 && !lookBehind)
		return (int16_t) ((axis + ((axis > 0) ? -85 : 85)) * (127.0f / 32.0f));

	// The mouse camera has its own dead zone
	if (mouseCam && fabsf(axis) > mouseDeadZone)
		return (int16_t) ((axis + ((axis > 0) ? -mouseDeadZone : mouseDeadZone)) * (127.0f / 64.0f));

	return 0;
}
//- rouz edit (Claude)
