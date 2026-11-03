//+ rouz edit (ChatGPT)
#pragma once
#include "MoneyMessages.h"
#ifdef __cplusplus
extern "C" {
#endif
bool MoneyMessages_BackendProject(MoneyMessageVector position,MoneyMessageVector *out,float *width,float *height,bool farclip);
float MoneyMessages_BackendScreenWidth(void);
float MoneyMessages_BackendScreenHeight(void);
void MoneyMessages_BackendFontScale(float x,float y);
void MoneyMessages_BackendFontCentreSize(float value);
void MoneyMessages_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void MoneyMessages_BackendPrintString(float x,float y,uint16_t *text);
bool MoneyMessages_BackendJapanese(void);
void MoneyMessages_BackendFontPropOn(void);
void MoneyMessages_BackendFontBackgroundOff(void);
void MoneyMessages_BackendFontCentreOn(void);
void MoneyMessages_BackendFontJustifyOff(void);
void MoneyMessages_BackendFontBackGroundOnlyTextOff(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
