//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
enum eAspectRatio 
{
	// Make sure these work the same as FrontEndMenuManager.m_PrefsUseWideScreen
	// without widescreen support
	AR_AUTO,
	AR_4_3,
	AR_5_4,
	AR_16_10,
	AR_16_9,
	AR_21_9,

	AR_MAX,
};
#ifdef __cplusplus
extern "C" {
#endif
extern float Draw_ms_fLODDistance;
extern uint8_t Draw_FadeValue, Draw_FadeRed, Draw_FadeGreen, Draw_FadeBlue;
#ifdef PROPER_SCALING
extern bool Draw_ms_bProperScaling;
#endif
#ifdef FIX_RADAR
extern bool Draw_ms_bFixRadar;
#endif
#ifdef FIX_SPRITES
extern bool Draw_ms_bFixSprites;
#endif
void Draw_SetNearClipZ(float nearclip);
float Draw_GetNearClipZ(void);
void Draw_SetFarClipZ(float farclip);
float Draw_GetFarClipZ(void);
void Draw_SetFOV(float fov);
float Draw_GetFOV(void);
float Draw_GetScaledFOV(void);
float Draw_CalculateAspectRatio(void);
#ifdef ASPECT_RATIO_SCALE
float Draw_ConvertFOV(float fov);
#endif
float Draw_GetAspectRatio(void);
void Draw_SetAspectRatio(float ratio);
#ifdef PROPER_SCALING
float Draw_ScaleY(float y);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
