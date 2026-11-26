//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma once

#include "Sprite2d.h"

#include "FontText.h" // rouz edit (ChatGPT)

struct CFontDetails
{
	CRGBA color;
	float scaleX;
	float scaleY;
	float slant;
	float slantRefX;
	float slantRefY;
	bool8 justify;
	bool8 centre;
	bool8 rightJustify;
	bool8 background;
	bool8 backgroundOnlyText;
	bool8 proportional;
	bool8 bIsShadow;
	bool8 bFlash;
	bool8 bBold;
	float alphaFade;
	CRGBA backgroundColor;
	float wrapX;
	float centreSize;
	float rightJustifyWrap;
	int16 style;
	bool8 bFontHalfTexture;
	uint32 bank;
	int16 dropShadowPosition;
	CRGBA dropColor;
	bool8 bFlashState;
	int nFlashTimer;
	bool8 anonymous_23;
	uint32 anonymous_25;
};

struct CFontRenderState
{
	uint32 anonymous_0;
	float fTextPosX;
	float fTextPosY;
	float scaleX;
	float scaleY;
	CRGBA color;
	float fExtraSpace;
	float slant;
	float slantRefX;
	float slantRefY;
	bool8 bIsShadow;
	bool8 bFontHalfTexture;
	bool8 proportional;
	bool8 anonymous_14;
	int16 style;
};

struct Sprite2d;

#include "FontStyles.h"

enum {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
};

#ifdef MORE_LANGUAGES
enum
{
	FONT_LANGSET_EFIGS,
	FONT_LANGSET_RUSSIAN,
	FONT_LANGSET_POLISH,
	FONT_LANGSET_JAPANESE,
	LANGSET_MAX
};

#define FONT_LOCALE(style) (Font_IsJapanese() ? FONT_JAPANESE : style)
#else
#define FONT_LOCALE(style) (style)
#endif

#ifdef BUTTON_ICONS
enum
{
	BUTTON_NONE = -1,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_CROSS,
	BUTTON_CIRCLE,
	BUTTON_SQUARE,
	BUTTON_TRIANGLE,
	BUTTON_L1,
	BUTTON_L2,
	BUTTON_L3,
	BUTTON_R1,
	BUTTON_R2,
	BUTTON_R3,
	BUTTON_RSTICK_UP,
	BUTTON_RSTICK_DOWN,
	BUTTON_RSTICK_LEFT,
	BUTTON_RSTICK_RIGHT,
	MAX_BUTTON_ICONS
};
#endif // BUTTON_ICONS


//+ rouz edit (Claude)
// Formerly the static class CFont, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
#ifdef MORE_LANGUAGES
extern int16 Font_Size[LANGSET_MAX][MAX_FONTS][210];
extern uint8 Font_LanguageSet;
extern int32 Font_Slot;
#else
extern int16 Font_Size[MAX_FONTS][210];
#endif
extern bool16 Font_NewLine;
extern Sprite2d Font_Sprite[MAX_FONTS];
#ifdef BUTTON_ICONS
extern int32 Font_ButtonsSlot;
extern int Font_PS2Symbol;
void Font_LoadButtons(const char *txdPath);
void Font_DrawButton(float x, float y);
#endif // BUTTON_ICONS
void Font_Initialise(void);
void Font_Shutdown(void);
void Font_InitPerFrame(void);
void Font_PrintChar(float x, float y, wchar c);
#ifdef XBOX_SUBTITLES
void Font_PrintOutlinedString(float x, float y, wchar *str, float outlineStrength, bool fromBottom, CRGBA outlineColor);
#endif
int Font_GetNumberLines(float xstart, float ystart, wchar *s);
void Font_GetTextRect(CRect *rect, float xstart, float ystart, wchar *s);
float Font_GetCharacterWidth(wchar c);
float Font_GetCharacterSize(wchar c);
#ifdef MORE_LANGUAGES
float Font_GetStringWidth_Jap(wchar* s);
#endif
uint16 *Font_GetNextSpace(wchar *s);
void Font_DrawFonts(void);
void Font_RenderFontBuffer(void);
uint16 Font_character_code(uint8 c);
void Font_SetScale(float x, float y);
void Font_SetSlantRefPoint(float x, float y);
void Font_SetSlant(float s);
void Font_SetJustifyOn(void);
void Font_SetJustifyOff(void);
void Font_SetRightJustifyOn(void);
void Font_SetRightJustifyOff(void);
void Font_SetCentreOn(void);
void Font_SetCentreOff(void);
void Font_SetWrapx(float x);
void Font_SetCentreSize(float s);
void Font_SetBackgroundOn(void);
void Font_SetBackgroundOff(void);
void Font_SetBackGroundOnlyTextOn(void);
void Font_SetBackGroundOnlyTextOff(void);
void Font_SetPropOn(void);
void Font_SetPropOff(void);
void Font_SetFontStyle(int16 style);
void Font_SetRightJustifyWrap(float wrap);
void Font_SetAlphaFade(float fade);
void Font_SetDropShadowPosition(int16 pos);
void Font_SetBackgroundColor(CRGBA col);
void Font_SetColor(CRGBA col);
void Font_SetDropColor(CRGBA col);
wchar Font_FindNewCharacter(wchar c);
void Font_FilterOutTokensFromString(wchar*);
#ifdef MORE_LANGUAGES
void Font_ReloadFonts(uint8 set);
bool Font_IsAnsiCharacter(wchar* s);
bool Font_IsJapanesePunctuation(wchar* str);
#endif
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
#ifdef MORE_LANGUAGES
#else
#endif
extern CFontDetails Font_Details;
extern CFontRenderState Font_RenderState;
#ifdef BUTTON_ICONS
#endif // BUTTON_ICONS
void Font_PrintString(float x, float y, wchar *s);
#ifdef XBOX_SUBTITLES
void Font_PrintStringFromBottom(float x, float y, wchar *str);
#endif
void Font_PrintString(float x, float y, uint32, wchar *start, wchar *end, float spwidth);
void Font_PrintStringFromBottom(float x, float y, wchar *str);
float Font_GetStringWidth(wchar *s, bool spaces = false);
#ifdef MORE_LANGUAGES
#endif
uint16 *Font_ParseToken(wchar *s);
uint16 *Font_ParseToken(wchar *s, CRGBA &color, bool &flash, bool &bold);
#ifdef MORE_LANGUAGES
static inline bool Font_IsJapanese();
static inline bool Font_IsJapaneseFont();
#endif
#ifdef MORE_LANGUAGES
static inline bool Font_IsJapanese() { return Font_LanguageSet == FONT_LANGSET_JAPANESE; }
#endif
#ifdef MORE_LANGUAGES
static inline bool Font_IsJapaneseFont() { return Font_IsJapanese() && (Font_Details.style == FONT_JAPANESE);  }
#endif
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
