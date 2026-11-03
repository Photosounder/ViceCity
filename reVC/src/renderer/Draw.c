//+ rouz edit (ChatGPT)
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw.h"
#include "Draw_constants.h"
#include "Draw_backend.h"
#include "CutsceneState.h"

static float Draw_ms_fAspectRatio = DEFAULT_ASPECT_RATIO;
#ifdef ASPECT_RATIO_SCALE
static float Draw_ms_fScaledFOV = 45.0f;
#endif

static float Draw_ms_fNearClipZ;
static float Draw_ms_fFarClipZ;
static float Draw_ms_fFOV = 45.0f;
float Draw_ms_fLODDistance;

uint8_t Draw_FadeValue;
uint8_t Draw_FadeRed;
uint8_t Draw_FadeGreen;
uint8_t Draw_FadeBlue;

#ifdef PROPER_SCALING	
bool Draw_ms_bProperScaling = true;
#endif
#ifdef  FIX_RADAR
bool Draw_ms_bFixRadar = true;	
#endif
#ifdef FIX_SPRITES
bool Draw_ms_bFixSprites = true;	
#endif

#ifdef ASPECT_RATIO_SCALE
static float
Draw_FindAspectRatio(void)
{
	// Select the original signed aspect preference and platform screen ratio
	switch (Draw_BackendAspectPreference()) {
	case AR_AUTO:
		return Draw_BackendScreenWidth() / Draw_BackendScreenHeight();
	default:
	case AR_4_3:
		return 4.0f / 3.0f;
	case AR_5_4:
		return 5.0f / 4.0f;
	case AR_16_10:
		return 16.0f / 10.0f;
	case AR_16_9:
		return 16.0f / 9.0f;
	case AR_21_9:
		return 21.0f / 9.0f;
	};
}
#endif

float
Draw_CalculateAspectRatio(void)
{
	// Retain the original preference branches and theatrical aspect calculation
#ifdef ASPECT_RATIO_SCALE
	if (Draw_BackendWidescreen())
		Draw_ms_fAspectRatio = (5.f / 3.f) * Draw_FindAspectRatio() / (16.f / 9.f); // It's used on theatrical showings according to Wiki
	else
		Draw_ms_fAspectRatio = Draw_FindAspectRatio();
#else
	if(Draw_BackendAspectPreference()) {
		if (Draw_BackendWidescreen())
			Draw_ms_fAspectRatio = 5.f / 3.f; // It's used on theatrical showings according to Wiki
		else
			Draw_ms_fAspectRatio = 16.f / 9.f;
	} else if (Draw_BackendWidescreen()) {
		Draw_ms_fAspectRatio = 5.f/4.f;
	} else {
		Draw_ms_fAspectRatio = 4.f/3.f;
	}
#endif
	return Draw_ms_fAspectRatio;
}

#ifdef ASPECT_RATIO_SCALE
// convert a 4:3 hFOV to vFOV,
// then convert that vFOV to hFOV for our aspect ratio,
// i.e. HOR+
float
Draw_ConvertFOV(float hfov)
{
	// Retain double tangent, float arctangent and the original conversion order
	// => tan(hFOV/2) = tan(vFOV/2)*aspectRatio
	// => tan(vFOV/2) = tan(hFOV/2)/aspectRatio
	float ar1 = DEFAULT_ASPECT_RATIO;
	float ar2 = Draw_GetAspectRatio();
	hfov = DEGTORAD(hfov);
	float vfov = atanf((float)(tan(hfov/2) / ar1)) *2;
	hfov = atanf((float)(tan(vfov/2) * ar2)) *2;
	return RADTODEG(hfov);
}
#endif

float Draw_GetScaledFOV(void)
{
	// Apply the live multiplier only in the original aspect-scaling profile
	#ifdef ASPECT_RATIO_SCALE
	return Draw_ms_fScaledFOV * Draw_BackendFovMultiplier();
	#else
	return Draw_ms_fFOV;
	#endif
}

void
Draw_SetFOV(float fov)
{
	// Update scaled FOV before the original stored FOV
#ifdef ASPECT_RATIO_SCALE
	if (!Cutscene_IsRunning())
		Draw_ms_fScaledFOV = Draw_ConvertFOV(fov);
	else
		Draw_ms_fScaledFOV = fov;
#endif
	Draw_ms_fFOV = fov;
}

#ifdef PROPER_SCALING	
float Draw_ScaleY(float y)
{
	// Retain the original live proper-scaling flag and screen-height factor
	return Draw_ms_bProperScaling ? y : y * ((float)DEFAULT_SCREEN_HEIGHT/SCREEN_HEIGHT_NTSC);
}
#endif

void
Draw_SetNearClipZ(float nearclip)
{
	// Preserve the original scalar getter or setter
	Draw_ms_fNearClipZ = nearclip;
}

float
Draw_GetNearClipZ(void)
{
	// Preserve the original scalar getter or setter
	return Draw_ms_fNearClipZ;
}

void
Draw_SetFarClipZ(float farclip)
{
	// Preserve the original scalar getter or setter
	Draw_ms_fFarClipZ = farclip;
}

float
Draw_GetFarClipZ(void)
{
	// Preserve the original scalar getter or setter
	return Draw_ms_fFarClipZ;
}

float
Draw_GetFOV(void)
{
	// Preserve the original scalar getter or setter
	return Draw_ms_fFOV;
}

float
Draw_GetAspectRatio(void)
{
	// Preserve the original scalar getter or setter
	return Draw_ms_fAspectRatio;
}

void
Draw_SetAspectRatio(float ratio)
{
	// Preserve the original scalar getter or setter
	Draw_ms_fAspectRatio = ratio;
}
//- rouz edit (ChatGPT)
