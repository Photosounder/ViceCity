//+ rouz edit (Claude)
#define _USE_MATH_DEFINES
#include <math.h>
#include "WaterLevelMath.h"

// Constants of the water sectors, as in WaterLevel.h
#define WLM_PI ((float)3.14159265358979323846)
#define WLM_TWOPI (WLM_PI*2)
#define WLM_SMALL_SECTOR_SIZE 32
#define WLM_X_OFFSET (400.0f)
#define WLM_UNSIGN_X(x) ( (x) + ((2048.0f - -2048.0f) /2) )
#define WLM_UNSIGN_Y(y) ( (y) + ((2048.0f - -2048.0f) /2) )

bool WaterMath_LevelWithWaves(float *level, uint32_t timeMs, float windClipped, float fX, float fY, float fZ, bool dontCheckZ, int32_t x, int32_t y)
{
	// The waves move with the time
	float fAngle = (timeMs & 4095) * (WLM_TWOPI / 4096.0f);

	float fWave = sinf
	(
		( WLM_UNSIGN_Y(fY)                  - y*WLM_SMALL_SECTOR_SIZE
		+ WLM_UNSIGN_X(fX + WLM_X_OFFSET) - x*WLM_SMALL_SECTOR_SIZE )

		* (WLM_TWOPI / WLM_SMALL_SECTOR_SIZE ) + fAngle
	);

	float fWindFactor = windClipped * 0.4f + 0.2f;

	*level += fWave * fWindFactor;

	if ( dontCheckZ == false && (*level - fZ) > 3.0f )
	{
		*level = 0.0f;
		return false;
	}

	return true;
}

float WaterMath_WavesOnly(uint32_t timeMs, float windClipped, int16_t x, int16_t y)
{
	float fAngle = (timeMs & 4095) * (WLM_TWOPI / 4096.0f);

	float fWindFactor = windClipped * 0.7f + 0.3f;

	float fWave = sinf( (float)((float)(4 * y + 4 * x) * (WLM_TWOPI / WLM_SMALL_SECTOR_SIZE )) + fAngle );

	return fWave * fWindFactor;
}

VectorMath3 WaterMath_Normal(uint32_t timeMs, float windClipped, float fX, float fY, int32_t x, int32_t y)
{
	float fAngle = (timeMs & 4095) * (WLM_TWOPI / 4096.0f);
	float fWindFactor = windClipped * 0.4f + 0.2f;

	float _fWave = (WLM_UNSIGN_Y(fY) - y*WLM_SMALL_SECTOR_SIZE + WLM_UNSIGN_X(fX) - x*WLM_SMALL_SECTOR_SIZE)
			* (WLM_TWOPI / WLM_SMALL_SECTOR_SIZE ) + fAngle;

	// Two vectors on the surface along x and y
	VectorMath3 vA = { 1.0f, 0.0f, fWindFactor * (WLM_TWOPI / WLM_SMALL_SECTOR_SIZE ) * cosf(_fWave) };
	VectorMath3 vB = { 0.0f, 1.0f, fWindFactor * (WLM_TWOPI / WLM_SMALL_SECTOR_SIZE ) * cosf(_fWave) };

	VectorMath3 norm = VectorMath_Cross(vA, vB);
	VectorMath_Normalise(&norm);
	return norm;
}

float WaterMath_WaterDrawDist(float cameraZ)
{
	if     ( cameraZ > 40.0f  ) return 4000.0f;
	if     ( cameraZ < 15.0f  ) return 1200.0f;
	return ( cameraZ + -15.0f ) * 800.0f / 45.0f + 1200.0f;
}

float WaterMath_WavyDrawDist(bool playerInBoat)
{
	if ( playerInBoat )
		return 120.0f;
	else
		return 70.0f;
}

void WaterMath_CamBounds(float camForwardX, float camForwardY, float camForwardZ, bool *useCamStartY, bool *useCamEndY, bool *useCamStartX, bool *useCamEndX)
{
	if ( fabsf(camForwardZ) < 0.8f )
	{
		if ( fabsf(camForwardX) > fabsf(camForwardY) )
		{
			if ( camForwardX > 0.0f )
				*useCamStartX = true;
			else
				*useCamEndX = true;
		}
		else
		{
			if ( camForwardY > 0.0f )
				*useCamStartY = true;
			else
				*useCamEndY = true;
		}
	}
}

float WaterMath_SectorRadius(float size)
{
	return sqrtf(powf(size, 2) + powf(size, 2));
}

bool WaterMath_CollidesWithBlock(const int8_t blockList[128][128], int32_t x, int32_t y, int32_t *block)
{
	*block = blockList[x + 0][y + 0];
	if (*block >= 0)
		return true;

	*block = blockList[x + 0][y + 1];
	if (*block >= 0)
	{
		*block = blockList[x + 0][y + 2];
		if (*block >= 0)
			return true;
	}

	*block = blockList[x + 1][y + 0];
	if (*block >= 0)
		return true;

	*block = blockList[x + 1][y + 1];
	if (*block >= 0)
	{
		*block = blockList[x + 1][y + 2];
		if (*block >= 0)
			return true;
	}

	*block = blockList[x + 2][y + 0];
	if (*block >= 0)
		return true;

	*block = blockList[x + 2][y + 1];
	if (*block >= 0)
	{
		*block = blockList[x + 2][y + 2];
		if (*block >= 0)
			return true;
	}

	return false;
}
void WaterMath_WaveAngles(float morphVertXHalf, float morphVertYHalf, float angle, float wave2InvLength, float *waveMulA, float *waveMulB)
{
	*waveMulA = (morphVertYHalf + morphVertXHalf) * (WLM_TWOPI / 16.0f) + angle;
	*waveMulB = (morphVertYHalf - morphVertXHalf) * (WLM_TWOPI / (16.0f * wave2InvLength)) + angle;
}

void WaterMath_WaveHeights(float waveMulA, float waveMulB, float wind, float waveWind, float waveA, float waveB, float *z, float *tangentAZ, float *tangentBZ)
{
	*z = wind * sinf(waveMulA) + waveWind * sinf(waveMulB);
	*tangentAZ = (waveA * cosf(waveMulA)) - (waveB * cosf(waveMulB));
	*tangentBZ = (waveA * cosf(waveMulA)) + (waveB * cosf(waveMulB));
}

void WaterMath_ScrollTextures(float angle, float windAddUV, float timeStep, float *maskU, float *maskV, float *wakeU, float *wakeV)
{
	*maskU += (sinf(angle) * 0.0005f + 1.1f * windAddUV) * timeStep;
	*maskV -= (cosf(angle * 1.3f) * 0.0005f + 1.2f * windAddUV) * timeStep;
	*wakeU -= (sinf(angle) * 0.0003f + windAddUV) * timeStep;
	*wakeV += (cosf(angle * 0.7f) * 0.0003f + windAddUV) * timeStep;
}
void WaterMath_WaveSlopes(float camX, float camY, float windClipped, float dirScalar1, float normMult, float dirScalar2, float normMultB, float wave2NormScale, float *waveA, float *waveB)
{
	*waveA = (WLM_TWOPI / 16.0f)
		* ((dirScalar1 * fabsf(camX + camY) + normMult) * (windClipped * 0.4f + 0.2f));
	*waveB = WLM_TWOPI / (16.0f * wave2NormScale)
		* ((dirScalar2 * fabsf(camY - camX) + normMultB) * (windClipped * 0.2f + 0.1f));
}
int32_t WaterMath_ClampSectorIndex(int32_t index, int32_t lastSector)
{
	return index < 0 ? 0 : index > lastSector ? lastSector : index;
}
float WaterMath_WakeSizeInIntervals(int index, float size, float lifeTimeMult, float timeLeft, float minInterval)
{
	float clipped = timeLeft > 0.0f ? timeLeft : 0.0f;
	float wakeSize = ((float)index * 0.19f) + size - lifeTimeMult * clipped;
	float sizeB = wakeSize / minInterval;

	if (sizeB < 0.0f)
		sizeB = 1.0f;
	return sizeB;
}

float WaterMath_WakeAlpha(float wakeLifetime, float pointLifetime)
{
	float val = 500.0f - (wakeLifetime - pointLifetime)
		* 600.0f / wakeLifetime;

	return val < 0.0f ? 0.0f : val > 255.0f ? 255.0f : val;
}
void WaterMath_WakeSegmentCorners(const float pointA[2], const float pointB[2], float sizeA, float sizeB, const float distA[2], const float distB[2], float corners[8])
{
	corners[0] = pointA[0] - (sizeA * distA[1]);
	corners[1] = pointA[1] + (sizeA * distA[0]);
	corners[2] = pointA[0] + (sizeA * distA[1]);
	corners[3] = pointA[1] - (sizeA * distA[0]);
	corners[4] = pointB[0] + (sizeB * distB[1]);
	corners[5] = pointB[1] - (sizeB * distB[0]);
	corners[6] = pointB[0] - (sizeB * distB[1]);
	corners[7] = pointB[1] + (sizeB * distB[0]);
}
float WaterMath_FlatWaterBlend(float distX, float distY, float blendRange, float blendDrawDist, float minAlphaMult)
{
	double d = sqrt(distX + distY) - blendDrawDist;
	double m = d > minAlphaMult ? d : (double)minAlphaMult;
	double v = blendRange * m;

	return (float)(v < 1.0f ? v : (double)1.0f);
}

bool WaterMath_NormaliseWakeDirection(float dir[2], float squaredDistance)
{
	bool render = true;

	if (squaredDistance > 3.0f * 3.0f) {
		float norm = (float)(1.0f / sqrt(squaredDistance));

		dir[0] *= norm;
		dir[1] *= norm;
		if (sqrt(squaredDistance) > 13.0f)
			render = false;
	}
	return render;
}
int WaterLevel_MaskAlpha(int alpha, float alphaMul, float dx, float dy)
{
	int a = (int)((float)alpha - (alphaMul * (fabsf(dx) + fabsf(dy))));

	return 0 > a ? 0 : a;
}
bool WaterLevel_MovedFarther(float dx, float dy, float dz, float limit)
{
	return fabsf(dx) + fabsf(dy) + fabsf(dz) > limit;
}
//- rouz edit (Claude)
