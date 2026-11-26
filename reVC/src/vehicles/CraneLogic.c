//+ rouz edit (Claude)
#include <math.h>
#include "CraneLogic.h"
#include "config.h"
#include "ModelIds.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"

#define HOOK_ANGLE_MOVEMENT_SPEED (0.004f)
#define HOOK_OFFSET_MOVEMENT_SPEED (0.1f)
#define HOOK_HEIGHT_MOVEMENT_SPEED (0.06f)

// Move a value towards a target by at most step, returns true when it arrived
static bool Approach(float *value, float target, float step)
{
	float delta = target - *value;
	if (fabsf(delta) < step) {
		*value = target;
		return true;
	}
	if (delta < 0.0f)
		*value -= step;
	else
		*value += step;
	return false;
}

bool Crane_GoTowardsTarget(CCraneData *self, float angleToTarget, float distanceToTarget, float targetHeight, float speedMultiplier, float timeStep)
{
	bool angleDone, offsetDone, heightDone;
	float angleDelta = angleToTarget - self->m_fHookAngle;
	float angleStep;

	// Turn the hook the short way round
	while (angleDelta > PI)
		angleDelta -= TWOPI;
	while (angleDelta < -PI)
		angleDelta += TWOPI;
	angleStep = speedMultiplier * timeStep * HOOK_ANGLE_MOVEMENT_SPEED;
	if (fabsf(angleDelta) < angleStep) {
		self->m_fHookAngle = angleToTarget;
		angleDone = true;
	} else {
		if (angleDelta < 0.0f) {
			self->m_fHookAngle -= angleStep;
			if (self->m_fHookAngle < 0.0f)
				self->m_fHookAngle += TWOPI;
		} else {
			self->m_fHookAngle += angleStep;
			if (self->m_fHookAngle > TWOPI)
				self->m_fHookAngle -= TWOPI;
		}
		angleDone = false;
	}

	// Slide the hook in or out and up or down
	offsetDone = Approach(&self->m_fHookOffset, distanceToTarget, speedMultiplier * timeStep * HOOK_OFFSET_MOVEMENT_SPEED);
	heightDone = Approach(&self->m_fHookHeight, targetHeight, speedMultiplier * timeStep * HOOK_HEIGHT_MOVEMENT_SPEED);
	return angleDone && offsetDone && heightDone;
}

bool Crane_GoTowardsHeightTarget(CCraneData *self, float targetHeight, float speedMultiplier, float timeStep)
{
	// Only the height changes
	return Approach(&self->m_fHookHeight, targetHeight, speedMultiplier * timeStep * HOOK_HEIGHT_MOVEMENT_SPEED);
}

bool Crane_DoesPickUpThisCarType(const CCraneData *self, uint32_t mi)
{
	// The crusher takes everything but a few big vehicles
	if (self->m_bIsCrusher) {
		return mi != MI_FIRETRUCK &&
			mi != MI_TRASH &&
#ifdef FIX_BUGS
			mi != MI_COACH &&
#endif
			mi != MI_SECURICA &&
			mi != MI_BUS &&
			mi != MI_DODO &&
			mi != MI_RHINO;
	}

	// The military crane takes only seven models
	if (self->m_bIsMilitaryCrane) {
		return mi == MI_FIRETRUCK ||
			mi == MI_AMBULAN ||
			mi == MI_ENFORCER ||
			mi == MI_FBIRANCH ||
			mi == MI_RHINO ||
			mi == MI_BARRACKS ||
			mi == MI_POLICE;
	}
	return true;
}

// Bit of a model in the military crane mask, zero for other models
static uint32_t MilitaryBit(uint32_t mi)
{
	switch (mi) {
	case MI_FIRETRUCK: return 1;
	case MI_AMBULAN: return 2;
	case MI_ENFORCER: return 4;
	case MI_FBIRANCH: return 8;
	case MI_RHINO: return 0x10;
	case MI_BARRACKS: return 0x20;
	case MI_POLICE: return 0x40;
	default: return 0;
	}
}

bool CraneLogic_DoesMilitaryCraneHaveThisOneAlready(uint32_t collected, uint32_t mi)
{
	// Other models are never collected
	return (collected & MilitaryBit(mi)) != 0;
}

void CraneLogic_RegisterCarForMilitaryCrane(uint32_t *collected, uint32_t mi)
{
	// Set the bit of the model
	*collected |= MilitaryBit(mi);
}

bool CraneLogic_HaveAllCarsBeenCollectedByMilitaryCrane(uint32_t collected)
{
	// All seven bits
	return (collected & 0x7F) == 0x7F;
}
void Crane_HookOffset(float hookAngle, float hookOffset, float *dx, float *dy)
{
	*dx = cosf(hookAngle) * hookOffset;
	*dy = sinf(hookAngle) * hookOffset;
}
bool Crane_RotationStep(float angleDelta, float rotationSpeed, float timeStep, float *rotation)
{
	float deltaThisFrame = rotationSpeed * timeStep;

	if (fabsf(angleDelta) <= deltaThisFrame)
		return true;
	*rotation = angleDelta < 0 ? -deltaThisFrame : deltaThisFrame;
	return false;
}
//- rouz edit (Claude)
