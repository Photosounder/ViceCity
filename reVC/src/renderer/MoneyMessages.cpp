//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "MoneyMessages_backend.h"
#include "Sprite.h"
#include "Font.h"

float MoneyMessages_BackendScreenWidth(void)
{
    // Read the original live screen width at its use point
    return SCREEN_WIDTH;
}
float MoneyMessages_BackendScreenHeight(void)
{
    // Read the original live screen height at its use point
    return SCREEN_HEIGHT;
}
void MoneyMessages_BackendFontScale(float x,float y)
{
    // Set the original font scale after the original C calculations
    Font_SetScale(x,y);
}
void MoneyMessages_BackendFontCentreSize(float value)
{
    // Retain the original full-screen centre size
    Font_SetCentreSize(value);
}
void MoneyMessages_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Retain the original color construction and alpha narrowing
    Font_SetColor(CRGBA(r,g,b,a));
}
void MoneyMessages_BackendPrintString(float x,float y,uint16_t *text)
{
    // Enter the exact original mutable Unicode print overload
    Font_PrintString(x,y,text);
}
bool MoneyMessages_BackendJapanese(void)
{
    // Preserve the original otherwise-unused locale expression
    #ifdef MORE_LANGUAGES
    return Font_IsJapanese();
#else
    return false;
#endif
}
void MoneyMessages_BackendFontPropOn(void)
{
    // Retain the original typed font PropOn operation
    Font_SetPropOn();
}
void MoneyMessages_BackendFontBackgroundOff(void)
{
    // Retain the original typed font BackgroundOff operation
    Font_SetBackgroundOff();
}
void MoneyMessages_BackendFontCentreOn(void)
{
    // Retain the original typed font CentreOn operation
    Font_SetCentreOn();
}
void MoneyMessages_BackendFontJustifyOff(void)
{
    // Retain the original typed font JustifyOff operation
    Font_SetJustifyOff();
}
void MoneyMessages_BackendFontBackGroundOnlyTextOff(void)
{
    // Retain the original typed font BackGroundOnlyTextOff operation
    Font_SetBackGroundOnlyTextOff();
}

MoneyMessageVector MoneyMessages_FromVector(CVector position)
{
    // Copy the actual typed components without assuming the CVector object layout
    MoneyMessageVector result={position.x,position.y,position.z};return result;
}
void MoneyMessages_RegisterOneLegacy(CVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity)
{
    // Preserve the original by-value position capture before any text conversion
    MoneyMessages_RegisterOne(MoneyMessages_FromVector(position),text,r,g,b,size,opacity);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
