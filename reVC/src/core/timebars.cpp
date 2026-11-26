//+ rouz edit (ChatGPT)
#include "common.h"
#ifndef MASTER
#include "Font.h"
#include "Frontend.h"
#include "timebars_backend.h"

void TimeBars_PrepareFont(void)
{
    // Preserve the original ordered font state setup
    Font_SetBackgroundOff();
    Font_SetBackgroundColor(CRGBA(0, 0, 0, 128));
    Font_SetScale(0.48f, 1.12f);
    Font_SetCentreOff();
    Font_SetJustifyOff();
    Font_SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
    Font_SetRightJustifyOff();
    Font_SetPropOn();
    Font_SetFontStyle(FONT_STANDARD);
}
void TimeBars_SetTextColor(void)
{
    // Select the original opaque white text color
    Font_SetColor(CRGBA(255, 255, 255, 255));
}
bool TimeBars_IsVisible(void)
{
    // Retain the original short-circuited menu preference checks
    return !CMenuManager::m_PrefsMarketing || !CMenuManager::m_PrefsDisableTutorials;
}
void TimeBars_Print(float y, uint16_t *text)
{
    // Scale the original screen coordinates at each print
    Font_PrintString(RsGlobal.maximumWidth * (4.0f / DEFAULT_SCREEN_WIDTH), RsGlobal.maximumHeight * (y / DEFAULT_SCREEN_HEIGHT), text);
}
#endif
//- rouz edit (ChatGPT)
