//+ rouz edit (ChatGPT)
#pragma once
#include "PointLights.h"
#ifdef __cplusplus
extern "C" {
#endif
PointLightsVector PointLights_BackendReadVector(const void *value);
PointLightsVector PointLights_BackendCamera(void);
bool PointLights_BackendEnvMap(void);
void * PointLights_BackendEnvCamera(void);
void * PointLights_BackendEnvFrame(void *camera);
void * PointLights_BackendFrameLTM(void *frame);
void * PointLights_BackendCameraFrame(void *camera);
void * PointLights_BackendFrameMatrix(void *frame);
bool PointLights_BackendCutscene(void);
float PointLights_BackendFogginess(void);
void * PointLights_BackendRaster(void);
bool PointLights_BackendVerticalLine(PointLightsVector coors,float z2,float *groundZ);
void PointLights_BackendExtraLight(float x,float y,float z,float red,float green,float blue);
void PointLights_BackendRemoveExtra(void);
void PointLights_BackendSpriteInit(void);
void PointLights_BackendSpriteFlush(void);
bool PointLights_BackendScreen(PointLightsVector coors,PointLightsVector *screen,float *width,float *height,bool farclip);
void PointLights_BackendSprite(float x,float y,float z,float width,float height,uint8_t red,uint8_t green,uint8_t blue,int16_t intensity,float reciprocal,float rotation,uint8_t alpha);
void PointLights_BackendBufferRender(void);
void PointLights_BackendBufferClear(void);
void PointLights_BackendStart(int indicesCount,int verticesCount,uint16_t **indices,void **vertices);
void PointLights_BackendStop(void);
void PointLights_BackendPush(const char *name);
void PointLights_BackendPop(void);
void PointLights_BackendSetRaster(void *raster);
PointLightsVector PointLights_BackendMatrixPosition(void *matrix);
PointLightsVector PointLights_BackendMatrixForward(void *matrix);
PointLightsVector PointLights_BackendMatrixRight(void *matrix);
PointLightsVector PointLights_BackendMatrixUp(void *matrix);
void PointLights_BackendZWrite(bool enabled);
void PointLights_BackendZTest(bool enabled);
void PointLights_BackendVertexAlpha(bool enabled);
void PointLights_BackendFog(bool enabled);
void PointLights_BackendSrcBlend(bool special);
void PointLights_BackendDestBlend(bool special);
void PointLights_BackendCull(bool special);
void PointLights_BackendSetPos(void *vertices,int index,float x,float y,float z);
void PointLights_BackendSetRGBA(void *vertices,int index,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha);
void PointLights_BackendSetU(void *vertices,int index,float value);
void PointLights_BackendSetV(void *vertices,int index,float value);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
