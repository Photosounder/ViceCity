//+ rouz edit (ChatGPT)
#pragma once
#include "Clouds_control.h"
#ifdef __cplusplus
extern "C" {
#endif
void Clouds_BackendPushTxd(void);
int32_t Clouds_BackendFindTxd(const char *name);
void Clouds_BackendSetTxd(int32_t slot);
void Clouds_BackendPopTxd(void);
float Clouds_BackendOrientation(void);
bool Clouds_BackendOutside(void);
float Clouds_BackendScreenWidth(void);
CloudsVector Clouds_BackendCameraPosition(void);
bool * Clouds_BackendSunBlocked(void);
float Clouds_BackendMoonSize(void);
float Clouds_BackendSunX(void);
float Clouds_BackendSunY(void);
void * Clouds_BackendCoronaTexture(int32_t index);
bool Clouds_BackendProject(CloudsVector value,CloudsVector *out,float *width,float *height,bool farclip);
void Clouds_BackendInitSprites(void);
void Clouds_BackendFlushSprites(void);
void Clouds_BackendPush(const char *name);
void Clouds_BackendPop(void);
float Clouds_BackendWind(void);
float Clouds_BackendFoggyness(void);
float Clouds_BackendCloudCoverage(void);
float Clouds_BackendExtraSunnyness(void);
float Clouds_BackendRainbow(void);
void Clouds_BackendZWrite(bool enabled);
void Clouds_BackendZTest(bool enabled);
void Clouds_BackendVertexAlpha(bool enabled);
void Clouds_BackendFog(bool enabled);
void Clouds_BackendSrcBlend(bool additive);
void Clouds_BackendDestBlend(bool additive);
void Clouds_BackendCull(bool none);
void Clouds_BackendRenderOneXLUSprite(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,uint8_t a);
void Clouds_BackendRenderBufferedOneXLUSprite(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,uint8_t a);
void Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Dimension(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,float roll,uint8_t a);
void Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Aspect(float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz,float roll,uint8_t a);
void Clouds_BackendRenderBufferedOneXLUSprite_Rotate_2Colours(float x,float y,float z,float w,float h,uint8_t r1,uint8_t g1,uint8_t b1,uint8_t r2,uint8_t g2,uint8_t b2,float cx,float cy,float recipz,float rotation,uint8_t a);
#ifdef REVC_SOFTWARE_POLYGONS
void * Clouds_BackendCameraFrame(void *camera);
void * Clouds_BackendFrameMatrix(void *frame);
float Clouds_BackendNearClip(void *camera);
float Clouds_BackendFarClip(void *camera);
CloudsVector Clouds_BackendSunDirection(void);
CloudsVector Clouds_BackendMatrixPosition(void *matrix);
CloudsVector Clouds_BackendMatrixRight(void *matrix);
CloudsVector Clouds_BackendMatrixUp(void *matrix);
CloudsVector Clouds_BackendMatrixForward(void *matrix);
void Clouds_BackendSetPos(void *vertices,int32_t index,float x,float y,float z);
void Clouds_BackendSetRGBA(void *vertices,int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void Clouds_BackendSetU(void *vertices,int32_t index,float value);
void Clouds_BackendSetV(void *vertices,int32_t index,float value);
#endif
enum { CLOUDS_COLOR_TOP,CLOUDS_COLOR_BOTTOM,CLOUDS_COLOR_BACKGROUND };
typedef struct CloudsColorFields { uint8_t *r,*g,*b,*a; } CloudsColorFields;
typedef struct CloudsViewMatrix { float rx,ry,rz,fx,fy,fz,ux,uy,uz,px,py,pz; } CloudsViewMatrix;
CloudsColorFields Clouds_BackendColorFields(int32_t index);
CloudsVector Clouds_BackendCameraUp(void);
CloudsVector Clouds_BackendCameraForward(void);
float Clouds_BackendForwardAxis(int32_t axis);
bool Clouds_BackendCollisionPolys(void);
float Clouds_BackendScreenHeight(void);
CloudsViewMatrix Clouds_BackendViewMatrix(void);
bool Clouds_BackendCapturingWorldEffects(void);
void Clouds_BackendDrawBackgroundQuad(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,int32_t top,int32_t bottom);
void Clouds_BackendDrawBackgroundRect(float left,float top,float right,float bottom,int32_t lower,int32_t upper);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
