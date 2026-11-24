//+ rouz edit (Claude)
#include "GlassLogic.h"
#include "General.h"
#include "config.h"

static float Dot(VectorMath3 a, VectorMath3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void Glass_StepFallingPane(bool carGlass, float timeStep, VectorMath3 *pos, VectorMath3 *moveSpeed, VectorMath3 *right, VectorMath3 *forward, VectorMath3 *up, VectorMath3 turn)
{
	VectorMath3 c;

	// Apply the move speed
	if (carGlass) {
		pos->x += moveSpeed->x * timeStep * 0.35f;
		pos->y += moveSpeed->y * timeStep * 0.35f;
		pos->z += moveSpeed->z * timeStep * 0.35f;
	} else {
		pos->x += moveSpeed->x * timeStep;
		pos->y += moveSpeed->y * timeStep;
		pos->z += moveSpeed->z * timeStep;
	}

	// Apply gravity
	if (carGlass)
		moveSpeed->z -= 0.01f * timeStep;
	else
		moveSpeed->z -= 0.02f * timeStep;

	// Apply the turn speed to each axis
	c = VectorMath_Cross(turn, *right);
	right->x += c.x; right->y += c.y; right->z += c.z;
	c = VectorMath_Cross(turn, *forward);
	forward->x += c.x; forward->y += c.y; forward->z += c.z;
	c = VectorMath_Cross(turn, *up);
	up->x += c.x; up->y += c.y; up->z += c.z;
}

int32_t Glass_CalcAlphaWithNormal(VectorMath3 normal, VectorMath3 cameraForward)
{
	// Mirror the forward vector around the normal and compare it to a fixed light direction
	float fwdDir = 2.0f * Dot(normal, cameraForward);
	VectorMath3 reflected = { cameraForward.x - fwdDir * normal.x, cameraForward.y - fwdDir * normal.y, cameraForward.z - fwdDir * normal.z };
	VectorMath3 light = { 0.57f, 0.57f, -0.57f };
	float fwdDot = Dot(reflected, light);
	float v = fwdDot * fwdDot * fwdDot * fwdDot * fwdDot * fwdDot;

	return (int32_t)(v * (255.0f - 20.0f) + 20.0f);
}
uint8_t Glass_HiLightColor(uint8_t alpha, uint32_t now, uint32_t startTime, bool carGlass)
{
	// Time since the start clamped to half a second
#ifdef FIX_BUGS
	uint32_t t = now > startTime ? now - startTime : 0u;
	uint16_t time = (uint16_t)(t < 500u ? t : 500u);
#else
	int32_t t = (int32_t)(now - startTime);
	uint16_t time = (uint16_t)(t < 0 ? 0 : (t > 500 ? 500 : t));
#endif

	uint8_t color = (uint8_t)(int32_t)((float)alpha * ((float)time / 500));

	if (carGlass && color < 64)
		color = 64;

	return color;
}

uint8_t Glass_ShatteredColor(float distToCamera)
{
	uint8_t shatteredColor = 140;
	if (distToCamera > 30.0f)
		shatteredColor = (uint8_t)(int32_t)((1.0f - (distToCamera - 30.0f) * 4.0f / 40.0f) * 140);

	return shatteredColor;
}

void Glass_ComputeSteps(float upLen, float rightLen, int32_t stepmul, bool explosion, uint32_t *ysteps, uint32_t *xsteps)
{
	// Windows are split in about one pane per unit with at least one
	float upSteps = upLen + 0.75f;
	if (upSteps < 1.0f) upSteps = 1.0f;

	float rightSteps = rightLen + 0.75f;
	if (rightSteps < 1.0f) rightSteps = 1.0f;

	*ysteps = (uint32_t)stepmul * (uint32_t)upSteps;
	if (*ysteps > 3) *ysteps = 3;

	*xsteps = (uint32_t)stepmul * (uint32_t)rightSteps;
	if (*xsteps > 3) *xsteps = 3;

	if (explosion) {
		if (*ysteps > 1) *ysteps = 1;
		if (*xsteps > 1) *xsteps = 1;
	}
}

void Glass_InitPane(VectorMath3 right, VectorMath3 up, float rightLen, float upLen, float rightScl, float upScl, float stepx, float stepy, float centerX, float centerY,
	VectorMath3 pos, VectorMath3 speed, VectorMath3 center, float moveSpeed,
	VectorMath3 *outRight, VectorMath3 *outUp, VectorMath3 *outForward, VectorMath3 *outPos, VectorMath3 *outMoveSpeed, VectorMath3 *outTurn)
{
	VectorMath3 fwd;

	// Axes of the pane scaled to its size
	outRight->x = (right.x * rightScl) / rightLen;
	outRight->y = (right.y * rightScl) / rightLen;
	outRight->z = (right.z * rightScl) / rightLen;
	outUp->x = (up.x * upScl) / upLen;
	outUp->y = (up.y * upScl) / upLen;
	outUp->z = (up.z * upScl) / upLen;

	fwd = VectorMath_Cross(*outRight, *outUp);
	VectorMath_Normalise(&fwd);
	*outForward = fwd;

	// Position of the pane in the window
	float rs = rightScl * centerX + stepx;
	float us = upScl * centerY + stepy;
	outPos->x = right.x / rightLen * rs + up.x / upLen * us + pos.x;
	outPos->y = right.y / rightLen * rs + up.y / upLen * us + pos.y;
	outPos->z = right.z / rightLen * rs + up.z / upLen * us + pos.z;

	// Random speed spreading the pane out
	outMoveSpeed->x = (float)((General_GetRandomNumber() & 127) - 64) * 0.0015f + speed.x;
	outMoveSpeed->y = (float)((General_GetRandomNumber() & 127) - 64) * 0.0015f + speed.y;
	outMoveSpeed->z = 0.0f + speed.z;

	// Panes of an explosion are pushed away from its centre
	if (moveSpeed != 0.0f) {
		VectorMath3 dist = { outPos->x - center.x, outPos->y - center.y, outPos->z - center.z };
		VectorMath_Normalise(&dist);

		outMoveSpeed->x += moveSpeed * dist.x;
		outMoveSpeed->y += moveSpeed * dist.y;
		outMoveSpeed->z += moveSpeed * dist.z;
	}

	// Random turn
	outTurn->x = (float)((General_GetRandomNumber() & 127) - 64) * 0.002f;
	outTurn->y = (float)((General_GetRandomNumber() & 127) - 64) * 0.002f;
	outTurn->z = (float)((General_GetRandomNumber() & 127) - 64) * 0.002f;
}

#define GMin(a,b) ((a) < (b) ? (a) : (b))
#define GMax(a,b) ((a) > (b) ? (a) : (b))

void Glass_WindowBounds(VectorMath3 a, VectorMath3 b, VectorMath3 c, VectorMath3 d, VectorMath3 *mins, VectorMath3 *maxs)
{
	mins->x = GMin(GMin(a.x, b.x), GMin(c.x, d.x));
	maxs->x = GMax(GMax(a.x, b.x), GMax(c.x, d.x));
	mins->y = GMin(GMin(a.y, b.y), GMin(c.y, d.y));
	maxs->y = GMax(GMax(a.y, b.y), GMax(c.y, d.y));
	mins->z = GMin(GMin(a.z, b.z), GMin(c.z, d.z));
	maxs->z = GMax(GMax(a.z, b.z), GMax(c.z, d.z));
}

VectorMath3 Glass_ExplosionPush(VectorMath3 distToGlass, float fDistToGlass)
{
	float s = 0.3f / fDistToGlass;
	distToGlass.x *= s;
	distToGlass.y *= s;
	distToGlass.z *= s;
	return distToGlass;
}

void Glass_MinMax4(float a, float b, float c, float d, float *minimum, float *maximum)
{
	float lo1 = a < b ? a : b;
	float lo2 = c < d ? c : d;
	float hi1 = a > b ? a : b;
	float hi2 = c > d ? c : d;

	*minimum = lo1 < lo2 ? lo1 : lo2;
	*maximum = hi1 > hi2 ? hi1 : hi2;
}
//- rouz edit (Claude)
