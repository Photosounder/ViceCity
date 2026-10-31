//+ rouz edit (ChatGPT)
#include "common.h"
#include "Credits_backend.h"
#include "Font.h"
#include "Frontend.h"
#include "RwHelper.h"
#include "Camera.h"
#include "Text.h"
#include "Pad.h"
bool Credits_BackendMenuActive(void)
{
    // Read the original live menu-active flag
    return FrontEndMenuManager.m_bMenuActive;
}
bool Credits_BackendFrench(void)
{
    // Read the original French language predicate
    return FrontEndMenuManager.m_PrefsLanguage==CMenuManager::LANGUAGE_FRENCH;
}
bool Credits_BackendItalian(void)
{
    // Read the original Italian language predicate
    return FrontEndMenuManager.m_PrefsLanguage==CMenuManager::LANGUAGE_ITALIAN;
}
bool Credits_BackendSpanish(void)
{
    // Read the original Spanish language predicate
    return FrontEndMenuManager.m_PrefsLanguage==CMenuManager::LANGUAGE_SPANISH;
}
void Credits_BackendUpdatePads(void)
{
    // Update the original input owner before each credit line
    CPad::UpdatePads();
}
bool Credits_BackendCrossJustDown(void)
{
    // Retain the exact pad lookup and original button predicate
    return CPad::GetPad(0)->GetCrossJustDown();
}
void Credits_BackendDefinedState(void)
{
    // Restore the original defined render state
    DefinedState();
}
uint16_t * Credits_BackendText(const char *key)
{
    // Borrow the actual mutable Unicode text returned for each original key
    return Text_Get(&TheText,key);
}
float Credits_BackendScreenWidth(void)
{
    // Read the original live screen width
    return SCREEN_WIDTH;
}
float Credits_BackendScreenHeight(void)
{
    // Read the original live screen height
    return SCREEN_HEIGHT;
}
void Credits_BackendFontScale(float x,float y)
{
    // Set font scale after the original C screen calculations
    CFont::SetScale(x,y);
}
void Credits_BackendFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Retain the original typed color construction
    CFont::SetColor(CRGBA(r,g,b,a));
}
void Credits_BackendPrintString(float x,float y,uint16_t *text)
{
    // Borrow the original Unicode buffer through the exact print overload
    CFont::PrintString(x,y,text);
}
void Credits_BackendFontCentreSize(float value)
{
    // Set the original C-computed font centre size
    CFont::SetCentreSize(value);
}
void Credits_BackendFontStyle(int16_t style)
{
    // Select the exact original font style
    CFont::SetFontStyle(style);
}
void Credits_BackendDrawFonts(void)
{
    // Flush the original credits font submissions
    CFont::DrawFonts();
}
bool Credits_BackendCutsceneBorders(void)
{
    // Read the original optional cutscene-border preference
    #ifdef CUTSCENE_BORDERS_SWITCH
    return CMenuManager::m_PrefsCutsceneBorders;
#else
    return true;
#endif
}
bool Credits_BackendWidescreen(void)
{
    // Read the current camera widescreen flag
    return TheCamera.m_WideScreenOn;
}
void Credits_BackendDrawBorders(void)
{
    // Draw the original camera widescreen borders
    TheCamera.DrawBordersForWideScreen();
}
void Credits_BackendFontJustifyOff(void)
{
    // Retain the original font JustifyOff operation
    CFont::SetJustifyOff();
}
void Credits_BackendFontBackgroundOff(void)
{
    // Retain the original font BackgroundOff operation
    CFont::SetBackgroundOff();
}
void Credits_BackendFontCentreOn(void)
{
    // Retain the original font CentreOn operation
    CFont::SetCentreOn();
}
void Credits_BackendFontPropOn(void)
{
    // Retain the original font PropOn operation
    CFont::SetPropOn();
}
//- rouz edit (ChatGPT)
