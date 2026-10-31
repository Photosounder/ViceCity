//+ rouz edit (ChatGPT)
#include "common.h"
#include "VarConsole_backend.h"
#include "Font.h"
#include "Pad.h"
void VarConsole_BackendColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetColor(CRGBA(r,g,b,a));
}
void VarConsole_BackendStyle(void)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetFontStyle(FONT_STANDARD);
}
void VarConsole_BackendScale(float x,float y)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetScale(x,y);
}
float VarConsole_BackendScaleX(float value)
{
    // Preserve the actual typed native font or second-pad operation
    return SCREEN_SCALE_X(value);
}
float VarConsole_BackendScaleY(float value)
{
    // Preserve the actual typed native font or second-pad operation
    return SCREEN_SCALE_Y(value);
}
void VarConsole_BackendShadowPosition(int16_t position)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetDropShadowPosition(position);
}
void VarConsole_BackendShadowColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetDropColor(CRGBA(r,g,b,a));
}
void VarConsole_BackendProp(void)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetPropOn();
}
void VarConsole_BackendWrap(float x)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetWrapx(x);
}
void VarConsole_BackendRightWrap(float x)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetRightJustifyWrap(x);
}
void VarConsole_BackendUnicode(const char *source,uint16_t *target)
{
    // Preserve the actual typed native font or second-pad operation
    AsciiToUnicode(source,target);
}
void VarConsole_BackendRightOn(void)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetRightJustifyOn();
}
void VarConsole_BackendRightOff(void)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetRightJustifyOff();
}
void VarConsole_BackendPrint(float x,float y,uint16_t *text)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::PrintString(x,y,text);
}
void VarConsole_BackendBackgroundOn(void)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetBackgroundOn();
}
void VarConsole_BackendBackgroundOff(void)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetBackgroundOff();
}
void VarConsole_BackendBackgroundColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the actual typed native font or second-pad operation
    CFont::SetBackgroundColor(CRGBA(r,g,b,a));
}
bool VarConsole_BackendDPadDownJustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetDPadDownJustDown();
}
bool VarConsole_BackendAnaloguePadDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetAnaloguePadDown();
}
bool VarConsole_BackendDPadUpJustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetDPadUpJustDown();
}
bool VarConsole_BackendAnaloguePadUp(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetAnaloguePadUp();
}
bool VarConsole_BackendSquare(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetSquare();
}
bool VarConsole_BackendTriangle(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetTriangle();
}
bool VarConsole_BackendDPadLeftJustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetDPadLeftJustDown();
}
bool VarConsole_BackendAnaloguePadLeft(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetAnaloguePadLeft();
}
bool VarConsole_BackendDPadRightJustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetDPadRightJustDown();
}
bool VarConsole_BackendAnaloguePadRight(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetAnaloguePadRight();
}
bool VarConsole_BackendLeftShoulder2JustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetLeftShoulder2JustDown();
}
bool VarConsole_BackendRightShoulder2JustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetRightShoulder2JustDown();
}
bool VarConsole_BackendRightShoulder1JustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetRightShoulder1JustDown();
}
bool VarConsole_BackendLeftShoulder1JustDown(void)
{
    // Preserve the actual typed native font or second-pad operation
    return CPad::GetPad(1)->GetLeftShoulder1JustDown();
}
//- rouz edit (ChatGPT)
