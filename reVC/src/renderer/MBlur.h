//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef enum FxType
{
	FXTYPE_WATER1,
	FXTYPE_WATER2,
	FXTYPE_BLOOD1,
	FXTYPE_BLOOD2,
	FXTYPE_HEATHAZE,
	FXTYPE_SPLASH1,
	FXTYPE_SPLASH2,
	FXTYPE_SPLASH3
} FxType;
typedef struct MBlurRect {int32_t x,y,w,h;} MBlurRect;
typedef struct MBlurRectRef {int32_t *x,*y,*w,*h;} MBlurRectRef;
typedef struct MBlurColour {uint8_t red,green,blue,alpha;} MBlurColour;
static inline MBlurRectRef MBlur_RectRef(MBlurRect *rect)
{
    // Borrow the actual C rectangle fields without copying or delaying caller-visible clamping
    const MBlurRectRef value={&rect->x,&rect->y,&rect->w,&rect->h};return value;
}
#ifdef __cplusplus
extern "C" {
#endif
extern void * MBlur_pFrontBuffer;
extern bool MBlur_ms_bJustInitialised;
extern bool MBlur_ms_bScaledBlur;
extern bool MBlur_BlurOn;
extern float MBlur_Drunkness;
extern int32_t MBlur_pBufVertCount;
int32_t MBlur_MotionBlurOpen(void *camera);
int32_t MBlur_MotionBlurClose(void);
void MBlur_CreateImmediateModeData(void *camera,MBlurRectRef rect);
void MBlur_CreateImmediateModeVertices(void *camera,MBlurRectRef rect,void *vertices,MBlurColour colour,float u1,float v1,float u2,float v2,float z,int fullTexture);
void MBlur_MotionBlurRender(void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha);
void MBlur_OverlayRender(void *camera,void *raster,MBlurColour colour,int32_t type,int32_t alpha);
void MBlur_SetDrunkBlur(float drunkness);
void MBlur_ClearDrunkBlur(void);
bool MBlur_PosInside(MBlurRectRef rect,float x1,float y1,float x2,float y2);
bool MBlur_AddRenderFx(void *camera,MBlurRectRef rect,float z,FxType type);
void MBlur_OverlayRenderFx(void *camera,void *frontBuffer);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
