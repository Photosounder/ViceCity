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
//- rouz edit (Claude)
