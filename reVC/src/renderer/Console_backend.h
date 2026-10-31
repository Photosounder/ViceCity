//+ rouz edit (ChatGPT)
#pragma once
#include "Console.h"
#ifdef __cplusplus
extern "C" {
#endif
int32_t Console_BackendScreenWidth(void);
void Console_BackendFontScale(float x,float y);
void Console_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void Console_BackendPrintString(float x,float y,uint16_t *text);
void Console_BackendFontStyle(int16_t style);
void Console_BackendFontRightJustifyWrap(float wrap);
void Console_BackendFontWrapx(float wrap);
void Console_BackendFontPropOn(void);
void Console_BackendFontBackgroundOff(void);
void Console_BackendFontCentreOff(void);
void Console_BackendFontRightJustifyOff(void);
void Console_BackendFontJustifyOn(void);
void Console_BackendFontBackGroundOnlyTextOff(void);
void Console_BackendFontPropOff(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
