//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "VectorMath.h"

#ifdef __cplusplus
extern "C" {
#endif
// Moves a falling glass pane by one time step, car glass falls slower, the turn is applied to the three axes of the pane
void Glass_StepFallingPane(bool carGlass, float timeStep, VectorMath3 *pos, VectorMath3 *moveSpeed, VectorMath3 *right, VectorMath3 *forward, VectorMath3 *up, VectorMath3 turn);
// Brightness of a glass pane from its normal and the forward vector of the camera rendering it, from 20 to 255
int32_t Glass_CalcAlphaWithNormal(VectorMath3 normal, VectorMath3 cameraForward);
// Grey level of the highlight of a falling pane, it fades in over half a second from its start time, car glass is never darker than 64
uint8_t Glass_HiLightColor(uint8_t alpha, uint32_t now, uint32_t startTime, bool carGlass);
// Grey level of the cracks of a shattered pane, full up to 30 units from the camera and fading out after that
uint8_t Glass_ShatteredColor(float distToCamera);
// Number of panes a window of the given size breaks into along its up and right axes, at most 3 each and 1 each for an explosion
void Glass_ComputeSteps(float upLen, float rightLen, int32_t stepmul, bool explosion, uint32_t *ysteps, uint32_t *xsteps);
// Axes, position, speed and turn of one falling pane that is a triangle of a window, with the pseudo random speeds, centerX and centerY are the centre of the triangle
void Glass_InitPane(VectorMath3 right, VectorMath3 up, float rightLen, float upLen, float rightScl, float upScl, float stepx, float stepy, float centerX, float centerY,
	VectorMath3 pos, VectorMath3 speed, VectorMath3 center, float moveSpeed,
	VectorMath3 *outRight, VectorMath3 *outUp, VectorMath3 *outForward, VectorMath3 *outPos, VectorMath3 *outMoveSpeed, VectorMath3 *outTurn);
// Smallest and largest coordinates of the four corners of a window
void Glass_WindowBounds(VectorMath3 a, VectorMath3 b, VectorMath3 c, VectorMath3 d, VectorMath3 *mins, VectorMath3 *maxs);
// Push away from an explosion given to a window at the distance dist, the vector is scaled to a length of 0.3
VectorMath3 Glass_ExplosionPush(VectorMath3 distToGlass, float fDistToGlass);
// Smallest and largest of four values
void Glass_MinMax4(float a, float b, float c, float d, float *minimum, float *maximum);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
