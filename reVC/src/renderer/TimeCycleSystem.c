//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "Timecycle.h"
#include "Timecycle_backend.h"
#include "Clock.h"
#include "Weather.h"
#include "Timer.h"
#include "ZoneCull.h"
#include "FileMgr.h"
#include "Draw_constants.h"
#include <stdio.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
static void TimeCycle_Normalise(TimeCycleVector *value)
{
    // Preserve the complete original vector normalization arithmetic and zero-vector fallback
	float sq = value->x*value->x + value->y*value->y + value->z*value->z;
	if (sq > 0.0f) {
		float invsqrt = 1.0f/sqrtf(sq);
		value->x *= invsqrt;
		value->y *= invsqrt;
		value->z *= invsqrt;
	} else
		value->x = 1.0f;
}

uint8_t TimeCycle_m_nAmbientRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientRed_Obj[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientGreen_Obj[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientBlue_Obj[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientRed_Bl[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientGreen_Bl[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientBlue_Bl[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientRed_Obj_Bl[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientGreen_Obj_Bl[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nAmbientBlue_Obj_Bl[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nDirectionalRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nDirectionalGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nDirectionalBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSkyTopRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSkyTopGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSkyTopBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSkyBottomRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSkyBottomGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSkyBottomBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSunCoreRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSunCoreGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSunCoreBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSunCoronaRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSunCoronaGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nSunCoronaBlue[NUMHOURS][NUMWEATHERS];
int8_t TimeCycle_m_fSunSize[NUMHOURS][NUMWEATHERS];
int8_t TimeCycle_m_fSpriteSize[NUMHOURS][NUMWEATHERS];
int8_t TimeCycle_m_fSpriteBrightness[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nShadowStrength[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nLightShadowStrength[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nPoleShadowStrength[NUMHOURS][NUMWEATHERS];
int16_t TimeCycle_m_fFogStart[NUMHOURS][NUMWEATHERS];
int16_t TimeCycle_m_fFarClip[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fLightsOnGroundBrightness[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nLowCloudsRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nLowCloudsGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nLowCloudsBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nFluffyCloudsTopRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nFluffyCloudsTopGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nFluffyCloudsTopBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nFluffyCloudsBottomRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nFluffyCloudsBottomGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_nFluffyCloudsBottomBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fBlurRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fBlurGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fBlurBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fWaterRed[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fWaterGreen[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fWaterBlue[NUMHOURS][NUMWEATHERS];
uint8_t TimeCycle_m_fWaterAlpha[NUMHOURS][NUMWEATHERS];


float TimeCycle_m_fCurrentAmbientRed;
float TimeCycle_m_fCurrentAmbientGreen;
float TimeCycle_m_fCurrentAmbientBlue;
float TimeCycle_m_fCurrentAmbientRed_Obj;
float TimeCycle_m_fCurrentAmbientGreen_Obj;
float TimeCycle_m_fCurrentAmbientBlue_Obj;
float TimeCycle_m_fCurrentAmbientRed_Bl;
float TimeCycle_m_fCurrentAmbientGreen_Bl;
float TimeCycle_m_fCurrentAmbientBlue_Bl;
float TimeCycle_m_fCurrentAmbientRed_Obj_Bl;
float TimeCycle_m_fCurrentAmbientGreen_Obj_Bl;
float TimeCycle_m_fCurrentAmbientBlue_Obj_Bl;
float TimeCycle_m_fCurrentDirectionalRed;
float TimeCycle_m_fCurrentDirectionalGreen;
float TimeCycle_m_fCurrentDirectionalBlue;
int32_t TimeCycle_m_nCurrentSkyTopRed;
int32_t TimeCycle_m_nCurrentSkyTopGreen;
int32_t TimeCycle_m_nCurrentSkyTopBlue;
int32_t TimeCycle_m_nCurrentSkyBottomRed;
int32_t TimeCycle_m_nCurrentSkyBottomGreen;
int32_t TimeCycle_m_nCurrentSkyBottomBlue;
int32_t TimeCycle_m_nCurrentSunCoreRed;
int32_t TimeCycle_m_nCurrentSunCoreGreen;
int32_t TimeCycle_m_nCurrentSunCoreBlue;
int32_t TimeCycle_m_nCurrentSunCoronaRed;
int32_t TimeCycle_m_nCurrentSunCoronaGreen;
int32_t TimeCycle_m_nCurrentSunCoronaBlue;
float TimeCycle_m_fCurrentSunSize;
float TimeCycle_m_fCurrentSpriteSize;
float TimeCycle_m_fCurrentSpriteBrightness;
int32_t TimeCycle_m_nCurrentShadowStrength;
int32_t TimeCycle_m_nCurrentLightShadowStrength;
int32_t TimeCycle_m_nCurrentPoleShadowStrength;
float TimeCycle_m_fCurrentFogStart;
float TimeCycle_m_fCurrentFarClip;
float TimeCycle_m_fCurrentLightsOnGroundBrightness;
int32_t TimeCycle_m_nCurrentLowCloudsRed;
int32_t TimeCycle_m_nCurrentLowCloudsGreen;
int32_t TimeCycle_m_nCurrentLowCloudsBlue;
int32_t TimeCycle_m_nCurrentFluffyCloudsTopRed;
int32_t TimeCycle_m_nCurrentFluffyCloudsTopGreen;
int32_t TimeCycle_m_nCurrentFluffyCloudsTopBlue;
int32_t TimeCycle_m_nCurrentFluffyCloudsBottomRed;
int32_t TimeCycle_m_nCurrentFluffyCloudsBottomGreen;
int32_t TimeCycle_m_nCurrentFluffyCloudsBottomBlue;
float TimeCycle_m_fCurrentBlurRed;
float TimeCycle_m_fCurrentBlurGreen;
float TimeCycle_m_fCurrentBlurBlue;
float TimeCycle_m_fCurrentWaterRed;
float TimeCycle_m_fCurrentWaterGreen;
float TimeCycle_m_fCurrentWaterBlue;
float TimeCycle_m_fCurrentWaterAlpha;
int32_t TimeCycle_m_nCurrentFogColourRed;
int32_t TimeCycle_m_nCurrentFogColourGreen;
int32_t TimeCycle_m_nCurrentFogColourBlue;

int32_t TimeCycle_m_FogReduction;
int32_t TimeCycle_m_bExtraColourOn;
int32_t TimeCycle_m_ExtraColour;
float TimeCycle_m_ExtraColourInter;

int32_t TimeCycle_m_CurrentStoredValue;
TimeCycleVector TimeCycle_m_VectorToSun[16];
float TimeCycle_m_fShadowFrontX[16];
float TimeCycle_m_fShadowFrontY[16];
float TimeCycle_m_fShadowSideX[16];
float TimeCycle_m_fShadowSideY[16];
float TimeCycle_m_fShadowDisplacementX[16];
float TimeCycle_m_fShadowDisplacementY[16];

void
TimeCycle_Initialise(void)
{
    // Preserve the complete original time-cycle operation and its ordered live accesses
	int w, h;
	int li, bi;
	char line[1040];

	int ambR, ambG, ambB;
	int ambobjR, ambobjG, ambobjB;
	int ambblR, ambblG, ambblB;
	int ambobjblR, ambobjblG, ambobjblB;
	int dirR, dirG, dirB;
	int skyTopR, skyTopG, skyTopB;
	int skyBotR, skyBotG, skyBotB;
	int sunCoreR, sunCoreG, sunCoreB;
	int sunCoronaR, sunCoronaG, sunCoronaB;
	float sunSz, sprSz, sprBght;
	int shad, lightShad, poleShad;
	float farClp, fogSt, lightGnd;
	int cloudR, cloudG, cloudB;
	int fluffyTopR, fluffyTopG, fluffyTopB;
	int fluffyBotR, fluffyBotG, fluffyBotB;
	float blurR, blurG, blurB;
	float waterR, waterG, waterB, waterA;

	TimeCycle_BackendDebug("Intialising CTimeCycle...\n");

	FileMgr_SetDir("DATA");
	FileMgr_LoadFile("TIMECYC.DAT", (*TimeCycle_BackendWorkBuffer()), sizeof((*TimeCycle_BackendWorkBuffer())), "rb");
	FileMgr_SetDir("");

	line[0] = '\0';
	bi = 0;
	for(w = 0; w < NUMWEATHERS; w++)
		for(h = 0; h < NUMHOURS; h++){
			li = 0;
			while((*TimeCycle_BackendWorkBuffer())[bi] == '/' || (*TimeCycle_BackendWorkBuffer())[bi] == '\n' ||
			      (*TimeCycle_BackendWorkBuffer())[bi] == '\0' || (*TimeCycle_BackendWorkBuffer())[bi] == ' ' || (*TimeCycle_BackendWorkBuffer())[bi] == '\r'){
				while((*TimeCycle_BackendWorkBuffer())[bi] != '\n' && (*TimeCycle_BackendWorkBuffer())[bi] != '\0' && (*TimeCycle_BackendWorkBuffer())[bi] != '\r')
					bi++;
				bi++;
			}
			while((*TimeCycle_BackendWorkBuffer())[bi] != '\n'
#ifdef FIX_BUGS
			&& (*TimeCycle_BackendWorkBuffer())[bi] != '\0'
#endif
			)
				line[li++] = (*TimeCycle_BackendWorkBuffer())[bi++];
			line[li] = '\0';
			bi++;

			sscanf(line, "%d %d %d  %d %d %d  %d %d %d  %d %d %d "
			             "%d %d %d  %d %d %d  %d %d %d "
			             "%d %d %d  %d %d %d  %f %f %f %d %d %d %f %f %f "
			             "%d %d %d  %d %d %d  %d %d %d  %f %f %f  %f %f %f %f",
				&ambR, &ambG, &ambB,
				 &ambobjR, &ambobjG, &ambobjB,
				 &ambblR, &ambblG, &ambblB,
				 &ambobjblR, &ambobjblG, &ambobjblB,
				&dirR, &dirG, &dirB,
				 &skyTopR, &skyTopG, &skyTopB,
				 &skyBotR, &skyBotG, &skyBotB,
				&sunCoreR, &sunCoreG, &sunCoreB,
				 &sunCoronaR, &sunCoronaG, &sunCoronaB,
				 &sunSz, &sprSz, &sprBght,
				 &shad, &lightShad, &poleShad,
				 &farClp, &fogSt, &lightGnd,
				&cloudR, &cloudG, &cloudB,
				 &fluffyTopR, &fluffyTopG, &fluffyTopB,
				 &fluffyBotR, &fluffyBotG, &fluffyBotB,
				 &blurR, &blurG, &blurB,
				 &waterR, &waterG, &waterB, &waterA);

			TimeCycle_m_nAmbientRed[h][w] = ambR;
			TimeCycle_m_nAmbientGreen[h][w] = ambG;
			TimeCycle_m_nAmbientBlue[h][w] = ambB;
			TimeCycle_m_nAmbientRed_Obj[h][w] = ambobjR;
			TimeCycle_m_nAmbientGreen_Obj[h][w] = ambobjG;
			TimeCycle_m_nAmbientBlue_Obj[h][w] = ambobjB;
			TimeCycle_m_nAmbientRed_Bl[h][w] = ambblR;
			TimeCycle_m_nAmbientGreen_Bl[h][w] = ambblG;
			TimeCycle_m_nAmbientBlue_Bl[h][w] = ambblB;
			TimeCycle_m_nAmbientRed_Obj_Bl[h][w] = ambobjblR;
			TimeCycle_m_nAmbientGreen_Obj_Bl[h][w] = ambobjblG;
			TimeCycle_m_nAmbientBlue_Obj_Bl[h][w] = ambobjblB;
			TimeCycle_m_nDirectionalRed[h][w] = dirR;
			TimeCycle_m_nDirectionalGreen[h][w] = dirG;
			TimeCycle_m_nDirectionalBlue[h][w] = dirB;
			TimeCycle_m_nSkyTopRed[h][w] = skyTopR;
			TimeCycle_m_nSkyTopGreen[h][w] = skyTopG;
			TimeCycle_m_nSkyTopBlue[h][w] = skyTopB;
			TimeCycle_m_nSkyBottomRed[h][w] = skyBotR;
			TimeCycle_m_nSkyBottomGreen[h][w] = skyBotG;
			TimeCycle_m_nSkyBottomBlue[h][w] = skyBotB;
			TimeCycle_m_nSunCoreRed[h][w] = sunCoreR;
			TimeCycle_m_nSunCoreGreen[h][w] = sunCoreG;
			TimeCycle_m_nSunCoreBlue[h][w] = sunCoreB;
			TimeCycle_m_nSunCoronaRed[h][w] = sunCoronaR;
			TimeCycle_m_nSunCoronaGreen[h][w] = sunCoronaG;
			TimeCycle_m_nSunCoronaBlue[h][w] = sunCoronaB;
			TimeCycle_m_fSunSize[h][w] = sunSz * 10.0f;
			TimeCycle_m_fSpriteSize[h][w] = sprSz * 10.0f;
			TimeCycle_m_fSpriteBrightness[h][w] = sprBght * 10.0f;
			TimeCycle_m_nShadowStrength[h][w] = shad;
			TimeCycle_m_nLightShadowStrength[h][w] = lightShad;
			TimeCycle_m_nPoleShadowStrength[h][w] = poleShad;
			TimeCycle_m_fFarClip[h][w] = farClp;
			TimeCycle_m_fFogStart[h][w] = fogSt;
			TimeCycle_m_fLightsOnGroundBrightness[h][w] = lightGnd * 10.0f;
			TimeCycle_m_nLowCloudsRed[h][w] = cloudR;
			TimeCycle_m_nLowCloudsGreen[h][w] = cloudG;
			TimeCycle_m_nLowCloudsBlue[h][w] = cloudB;
			TimeCycle_m_nFluffyCloudsTopRed[h][w] = fluffyTopR;
			TimeCycle_m_nFluffyCloudsTopGreen[h][w] = fluffyTopG;
			TimeCycle_m_nFluffyCloudsTopBlue[h][w] = fluffyTopB;
			TimeCycle_m_nFluffyCloudsBottomRed[h][w] = fluffyBotR;
			TimeCycle_m_nFluffyCloudsBottomGreen[h][w] = fluffyBotG;
			TimeCycle_m_nFluffyCloudsBottomBlue[h][w] = fluffyBotB;
			TimeCycle_m_fBlurRed[h][w] = blurR;
			TimeCycle_m_fBlurGreen[h][w] = blurG;
			TimeCycle_m_fBlurBlue[h][w] = blurB;
			TimeCycle_m_fWaterRed[h][w] = waterR;
			TimeCycle_m_fWaterGreen[h][w] = waterG;
			TimeCycle_m_fWaterBlue[h][w] = waterB;
			TimeCycle_m_fWaterAlpha[h][w] = waterA;
		}

	TimeCycle_m_FogReduction = 0;

	TimeCycle_BackendDebug("CTimeCycle ready\n");
}

static float interp_c0, interp_c1, interp_c2, interp_c3;

float TimeCycle_InterpolateI8(int8_t *a, int8_t *b)
{
    // Preserve the complete original time-cycle operation and its ordered live accesses
	return a[Weather_OldWeatherType] * interp_c0 +
		b[Weather_OldWeatherType] * interp_c1 +
		a[Weather_NewWeatherType] * interp_c2 +
		b[Weather_NewWeatherType] * interp_c3;
}

float TimeCycle_InterpolateU8(uint8_t *a, uint8_t *b)
{
    // Preserve the complete original time-cycle operation and its ordered live accesses
	return a[Weather_OldWeatherType] * interp_c0 +
		b[Weather_OldWeatherType] * interp_c1 +
		a[Weather_NewWeatherType] * interp_c2 +
		b[Weather_NewWeatherType] * interp_c3;
}

float TimeCycle_InterpolateI16(int16_t *a, int16_t *b)
{
    // Preserve the complete original time-cycle operation and its ordered live accesses
	return a[Weather_OldWeatherType] * interp_c0 +
		b[Weather_OldWeatherType] * interp_c1 +
		a[Weather_NewWeatherType] * interp_c2 +
		b[Weather_NewWeatherType] * interp_c3;
}

void
TimeCycle_StartExtraColour(int32_t c, bool fade)
{
    // Preserve the complete original time-cycle operation and its ordered live accesses
	TimeCycle_m_bExtraColourOn = true;
	TimeCycle_m_ExtraColour = c;
	if(fade)
		TimeCycle_m_ExtraColourInter = 0.0f;
	else
		TimeCycle_m_ExtraColourInter = 1.0f;
}

void
TimeCycle_StopExtraColour(bool fade)
{
    // Preserve the complete original time-cycle operation and its ordered live accesses
	TimeCycle_m_bExtraColourOn = false;
	if(!fade)
		TimeCycle_m_ExtraColourInter = 0.0f;
}

void
TimeCycle_Update(void)
{
    // Preserve the complete original time-cycle operation and its ordered live accesses
	int h1 = Clock_GetHours();
	int h2 = (h1+1)%24;
	int w1 = Weather_OldWeatherType;
	int w2 = Weather_NewWeatherType;
	float timeInterp = (Clock_GetMinutes() + Clock_GetSeconds()/60.0f)/60.0f;
	// coefficients for a bilinear interpolation
	interp_c0 = (1.0f-timeInterp) * (1.0f-Weather_InterpolationValue);
	interp_c1 = timeInterp * (1.0f-Weather_InterpolationValue);
	interp_c2 = (1.0f-timeInterp) * Weather_InterpolationValue;
	interp_c3 = timeInterp * Weather_InterpolationValue;



	TimeCycle_m_nCurrentSkyTopRed = TimeCycle_InterpolateU8(TimeCycle_m_nSkyTopRed[h1],TimeCycle_m_nSkyTopRed[h2]);
	TimeCycle_m_nCurrentSkyTopGreen = TimeCycle_InterpolateU8(TimeCycle_m_nSkyTopGreen[h1],TimeCycle_m_nSkyTopGreen[h2]);
	TimeCycle_m_nCurrentSkyTopBlue = TimeCycle_InterpolateU8(TimeCycle_m_nSkyTopBlue[h1],TimeCycle_m_nSkyTopBlue[h2]);

	TimeCycle_m_nCurrentSkyBottomRed = TimeCycle_InterpolateU8(TimeCycle_m_nSkyBottomRed[h1],TimeCycle_m_nSkyBottomRed[h2]);
	TimeCycle_m_nCurrentSkyBottomGreen = TimeCycle_InterpolateU8(TimeCycle_m_nSkyBottomGreen[h1],TimeCycle_m_nSkyBottomGreen[h2]);
	TimeCycle_m_nCurrentSkyBottomBlue = TimeCycle_InterpolateU8(TimeCycle_m_nSkyBottomBlue[h1],TimeCycle_m_nSkyBottomBlue[h2]);

	TimeCycle_m_fCurrentAmbientRed = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientRed[h1],TimeCycle_m_nAmbientRed[h2]);
	TimeCycle_m_fCurrentAmbientGreen = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientGreen[h1],TimeCycle_m_nAmbientGreen[h2]);
	TimeCycle_m_fCurrentAmbientBlue = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientBlue[h1],TimeCycle_m_nAmbientBlue[h2]);

	TimeCycle_m_fCurrentAmbientRed_Obj = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientRed_Obj[h1],TimeCycle_m_nAmbientRed_Obj[h2]);
	TimeCycle_m_fCurrentAmbientGreen_Obj = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientGreen_Obj[h1],TimeCycle_m_nAmbientGreen_Obj[h2]);
	TimeCycle_m_fCurrentAmbientBlue_Obj = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientBlue_Obj[h1],TimeCycle_m_nAmbientBlue_Obj[h2]);

	TimeCycle_m_fCurrentAmbientRed_Bl = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientRed_Bl[h1],TimeCycle_m_nAmbientRed_Bl[h2]);
	TimeCycle_m_fCurrentAmbientGreen_Bl = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientGreen_Bl[h1],TimeCycle_m_nAmbientGreen_Bl[h2]);
	TimeCycle_m_fCurrentAmbientBlue_Bl = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientBlue_Bl[h1],TimeCycle_m_nAmbientBlue_Bl[h2]);

	TimeCycle_m_fCurrentAmbientRed_Obj_Bl = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientRed_Obj_Bl[h1],TimeCycle_m_nAmbientRed_Obj_Bl[h2]);
	TimeCycle_m_fCurrentAmbientGreen_Obj_Bl = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientGreen_Obj_Bl[h1],TimeCycle_m_nAmbientGreen_Obj_Bl[h2]);
	TimeCycle_m_fCurrentAmbientBlue_Obj_Bl = TimeCycle_InterpolateU8(TimeCycle_m_nAmbientBlue_Obj_Bl[h1],TimeCycle_m_nAmbientBlue_Obj_Bl[h2]);

	TimeCycle_m_fCurrentDirectionalRed = TimeCycle_InterpolateU8(TimeCycle_m_nDirectionalRed[h1],TimeCycle_m_nDirectionalRed[h2]);
	TimeCycle_m_fCurrentDirectionalGreen = TimeCycle_InterpolateU8(TimeCycle_m_nDirectionalGreen[h1],TimeCycle_m_nDirectionalGreen[h2]);
	TimeCycle_m_fCurrentDirectionalBlue = TimeCycle_InterpolateU8(TimeCycle_m_nDirectionalBlue[h1],TimeCycle_m_nDirectionalBlue[h2]);

	TimeCycle_m_nCurrentSunCoreRed = TimeCycle_InterpolateU8(TimeCycle_m_nSunCoreRed[h1],TimeCycle_m_nSunCoreRed[h2]);
	TimeCycle_m_nCurrentSunCoreGreen = TimeCycle_InterpolateU8(TimeCycle_m_nSunCoreGreen[h1],TimeCycle_m_nSunCoreGreen[h2]);
	TimeCycle_m_nCurrentSunCoreBlue = TimeCycle_InterpolateU8(TimeCycle_m_nSunCoreBlue[h1],TimeCycle_m_nSunCoreBlue[h2]);

	TimeCycle_m_nCurrentSunCoronaRed = TimeCycle_InterpolateU8(TimeCycle_m_nSunCoronaRed[h1],TimeCycle_m_nSunCoronaRed[h2]);
	TimeCycle_m_nCurrentSunCoronaGreen = TimeCycle_InterpolateU8(TimeCycle_m_nSunCoronaGreen[h1],TimeCycle_m_nSunCoronaGreen[h2]);
	TimeCycle_m_nCurrentSunCoronaBlue = TimeCycle_InterpolateU8(TimeCycle_m_nSunCoronaBlue[h1],TimeCycle_m_nSunCoronaBlue[h2]);

	TimeCycle_m_fCurrentSunSize = TimeCycle_InterpolateI8(TimeCycle_m_fSunSize[h1],TimeCycle_m_fSunSize[h2])/10.0f;
	TimeCycle_m_fCurrentSpriteSize = TimeCycle_InterpolateI8(TimeCycle_m_fSpriteSize[h1],TimeCycle_m_fSpriteSize[h2])/10.0f;
	TimeCycle_m_fCurrentSpriteBrightness = TimeCycle_InterpolateI8(TimeCycle_m_fSpriteBrightness[h1],TimeCycle_m_fSpriteBrightness[h2])/10.0f;
	TimeCycle_m_nCurrentShadowStrength = TimeCycle_InterpolateU8(TimeCycle_m_nShadowStrength[h1],TimeCycle_m_nShadowStrength[h2]);
	TimeCycle_m_nCurrentLightShadowStrength = TimeCycle_InterpolateU8(TimeCycle_m_nLightShadowStrength[h1],TimeCycle_m_nLightShadowStrength[h2]);
	TimeCycle_m_nCurrentPoleShadowStrength = TimeCycle_InterpolateU8(TimeCycle_m_nPoleShadowStrength[h1],TimeCycle_m_nPoleShadowStrength[h2]);
	TimeCycle_m_fCurrentFarClip = TimeCycle_InterpolateI16(TimeCycle_m_fFarClip[h1],TimeCycle_m_fFarClip[h2]);
	TimeCycle_m_fCurrentFogStart = TimeCycle_InterpolateI16(TimeCycle_m_fFogStart[h1],TimeCycle_m_fFogStart[h2]);
	TimeCycle_m_fCurrentLightsOnGroundBrightness = TimeCycle_InterpolateU8(TimeCycle_m_fLightsOnGroundBrightness[h1],TimeCycle_m_fLightsOnGroundBrightness[h2])/10.0f;

	TimeCycle_m_nCurrentLowCloudsRed = TimeCycle_InterpolateU8(TimeCycle_m_nLowCloudsRed[h1],TimeCycle_m_nLowCloudsRed[h2]);
	TimeCycle_m_nCurrentLowCloudsGreen = TimeCycle_InterpolateU8(TimeCycle_m_nLowCloudsGreen[h1],TimeCycle_m_nLowCloudsGreen[h2]);
	TimeCycle_m_nCurrentLowCloudsBlue = TimeCycle_InterpolateU8(TimeCycle_m_nLowCloudsBlue[h1],TimeCycle_m_nLowCloudsBlue[h2]);

	TimeCycle_m_nCurrentFluffyCloudsTopRed = TimeCycle_InterpolateU8(TimeCycle_m_nFluffyCloudsTopRed[h1],TimeCycle_m_nFluffyCloudsTopRed[h2]);
	TimeCycle_m_nCurrentFluffyCloudsTopGreen = TimeCycle_InterpolateU8(TimeCycle_m_nFluffyCloudsTopGreen[h1],TimeCycle_m_nFluffyCloudsTopGreen[h2]);
	TimeCycle_m_nCurrentFluffyCloudsTopBlue = TimeCycle_InterpolateU8(TimeCycle_m_nFluffyCloudsTopBlue[h1],TimeCycle_m_nFluffyCloudsTopBlue[h2]);

	TimeCycle_m_nCurrentFluffyCloudsBottomRed = TimeCycle_InterpolateU8(TimeCycle_m_nFluffyCloudsBottomRed[h1],TimeCycle_m_nFluffyCloudsBottomRed[h2]);
	TimeCycle_m_nCurrentFluffyCloudsBottomGreen = TimeCycle_InterpolateU8(TimeCycle_m_nFluffyCloudsBottomGreen[h1],TimeCycle_m_nFluffyCloudsBottomGreen[h2]);
	TimeCycle_m_nCurrentFluffyCloudsBottomBlue = TimeCycle_InterpolateU8(TimeCycle_m_nFluffyCloudsBottomBlue[h1],TimeCycle_m_nFluffyCloudsBottomBlue[h2]);

	TimeCycle_m_fCurrentBlurRed = TimeCycle_InterpolateU8(TimeCycle_m_fBlurRed[h1],TimeCycle_m_fBlurRed[h2]);
	TimeCycle_m_fCurrentBlurGreen = TimeCycle_InterpolateU8(TimeCycle_m_fBlurGreen[h1],TimeCycle_m_fBlurGreen[h2]);
	TimeCycle_m_fCurrentBlurBlue = TimeCycle_InterpolateU8(TimeCycle_m_fBlurBlue[h1],TimeCycle_m_fBlurBlue[h2]);

	TimeCycle_m_fCurrentWaterRed = TimeCycle_InterpolateU8(TimeCycle_m_fWaterRed[h1],TimeCycle_m_fWaterRed[h2]);
	TimeCycle_m_fCurrentWaterGreen = TimeCycle_InterpolateU8(TimeCycle_m_fWaterGreen[h1],TimeCycle_m_fWaterGreen[h2]);
	TimeCycle_m_fCurrentWaterBlue = TimeCycle_InterpolateU8(TimeCycle_m_fWaterBlue[h1],TimeCycle_m_fWaterBlue[h2]);
	TimeCycle_m_fCurrentWaterAlpha = TimeCycle_InterpolateU8(TimeCycle_m_fWaterAlpha[h1],TimeCycle_m_fWaterAlpha[h2]);
#undef INTERP

	if(TimeCycle_m_FogReduction != 0)
		TimeCycle_m_fCurrentFarClip = Max(TimeCycle_m_fCurrentFarClip, TimeCycle_m_FogReduction/64.0f * 650.0f);

	TimeCycle_m_CurrentStoredValue = (TimeCycle_m_CurrentStoredValue+1)&0xF;

	float sunAngle = 2*PI*(Clock_GetSeconds()/60.0f + Clock_GetMinutes() + Clock_GetHours()*60)/(24*60);
	TimeCycleVector *sunPos = TimeCycle_GetSunDirection();
	sunPos->x = sinf(sunAngle);
	sunPos->y = 1.0f;
	sunPos->z = 0.2f - cosf(sunAngle);
	TimeCycle_Normalise(sunPos);

	if(TimeCycle_m_bExtraColourOn)
		TimeCycle_m_ExtraColourInter = Min(1.0f, TimeCycle_m_ExtraColourInter + (*Timer_GetTimeStep())/120.0f);
	else
		TimeCycle_m_ExtraColourInter = Max(-.0f, TimeCycle_m_ExtraColourInter - (*Timer_GetTimeStep())/120.0f);
	if(TimeCycle_m_ExtraColourInter > 0.0f){
#define INTERP(extra,cur) (TimeCycle_m_ExtraColourInter*extra[TimeCycle_m_ExtraColour][WEATHER_EXTRACOLOURS] + (1.0f-TimeCycle_m_ExtraColourInter)*cur)
#define INTERPscl(extra,scl,cur) (TimeCycle_m_ExtraColourInter*extra[TimeCycle_m_ExtraColour][WEATHER_EXTRACOLOURS]/scl + (1.0f-TimeCycle_m_ExtraColourInter)*cur)
		if(TimeCycle_m_nSkyTopRed[TimeCycle_m_ExtraColour][WEATHER_EXTRACOLOURS] != 0 ||
		   TimeCycle_m_nSkyTopGreen[TimeCycle_m_ExtraColour][WEATHER_EXTRACOLOURS] != 0 ||
		   TimeCycle_m_nSkyTopBlue[TimeCycle_m_ExtraColour][WEATHER_EXTRACOLOURS] != 0){
			TimeCycle_m_nCurrentSkyTopRed = INTERP(TimeCycle_m_nSkyTopRed,TimeCycle_m_nCurrentSkyTopRed);
			TimeCycle_m_nCurrentSkyTopGreen = INTERP(TimeCycle_m_nSkyTopGreen,TimeCycle_m_nCurrentSkyTopGreen);
			TimeCycle_m_nCurrentSkyTopBlue = INTERP(TimeCycle_m_nSkyTopBlue,TimeCycle_m_nCurrentSkyTopBlue);

			TimeCycle_m_nCurrentSkyBottomRed = INTERP(TimeCycle_m_nSkyBottomRed,TimeCycle_m_nCurrentSkyBottomRed);
			TimeCycle_m_nCurrentSkyBottomGreen = INTERP(TimeCycle_m_nSkyBottomGreen,TimeCycle_m_nCurrentSkyBottomGreen);
			TimeCycle_m_nCurrentSkyBottomBlue = INTERP(TimeCycle_m_nSkyBottomBlue,TimeCycle_m_nCurrentSkyBottomBlue);

			TimeCycle_m_nCurrentSunCoreRed = INTERP(TimeCycle_m_nSunCoreRed,TimeCycle_m_nCurrentSunCoreRed);
			TimeCycle_m_nCurrentSunCoreGreen = INTERP(TimeCycle_m_nSunCoreGreen,TimeCycle_m_nCurrentSunCoreGreen);
			TimeCycle_m_nCurrentSunCoreBlue = INTERP(TimeCycle_m_nSunCoreBlue,TimeCycle_m_nCurrentSunCoreBlue);

			TimeCycle_m_nCurrentSunCoronaRed = INTERP(TimeCycle_m_nSunCoronaRed,TimeCycle_m_nCurrentSunCoronaRed);
			TimeCycle_m_nCurrentSunCoronaGreen = INTERP(TimeCycle_m_nSunCoronaGreen,TimeCycle_m_nCurrentSunCoronaGreen);
			TimeCycle_m_nCurrentSunCoronaBlue = INTERP(TimeCycle_m_nSunCoronaBlue,TimeCycle_m_nCurrentSunCoronaBlue);

			TimeCycle_m_fCurrentSunSize = INTERPscl(TimeCycle_m_fSunSize,10.0f,TimeCycle_m_fCurrentSunSize);

			TimeCycle_m_nCurrentLowCloudsRed = INTERP(TimeCycle_m_nLowCloudsRed,TimeCycle_m_nCurrentLowCloudsRed);
			TimeCycle_m_nCurrentLowCloudsGreen = INTERP(TimeCycle_m_nLowCloudsGreen,TimeCycle_m_nCurrentLowCloudsGreen);
			TimeCycle_m_nCurrentLowCloudsBlue = INTERP(TimeCycle_m_nLowCloudsBlue,TimeCycle_m_nCurrentLowCloudsBlue);

			TimeCycle_m_nCurrentFluffyCloudsTopRed = INTERP(TimeCycle_m_nFluffyCloudsTopRed,TimeCycle_m_nCurrentFluffyCloudsTopRed);
			TimeCycle_m_nCurrentFluffyCloudsTopGreen = INTERP(TimeCycle_m_nFluffyCloudsTopGreen,TimeCycle_m_nCurrentFluffyCloudsTopGreen);
			TimeCycle_m_nCurrentFluffyCloudsTopBlue = INTERP(TimeCycle_m_nFluffyCloudsTopBlue,TimeCycle_m_nCurrentFluffyCloudsTopBlue);

			TimeCycle_m_nCurrentFluffyCloudsBottomRed = INTERP(TimeCycle_m_nFluffyCloudsBottomRed,TimeCycle_m_nCurrentFluffyCloudsBottomRed);
			TimeCycle_m_nCurrentFluffyCloudsBottomGreen = INTERP(TimeCycle_m_nFluffyCloudsBottomGreen,TimeCycle_m_nCurrentFluffyCloudsBottomGreen);
			TimeCycle_m_nCurrentFluffyCloudsBottomBlue = INTERP(TimeCycle_m_nFluffyCloudsBottomBlue,TimeCycle_m_nCurrentFluffyCloudsBottomBlue);

			TimeCycle_m_fCurrentWaterRed = INTERP(TimeCycle_m_fWaterRed,TimeCycle_m_fCurrentWaterRed);
			TimeCycle_m_fCurrentWaterGreen = INTERP(TimeCycle_m_fWaterGreen,TimeCycle_m_fCurrentWaterGreen);
			TimeCycle_m_fCurrentWaterBlue = INTERP(TimeCycle_m_fWaterBlue,TimeCycle_m_fCurrentWaterBlue);
			TimeCycle_m_fCurrentWaterAlpha = INTERP(TimeCycle_m_fWaterAlpha,TimeCycle_m_fCurrentWaterAlpha);
		}

		TimeCycle_m_fCurrentAmbientRed = INTERP(TimeCycle_m_nAmbientRed,TimeCycle_m_fCurrentAmbientRed);
		TimeCycle_m_fCurrentAmbientGreen = INTERP(TimeCycle_m_nAmbientGreen,TimeCycle_m_fCurrentAmbientGreen);
		TimeCycle_m_fCurrentAmbientBlue = INTERP(TimeCycle_m_nAmbientBlue,TimeCycle_m_fCurrentAmbientBlue);

		TimeCycle_m_fCurrentAmbientRed_Obj = INTERP(TimeCycle_m_nAmbientRed_Obj,TimeCycle_m_fCurrentAmbientRed_Obj);
		TimeCycle_m_fCurrentAmbientGreen_Obj = INTERP(TimeCycle_m_nAmbientGreen_Obj,TimeCycle_m_fCurrentAmbientGreen_Obj);
		TimeCycle_m_fCurrentAmbientBlue_Obj = INTERP(TimeCycle_m_nAmbientBlue_Obj,TimeCycle_m_fCurrentAmbientBlue_Obj);

		TimeCycle_m_fCurrentAmbientRed_Bl = INTERP(TimeCycle_m_nAmbientRed_Bl,TimeCycle_m_fCurrentAmbientRed_Bl);
		TimeCycle_m_fCurrentAmbientGreen_Bl = INTERP(TimeCycle_m_nAmbientGreen_Bl,TimeCycle_m_fCurrentAmbientGreen_Bl);
		TimeCycle_m_fCurrentAmbientBlue_Bl = INTERP(TimeCycle_m_nAmbientBlue_Bl,TimeCycle_m_fCurrentAmbientBlue_Bl);

		TimeCycle_m_fCurrentAmbientRed_Obj_Bl = INTERP(TimeCycle_m_nAmbientRed_Obj_Bl,TimeCycle_m_fCurrentAmbientRed_Obj_Bl);
		TimeCycle_m_fCurrentAmbientGreen_Obj_Bl = INTERP(TimeCycle_m_nAmbientGreen_Obj_Bl,TimeCycle_m_fCurrentAmbientGreen_Obj_Bl);
		TimeCycle_m_fCurrentAmbientBlue_Obj_Bl = INTERP(TimeCycle_m_nAmbientBlue_Obj_Bl,TimeCycle_m_fCurrentAmbientBlue_Obj_Bl);

		TimeCycle_m_fCurrentDirectionalRed = INTERP(TimeCycle_m_nDirectionalRed,TimeCycle_m_fCurrentDirectionalRed);
		TimeCycle_m_fCurrentDirectionalGreen = INTERP(TimeCycle_m_nDirectionalGreen,TimeCycle_m_fCurrentDirectionalGreen);
		TimeCycle_m_fCurrentDirectionalBlue = INTERP(TimeCycle_m_nDirectionalBlue,TimeCycle_m_fCurrentDirectionalBlue);

		TimeCycle_m_fCurrentSpriteSize = INTERPscl(TimeCycle_m_fSpriteSize,10.0f,TimeCycle_m_fCurrentSpriteSize);
		TimeCycle_m_fCurrentSpriteBrightness = INTERPscl(TimeCycle_m_fSpriteBrightness,10.0f,TimeCycle_m_fCurrentSpriteBrightness);
		TimeCycle_m_nCurrentShadowStrength = INTERP(TimeCycle_m_nShadowStrength,TimeCycle_m_nCurrentShadowStrength);
		TimeCycle_m_nCurrentLightShadowStrength = INTERP(TimeCycle_m_nLightShadowStrength,TimeCycle_m_nCurrentLightShadowStrength);
		TimeCycle_m_nCurrentPoleShadowStrength = INTERP(TimeCycle_m_nPoleShadowStrength,TimeCycle_m_nCurrentPoleShadowStrength);
		TimeCycle_m_fCurrentFarClip = INTERP(TimeCycle_m_fFarClip,TimeCycle_m_fCurrentFarClip);
		TimeCycle_m_fCurrentFogStart = INTERP(TimeCycle_m_fFogStart,TimeCycle_m_fCurrentFogStart);
		TimeCycle_m_fCurrentLightsOnGroundBrightness = INTERPscl(TimeCycle_m_fLightsOnGroundBrightness,10.0f,TimeCycle_m_fCurrentLightsOnGroundBrightness);

		TimeCycle_m_fCurrentBlurRed = INTERP(TimeCycle_m_fBlurRed,TimeCycle_m_fCurrentBlurRed);
		TimeCycle_m_fCurrentBlurGreen = INTERP(TimeCycle_m_fBlurGreen,TimeCycle_m_fCurrentBlurGreen);
		TimeCycle_m_fCurrentBlurBlue = INTERP(TimeCycle_m_fBlurBlue,TimeCycle_m_fCurrentBlurBlue);

#undef INTERP
#undef INTERPscl
	}

	if(TimeCycle_BackendBlurType() == TIMECYCLE_MOTION_BLUR_NONE || TimeCycle_BackendBlurType() == TIMECYCLE_MOTION_BLUR_LIGHT_SCENE)
		TimeCycle_BackendSetMotionBlur(TimeCycle_m_fCurrentBlurRed, TimeCycle_m_fCurrentBlurGreen, TimeCycle_m_fCurrentBlurBlue, 5, TIMECYCLE_MOTION_BLUR_LIGHT_SCENE);

	TimeCycle_m_nCurrentFogColourRed = (TimeCycle_m_nCurrentSkyTopRed + 2*TimeCycle_m_nCurrentSkyBottomRed) / 3;
	TimeCycle_m_nCurrentFogColourGreen = (TimeCycle_m_nCurrentSkyTopGreen + 2*TimeCycle_m_nCurrentSkyBottomGreen) / 3;
	TimeCycle_m_nCurrentFogColourBlue = (TimeCycle_m_nCurrentSkyTopBlue + 2*TimeCycle_m_nCurrentSkyBottomBlue) / 3;

	TimeCycle_m_fCurrentAmbientRed /= 255.0f;
	TimeCycle_m_fCurrentAmbientGreen /= 255.0f;
	TimeCycle_m_fCurrentAmbientBlue /= 255.0f;
	TimeCycle_m_fCurrentAmbientRed_Obj /= 255.0f;
	TimeCycle_m_fCurrentAmbientGreen_Obj /= 255.0f;
	TimeCycle_m_fCurrentAmbientBlue_Obj /= 255.0f;
	TimeCycle_m_fCurrentAmbientRed_Bl /= 255.0f;
	TimeCycle_m_fCurrentAmbientGreen_Bl /= 255.0f;
	TimeCycle_m_fCurrentAmbientBlue_Bl /= 255.0f;
	TimeCycle_m_fCurrentAmbientRed_Obj_Bl /= 255.0f;
	TimeCycle_m_fCurrentAmbientGreen_Obj_Bl /= 255.0f;
	TimeCycle_m_fCurrentAmbientBlue_Obj_Bl /= 255.0f;
	TimeCycle_m_fCurrentDirectionalRed /= 255.0f;
	TimeCycle_m_fCurrentDirectionalGreen /= 255.0f;
	TimeCycle_m_fCurrentDirectionalBlue /= 255.0f;

	TimeCycle_BackendShadowValues(*sunPos, 
		&TimeCycle_m_fShadowFrontX[TimeCycle_m_CurrentStoredValue], &TimeCycle_m_fShadowFrontY[TimeCycle_m_CurrentStoredValue],
		&TimeCycle_m_fShadowSideX[TimeCycle_m_CurrentStoredValue], &TimeCycle_m_fShadowSideY[TimeCycle_m_CurrentStoredValue],
		&TimeCycle_m_fShadowDisplacementX[TimeCycle_m_CurrentStoredValue], &TimeCycle_m_fShadowDisplacementY[TimeCycle_m_CurrentStoredValue]);

	if(TimeCycle_BackendCameraForwardZ() < -0.9f ||
	   !Weather_bScriptsForceRain && (CullZones_PlayerNoRain() || CullZones_CamNoRain() || TimeCycle_BackendCutscene()))
		TimeCycle_m_FogReduction = Min(TimeCycle_m_FogReduction+1, 64);
	else
		TimeCycle_m_FogReduction = Max(TimeCycle_m_FogReduction-1, 0);
}

//- rouz edit (ChatGPT)

float TimeCycle_GetAmbientRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientRed;
}

float TimeCycle_GetAmbientGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientGreen;
}

float TimeCycle_GetAmbientBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientBlue;
}

float TimeCycle_GetAmbientRed_Obj(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientRed_Obj;
}

float TimeCycle_GetAmbientGreen_Obj(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientGreen_Obj;
}

float TimeCycle_GetAmbientBlue_Obj(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientBlue_Obj;
}

float TimeCycle_GetAmbientRed_Bl(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientRed_Bl;
}

float TimeCycle_GetAmbientGreen_Bl(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientGreen_Bl;
}

float TimeCycle_GetAmbientBlue_Bl(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientBlue_Bl;
}

float TimeCycle_GetAmbientRed_Obj_Bl(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientRed_Obj_Bl;
}

float TimeCycle_GetAmbientGreen_Obj_Bl(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientGreen_Obj_Bl;
}

float TimeCycle_GetAmbientBlue_Obj_Bl(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentAmbientBlue_Obj_Bl;
}

float TimeCycle_GetDirectionalRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentDirectionalRed;
}

float TimeCycle_GetDirectionalGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentDirectionalGreen;
}

float TimeCycle_GetDirectionalBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentDirectionalBlue;
}

int32_t TimeCycle_GetSkyTopRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSkyTopRed;
}

int32_t TimeCycle_GetSkyTopGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSkyTopGreen;
}

int32_t TimeCycle_GetSkyTopBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSkyTopBlue;
}

int32_t TimeCycle_GetSkyBottomRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSkyBottomRed;
}

int32_t TimeCycle_GetSkyBottomGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSkyBottomGreen;
}

int32_t TimeCycle_GetSkyBottomBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSkyBottomBlue;
}

int32_t TimeCycle_GetSunCoreRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSunCoreRed;
}

int32_t TimeCycle_GetSunCoreGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSunCoreGreen;
}

int32_t TimeCycle_GetSunCoreBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSunCoreBlue;
}

int32_t TimeCycle_GetSunCoronaRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSunCoronaRed;
}

int32_t TimeCycle_GetSunCoronaGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSunCoronaGreen;
}

int32_t TimeCycle_GetSunCoronaBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentSunCoronaBlue;
}

float TimeCycle_GetSunSize(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentSunSize;
}

float TimeCycle_GetSpriteBrightness(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentSpriteBrightness;
}

float TimeCycle_GetSpriteSize(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentSpriteSize;
}

int32_t TimeCycle_GetShadowStrength(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentShadowStrength;
}

int32_t TimeCycle_GetLightShadowStrength(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentLightShadowStrength;
}

float TimeCycle_GetLightOnGroundBrightness(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentLightsOnGroundBrightness;
}

float TimeCycle_GetFarClip(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentFarClip;
}

float TimeCycle_GetFogStart(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentFogStart;
}

int32_t TimeCycle_GetLowCloudsRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentLowCloudsRed;
}

int32_t TimeCycle_GetLowCloudsGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentLowCloudsGreen;
}

int32_t TimeCycle_GetLowCloudsBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentLowCloudsBlue;
}

int32_t TimeCycle_GetFluffyCloudsTopRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFluffyCloudsTopRed;
}

int32_t TimeCycle_GetFluffyCloudsTopGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFluffyCloudsTopGreen;
}

int32_t TimeCycle_GetFluffyCloudsTopBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFluffyCloudsTopBlue;
}

int32_t TimeCycle_GetFluffyCloudsBottomRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFluffyCloudsBottomRed;
}

int32_t TimeCycle_GetFluffyCloudsBottomGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFluffyCloudsBottomGreen;
}

int32_t TimeCycle_GetFluffyCloudsBottomBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFluffyCloudsBottomBlue;
}

int32_t TimeCycle_GetFogRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFogColourRed;
}

int32_t TimeCycle_GetFogGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFogColourGreen;
}

int32_t TimeCycle_GetFogBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_nCurrentFogColourBlue;
}

int32_t TimeCycle_GetFogReduction(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_FogReduction;
}

int32_t TimeCycle_GetBlurRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentBlurRed;
}

int32_t TimeCycle_GetBlurGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentBlurGreen;
}

int32_t TimeCycle_GetBlurBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentBlurBlue;
}

int32_t TimeCycle_GetWaterRed(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentWaterRed;
}

int32_t TimeCycle_GetWaterGreen(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentWaterGreen;
}

int32_t TimeCycle_GetWaterBlue(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentWaterBlue;
}

int32_t TimeCycle_GetWaterAlpha(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fCurrentWaterAlpha;
}

TimeCycleVector * TimeCycle_GetSunDirection(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return &TimeCycle_m_VectorToSun[TimeCycle_m_CurrentStoredValue];
}

float TimeCycle_GetShadowFrontX(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fShadowFrontX[TimeCycle_m_CurrentStoredValue];
}

float TimeCycle_GetShadowFrontY(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fShadowFrontY[TimeCycle_m_CurrentStoredValue];
}

float TimeCycle_GetShadowSideX(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fShadowSideX[TimeCycle_m_CurrentStoredValue];
}

float TimeCycle_GetShadowSideY(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fShadowSideY[TimeCycle_m_CurrentStoredValue];
}

float TimeCycle_GetShadowDisplacementX(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fShadowDisplacementX[TimeCycle_m_CurrentStoredValue];
}

float TimeCycle_GetShadowDisplacementY(void)
{
    // Preserve the original inline accessor value or live borrowed storage
    return TimeCycle_m_fShadowDisplacementY[TimeCycle_m_CurrentStoredValue];
}
//- rouz edit (ChatGPT)
