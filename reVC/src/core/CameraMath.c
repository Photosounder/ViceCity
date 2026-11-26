//+ rouz edit (Claude)
#include <math.h>
#include "CameraMath.h"
#include "Vector.h" // rouz edit (Claude)
#include "VectorMath.h"
#include "General.h"
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

float Camera_SmoothInterpolation(float numerator, float denominator)
{
	float inter;

	if (denominator == 0.0f)
		inter = 0.0f;
	else
		inter = numerator / denominator;
	// Smooth it
	return 0.5f - 0.5 * cosf(inter * PI);
}
void Camera_ObbeOffsetDirection(float speedX, float speedY, float speedZ, float angleOffset, bool flatten, CVector *dir)
{
	VectorMath3 v;
	float angle;

	v.x = speedX;
	v.y = speedY;
	v.z = speedZ;
	if (flatten) {
		v.z = 0.0f;
		VectorMath_Normalise(&v);
	}
	angle = General_GetATanOfXY(v.x, v.y) + angleOffset;
	v.x += cosf(angle);
	v.y += sinf(angle);
	v.z += 0.0f;
	VectorMath_Normalise(&v);
	dir->x = v.x;
	dir->y = v.y;
	dir->z = v.z;
}
float Camera_ShakeDistanceMultiplier(float dx, float dy, float dz)
{
	float dist2d = sqrtf(dx * dx + dy * dy);
	float dist3d = sqrtf(dist2d * dist2d + dz * dz);

	if (dist3d > 100.0f)
		dist3d = 100.0f;
	if (dist3d < 0.0f)
		dist3d = 0.0f;
	return 1.0f - dist3d / 100.0f;
}

bool Camera_ShakeReplaces(float mult, float shakeForce, uint32_t elapsed, float strength)
{
	float curForce = mult * (shakeForce - elapsed / 1000.0f);

	curForce = curForce < 0.0f ? 0.0f : curForce > 2.0f ? 2.0f : curForce;
	return curForce < strength;
}
void Camera_FrustumNormals(float halfFov, float aspectRatio, float normals[4][3])
{
	float c = cosf(halfFov);
	float s = sinf(halfFov);

	// Right plane
	normals[0][0] = c;
	normals[0][1] = -s;
	normals[0][2] = 0.0f;
	// Left plane
	normals[1][0] = -c;
	normals[1][1] = -s;
	normals[1][2] = 0.0f;

	c /= aspectRatio;
	s /= aspectRatio;
	// Bottom plane
	normals[2][0] = 0.0f;
	normals[2][1] = -s;
	normals[2][2] = -c;
	// Top plane
	normals[3][0] = 0.0f;
	normals[3][1] = -s;
	normals[3][2] = c;
}

void Camera_FrontNormalised(float frontX, float frontY, float *normX, float *normY)
{
	float l = sqrtf(frontX * frontX + frontY * frontY);

	*normX = frontX;
	*normY = frontY;
	if (l == 0.0f)
		*normX = 1.0f;
	else {
		*normX /= l;
		*normY /= l;
	}
}
void Camera_AvoidFront(float distX, float distY, float distZ, float forwardX, float forwardY, CVector *front, float *dist)
{
	float beta = 0.0f;
	float alpha = 0.0f;
	float distOnGround;

	*dist = sqrtf(distX * distX + distY * distY + distZ * distZ);
	distOnGround = sqrtf(distX * distX + distY * distY);
	if (distX == 0.0f && distY == 0.0f)
		beta = General_GetATanOfXY(forwardX, forwardY);
	else
		beta = General_GetATanOfXY(distX, distY);
	if (distOnGround != 0.0f || distZ != 0.0f)
		alpha = General_GetATanOfXY(distOnGround, distZ);
	front->x = cosf(alpha) * cosf(beta);
	front->y = cosf(alpha) * sinf(beta);
	front->z = sinf(alpha);
}
float Camera_AvoidNearClip(float frontDist, float closeLimit, float defaultNear, float currentNear)
{
	if (frontDist < defaultNear && frontDist > closeLimit) {
		if (frontDist < currentNear)
			return frontDist;
	} else if (frontDist < closeLimit)
		return closeLimit;
	return currentNear;
}

float Camera_AvoidViewPlaneWidth(float fov, float aspectRatio, float tweakFOV)
{
	float viewPlaneHeight = tanf(fov * PI / 180.0f / 2.0f);

	return viewPlaneHeight * aspectRatio * tweakFOV;
}

float Camera_GeomProbsTimer(float timer, float targetNormalDir, float timeStep)
{
	if (targetNormalDir < 0.0f) {
		if (timer < 0.0f)
			timer = 0.0f;
		timer += timeStep;
	} else if (targetNormalDir > 0.5f) {
		if (timer > 0.0f)
			timer = 0.0f;
		timer -= timeStep;
	}
	return timer;
}
void Camera_OrbitOffset(float distance, float angle, float *x, float *y)
{
	*x = distance * cosf(angle);
	*y = distance * sinf(angle);
}
void Camera_SphericalDirection(float alpha, float beta, CVector *dir)
{
	dir->x = cosf(alpha) * cosf(beta);
	dir->y = cosf(alpha) * sinf(beta);
	dir->z = sinf(alpha);
}

void Camera_UpFromRoll(float roll, CVector *up)
{
	const float halfPi = (float)3.14159265358979323846 / 2;

	up->x = cosf(roll + halfPi);
	up->y = 0.0f;
	up->z = sinf(roll + halfPi);
}
float Camera_CarTargetRoll(bool dPad, bool dPadLeft, float moveSpeedAlongForward, float leftStickX, float tiltOvershoot, float maxRollAngle, float forwardDotFront)
{
	const float pi = (float)3.14159265358979323846;
	float fwdSpeed = 180.0f * moveSpeedAlongForward;
	float targetRoll, angleDiff, a;

	if (fwdSpeed > 210.0f)
		fwdSpeed = 210.0f;
	a = fabsf(forwardDotFront);
	angleDiff = acosf(a < 1.0f ? a : 1.0f);
	if (dPad) {
		if (dPadLeft)
			targetRoll = (10.0f * pi / 180.0f) * tiltOvershoot + maxRollAngle;
		else
			targetRoll = -((10.0f * pi / 180.0f) * tiltOvershoot + maxRollAngle);
		targetRoll *= fwdSpeed / 210.0f * sinf(angleDiff);
	} else {
		targetRoll = leftStickX / 128.0f * fwdSpeed / 210.0f;
		targetRoll *= ((10.0f * pi / 180.0f) * tiltOvershoot + maxRollAngle) * sinf(angleDiff);
	}
	return targetRoll;
}
float Camera_AlphaRecentre(float alpha, float defaultAlpha)
{
	if (alpha < defaultAlpha - 0.05f)
		return 0.05f;
	if (alpha < defaultAlpha)
		return defaultAlpha - alpha;
	if (alpha > defaultAlpha + 0.05f)
		return -0.05f;
	if (alpha > defaultAlpha)
		return defaultAlpha - alpha;
	return 0.0f;
}

void Camera_StickLookOffsets(float lookLeftRight, float lookUpDown, float stickSens, float fov, float timeStep, float *betaOffset, float *alphaOffset)
{
	*betaOffset = lookLeftRight * stickSens * (1.0f / 14.0f) * fov / 80.0f * timeStep;
	*alphaOffset = lookUpDown * stickSens * (0.6f / 14.0f) * fov / 80.0f * timeStep;
}
bool Camera_MouseAvoidDistance(float targetDist, float returnRate, float timeStep, float *distance)
{
	if (targetDist < *distance) {
		*distance = targetDist;
		return false;
	} else {
		float f = powf(returnRate, timeStep);

		*distance = (1.0f - f) * targetDist + f * *distance;
		return true;
	}
}
float Camera_HeliAlpha(float forwardXY, float forwardZ, float fwdSpeed)
{
	float carAlpha = 0.0f;
	float a = fabsf(fwdSpeed / 90.0f);
	float alphaAmount = a < 1.0f ? a : 1.0f;

	if (forwardXY != 0.0f || forwardZ != 0.0f)
		carAlpha = General_GetATanOfXY(forwardXY, fabsf(forwardZ)) * alphaAmount;
	return carAlpha;
}

float Camera_ClampCarAlpha(float carAlpha)
{
	const float pi = (float)3.14159265358979323846;

	carAlpha = General_LimitRadianAngle(carAlpha);
	if (carAlpha < 0.0f)
		carAlpha = 0.0f;
	if (carAlpha > 89.0f * pi / 180.0f)
		carAlpha = 89.0f * pi / 180.0f;
	return carAlpha;
}
float Camera_FiretruckTracking(float targetBeta, float beta, float distance, float trackingMult)
{
	const float pi = (float)3.14159265358979323846;
	float deltaBeta = targetBeta - beta;
	float clamped;

	if (deltaBeta > pi)
		deltaBeta -= pi * 2;
	else if (deltaBeta < -pi)
		deltaBeta += pi * 2;
	clamped = deltaBeta < -0.8f ? -0.8f : deltaBeta > 0.8f ? 0.8f : deltaBeta;
	return trackingMult * distance * clamped;
}
float Camera_PedAverageSpeed(float average, float speedSum)
{
	return 0.4f * average + 0.6 * (speedSum / 5.0f);
}

float Camera_TopDownHeightTarget(float pedAverageSpeed)
{
	if (pedAverageSpeed > 0.01f && pedAverageSpeed <= 0.04f)
		return 2.5f;
	if (pedAverageSpeed > 0.04f && pedAverageSpeed <= 0.145f)
		return 4.5f;
	if (pedAverageSpeed > 0.145f)
		return 7.0f;
	return 0.0f;
}
void Camera_LookPoint(float alpha, float beta, float sourceX, float sourceY, float sourceZ, CVector *point)
{
	point->x = 3.0f * cosf(alpha) * cosf(beta) + sourceX;
	point->y = 3.0f * cosf(alpha) * sinf(beta) + sourceY;
	point->z = 3.0f * sinf(alpha) + sourceZ;
}
float Camera_MouseLookDelta(float mouseAccel, float look, float fov)
{
	return mouseAccel * look * fov / 80.0f;
}

float Camera_PadLookDelta(float look, float divisor, float direction, float scale, float fov, float timeStep)
{
	float v = look / divisor;

	return (v * v) * direction * scale / 14.0f * fov / 80.0f * timeStep;
}
float Camera_ClampAlpha(float alpha, float maxDegrees)
{
	const float pi = (float)3.14159265358979323846;

	if (alpha > maxDegrees * pi / 180.0f)
		alpha = maxDegrees * pi / 180.0f;
	else if (alpha < -(89.5f * pi / 180.0f))
		alpha = -(89.5f * pi / 180.0f);
	return alpha;
}
float Camera_DebugSpeed(float speed, bool speedUp, bool slowDown)
{
	if (speedUp)
		speed += 0.1f;
	else if (slowDown)
		speed -= 0.1f;
	else
		speed = 0.0f;
	if (speed > 70.0f)
		speed = 70.0f;
	if (speed < -70.0f)
		speed = -70.0f;
	return speed;
}
float Camera_NearClipFromCollision(float currentNear, float dist)
{
	float m = currentNear < dist ? currentNear : dist;

	return 0.1f > m ? 0.1f : m;
}
float Camera_CarFOV(float fov, float defaultFOV, bool widens, float forwardSpeed, float timeStep)
{
	if (widens) {
		if (forwardSpeed > 0.4f)
			fov += (forwardSpeed - 0.4f) * timeStep;
	}

	if (fov > defaultFOV)
		fov = powf(0.98f, timeStep) * (fov - defaultFOV) + defaultFOV;

	return fov < defaultFOV ? defaultFOV : fov > defaultFOV + 30.0f ? defaultFOV + 30.0f : fov;
}
float Camera_ClampBetween(float value, float low, float high)
{
	if (value <= high) {
		if (value < low)
			value = low;
	} else {
		value = high;
	}
	return value;
}
float Camera_ClampAlphaStop(float alpha, float low, float high, float *alphaSpeed)
{
	if (alpha <= high) {
		if (low > alpha) {
			alpha = low;
			*alphaSpeed = 0.0f;
		}
	} else {
		alpha = high;
		*alphaSpeed = 0.0f;
	}
	return alpha;
}
void Camera_TargetHistoryPos(const CVector *target, float beta, float alpha, float distance, CVector *pos)
{
	float frontX = -(cosf(beta) * cosf(alpha));
	float frontY = -(sinf(beta) * cosf(alpha));
	float frontZ = sinf(alpha);

	pos->x = target->x - frontX * distance;
	pos->y = target->y - frontY * distance;
	pos->z = target->z - frontZ * distance;
}
float Camera_TurretFace(float angleToFace, float gunLR, float turnPerFrame)
{
	const float pi = (float)3.14159265358979323846;
	float neededTurn;

	if (angleToFace <= gunLR + pi) {
		if (angleToFace < gunLR - pi)
			angleToFace = angleToFace + pi * 2;
	} else {
		angleToFace = angleToFace - pi * 2;
	}

	neededTurn = angleToFace - gunLR;
	if (neededTurn <= turnPerFrame) {
		if (neededTurn < -turnPerFrame)
			angleToFace = gunLR - turnPerFrame;
	} else {
		angleToFace = turnPerFrame + gunLR;
	}
	return angleToFace;
}

float Camera_TurretAlpha(float alphaToFace, float gunUD, float alphaTurnPerFrame)
{
	const float pi = (float)3.14159265358979323846;
	float neededAlphaTurn = alphaToFace - gunUD;
	float turretMinY = -(20.0f * pi / 180.0f);
	float turretMaxY = 20.0f * pi / 180.0f;

	if (neededAlphaTurn > alphaTurnPerFrame) {
		gunUD = alphaTurnPerFrame + gunUD;
	} else {
		if (neededAlphaTurn >= -alphaTurnPerFrame)
			gunUD = alphaToFace;
		else
			gunUD = gunUD - alphaTurnPerFrame;
	}

	if (turretMinY <= gunUD) {
		if (gunUD > turretMaxY)
			gunUD = turretMaxY;
	} else {
		gunUD = turretMinY;
	}
	return gunUD;
}
float Camera_StickMovement(float stick, float fov, float scale)
{
	return fabsf(stick) * (fov / 80.0f * scale / 70.0f) * stick * 0.007f * 0.007f;
}
float Camera_BlendBetaSpeed(float targetBlendAmount, float angleChangeStep, float betaSpeed)
{
	float angleChangeStepLeft = 1.0f - angleChangeStep;

	betaSpeed = targetBlendAmount * angleChangeStepLeft + angleChangeStep * betaSpeed;
	if (fabsf(betaSpeed) < 0.0001f)
		betaSpeed = 0.0f;
	return betaSpeed;
}
float Camera_VehicleSpeedVar(float forwardX, float forwardY, float moveSpeedX, float moveSpeedY, float speedVar, float *targetSpeedVar)
{
	float fx = forwardX;
	float fy = forwardY;
	float fz = 0.0f;
	float sq = fx * fx + fy * fy + fz * fz;
	float fwdLength, fwdSpeedX, fwdSpeedY, m;

	// Normalise the forward vector, x is set to 1 for a null vector
	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		fx *= invsqrt;
		fy *= invsqrt;
	} else
		fx = 1.0f;
	fwdLength = sqrtf(fx * fx + fy * fy);
	if (fwdLength != 0.0f) {
		fx /= fwdLength;
		fy /= fwdLength;
	}

	fwdSpeedX = moveSpeedX * fx;
	fwdSpeedY = moveSpeedY * fy;
	m = sqrtf(fwdSpeedX * fwdSpeedX + fwdSpeedY * fwdSpeedY);
	if (fwdSpeedX + fwdSpeedY > 0.0f) {
		float v = m / 0.9f;
		*targetSpeedVar = v < 1.0f ? v : 1.0f;
	} else {
		float v = m / 1.8f;
		*targetSpeedVar = -(v < 0.5f ? v : 0.5f);
	}
	return 0.895f * speedVar + 0.105 * *targetSpeedVar;
}
float Camera_SyphonBetaOffset(float lineDist, float colDist, float playerToTargetDist, float groundDist)
{
	float newBetaOffset = 0.0f;

	if (lineDist > 0.0f && colDist > 0.1f) {
		float distOffset, v;

		// Scale the offset at the centre to the offset at the player
		distOffset = lineDist / colDist * playerToTargetDist;
		v = distOffset / groundDist;
		// Turn it into an angle
		newBetaOffset = 0.9f * asinf(v < 1.0f ? v : 1.0f);
	}
	return newBetaOffset;
}
void Camera_FrontFromAngles(float alpha, float beta, CVector *front)
{
	front->x = -(cosf(beta) * cosf(alpha));
	front->y = -(sinf(beta) * cosf(alpha));
	front->z = sinf(alpha);
}
void Camera_DebugTarget(float alpha, float beta, float sourceX, float sourceY, float sourceZ, float zScale, CVector *target)
{
	target->x = sourceX + cosf(alpha) * sinf(beta) * 7.0f;
	target->y = sourceY + cosf(alpha) * cosf(beta) * 7.0f;
	target->z = sourceZ + sinf(alpha) * zScale;
}
float Camera_BetaOfVector(float x, float y)
{
	if (x == 0.0f && y == 0.0f)
		return 0.0f;
	return General_GetATanOfXY(x, y);
}
void Camera_BufferStep(float *value, float *speed, float targetSpeed, float acceleration, float timeStep)
{
	*speed += acceleration * (targetSpeed - *speed) * timeStep;

	/* Clamp speed if we overshot */
	if (targetSpeed < 0.0f && *speed < targetSpeed)
		*speed = targetSpeed;
	else if (targetSpeed > 0.0f && *speed > targetSpeed)
		*speed = targetSpeed;

	*value += *speed * (10.0f < timeStep ? 10.0f : timeStep);
}

void Camera_FollowBetaStep(float *beta, float *betaSpeed, float reqSpeed, float speedStep, float timeStep)
{
	/* Add or subtract absolute depending on sign */
	if (reqSpeed - *betaSpeed > 0.0f)
		*betaSpeed += speedStep * fabsf(reqSpeed - *betaSpeed) * timeStep;
	else
		*betaSpeed -= speedStep * fabsf(reqSpeed - *betaSpeed) * timeStep;

	if (reqSpeed < 0.0f && *betaSpeed < reqSpeed)
		*betaSpeed = reqSpeed;
	else if (reqSpeed > 0.0f && *betaSpeed > reqSpeed)
		*betaSpeed = reqSpeed;

	*beta += *betaSpeed * (10.0f < timeStep ? 10.0f : timeStep);
}
float Camera_PedCamDistance(float baseDist, float alpha, float falloff, float angleDist, bool useFalloff)
{
	const float halfpi = (float)3.14159265358979323846 / 2;

	if (useFalloff) {
		float a = alpha * falloff;

		return baseDist + cosf(a < halfpi ? a : halfpi) * angleDist;
	}
	return baseDist + cosf(alpha) * angleDist;
}
float Camera_HeliTestHeight(float targetZ, float length, float alpha, float alphaOffset, float heightOffset)
{
	return targetZ + 0.2f + length * sinf(alpha + alphaOffset) + heightOffset;
}

float Camera_AngleAboveGround(float groundZ, float targetZ, float heightOffset, float length, float alphaOffset)
{
	float s = (groundZ - targetZ - 0.2f - heightOffset) / length;

	return asinf(s < -1.0f ? -1.0f : s > 1.0f ? 1.0f : s) - alphaOffset;
}

float Camera_CarSourceHeight(float targetZ, float length, float alpha, float alphaOffset, float heightOffset)
{
	return targetZ + sinf(alpha + alphaOffset) * length + heightOffset;
}
bool Camera_NeedsBetaFix(float deltaBeta, float maxDiffBeta)
{
	const float pi = (float)3.14159265358979323846;

	return fabsf(deltaBeta) > pi - maxDiffBeta;
}

bool Camera_BetaFixDone(float deltaBeta)
{
	const float pi = (float)3.14159265358979323846;

	return fabsf(deltaBeta) < 2.0f * pi / 180.0f;
}
float Camera_HeliCannonLookDelta(float look, float sensitivity, float fov, float timeStep)
{
	return look * fabsf(look) * sensitivity / 14.0f * fov / 80.0f * timeStep;
}

float Camera_GroundHeightGain(float groundDist, float alpha, float alphaOffset)
{
	return groundDist * sinf(alpha + alphaOffset) + groundDist * 0.2f;
}
float Camera_LimitSniperFOV(float fov, float defaultFOV, bool isCameraMode)
{
	if (fov > defaultFOV)
		fov = defaultFOV;
	if (isCameraMode) {
		if (fov < 3.0f)
			fov = 3.0f;
	} else {
		if (fov < 15.0f)
			fov = 15.0f;
	}
	return fov;
}
float Camera_HighAltitudeScale(float cameraZ)
{
	float v = (cameraZ - 55.0f) / 60.0f;

	return 1.0f + (v > 0.0f ? v : 0.0f);
}
float Camera_ZoomStep(float zoom, float target, float timeStep)
{
	if (zoom < target) {
		zoom += 0.12f * timeStep;
		zoom = zoom < target ? zoom : target;
	} else {
		zoom -= 0.12f * timeStep;
		zoom = zoom > target ? zoom : target;
	}
	return zoom;
}
float Camera_ShakeStrength(float force, uint32_t startMs, uint32_t nowMs)
{
	float strength = force - 0.28f * (nowMs - startMs) / 1000.0f;

	return strength < 0.0f ? 0.0f : strength > 2.0f ? 2.0f : strength;
}

void Camera_ShakeOffsets(float strength, int rand, CVector *offsets)
{
	float shakeOffset = strength * 0.1f;

	offsets->x = shakeOffset * ((rand & 0xF) - 7);
	offsets->y = shakeOffset * (((rand & 0xF0) >> 4) - 7);
	offsets->z = shakeOffset * (((rand & 0xF00) >> 8) - 7);
}
float Camera_LODDistMultiplier(float fov, bool highAltitude, float cameraZ)
{
	float multiplier = 70.0f / fov;

	if (highAltitude)
		multiplier *= Camera_HighAltitudeScale(cameraZ);
	if (multiplier > 2.2f)
		multiplier = 2.2f;
	return multiplier;
}
float Camera_SmoothingFactor(float constantInv, float timeStep)
{
	return 1.0f - powf(constantInv, timeStep);
}
bool Camera_PedWillBlockView(float dist, float speedDiff, float heightOffset)
{
	return speedDiff > 0.01f &&
		(heightOffset > 0.0f && (dist - 2.1f) / speedDiff < 75.0f ||
		 heightOffset <= 0.0f && (dist - 2.1f) / speedDiff < 75.0f * 0.1f);
}
float Camera_Clamp01(float a)
{
	return a < 0.0f ? 0.0f : a > 1.0f ? 1.0f : a;
}

float Camera_BezierBlend(float a, float p0, float p1, float p2, float p3)
{
	float b = 1.0f - a;

	return b * b * b * p0 +
		3.0f * a * b * b * p1 +
		3.0f * a * a * b * p2 +
		a * a * a * p3;
}
float Camera_SplineFraction(const float *spline, uint32_t time, uint32_t *marker, int stride, bool *markerOutOfRange)
{
#define SPLINE_MS(t) (uint32_t)((t)*1000.0f)
	uint32_t numFrames = (uint32_t)spline[0];
	uint32_t timeDelta = SPLINE_MS(spline[*marker] - spline[*marker - stride]);
	uint32_t endTime = SPLINE_MS(spline[stride * (numFrames - 1) + 1]);
	float a;

	if (time < endTime) {
		bool canAdvance = true;

		if ((*marker - 1) / stride > numFrames) {
			canAdvance = false;
			*marker = stride * (numFrames - 1) + 1;
		}
		/* Skipping over small time deltas */
		while (timeDelta <= 75 && canAdvance) {
			*marker += stride;
			if ((*marker - 1) / stride > numFrames) {
				canAdvance = false;
				*marker = stride * (numFrames - 1) + 1;
			}
			timeDelta = (uint32_t)((spline[*marker] - spline[*marker - stride]) * 1000.0f);
		}
	}
	*markerOutOfRange = (*marker - 1) / stride > numFrames;
	a = ((float)time - (float)SPLINE_MS(spline[*marker - stride])) / (float)SPLINE_MS(spline[*marker] - spline[*marker - stride]);
#undef SPLINE_MS
	return Camera_Clamp01(a);
}
float Camera_FlyByUp(float rollDegrees, float *upX, float *upZ)
{
	const float pi = (float)3.14159265358979323846;
	float angle = rollDegrees * pi / 180.0f + pi / 2;

	*upX = cosf(angle);
	*upZ = sinf(angle);
	return angle;
}
void Camera_WheelCamRoll(uint32_t timeMs, float *cosine, float *sine)
{
	const float twopi = (float)3.14159265358979323846 * 2;
	float roll = cosf((timeMs & 0x1FFFF) / (float)0x1FFFF * twopi);

	*cosine = cosf(roll * 0.4f);
	*sine = sinf(roll * 0.4f);
}

float Camera_WaterLuminance(float red, float green, float blue)
{
	return sqrtf(red * red + green * green + blue * blue);
}
void Camera_FixedModeUp(const CVector *front, const CVector *upOffset, CVector *up)
{
	VectorMath3 f = { front->x, front->y, front->z };
	VectorMath3 u = { 0.0f + upOffset->x, 0.0f + upOffset->y, 1.0f + upOffset->z };
	VectorMath3 r;

	VectorMath_Normalise(&u);
	r = VectorMath_Cross(f, u);
	VectorMath_Normalise(&r);
	u = VectorMath_Cross(r, f);
	up->x = u.x;
	up->y = u.y;
	up->z = u.z;
}
void Camera_SyphonBasis(CVector *up, const CVector *upOffset, CVector *front)
{
	VectorMath3 u = { up->x, up->y, up->z };
	VectorMath3 f = { front->x, front->y, front->z };
	VectorMath3 left;

	u.x += upOffset->x;
	u.y += upOffset->y;
	u.z += upOffset->z;
	VectorMath_Normalise(&u);
	left = VectorMath_Cross(u, f);
	VectorMath_Normalise(&left);
	f = VectorMath_Cross(left, u);
	VectorMath_Normalise(&f);
	up->x = u.x; up->y = u.y; up->z = u.z;
	front->x = f.x; front->y = f.y; front->z = f.z;
}
void Camera_PerpendicularUp(const CVector *front, const CVector *up, bool normaliseLeft, CVector *outUp)
{
	VectorMath3 f = { front->x, front->y, front->z };
	VectorMath3 u = { up->x, up->y, up->z };
	VectorMath3 left = VectorMath_Cross(u, f);

	if (normaliseLeft)
		VectorMath_Normalise(&left);
	u = VectorMath_Cross(f, left);
	VectorMath_Normalise(&u);
	outUp->x = u.x;
	outUp->y = u.y;
	outUp->z = u.z;
}
void Camera_RightAndUp(const CVector *front, const CVector *up, bool normaliseUp, CVector *right, CVector *outUp)
{
	VectorMath3 f = { front->x, front->y, front->z };
	VectorMath3 u = { up->x, up->y, up->z };
	VectorMath3 r = VectorMath_Cross(f, u);

	VectorMath_Normalise(&r);
	u = VectorMath_Cross(r, f);
	if (normaliseUp)
		VectorMath_Normalise(&u);
	right->x = r.x; right->y = r.y; right->z = r.z;
	outUp->x = u.x; outUp->y = u.y; outUp->z = u.z;
}
void Camera_FixedPitchFront(float angle, CVector *front)
{
	front->x = cosf(0.3f) * sinf(angle);
	front->y = cosf(0.3f) * cosf(angle);
	front->z = -sinf(0.3f);
}
void Camera_DeadCamRight(const CVector *forward, CVector *right)
{
	VectorMath3 f = { forward->x, forward->y, forward->z };
	VectorMath3 up = { 0.0f, 0.0f, 1.0f };
	VectorMath3 r = VectorMath_Cross(f, up);

	r.z = 0.0f;
	VectorMath_Normalise(&r);
	right->x = r.x;
	right->y = r.y;
	right->z = r.z;
}

void Camera_DeadCamFront(float time, float waftRate, float tiltAmp, const CVector *right, CVector *front)
{
	VectorMath3 f = { 0.0f, 0.0f, -1.0f };
	float t2 = time < 2000.0f ? time : 2000.0f;
	float k = cosf(time / waftRate) * tiltAmp * t2 / 2000.0f;

	f.x += k * right->x;
	f.y += k * right->y;
	f.z += k * right->z;
	VectorMath_Normalise(&f);
	front->x = f.x;
	front->y = f.y;
	front->z = f.z;
}

void Camera_DeadCamWaftOffset(float time, float waftAmplitude, float waftRate, const CVector *right, CVector *offset)
{
	float t1 = 1000.0f < time ? 1000.0f : time;
	float k = waftAmplitude * t1 / 1000.0f * sinf(time / waftRate);

	offset->x = k * right->x;
	offset->y = k * right->y;
	offset->z = k * right->z;
}
void Camera_OverShoulderPos(const CVector *cop, const CVector *target, float rightOfCop, float behindCop, float minFromTarget, CVector *out)
{
	VectorMath3 copCoors = { cop->x, cop->y, cop->z };
	VectorMath3 copToTarget = { target->x - cop->x, target->y - cop->y, target->z - cop->z };
	VectorMath3 up = { 0.0f, 0.0f, 1.0f };
	VectorMath3 side = VectorMath_Cross(copToTarget, up);
	float dist;

	VectorMath_Normalise(&side);
	copCoors.x += rightOfCop * side.x;
	copCoors.y += rightOfCop * side.y;
	copCoors.z += rightOfCop * side.z;
	VectorMath_Normalise(&copToTarget);
	if (copToTarget.z < -0.7071f) {
		float groundDist;

		copToTarget.z = -0.7071f;
		groundDist = sqrtf(copToTarget.x * copToTarget.x + copToTarget.y * copToTarget.y);
		if (groundDist > 0.0f) {
			copToTarget.x *= 0.7071f / groundDist;
			copToTarget.y *= 0.7071f / groundDist;
		}
		VectorMath_Normalise(&copToTarget);
	} else {
		if (copToTarget.z > 0.0f) {
			copToTarget.z = 0.0f;
			VectorMath_Normalise(&copToTarget);
		}
	}
	copCoors.x -= behindCop * copToTarget.x;
	copCoors.y -= behindCop * copToTarget.y;
	copCoors.z -= behindCop * copToTarget.z;
	copToTarget.x = target->x - copCoors.x;
	copToTarget.y = target->y - copCoors.y;
	copToTarget.z = target->z - copCoors.z;
	dist = sqrtf(copToTarget.x * copToTarget.x + copToTarget.y * copToTarget.y + copToTarget.z * copToTarget.z);
	if (dist < minFromTarget && dist > 0.0f) {
		copToTarget.x *= minFromTarget / dist;
		copToTarget.y *= minFromTarget / dist;
		copToTarget.z *= minFromTarget / dist;
	}
	out->x = target->x - copToTarget.x;
	out->y = target->y - copToTarget.y;
	out->z = target->z - copToTarget.z;
}
void Camera_AlongGroundPos(const CVector *target, const CVector *cop, float alongGround, float sideGround, CVector *out)
{
	VectorMath3 copToTarget = { target->x - cop->x, target->y - cop->y, target->z - cop->z };
	VectorMath3 up = { 0.0f, 0.0f, 1.0f };
	VectorMath3 side;

	copToTarget.z = 0.0f;
	VectorMath_Normalise(&copToTarget);
	out->x = target->x + alongGround * copToTarget.x;
	out->y = target->y + alongGround * copToTarget.y;
	out->z = target->z + alongGround * copToTarget.z;
	side = VectorMath_Cross(copToTarget, up);
	out->x += sideGround * side.x;
	out->y += sideGround * side.y;
	out->z += sideGround * side.z;
	out->z += 5.0f;
}
int Camera_ArrestVariantIndex(float rnd)
{
	if (rnd < 1.0f) return 0;
	else if (rnd < 2.0f) return 1;
	else if (rnd < 3.0f) return 2;
	else if (rnd < 4.0f) return 3;
	return 4;
}
void Camera_LampPostFront(const CVector *target, const CVector *camSource, float rotateDist, CVector *front, CVector *up)
{
	VectorMath3 toTarget = { target->x - camSource->x, target->y - camSource->y, target->z - camSource->z };
	VectorMath3 f = toTarget;
	VectorMath3 u = { 0.0f, 0.0f, 1.0f };
	VectorMath3 right;

	f.z = 0.0f;
	VectorMath_Normalise(&f);
	right = VectorMath_Cross(f, u);
	VectorMath_Normalise(&right);
	f.x = toTarget.x + right.x * rotateDist;
	f.y = toTarget.y + right.y * rotateDist;
	f.z = toTarget.z + right.z * rotateDist;
	f.z = 0.0f;
	VectorMath_Normalise(&f);
	front->x = f.x; front->y = f.y; front->z = f.z;
	up->x = u.x; up->y = u.y; up->z = u.z;
}
float Camera_DragDistance(CVector *dist, float minDist, float maxDist)
{
	float camDist = sqrtf(dist->x * dist->x + dist->y * dist->y + dist->z * dist->z);

	if (camDist == 0.0f) {
		dist->x = 1.0f;
		dist->y = 1.0f;
		dist->z = 0.0f;
	} else if (camDist < minDist) {
		dist->x *= minDist / camDist;
		dist->y *= minDist / camDist;
		dist->z *= minDist / camDist;
	} else if (camDist > maxDist) {
		dist->x *= maxDist / camDist;
		dist->y *= maxDist / camDist;
		dist->z *= maxDist / camDist;
	}
	return sqrtf(dist->x * dist->x + dist->y * dist->y + dist->z * dist->z);
}
float Camera_TransitionBetaAngle(float transitionBeta)
{
	return General_GetATanOfXY(-cosf(transitionBeta), -sinf(transitionBeta));
}
float Camera_NearClipNearPed(float pedColDist)
{
	float v = pedColDist - 0.3f;

	return v > 0.05f ? v : 0.05f;
}

float Camera_NearClipPedInWay(float colCamDist, float defaultNear)
{
	float v = colCamDist - 0.35f;

	return v < defaultNear ? v : defaultNear;
}
float Camera_DirectionAngle(float dot)
{
	float a = fabsf(dot);

	return acosf(a < 1.0f ? a : 1.0f);
}

float Camera_SmallerDelta(float deltaLeft, float deltaRight)
{
	return fabsf(deltaLeft) < fabsf(deltaRight) ? deltaLeft : deltaRight;
}
void Camera_CarCamDistances(const float row[5], float zoomSmooth, float approxCarLength, float colMaxZ, int rcKind, float rcHeliExtra, float rcPlaneExtra, bool staysOutside, bool isGoblin, bool nearZoomSmallCar, float *alphaOffset, float *targetZAdd, float *newDistance, float *minDistForThisCar, float *nextDistance)
{
	float minDistForVehType;

	*targetZAdd = 0.0f;
	*newDistance = zoomSmooth + row[1] + approxCarLength;
	if (rcKind == 1)
		*newDistance += rcHeliExtra;
	else if (rcKind == 2)
		*newDistance += rcPlaneExtra;

	*minDistForThisCar = approxCarLength * row[3];

	if (staysOutside) {
		float radiusToStayOutside = colMaxZ * row[0] - row[2];

		if (radiusToStayOutside > 0.0f) {
			*targetZAdd = radiusToStayOutside;
			*newDistance += radiusToStayOutside;
			*alphaOffset += 0.3f / *newDistance * radiusToStayOutside;
		}
	}
	if (isGoblin)
		*alphaOffset += 0.178997f;

	minDistForVehType = row[4];
	if (nearZoomSmallCar)
		minDistForVehType = minDistForVehType * 0.65f;
	*nextDistance = *newDistance > minDistForVehType ? *newDistance : minDistForVehType;
}
float Camera_FollowCarTargetBeta(float frontHeading, float speed2D, float speedHeading, float beta, float timeStep, float betaChangeFactor, float betaChangeLimitFactor, float sideSpeed)
{
	const float pi = (float)3.14159265358979323846;
	const float twopi = pi * 2;
	const float halfpi = pi / 2;
	float camRightHeading = frontHeading - halfpi;
	float velocityRightHeading;
	float betaChangeMult1, betaChangeLimit, betaChange, targetBeta, m;

	if (camRightHeading < -pi)
		camRightHeading = camRightHeading + twopi;

	if (speed2D <= 0.02f)
		velocityRightHeading = camRightHeading;
	else
		velocityRightHeading = speedHeading - halfpi;

	if (velocityRightHeading < camRightHeading - pi)
		velocityRightHeading = velocityRightHeading + twopi;
	else if (velocityRightHeading > camRightHeading + pi)
		velocityRightHeading = velocityRightHeading - twopi;

	betaChangeMult1 = timeStep * betaChangeFactor;
	betaChangeLimit = timeStep * betaChangeLimitFactor;
	m = betaChangeMult1 * sideSpeed;
	betaChange = (1.0f < m ? 1.0f : m) * (velocityRightHeading - camRightHeading);
	betaChange = Camera_ClampBetween(betaChange, -betaChangeLimit, betaChangeLimit);
	targetBeta = camRightHeading + betaChange;

	if (targetBeta < beta - halfpi)
		targetBeta += twopi;
	else if (targetBeta > beta + pi)
		targetBeta -= twopi;
	return targetBeta;
}
float Camera_FollowCarTargetAlpha(float frontZ, float alphaOffset, float lowerLimit, float upperLimit)
{
	float targetAlpha = General_AsinClamped(frontZ) - alphaOffset;

	return Camera_ClampBetween(targetAlpha, -lowerLimit, upperLimit);
}

float Camera_FollowCarAlphaBlend(float targetAlpha, float alpha, float blendBase, float speedFactor, float timeStep)
{
	float maxAlphaBlendAmount = timeStep * speedFactor;
	float targetAlphaBlendAmount = (1.0f - powf(blendBase, timeStep)) * (targetAlpha - alpha);

	return Camera_ClampBetween(targetAlphaBlendAmount, -maxAlphaBlendAmount, maxAlphaBlendAmount);
}
float Camera_ClearedHeight(float targetZ, float cornerZ, float origZ, float sourceZ)
{
	float top = targetZ > cornerZ ? targetZ : cornerZ;
	float v = top + 0.1f;
	float height = v < origZ ? v : origZ;

	return height > sourceZ ? height : sourceZ;
}
float Camera_BufferedHeightOffset(float bufferedHeight)
{
	float v = bufferedHeight / 2.0f;

	return 1.0f < v ? 1.0f : v;
}
float Camera_MinCarDistance(float baseDist)
{
	float v = baseDist * 0.6f;

	return v < 3.5f ? v : 3.5f;
}
float Camera_SniperNearClip(float fov, float nearClipMult, float defaultNear)
{
	float f = fov < 15.0f ? fov : 15.0f;

	return ((15.0f - f) * nearClipMult + 1.0f) * defaultNear;
}
float Camera_ClampedGroundRatio(float ratio)
{
	return ratio > 0.5f ? ratio : 0.5f;
}
float Camera_StepsLeftAfterTimeStep(float stepsLeft, float timeStep)
{
	float v = stepsLeft - timeStep;

	return 0.0f > v ? 0.0f : v;
}
float Camera_BetaStickBlendDivisor(float timeStep)
{
	return timeStep > 1.0f ? timeStep : 1.0f;
}
float Camera_ScriptNearClip(float magnitude)
{
	float v = magnitude * 0.5f;

	return v > 0.05f ? v : 0.05f;
}
bool Camera_ScriptTargetPullingAway(float distance, float speedTowardsDir, float limit)
{
	return distance > limit && speedTowardsDir > 0.0f;
}
bool Camera_ScriptDistanceEnds(float distance, float speedTowardsDir, float farLimit, float nearLimit)
{
	return (distance > farLimit && speedTowardsDir > 0.0f) || distance < nearLimit;
}
bool Camera_ScriptDistanceOutOfRange(float distance, float farLimit, float nearLimit)
{
	return distance > farLimit || distance < nearLimit;
}
void Camera_AheadPosition(const CVector *coors, const CVector *speed, float ahead, CVector *out)
{
	VectorMath3 v = { speed->x, speed->y, 0.0f };

	VectorMath_Normalise(&v);
	out->x = coors->x + ahead * v.x;
	out->y = coors->y + ahead * v.y;
	out->z = coors->z + ahead * v.z;
}
void Camera_AheadAndSidePosition(const CVector *coors, const CVector *speed, float ahead, float side, CVector *out)
{
	VectorMath3 v = { speed->x, speed->y, 0.0f };

	VectorMath_Normalise(&v);
	out->x = coors->x + ahead * v.x;
	out->y = coors->y + ahead * v.y;
	out->z = coors->z + ahead * v.z;
	out->x += side * v.y;
	out->y += side * -v.x;
	out->z += side * 0.0f;
}
int Camera_ScriptStartVerdict(float distance, float speedTowardsDir, float farLimit, float nearLimit)
{
	if (distance > farLimit && speedTowardsDir > 0.0f)
		return 0;
	if (distance < nearLimit)
		return 1;
	return 2;
}
bool Camera_CopCarFollowsPlayer(float dx, float dy, float playerFwdX, float playerFwdY, float carFwdX, float carFwdY)
{
	return dx * playerFwdX + dy * playerFwdY < 0.0f &&
		carFwdX * playerFwdX + carFwdY * playerFwdY > 0.8f;
}
int Camera_ScriptStartRangeVerdict(float distance, float farLimit, float nearLimit)
{
	if (distance > farLimit)
		return 0;
	if (distance < nearLimit)
		return 1;
	return 2;
}
float Camera_RaisedAboveWater(float camZ, float waterZ, bool isBoat)
{
	float offset = isBoat ? -2.0f : 1.0f;

	if (camZ < waterZ + offset)
		return waterZ + offset;
	return camZ;
}
float Camera_RaisedAboveGround(float camZ, float groundZ)
{
	return camZ < groundZ ? groundZ + 0.5f : camZ;
}
float Camera_TargetForwardOrientation(const CVector *forward)
{
	VectorMath3 v = { forward->x, forward->y, forward->z };

	VectorMath_Normalise(&v);
	return General_GetATanOfXY(v.x, v.y);
}
void Camera_SideFront(const CVector *upIn, const CVector *forwardIn, bool negate, CVector *upOut, CVector *frontOut)
{
	VectorMath3 up = { upIn->x, upIn->y, upIn->z };
	VectorMath3 front = { forwardIn->x, forwardIn->y, forwardIn->z };

	// Normalise the up and forward directions
	VectorMath_Normalise(&up);
	VectorMath_Normalise(&front);

	// Direction to the side from their cross product, negated for the other side
	front = VectorMath_Cross(front, up);
	if (negate) {
		front.x = -front.x;
		front.y = -front.y;
		front.z = -front.z;
	}
	VectorMath_Normalise(&front);
	upOut->x = up.x; upOut->y = up.y; upOut->z = up.z;
	frontOut->x = front.x; frontOut->y = front.y; frontOut->z = front.z;
}
void Camera_NormalisedDifference(const CVector *a, const CVector *b, CVector *out)
{
	VectorMath3 v = { a->x - b->x, a->y - b->y, a->z - b->z };

	VectorMath_Normalise(&v);
	out->x = v.x; out->y = v.y; out->z = v.z;
}
void Camera_OrthonormalBasis(const CVector *frontIn, const CVector *upIn, CVector *frontOut, CVector *rightOut, CVector *upOut)
{
	VectorMath3 front = { frontIn->x, frontIn->y, frontIn->z };
	VectorMath3 up = { upIn->x, upIn->y, upIn->z };
	VectorMath3 right;

	// Normalise the front and up directions
	VectorMath_Normalise(&front);
	VectorMath_Normalise(&up);

	// Right direction perpendicular to both, then the up direction perpendicular to front and right
	right = VectorMath_Cross(front, up);
	VectorMath_Normalise(&right);
	up = VectorMath_Cross(right, front);
	VectorMath_Normalise(&up);
	frontOut->x = front.x; frontOut->y = front.y; frontOut->z = front.z;
	rightOut->x = right.x; rightOut->y = right.y; rightOut->z = right.z;
	upOut->x = up.x; upOut->y = up.y; upOut->z = up.z;
}
void Camera_TopDownBasis(const CVector *frontIn, CVector *frontOut, CVector *rightOut, CVector *upOut)
{
	VectorMath3 front = { frontIn->x, frontIn->y, frontIn->z };
	VectorMath3 right = { -1.0f, 0.0f, 0.0f };
	VectorMath3 up;

	// Looking down: right direction is fixed, up is perpendicular to front and right
	VectorMath_Normalise(&front);
	up = VectorMath_Cross(front, right);
	VectorMath_Normalise(&up);
	frontOut->x = front.x; frontOut->y = front.y; frontOut->z = front.z;
	rightOut->x = right.x; rightOut->y = right.y; rightOut->z = right.z;
	upOut->x = up.x; upOut->y = up.y; upOut->z = up.z;
}
//- rouz edit (Claude)
