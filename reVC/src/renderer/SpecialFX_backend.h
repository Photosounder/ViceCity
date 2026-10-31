//+ rouz edit (ChatGPT)
#pragma once
#include "SpecialFX_control.h"
#include "MotionBlurStreaks.h"
typedef struct SpecialFXMatrix { float rx,ry,rz,fx,fy,fz,ux,uy,uz,px,py,pz; } SpecialFXMatrix;
#ifdef __cplusplus
extern "C" {
#endif
void * SpecialFX_BackendWeaponMatrix(void);
void * SpecialFX_BackendCallbackMatrix(void);
void SpecialFX_BackendSetMatrix(void *target,void *source);
SpecialFXMatrix SpecialFX_BackendReadMatrix(void *matrix);
void * SpecialFX_BackendPlayerPed(void);
void * SpecialFX_BackendWeapon(void *ped);
void * SpecialFX_BackendAtomicFrame(void *atomic);
void * SpecialFX_BackendFrameLTM(void *frame);
void * SpecialFX_BackendModelInfo(void *object);
float SpecialFX_BackendScreenWidth(void);
float SpecialFX_BackendScreenHeight(void);
char * SpecialFX_BackendString(void);
uint16_t * SpecialFX_BackendUnicode(void);
void SpecialFX_BackendAsciiToUnicode(const char *source,uint16_t *target);
void SpecialFX_BackendFontScale(float x,float y);
void SpecialFX_BackendFontCentreSize(float value);
void SpecialFX_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void SpecialFX_BackendFontStyle(int16_t style);
bool SpecialFX_BackendJapanese(void);
void SpecialFX_BackendPrintString(float x,float y,uint16_t *text);
void SpecialFX_BackendPolygon(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void SpecialFX_BackendRect(float left,float top,float right,float bottom,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void SpecialFX_BackendPush(const char *name);
void SpecialFX_BackendPop(void);
bool SpecialFX_BackendNewRenderer(void);
bool SpecialFX_BackendFirstPerson(void);
void SpecialFX_BackendFontJustifyOff(void);
void SpecialFX_BackendFontBackgroundOff(void);
void SpecialFX_BackendFontCentreOff(void);
void SpecialFX_BackendFontPropOn(void);
void SpecialFX_BackendSrcBlend(bool additive);
void SpecialFX_BackendDestBlend(bool additive);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
