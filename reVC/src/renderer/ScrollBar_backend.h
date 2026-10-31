//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma once
#include "ScrollBar.h"
typedef struct ScrollBarPoint { void *owner;float *x,*y,*z; } ScrollBarPoint;
typedef struct ScrollBarPoints { ScrollBarPoint input,output; } ScrollBarPoints;
typedef struct ScrollBarBasis { ScrollBarVector position,right,up,forward; } ScrollBarBasis;
typedef void (*ScrollBarPointsCallback)(ScrollBar *,ScrollBarPoints,uint8_t,uint8_t,uint8_t);
#ifdef __cplusplus
extern "C" {
#endif
ScrollBarVector ScrollBar_BackendCameraPosition(void);
bool ScrollBar_BackendSphereVisible(ScrollBarVector value,float radius);
void ScrollBar_BackendInitSprites(void);
void ScrollBar_BackendFlushSprites(void);
void ScrollBar_BackendWithPoints(ScrollBar *bar,uint8_t r,uint8_t g,uint8_t b,ScrollBarPointsCallback callback);
bool ScrollBar_BackendProject(ScrollBarPoint input,ScrollBarPoint output,float *width,float *height,bool farclip);
void ScrollBar_BackendSprite(float x,float y,float z,float width,float height,uint8_t r,uint8_t g,uint8_t b,int16_t intensity,float reciprocal,uint8_t alpha);
void * ScrollBar_BackendTexture(void);
void * ScrollBar_BackendRaster(void *texture);
void ScrollBar_BackendSetRaster(void *raster);
void ScrollBar_BackendZWrite(bool enabled);
void ScrollBar_BackendZTest(bool enabled);
void ScrollBar_BackendVertexAlpha(bool enabled);
void ScrollBar_BackendSrcBlend(bool additive);
void ScrollBar_BackendDestBlend(bool additive);
void ScrollBar_BackendLinear(void);
void ScrollBar_BackendFog(bool enabled);
void ScrollBar_BackendCull(bool none);
void ScrollBar_BackendPush(const char *name);
void ScrollBar_BackendPop(void);
void * ScrollBar_BackendCameraFrame(void *camera);
void * ScrollBar_BackendFrameMatrix(void *frame);
ScrollBarBasis ScrollBar_BackendBasis(void *matrix);
float ScrollBar_BackendNearClip(void *camera);
float ScrollBar_BackendFarClip(void *camera);
void ScrollBar_BackendSetPos(void *vertices,int32_t index,float x,float y,float z);
void ScrollBar_BackendSetRGBA(void *vertices,int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void ScrollBar_BackendSetU(void *vertices,int32_t index,float u);
void ScrollBar_BackendSetV(void *vertices,int32_t index,float value);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
