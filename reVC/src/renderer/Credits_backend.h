//+ rouz edit (ChatGPT)
#pragma once
#include "Credits.h"
#ifdef __cplusplus
extern "C" {
#endif
bool Credits_BackendMenuActive(void);
bool Credits_BackendFrench(void);
bool Credits_BackendItalian(void);
bool Credits_BackendSpanish(void);
void Credits_BackendUpdatePads(void);
bool Credits_BackendCrossJustDown(void);
void Credits_BackendDefinedState(void);
uint16_t * Credits_BackendText(const char *key);
float Credits_BackendScreenWidth(void);
float Credits_BackendScreenHeight(void);
void Credits_BackendFontScale(float x,float y);
void Credits_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void Credits_BackendPrintString(float x,float y,uint16_t *text);
void Credits_BackendFontCentreSize(float value);
void Credits_BackendFontStyle(int16_t style);
void Credits_BackendDrawFonts(void);
bool Credits_BackendCutsceneBorders(void);
bool Credits_BackendWidescreen(void);
void Credits_BackendDrawBorders(void);
void Credits_BackendFontJustifyOff(void);
void Credits_BackendFontBackgroundOff(void);
void Credits_BackendFontCentreOn(void);
void Credits_BackendFontPropOn(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
