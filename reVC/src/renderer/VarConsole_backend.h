//+ rouz edit (ChatGPT)
#pragma once
#include "VarConsole.h"
#ifdef __cplusplus
extern "C" {
#endif
void VarConsole_BackendColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void VarConsole_BackendStyle(void);
void VarConsole_BackendScale(float x,float y);
float VarConsole_BackendScaleX(float value);
float VarConsole_BackendScaleY(float value);
void VarConsole_BackendShadowPosition(int16_t position);
void VarConsole_BackendShadowColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void VarConsole_BackendProp(void);
void VarConsole_BackendWrap(float x);
void VarConsole_BackendRightWrap(float x);
void VarConsole_BackendUnicode(const char *source,uint16_t *target);
void VarConsole_BackendRightOn(void);
void VarConsole_BackendRightOff(void);
void VarConsole_BackendPrint(float x,float y,uint16_t *text);
void VarConsole_BackendBackgroundOn(void);
void VarConsole_BackendBackgroundOff(void);
void VarConsole_BackendBackgroundColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
bool VarConsole_BackendDPadDownJustDown(void);
bool VarConsole_BackendAnaloguePadDown(void);
bool VarConsole_BackendDPadUpJustDown(void);
bool VarConsole_BackendAnaloguePadUp(void);
bool VarConsole_BackendSquare(void);
bool VarConsole_BackendTriangle(void);
bool VarConsole_BackendDPadLeftJustDown(void);
bool VarConsole_BackendAnaloguePadLeft(void);
bool VarConsole_BackendDPadRightJustDown(void);
bool VarConsole_BackendAnaloguePadRight(void);
bool VarConsole_BackendLeftShoulder2JustDown(void);
bool VarConsole_BackendRightShoulder2JustDown(void);
bool VarConsole_BackendRightShoulder1JustDown(void);
bool VarConsole_BackendLeftShoulder1JustDown(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
