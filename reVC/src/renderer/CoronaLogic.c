//+ rouz edit (Claude)
#include <math.h>
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
void CoronaLogic_SunReflectionSegment(int segment, uint32_t timeMs, float *forwardLength, float *sideLength)
{
	const float twoPi = (float)3.14159265358979323846 * 2;
	int timeInc = 900 * segment;
	int sideInc = 970 * segment;
	int fwdInc = 1440 * segment;

	*forwardLength = fwdInc / 20 + 60;
	*sideLength = sideInc / 20 + 30;
	*sideLength += 10.0f * sinf((float)((timeMs + timeInc) & 0x7FF) / 0x800 * twoPi);
}
void CoronaLogic_SunGlare(float camAngle, float sunGlare, float dist, int coreRed, int coreGreen, int coreBlue, float rgb[3], float *size)
{
	float intens = sqrtf(camAngle) * sunGlare;

	rgb[0] = intens * (coreRed + 2 * 255) / 3.0f;
	rgb[1] = intens * (coreGreen + 2 * 255) / 3.0f;
	rgb[2] = intens * (coreBlue + 2 * 255) / 3.0f;
	*size = 0.5f * sunGlare * sqrtf(dist);
}
int CoronaLogic_ReflectionAlpha(int fadeAlpha, float distanceFade)
{
	float fade = distanceFade < 0.0f ? 0.0f : distanceFade > 1.0f ? 1.0f : distanceFade;

	return (int)(fadeAlpha * fade);
}

void CoronaLogic_ReflectionFog(float foggyness, float depth, int red, int green, int blue, bool isStreak, float someAngle, int rgb[3], float *widthScale, float *heightScale)
{
	const float pi = (float)3.14159265358979323846;
	float limitedDepth = depth < 40.0f ? depth : 40.0f;
	float fogScale = foggyness * limitedDepth / 40.0f + 1.0f;
	int v[3];
	int k;

	v[0] = (int)(red / fogScale);
	v[1] = (int)(green / fogScale);
	v[2] = (int)(blue / fogScale);
	for (k = 0; k < 3; k++)
		rgb[k] = (v[k]) < (0) ? (0) : (v[k]) > (255) ? (255) : (v[k]);
	*widthScale = fogScale;
	*heightScale = fogScale;
	if (isStreak) {
		float angleFactor = 1.0f - someAngle * 2.0f / pi;
		float a2 = angleFactor * angleFactor;
		float a4 = a2 * a2;
		float a8 = a4 * a4;
		float streakWidthScale = 6.0f * a8 + 0.5f;
		float h = 0.35f - (streakWidthScale - 0.5f) * 0.06f;

		*widthScale = streakWidthScale;
		*heightScale *= h > 0.15f ? h : 0.15f;
	}
}
int CoronaLogic_WetReflection(float depth, float drawDist, float heightAboveRoad, float wetRoads, int red, int green, int blue, float size, int rgb[3], float *halfWidth, float *halfLength)
{
	float fadeDistance = drawDist * 0.5f;
	float distanceFade = depth < fadeDistance ? 1.0f : 1.0f - (depth - fadeDistance) / fadeDistance;
	float heightFade = (20.0f - heightAboveRoad) / 20.0f;
	float clamped = distanceFade < 0.0f ? 0.0f : distanceFade > 1.0f ? 1.0f : distanceFade;
	int intensity = (int)(clamped * heightFade * 230.0f * wetRoads);
	float w = size * 0.75f;
	float l = size * 2.0f;

	rgb[0] = (intensity * red) >> 8;
	rgb[1] = (intensity * green) >> 8;
	rgb[2] = (intensity * blue) >> 8;
	*halfWidth = w > 0.25f ? w : 0.25f;
	*halfLength = l > 0.5f ? l : 0.5f;
	return intensity;
}
void CoronaLogic_StreakScales(float someAngle, float *widthScale, float *heightScale)
{
	const float pi = (float)3.14159265358979323846;
	float f = 1.0f - someAngle * 2.0f / pi;
	float f2 = f * f;
	float f4 = f2 * f2;
	float h;

	*widthScale = 6.0f * (f4 * f4) + 0.5f;
	h = 0.35f - (*widthScale - 0.5f) * 0.06f;
	*heightScale = h > 0.15f ? h : 0.15f;
}
float CoronaLogic_LightsMultRise(float lightsMult, float timeStep)
{
	float v = lightsMult + 0.03f * timeStep;

	return v < 1.0f ? v : 1.0f;
}

float CoronaLogic_LightsMultDrop(float lightsMult, float timeStep)
{
	float v = lightsMult - timeStep * 0.06f;

	return v > 0.6f ? v : 0.6f;
}

bool CoronaLogic_StreakBroken(float prevX0, float prevX1, float prevY0, float prevY1)
{
	return fabsf(prevX0 - prevX1) > 50.0f || fabsf(prevY0 - prevY1) > 50.0f;
}

float CoronaLogic_SunReflectionIntensity(float sunZDir, float cloudCoverage, float foggyness, float wind)
{
	return (0.3f - fabsf(sunZDir - 0.25f)) / 0.3f *
		(1.0f - cloudCoverage) *
		(1.0f - foggyness) *
		(1.0f - wind);
}
float CoronaLogic_ReflectionDrawDist(float drawDist)
{
	float v = 0.75f * drawDist;

	return v < 55.0f ? v : 55.0f;
}
float CoronaLogic_FogScale(float foggyness, float depth)
{
	float d = depth < 40.0f ? depth : 40.0f;

	return foggyness * d / 40.0f + 1.0f;
}
//- rouz edit (Claude)
