//+ rouz edit (Claude)
#include <stddef.h>
#include <math.h>
#include "HeliLogic.h"
#include "General.h"

void Heli_FindSwatPositionRelativeToHeli(int n, float *x, float *y, float *z)
{
	// Four rope positions below the cabin, left and right, back and front
	switch (n) {
	case 0: *x = -1.2f; *y = -1.0f; *z = -0.5f; break;
	case 1: *x =  1.2f; *y = -1.0f; *z = -0.5f; break;
	case 2: *x = -1.2f; *y =  1.0f; *z = -0.5f; break;
	case 3: *x =  1.2f; *y =  1.0f; *z = -0.5f; break;
	default: *x = 0.0f; *y = 0.0f; *z = 0.0f; break;
	}
}

void Heli_ClearNodes(CHeliData *self)
{
	// Forget all the frames
	int i;
	for (i = 0; i < NUM_HELI_NODES; i++)
		self->m_aHeliNodes[i] = NULL;
}
void Heli_ShootDown(CHeliData *self, bool spinPositive, uint32_t explosionTime)
{
	self->m_fAngularSpeed = spinPositive ? 0.05f : -0.05f;
	self->m_heliStatus = HELI_STATUS_SHOT_DOWN;
	self->m_nExplosionTimer = explosionTime;
}

bool Heli_AddBulletDamage(CHeliData *self, int32_t damage)
{
	// The Catalina helicopter is the weaker one
	self->m_nBulletDamage += damage;
	return (self->m_heliType == HELI_CATALINA && self->m_nBulletDamage > 400) ||
		(self->m_heliType != HELI_CATALINA && self->m_nBulletDamage > 700);
}

void Heli_BulletPosition(const float *line0, const float *line1, const float *heliPos, float *out)
{
	float dx = heliPos[0] - line0[0], dy = heliPos[1] - line0[1], dz = heliPos[2] - line0[2];
	float distToHeli = sqrtf(dx*dx + dy*dy + dz*dz);
	float lx = line1[0] - line0[0], ly = line1[1] - line0[1], lz = line1[2] - line0[2];
	float lineLength = sqrtf(lx*lx + ly*ly + lz*lz);
	float a = distToHeli-5.0f;
	float m = 1.0f > a ? 1.0f : a;

	// Five units before the helicopter, but at least one along the line
	out[0] = line0[0] + (lx*m)/lineLength;
	out[1] = line0[1] + (ly*m)/lineLength;
	out[2] = line0[2] + (lz*m)/lineLength;
}
float Heli_ChaseSpeed(float targetDist, int heliStatus)
{
	float speed;
	if (targetDist > 100.0f)
		speed = 1.0f;
	else if (targetDist > 75.0f)
		speed = 0.7f;
	else
		speed = 0.4f;
	if (heliStatus == HELI_STATUS_HOVER || heliStatus == HELI_STATUS_HOVER2 || heliStatus == HELI_STATUS_SHOT_DOWN)
		speed = 0.0f;
	return speed;
}

void Heli_UpdateGroundSpeed(float targetSpeedX, float targetSpeedY, float *moveSpeedX, float *moveSpeedY, float timeStep)
{
	float dirX = targetSpeedX - *moveSpeedX;
	float dirY = targetSpeedY - *moveSpeedY;
	float speedDiff = sqrtf(dirX*dirX + dirY*dirY);
	if (speedDiff != 0.0f) {
		dirX /= speedDiff;
		dirY /= speedDiff;
	} else
		dirX = 1.0f;
	float speedInc = timeStep * 0.002f;
	if (speedDiff < speedInc) {
		*moveSpeedX = targetSpeedX;
		*moveSpeedY = targetSpeedY;
	} else {
		*moveSpeedX += dirX * speedInc;
		*moveSpeedY += dirY * speedInc;
	}
}

float Heli_UpdateVerticalSpeed(float targetZ, float posZ, float speedZ, float timeStep, int heliStatus)
{
	float targetSpeedZ = (targetZ - posZ) * 0.01f;
	float speedDiffZ = targetSpeedZ - speedZ;
	float speedIncZ = timeStep * 0.001f;
	if (heliStatus == HELI_STATUS_FLY_AWAY)
		speedIncZ *= 1.5f;
	if (fabsf(speedDiffZ) < speedIncZ)
		speedZ = targetSpeedZ;
	else if (speedDiffZ < 0.0f)
		speedZ -= speedIncZ;
	else
		speedZ += speedIncZ * 1.5f;
	return speedZ;
}

void Heli_UpdateRotation(float *angularSpeed, float *rotation, float targetDist, float targetDirX, float targetDirY, float timeStep)
{
	float targetAngularSpeed;
	*angularSpeed *= powf(0.995f, timeStep);

	// An angle too high would cause infinite loops
	if (fabsf(*angularSpeed) > 0.1f)
		*angularSpeed = copysignf(0.1f, *angularSpeed);
	if (targetDist < 8.0f)
		targetAngularSpeed = 0.0f;
	else {
		float rotationDiff = General_GetATanOfXY(targetDirX, targetDirY) - *rotation;
		rotationDiff = General_RaiseBelow(rotationDiff, -3.14f, 6.28f);
		rotationDiff = General_ReduceAbove(rotationDiff, 3.14f, 6.28f);
		if (fabsf(rotationDiff) > 0.4f) {
			if (rotationDiff < 0.0f)
				targetAngularSpeed = -0.2f;
			else
				targetAngularSpeed = 0.2f;
		} else
			targetAngularSpeed = 0.0f;
	}
	float angularSpeedDiff = targetAngularSpeed - *angularSpeed;
	float angularSpeedInc = timeStep * 0.0001f;
	if (fabsf(angularSpeedDiff) < angularSpeedInc)
		*angularSpeed = targetAngularSpeed;
	else if (angularSpeedDiff < 0.0f)
		*angularSpeed -= angularSpeedInc;
	else
		*angularSpeed += angularSpeedInc;
	*rotation += *angularSpeed * timeStep;
}

float Heli_SearchLightIntensity(float searchLightDist)
{
	if (searchLightDist > 60.0f)
		return 0.0f;
	else if (searchLightDist < 40.0f)
		return 1.0f;
	return 1.0f - (40.0f - searchLightDist) / (60.0f - 40.0f);
}

int Heli_ShootTimeout(bool isRandomHeli, int wantedLevel, bool noPolice)
{
	int shootTimeout = 999999;
	if (isRandomHeli) {
		switch (wantedLevel) {
		case 0:
		case 1:
		case 2: shootTimeout = 999999; break;
		case 3: shootTimeout = 10000; break;
		case 4: shootTimeout = 5000; break;
		case 5: shootTimeout = 3500; break;
		case 6: shootTimeout = 2000; break;
		}
		if (noPolice)
			shootTimeout /= 2;
	} else
		shootTimeout = 1500;
	return shootTimeout;
}

float Heli_HoverTargetHeight(bool isCatalina, int heliId)
{
	if (isCatalina)
		return 8.0f;
	return 40.0f - heliId * 10.0f;
}

float Heli_ChaseTargetHeight(bool isCatalina, int heliId)
{
	if (isCatalina)
		return 4.0f;
	return 30.0f - heliId * 7.5f;
}
float Heli_TargetZ(int status, float groundZ, float targetZ, float targetOffset)
{
	float base = groundZ > targetZ ? groundZ : targetZ;

	switch (status) {
	case HELI_STATUS_HOVER:
		return base + 8.0f;
	case HELI_STATUS_SHOT_DOWN:
	case HELI_STATUS_HOVER2:
		return base + 8.0f + targetOffset;
	default:
		return base + 12.0f;
	}
}

void Heli_SpawnPosition(float playerX, float playerY, float angle, float *x, float *y)
{
	*x = playerX;
	*y = playerY;
	*x += 250.0f * sinf(angle);
	*y += 250.0f * cosf(angle);
	if (*x < -2000.0f - 400.0f || *x > 2000.0f - 400.0f || *y < -2000.0f || *y > 2000.0f) {
		*x = playerX;
		*y = playerY;
		*x -= 250.0f * sinf(angle);
		*y -= 250.0f * cosf(angle);
	}
}
void Heli_PushSearchLight(float *historyX, float *historyY, float newX, float newY)
{
	int i;

	for (i = 5; i > 0; i--) {
		historyX[i] = historyX[i - 1];
		historyY[i] = historyY[i - 1];
	}
	historyX[0] = newX;
	historyY[0] = newY;
}

float Heli_NextRotorRotation(float rotation)
{
	rotation += 3.14f / 6.5f;
	if (rotation > 6.28f)
		rotation -= 6.28f;
	return rotation;
}
float Heli_DustStrength(float timeStep)
{
	float v = 16.0f - 4.0f * timeStep;

	return v > 2.0f ? v : 2.0f;
}

void Heli_SearchDirection(float rotation, float dir[2])
{
	dir[0] = -cosf(rotation);
	dir[1] = -sinf(rotation);
}
//- rouz edit (Claude)
