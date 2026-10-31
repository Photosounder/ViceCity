//+ rouz edit (ChatGPT)
#pragma once
#include "ShinyTexts.h"
#ifdef __cplusplus
extern "C" {
#endif
ShinyTextVector ShinyTexts_BackendCamera(void);
bool ShinyTexts_BackendEnvMap(void);
void * ShinyTexts_BackendEnvCamera(void);
void * ShinyTexts_BackendEnvFrame(void *camera);
void * ShinyTexts_BackendFrameLTM(void *frame);
ShinyTextVector ShinyTexts_BackendMatrixPosition(void *matrix);
void * ShinyTexts_BackendWalkTexture(void);
void * ShinyTexts_BackendRaster(void *texture);
void ShinyTexts_BackendSetRaster(void *raster);
void ShinyTexts_BackendLittleTest(void);
void ShinyTexts_BackendVertexAlpha(bool enabled);
void ShinyTexts_BackendZWrite(bool enabled);
void ShinyTexts_BackendSrcBlend(bool additive);
void ShinyTexts_BackendDestBlend(bool additive);
void ShinyTexts_BackendSetRGBA(int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void ShinyTexts_BackendSetPos(int32_t index,float x,float y,float z);
void ShinyTexts_BackendSetU(int32_t index,float value);
void ShinyTexts_BackendSetV(int32_t index,float value);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
