//+ rouz edit (Claude)
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include "ParticleLogic.h"
#include <math.h>
#include "config.h"
#include "General.h"
#include "Draw_constants.h"

bool Particle_WaterDropStep(uint8_t *count, VectorMath3 *step, float cameraAverageSpeed, float posZ)
{
	bool extend = false;

	if (*count == 5) {
		// Every sixth frame the drop stops and lives longer
		step->x = 0.0f;
		step->y = 0.0f;
		step->z = 0.0f;
		extend = true;
		*count = 0;
	} else {
		// A fast moving camera drags the drops along
		if (cameraAverageSpeed > 0.35f) {
			float mag = sqrtf(step->x * step->x + step->y * step->y + step->z * step->z);
			if (mag > 0.5f) {
				if (mag > 0.4f && mag < 0.8f) {
					step->x += cameraAverageSpeed * 1.5f;
					step->y += cameraAverageSpeed * 1.5f;
				} else if (mag != 0.0f) {
					step->x += General_GetRandomNumberInRangeFloat(0.01f, 0.05f);
					step->y += General_GetRandomNumberInRangeFloat(0.01f, 0.05f);
				}
			}
		}

		(*count)++;
	}

	// Drops do not fall below the sea
	if (posZ <= 1.5f)
		step->z = 0.0f;

	return extend;
}

int32_t Particle_SinCosIndex(int16_t rotationDegrees, int32_t tableSize)
{
#ifdef FIX_BUGS
	return ((int32_t)(DEGTORAD((float)rotationDegrees) * (float)tableSize / TWOPI) + tableSize) % tableSize;
#else
	return (int32_t)(DEGTORAD((float)rotationDegrees) * (float)tableSize / TWOPI) % tableSize;
#endif
}

void Particle_ShipFade(uint32_t now, uint32_t timeWhenWillBeDestroyed, uint8_t *alpha, int16_t *fadeAlphaTimer)
{
	if (now > timeWhenWillBeDestroyed - 32000 && now < timeWhenWillBeDestroyed - 22000) {
		int a = *alpha + 1;
		*alpha = (uint8_t)(a < 96 ? a : 96);
	}
	if (now > timeWhenWillBeDestroyed - 10000)
		*fadeAlphaTimer = 1;
}

static int32_t ClampChannel(int32_t v)
{
	return v < 0 ? 0 : v > 255 ? 255 : v;
}

void Particle_UpdateColor(uint32_t now, uint32_t timeWhenColorWillBeChanged, uint32_t colorFadeTime, const uint8_t render[4], const uint8_t dest[4], uint8_t color[4])
{
	int i;

	if (timeWhenColorWillBeChanged == 0)
		return;

	if (timeWhenColorWillBeChanged > now) {
		// Fade between the two colours, alpha is kept
		float colorMul = 1.0f - (float)(timeWhenColorWillBeChanged - now) / (float)colorFadeTime;

		for (i = 0; i < 3; i++)
			color[i] = (uint8_t)ClampChannel(render[i] + (int32_t)((float)(dest[i] - render[i]) * colorMul));
	} else {
		// Past the end of the fade the destination colour is used
		for (i = 0; i < 4; i++)
			color[i] = dest[i];
	}
}

bool Particle_NextSize(float size, float expansionRate, float windMag, float moveMag, bool fumes, bool splash, bool waterDrop, float moveX, float moveZ, float *newSize, bool *dropVelZ)
{
	*dropVelZ = false;

	if (expansionRate > 0.0f) {
		float speed = windMag > moveMag ? windMag : moveMag;

		if (fumes)
			speed *= 2.0f;

		if (splash && size > 1.2f) {
			size = size - (1.0f + speed) * expansionRate;
			*dropVelZ = true;
		} else
			size = size + (1.0f + speed) * expansionRate;
	} else
		size = size + expansionRate;

	if (waterDrop)
		size = (size - fabsf(moveX) * 0.000150000007f) + (fabsf(moveZ) * 0.0500000007f);

	*newSize = size;
	return !(size < 0.0f);
}

uint8_t Particle_FadeToBlack(uint8_t intensity, int16_t amount)
{
	return (uint8_t)ClampChannel((int32_t)intensity - amount);
}

bool Particle_FadeAlpha(uint8_t *alpha, int16_t amount)
{
	*alpha = (uint8_t)ClampChannel((int32_t)*alpha - amount);
	return *alpha != 0;
}

void Particle_StepZRotation(uint16_t *timer, uint16_t changeTime, int16_t *angle, int16_t amount)
{
	if (*timer >= changeTime) {
		*timer = 0;
		*angle += amount;
	} else
		++*timer;
}

void Particle_StepZRadius(uint16_t *timer, uint16_t changeTime, float *radius, float amount)
{
	if (*timer >= changeTime) {
		*timer = 0;
		*radius += amount;
	} else
		++*timer;
}

void Particle_StepAnimation(int16_t *timer, uint16_t speed, uint8_t *frame, uint8_t startFrame, uint8_t finalFrame, bool cycle)
{
	if (*timer > speed) {
		*timer = 0;

		if (++*frame > finalFrame) {
			if (cycle)
				*frame = startFrame;
			else
				--*frame;
		}
	} else
		++*timer;
}

float Particle_ScreenZ(float posZ, float nearClip, float farClip, float nearScreenZ, float farScreenZ)
{
	return (posZ - nearClip)
		* (farScreenZ - nearScreenZ)
		* farClip
		/ ((farClip - nearClip) * posZ)
		+ nearScreenZ;
}

//- rouz edit (Claude)
