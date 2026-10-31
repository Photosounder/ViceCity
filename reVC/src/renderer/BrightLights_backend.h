//+ rouz edit (ChatGPT)
#pragma once
#include "BrightLights.h"
#ifdef __cplusplus
extern "C" {
#endif
BrightLightVector BrightLights_BackendCameraPosition(void);
BrightLightVector BrightLights_BackendCameraRight(void);
BrightLightVector BrightLights_BackendCameraUp(void);
bool BrightLights_BackendEnvMap(void);
void * BrightLights_BackendEnvCamera(void);
void * BrightLights_BackendCameraFrame(void *camera);
void * BrightLights_BackendFrameMatrix(void *frame);
void BrightLights_BackendLittleTest(void);
void BrightLights_BackendClearRaster(void);
BrightLightVector BrightLights_BackendMatrixPosition(const void *matrix);
BrightLightVector BrightLights_BackendMatrixRight(const void *matrix);
BrightLightVector BrightLights_BackendMatrixUp(const void *matrix);
void BrightLights_BackendVertexAlpha(bool enabled);
void BrightLights_BackendZWrite(bool enabled);
void BrightLights_BackendSrcBlend(void);
void BrightLights_BackendDestBlend(void);
void BrightLights_BackendSetRGBA(int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void BrightLights_BackendSetPos(int32_t index,float x,float y,float z);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
