//+ rouz edit (ChatGPT)
#pragma once
#include "Debug.h"
#ifdef __cplusplus
extern "C" {
#endif
void Debug_BackendSetPropOn(void);
void Debug_BackendSetPropOff(void);
void Debug_BackendSetBackgroundOff(void);
void Debug_BackendSetCentreOff(void);
void Debug_BackendSetRightJustifyOn(void);
void Debug_BackendSetRightJustifyOff(void);
void Debug_BackendSetJustifyOn(void);
void Debug_BackendSetJustifyOff(void);
void Debug_BackendSetBackGroundOnlyTextOff(void);
void Debug_BackendDrawFonts(void);
void Debug_BackendSetScale(float x,float y);
void Debug_BackendSetRightJustifyWrap(float wrap);
void Debug_BackendSetWrapx(float wrap);
void Debug_BackendSetFontStyle(int16_t style);
void Debug_BackendColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void Debug_BackendPrintString(float x,float y,uint16_t *text);
void Debug_BackendObrsPrintfString(const char *str,int16_t x,int16_t y);
void Debug_BackendTextureNone(void);
void Debug_BackendDepthTestOff(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
