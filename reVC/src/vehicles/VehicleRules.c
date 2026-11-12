//+ rouz edit (Claude)
#include "VehicleRules.h"
#include "ModelIds.h"
#include "WheelState.h"
#include "HandlingFlags.h"
#include "VehicleAppearance.h"

bool Vehicle_IsLawEnforcementModel(int modelIndex)
{
	switch (modelIndex) {
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_PREDATOR:
	case MI_RHINO:
	case MI_BARRACKS:
	case MI_FBIRANCH:
	case MI_VICECHEE:
		return true;
	default:
		return false;
	}
}

bool Vehicle_UsesSirenModel(int modelIndex)
{
	switch (modelIndex) {
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_FBICAR:
	case MI_MRWHOOP:
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_PREDATOR:
	case MI_FBIRANCH:
	case MI_VICECHEE:
		return true;
	default:
		return false;
	}
}

float Vehicle_ProcessWheelRotation(int wheelState, float forwardSpeed, float radius, float timeStep)
{
	float angularVelocity;

	// Spinning wheels turn at a constant rate, fixed ones do not turn
	switch (wheelState) {
	case WHEEL_STATE_SPINNING:
		angularVelocity = -1.1f;
		break;
	case WHEEL_STATE_FIXED:
		angularVelocity = 0.0f;
		break;
	default:
		angularVelocity = -forwardSpeed / radius;
		break;
	}
	return angularVelocity * timeStep;
}

int Vehicle_FindTyreNearestPoint(bool isBike, float fwd, float right)
{
	// Bikes have a front and a rear tyre only
	if (isBike)
		return fwd > 0.0f ? 0 : 2;
	if (fwd > 0.0f)
		return right > 0.0f ? 1 : 0;
	return right > 0.0f ? 3 : 2;
}
int Vehicle_UpdateClumpAlpha(int clumpAlpha, bool fadeOut)
{
	// Fade out by 8 down to nothing, otherwise fade in by 16 up to opaque
	if (fadeOut) {
		clumpAlpha -= 8;
		if (clumpAlpha < 0)
			clumpAlpha = 0;
	} else if (clumpAlpha < 255) {
		clumpAlpha += 16;
		if (clumpAlpha > 255)
			clumpAlpha = 255;
	}
	return clumpAlpha;
}

int Vehicle_GetVehicleAppearance(uint32_t handlingFlags)
{
	// One of the handling flags says what the vehicle is
	uint32_t flags = handlingFlags & 0xF0000;
	if (flags == 0)
		return VEHICLE_APPEARANCE_CAR;
	if (flags == HANDLING_IS_BIKE)
		return VEHICLE_APPEARANCE_BIKE;
	if (flags == HANDLING_IS_HELI)
		return VEHICLE_APPEARANCE_HELI;
	if (flags == HANDLING_IS_PLANE)
		return VEHICLE_APPEARANCE_PLANE;
	if (flags == HANDLING_IS_BOAT)
		return VEHICLE_APPEARANCE_BOAT;
	return VEHICLE_APPEARANCE_NONE;
}

void Vehicle_ChangeLawEnforcerState(bool *isLawEnforcer, int32_t *numLawEnforcerCars, bool enable)
{
	// Count only real changes of the flag
	if (enable) {
		if (!*isLawEnforcer) {
			*isLawEnforcer = true;
			(*numLawEnforcerCars)++;
		}
	} else {
		if (*isLawEnforcer) {
			*isLawEnforcer = false;
			(*numLawEnforcerCars)--;
		}
	}
}
//- rouz edit (Claude)
