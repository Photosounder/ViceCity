//+ rouz edit (Claude)
#include <math.h>
#include "CameraMath.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"
#include "CameraModes.h"
#include "Camera_effects.h"
#include "VehicleAppearance.h"
#include "Draw.h"

bool CameraMath_IsPointVisible(const VectorMath3 normals[4], VectorMath3 center, const float mat[4][4])
{
	// Move the point into camera space then test it against the clip distances and the four planes
	VectorMath3 c = VectorMath_MatrixTimesVector(mat, center);
	if (c.y < Draw_GetNearClipZ()) return false;
	if (c.y > Draw_GetFarClipZ()) return false;
	if (c.x * normals[0].x + c.y * normals[0].y > 0.0f) return false;
	if (c.x * normals[1].x + c.y * normals[1].y > 0.0f) return false;
	if (c.y * normals[2].y + c.z * normals[2].z > 0.0f) return false;
	if (c.y * normals[3].y + c.z * normals[3].z > 0.0f) return false;
	return true;
}

bool CameraMath_IsSphereVisible(const VectorMath3 normals[4], VectorMath3 center, float radius, const float mat[4][4])
{
	// Same as the point test with the sphere radius as margin
	VectorMath3 c = VectorMath_MatrixTimesVector(mat, center);
	if (c.y + radius < Draw_GetNearClipZ()) return false;
	if (c.y - radius > Draw_GetFarClipZ()) return false;
	if (c.x * normals[0].x + c.y * normals[0].y > radius) return false;
	if (c.x * normals[1].x + c.y * normals[1].y > radius) return false;
	if (c.y * normals[2].y + c.z * normals[2].z > radius) return false;
	if (c.y * normals[3].y + c.z * normals[3].z > radius) return false;
	return true;
}

bool CameraMath_IsBoxVisible(const VectorMath3 normals[4], VectorMath3 box[8], const float mat[4][4])
{
	int i;
	int frustumTests[6] = { 0 };
	for (i = 0; i < 8; i++)
		box[i] = VectorMath_MatrixTimesVector(mat, box[i]);

	for (i = 0; i < 8; i++) {
		if (box[i].y < Draw_GetNearClipZ()) frustumTests[0]++;
		if (box[i].y > Draw_GetFarClipZ()) frustumTests[1]++;
		if (box[i].x * normals[0].x + box[i].y * normals[0].y > 0.0f) frustumTests[2]++;
		if (box[i].x * normals[1].x + box[i].y * normals[1].y > 0.0f) frustumTests[3]++;
	}
	for (i = 0; i < 6; i++)
		if (frustumTests[i] == 8)
			return false;		// Box is completely outside of one plane
	return true;
}
void CameraMath_GetVectorsReadyForRW(VectorMath3 *front, VectorMath3 *up)
{
	VectorMath3 right;

	// Start from the world up vector
	up->x = 0.0f;
	up->y = 0.0f;
	up->z = 1.0f;
	VectorMath_Normalise(front);

	// Avoid the degenerate case of a vertical front vector
	if (front->x == 0.0f && front->y == 0.0f) {
		front->x = 0.0001f;
		front->y = 0.0001f;
	}

	// Right-handed basis from front
	right = VectorMath_Cross(*front, *up);
	VectorMath_Normalise(&right);
	*up = VectorMath_Cross(right, *front);
}
void CameraMath_GetArrPosForVehicleType(int appearance, int *index)
{
	// One slot per kind of vehicle
	switch (appearance) {
	case VEHICLE_APPEARANCE_CAR: *index = 0; break;
	case VEHICLE_APPEARANCE_BIKE: *index = 1; break;
	case VEHICLE_APPEARANCE_HELI: *index = 2; break;
	case VEHICLE_APPEARANCE_PLANE: *index = 3; break;
	case VEHICLE_APPEARANCE_BOAT: *index = 4; break;
	}
}

int CameraMath_GetScreenFadeStatus(float fade)
{
	// Fully in, fully out or on the way
	if (fade == 0.0f)
		return FADE_0;
	if (fade == 255.0f)
		return FADE_2;
	return FADE_1;
}

int CameraMath_GetFadingDirection(bool fading, int direction)
{
	// Only fade in and fade out are reported
	if (fading)
		return direction == FADE_IN ? FADE_IN : FADE_OUT;
	return FADE_NONE;
}

float CameraMath_Find3rdPersonQuickAimPitch(float frontZ, float crosshairMultY, float fov)
{
	float clampedFrontZ = frontZ < -1.0f ? -1.0f : (frontZ > 1.0f ? 1.0f : frontZ);
	float rot = asinf(clampedFrontZ);

	return -(DEGTORAD(((0.5f - crosshairMultY) * 1.8f * 0.5f * fov)) + rot);
}

bool CameraMath_Using1stPersonWeaponMode(int mode)
{
	switch (mode) {
	case CAMERA_MODE_SNIPER:
	case CAMERA_MODE_M16_1STPERSON:
	case CAMERA_MODE_ROCKETLAUNCHER:
	case CAMERA_MODE_HELICANNON_1STPERSON:
	case CAMERA_MODE_CAMERA:
		return true;
	}
	return false;
}
VectorMath3 CameraMath_DoAverageOnVector(VectorMath3 prev[2], int *pos, int *counter, bool reset, VectorMath3 vec)
{
	int i;
	VectorMath3 average = { 0.0f, 0.0f, 0.0f };

	if (reset) {
		*pos = 0;
		*counter = 1;
	}

	// Only two vectors are kept, the oldest is dropped once the buffer is full
	if (*counter == 3) {
		prev[0] = prev[1];
		prev[1] = vec;
	} else
		prev[*pos] = vec;

	for (i = 0; i <= *pos; i++) {
		average.x += prev[i].x;
		average.y += prev[i].y;
		average.z += prev[i].z;
	}
	average.x /= i;
	average.y /= i;
	average.z /= i;

	(*pos)++;
	(*counter)++;
	if (*pos >= 2)
		*pos = 2 - 1;
	if (*counter > 2 + 1)
		*counter = 2 + 1;

	return average;
}

bool CameraMath_IsTargetUnderWater(bool drowning, bool inWater, float z, float waterZ)
{
	return drowning || (inWater && z < waterZ);
}

void CameraMath_MakeAngleLessThan180(float *angle)
{
	while (*angle >= PI) *angle -= 2 * PI;
	while (*angle < -PI) *angle += 2 * PI;
}

void CameraMath_KeepTrackOfTheSpeed(CamSpeedState *st, bool justSwitched, VectorMath3 source, VectorMath3 target, VectorMath3 up, float alpha, float beta, float fov,
	VectorMath3 *sourceSpeed, VectorMath3 *targetSpeed, VectorMath3 *upSpeed, float *fovSpeed, float *betaSpeed, float *alphaSpeed)
{
	// After a switch the vectors are not compared to those of the previous camera
	if (justSwitched) {
		st->source = source;
		st->target = target;
		st->up = up;
	}

	// Differences to the previous frame, the angles are wrapped
	sourceSpeed->x = source.x - st->source.x;
	sourceSpeed->y = source.y - st->source.y;
	sourceSpeed->z = source.z - st->source.z;
	targetSpeed->x = target.x - st->target.x;
	targetSpeed->y = target.y - st->target.y;
	targetSpeed->z = target.z - st->target.z;
	upSpeed->x = up.x - st->up.x;
	upSpeed->y = up.y - st->up.y;
	upSpeed->z = up.z - st->up.z;
	*fovSpeed = fov - st->fov;
	*betaSpeed = beta - st->beta;
	CameraMath_MakeAngleLessThan180(betaSpeed);
	*alphaSpeed = alpha - st->alpha;
	CameraMath_MakeAngleLessThan180(alphaSpeed);

	// Remember this frame
	st->source = source;
	st->target = target;
	st->up = up;
	st->beta = beta;
	st->alpha = alpha;
	st->fov = fov;
}

bool CameraMath_NearClipForPedInFront(float fwdAngle, float pedAngle, float distX, float distY, float defaultNear, float *nearClip)
{
	float deltaAngle = fwdAngle - pedAngle;
	while (deltaAngle >= PI) deltaAngle -= 2 * PI;
	while (deltaAngle < -PI) deltaAngle += 2 * PI;

	// Only a ped within a quarter turn of the heading and closer than 1.25 moves the plane in
	if (fabsf(deltaAngle) < HALFPI) {
		float fDist = sqrtf(distX * distX + distY * distY);
		if (fDist < 1.25f) {
			float nearVal = defaultNear - (1.25f - fDist);
			if (nearVal < 0.05f)
				nearVal = 0.05f;
			*nearClip = nearVal;
			return true;
		}
	}

	return false;
}

void CameraMath_AvoidWallsHit(float dy, float yDistLimit, float *target, float *maxSpeed, float *acceleration)
{
	if (dy > yDistLimit)
		dy = yDistLimit;
	float speedMult = yDistLimit - fabsf(dy / yDistLimit);

	// Move away from the wall faster the closer it is
	*target = 2.5f;
	*maxSpeed += speedMult * 0.3f;
	*acceleration += speedMult * 0.03f;
}

int CameraMath_LightHouseMode(float *timer, float targetZ, float dist2D, float timeStep)
{
	int camMode;

	if (targetZ > 57.0f && dist2D > 3.2f) {
		// Outside at top
		if (*timer > 0.0f) {
			*timer -= timeStep;
			camMode = 1;
		} else {
			*timer = -24.0f;
			camMode = 2;
		}
	} else if (targetZ > 57.0f) {
		// Inside at top
		if (*timer < 0.0f) {
			*timer += timeStep;
			camMode = 2;
		} else {
			*timer = 24.0f;
			camMode = 1;
		}
	} else {
		*timer = 0.0f;
		camMode = 0;
	}

	return camMode;
}

int CameraMath_AdvanceSplineMarker(const float *pathData, int marker, uint32_t time, int step)
{
	while (time >= (pathData[marker] - pathData[1]) * 1000.0f)
		marker += step;

	return marker;
}

//- rouz edit (Claude)
