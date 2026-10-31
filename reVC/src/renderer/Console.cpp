//+ rouz edit (ChatGPT)
#include "common.h"
#include "Console_backend.h"
#include "Font.h"
int32_t Console_BackendScreenWidth(void)
{
    // Read the original live integer screen width
    return RsGlobal.width;
}
void Console_BackendFontScale(float x,float y)
{
    // Retain the original fixed font scale
    CFont::SetScale(x,y);
}
void Console_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Construct the original typed font color from live C record bytes
    CFont::SetColor(CRGBA(r,g,b,a));
}
void Console_BackendPrintString(float x,float y,uint16_t *text)
{
    // Borrow the actual mutable C Unicode row through the original print overload
    CFont::PrintString(x,y,text);
}
void Console_BackendFontStyle(int16_t style)
{
    // Select the original font style
    CFont::SetFontStyle(style);
}
void Console_BackendFontRightJustifyWrap(float wrap)
{
    // Retain the original right-justify wrap write
    CFont::SetRightJustifyWrap(wrap);
}
void Console_BackendFontWrapx(float wrap)
{
    // Retain the original live screen-width wrap value
    CFont::SetWrapx(wrap);
}
void Console_BackendFontPropOn(void)
{
    // Retain the original font PropOn operation
    CFont::SetPropOn();
}
void Console_BackendFontBackgroundOff(void)
{
    // Retain the original font BackgroundOff operation
    CFont::SetBackgroundOff();
}
void Console_BackendFontCentreOff(void)
{
    // Retain the original font CentreOff operation
    CFont::SetCentreOff();
}
void Console_BackendFontRightJustifyOff(void)
{
    // Retain the original font RightJustifyOff operation
    CFont::SetRightJustifyOff();
}
void Console_BackendFontJustifyOn(void)
{
    // Retain the original font JustifyOn operation
    CFont::SetJustifyOn();
}
void Console_BackendFontBackGroundOnlyTextOff(void)
{
    // Retain the original font BackGroundOnlyTextOff operation
    CFont::SetBackGroundOnlyTextOff();
}
void Console_BackendFontPropOff(void)
{
    // Retain the original font PropOff operation
    CFont::SetPropOff();
}
//- rouz edit (ChatGPT)
