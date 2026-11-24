//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Fade a corona towards its target alpha, blocked is the result of the line of sight and cloud test, returns true when the corona is visible and not fading out because of that
bool CoronaLogic_UpdateFade(uint8_t *alpha, uint8_t *fadeAlpha, bool registeredThisFrame, bool blocked, bool offScreen, bool changeBrightnessImmediately, float timeStep);
// Length along the sun direction and width of the segment of the reflection of the sun on the water, from the number of the segment and the time in milliseconds: the width wobbles with the time
void CoronaLogic_SunReflectionSegment(int segment, uint32_t timeMs, float *forwardLength, float *sideLength);
// Colour (rgb) and size of the corona of the sun glare on a vehicle, from the angle between the view and the sun, the strength of the glare of the weather, the distance to the camera and the red, green and blue of the core of the sun
void CoronaLogic_SunGlare(float camAngle, float sunGlare, float dist, int coreRed, int coreGreen, int coreBlue, float rgb[3], float *size);
// Alpha of a reflected corona, from its faded alpha and its distance fade (clamped to 0..1)
int CoronaLogic_ReflectionAlpha(int fadeAlpha, float distanceFade);
// Colour (rgb, 0..255) and size scales of a reflected corona tinted for the fog of the weather, from the fogginess, the depth of the corona, its colour, whether it is a streak and its angle (the streak shapes the width and height)
void CoronaLogic_ReflectionFog(float foggyness, float depth, int red, int green, int blue, bool isStreak, float someAngle, int rgb[3], float *widthScale, float *heightScale);
// Reflection of a corona on a wet road: returns its intensity (0 when invisible) from the depth, the draw distance, the height above the road and the wetness of the roads, with the faded colour (rgb) and the half width and length of the ground quad from the size of the corona
int CoronaLogic_WetReflection(float depth, float drawDist, float heightAboveRoad, float wetRoads, int red, int green, int blue, float size, int rgb[3], float *halfWidth, float *halfLength);
// Width and height scales of a streak corona from its angle
void CoronaLogic_StreakScales(float someAngle, float *widthScale, float *heightScale);
// Brightness multiplier of the lights after a frame in which the sun is not visible: it rises back to 1
float CoronaLogic_LightsMultRise(float lightsMult, float timeStep);
// Brightness multiplier of the lights after a frame in which the sun is visible: it drops to at least 0.6
float CoronaLogic_LightsMultDrop(float lightsMult, float timeStep);
// Whether the movement of a corona between two frames is too large to keep its streak
bool CoronaLogic_StreakBroken(float prevX0, float prevX1, float prevY0, float prevY1);
// Intensity of the sun reflection on the ground from the vertical direction of the sun and the weather (clouds, fog and wind)
float CoronaLogic_SunReflectionIntensity(float sunZDir, float cloudCoverage, float foggyness, float wind);
// Distance a corona or its reflection is drawn up to: three quarters of the draw distance of the corona, at most 55
float CoronaLogic_ReflectionDrawDist(float drawDist);
// Scale of a corona in the fog: grows with the fog up to a depth of 40
float CoronaLogic_FogScale(float foggyness, float depth);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
