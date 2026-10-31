//+ rouz edit (ChatGPT)
#pragma once
#include "Timecycle.h"
enum {TIMECYCLE_MOTION_BLUR_NONE=0,TIMECYCLE_MOTION_BLUR_LIGHT_SCENE=2};
#ifdef __cplusplus
extern "C" {
#endif
void TimeCycle_BackendDebug(const char *message);
uint8_t (*TimeCycle_BackendWorkBuffer(void))[55000];
int32_t TimeCycle_BackendBlurType(void);
void TimeCycle_BackendSetMotionBlur(int32_t red,int32_t green,int32_t blue,int32_t alpha,int32_t type);
void TimeCycle_BackendShadowValues(TimeCycleVector direction,float *frontX,float *frontY,float *sideX,float *sideY,float *displacementX,float *displacementY);
float TimeCycle_BackendCameraForwardZ(void);
bool TimeCycle_BackendCutscene(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
