//+ rouz edit (ChatGPT)
#include "common.h"
#ifndef MASTER
#include "Font.h"
#include "Frontend.h"
#include "timebars_backend.h"

void TimeBars_PrepareFont(void)
{
    // Preserve the original ordered font state setup
    CFont::SetBackgroundOff();
    CFont::SetBackgroundColor(CRGBA(0, 0, 0, 128));
    CFont::SetScale(0.48f, 1.12f);
    CFont::SetCentreOff();
    CFont::SetJustifyOff();
    CFont::SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
    CFont::SetRightJustifyOff();
    CFont::SetPropOn();
    CFont::SetFontStyle(FONT_STANDARD);
}
void TimeBars_SetTextColor(void)
{
    // Select the original opaque white text color
    CFont::SetColor(CRGBA(255, 255, 255, 255));
}
bool TimeBars_IsVisible(void)
{
    // Retain the original short-circuited menu preference checks
    return !CMenuManager::m_PrefsMarketing || !CMenuManager::m_PrefsDisableTutorials;
}
void TimeBars_Print(float y, uint16_t *text)
{
    // Scale the original screen coordinates at each print
    CFont::PrintString(RsGlobal.maximumWidth * (4.0f / DEFAULT_SCREEN_WIDTH), RsGlobal.maximumHeight * (y / DEFAULT_SCREEN_HEIGHT), text);
}
#endif
//- rouz edit (ChatGPT)
