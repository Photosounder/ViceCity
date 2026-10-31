//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef EXTENDED_COLOURFILTER
enum {POSTFX_OFF,POSTFX_SIMPLE,POSTFX_NORMAL,POSTFX_MOBILE,POSTFX_NUMAVERAGE=20};
#ifdef __cplusplus
extern "C" {
#endif
extern void * PostFX_pFrontBuffer;
extern void * PostFX_pBackBuffer;
extern bool PostFX_bJustInitialised;
extern int PostFX_EffectSwitch;
extern bool PostFX_BlurOn;
extern bool PostFX_MotionBlurOn;
extern float PostFX_Intensity;
extern int PostFX_PrevRed[POSTFX_NUMAVERAGE];
extern int PostFX_PrevGreen[POSTFX_NUMAVERAGE];
extern int PostFX_PrevBlue[POSTFX_NUMAVERAGE];
extern int PostFX_PrevAlpha[POSTFX_NUMAVERAGE];
extern int PostFX_AvgRed;
extern int PostFX_AvgGreen;
extern int PostFX_AvgBlue;
extern int PostFX_AvgAlpha;
extern int PostFX_Next;
extern int PostFX_NumValues;
void PostFX_InitOnce(void);
void PostFX_Open(void *camera);
void PostFX_Close(void);
void PostFX_RenderOverlayBlur(void *camera,int32_t red,int32_t green,int32_t blue,int32_t alpha);
void PostFX_RenderOverlaySniper(void *camera,int32_t red,int32_t green,int32_t blue,int32_t alpha);
void PostFX_RenderOverlayShader(void *camera,int32_t red,int32_t green,int32_t blue,int32_t alpha);
void PostFX_RenderMotionBlur(void *camera,uint32_t blur);
void PostFX_Render(void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha);
void PostFX_SmoothColor(uint32_t red,uint32_t green,uint32_t blue,uint32_t alpha);
bool PostFX_NeedBackBuffer(void);
bool PostFX_NeedFrontBuffer(int32_t type);
void PostFX_GetBackBuffer(void *camera);
bool PostFX_UseBlurColours(void);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
