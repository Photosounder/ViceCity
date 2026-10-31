//+ rouz edit (ChatGPT)
#pragma once
#include "MotionBlurStreaks.h"
#ifdef __cplusplus
extern "C" {
#endif
int32_t MotionBlurStreaks_BackendFogRed(void);
int32_t MotionBlurStreaks_BackendFogGreen(void);
int32_t MotionBlurStreaks_BackendFogBlue(void);
void MotionBlurStreaks_BackendFogColor(uint32_t color);
void MotionBlurStreaks_BackendSrcBlend(void);
void MotionBlurStreaks_BackendDestBlend(void);
void MotionBlurStreaks_BackendRaster(void);
void MotionBlurStreaks_BackendLittleTest(void);
bool MotionBlurStreaks_BackendTransform(void);
void MotionBlurStreaks_BackendRenderIndexed(uint16_t *indices);
void MotionBlurStreaks_BackendEnd(void);
void MotionBlurStreaks_BackendZWrite(bool value);
void MotionBlurStreaks_BackendVertexAlpha(bool value);
void MotionBlurStreaks_BackendFog(bool value);
void MotionBlurStreaks_BackendSetU(int index,float value);
void MotionBlurStreaks_BackendSetV(int index,float value);
void MotionBlurStreaks_BackendSetRGBA(int index,int r,int g,int b,int a);
void MotionBlurStreaks_BackendSetPos(int index,float x,float y,float z);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
