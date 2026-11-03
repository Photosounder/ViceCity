//+ rouz edit (ChatGPT)
#include "common.h"
#include "Debug_backend.h"
#include "Font.h"
#include "RwHelper.h"
#include "main.h"
#include "Text.h"
Debug_Vector Debug_FromVector(CVector value)
{
    // Copy the original three float components at the native argument boundary
    Debug_Vector result={value.x,value.y,value.z};return result;
}
void Debug_BackendSetPropOn(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetPropOn();
}

void Debug_BackendSetPropOff(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetPropOff();
}

void Debug_BackendSetBackgroundOff(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetBackgroundOff();
}

void Debug_BackendSetCentreOff(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetCentreOff();
}

void Debug_BackendSetRightJustifyOn(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetRightJustifyOn();
}

void Debug_BackendSetRightJustifyOff(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetRightJustifyOff();
}

void Debug_BackendSetJustifyOn(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetJustifyOn();
}

void Debug_BackendSetJustifyOff(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetJustifyOff();
}

void Debug_BackendSetBackGroundOnlyTextOff(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetBackGroundOnlyTextOff();
}

void Debug_BackendDrawFonts(void)
{
    // Access the original native primitive at its original C policy access point
    CFont::DrawFonts();
}

void Debug_BackendSetScale(float x,float y)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetScale(x,y);
}

void Debug_BackendSetRightJustifyWrap(float wrap)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetRightJustifyWrap(wrap);
}

void Debug_BackendSetWrapx(float wrap)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetWrapx(wrap);
}

void Debug_BackendSetFontStyle(int16_t style)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetFontStyle(style);
}

void Debug_BackendColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Access the original native primitive at its original C policy access point
    CFont::SetColor(CRGBA(r,g,b,a));
}

void Debug_BackendPrintString(float x,float y,uint16_t *text)
{
    // Access the original native primitive at its original C policy access point
    CFont::PrintString(x,y,text);
}



void Debug_BackendObrsPrintfString(const char *str,int16_t x,int16_t y)
{
    // Access the original native primitive at its original C policy access point
    ObrsPrintfString(str,x,y);
}

void Debug_BackendTextureNone(void)
{
    // Access the original native primitive at its original C policy access point
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);
}

void Debug_BackendDepthTestOff(void)
{
    // Access the original native primitive at its original C policy access point
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void*)FALSE);
}
//- rouz edit (ChatGPT)
