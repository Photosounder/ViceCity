//+ rouz edit (ChatGPT)
#include "Debug_backend.h"
#include "Lines.h"
#include "FontStyles.h"
#include <string.h>

bool gbDebugStuffInRelease = false;

#define DEBUG_X_POS (300)
#define DEBUG_Y_POS (41)
#define DEBUG_LINE_HEIGHT (22)

int16_t Debug_ms_nCurrentTextLine;
char Debug_ms_aTextBuffer[Debug_MAX_LINES][Debug_MAX_STR_LEN];

void
Debug_DebugInitTextBuffer(void)
{
    // Preserve the complete original DebugInitTextBuffer operation and every live queue access
	Debug_ms_nCurrentTextLine = 0;
}

void
Debug_DebugAddText(const char *str)
{
    // Preserve the complete original DebugAddText operation and every live queue access
	int32_t i = 0;
	if (*str != '\0') {
		while (i < Debug_MAX_STR_LEN - 1) {
			Debug_ms_aTextBuffer[Debug_ms_nCurrentTextLine][i++] = *(str++);
			if (*str == '\0')
				break;
		}
	}

	Debug_ms_aTextBuffer[Debug_ms_nCurrentTextLine++][i] = '\0';
	if (Debug_ms_nCurrentTextLine >= Debug_MAX_LINES)
		Debug_ms_nCurrentTextLine = 0;
}

void
Debug_DebugDisplayTextBuffer(void)
{
    // Preserve the complete original DebugDisplayTextBuffer operation and every live queue access
#ifndef MASTER
	if (gbDebugStuffInRelease)
	{
		int32_t i = 0;
		int32_t y = DEBUG_Y_POS;
#ifdef FIX_BUGS
		Debug_BackendSetPropOn();
		Debug_BackendSetBackgroundOff();
		Debug_BackendSetScale(1.0f, 1.0f);
		Debug_BackendSetCentreOff();
		Debug_BackendSetRightJustifyOff();
		Debug_BackendSetJustifyOn();
		Debug_BackendSetRightJustifyWrap(0.0f);
		Debug_BackendSetBackGroundOnlyTextOff();
		Debug_BackendSetFontStyle(FONT_STANDARD);
#else
		// this is not even readable
		Debug_BackendSetPropOff();
		Debug_BackendSetBackgroundOff();
		Debug_BackendSetScale(1.0f, 1.0f);
		Debug_BackendSetCentreOff();
		Debug_BackendSetRightJustifyOn();
		Debug_BackendSetRightJustifyWrap(0.0f);
		Debug_BackendSetBackGroundOnlyTextOff();
		Debug_BackendSetFontStyle(FONT_STANDARD);
		Debug_BackendSetPropOff();
#endif
		do {
			char *line;
			while (true) {
				line = Debug_ms_aTextBuffer[(Debug_ms_nCurrentTextLine + i++) % Debug_MAX_LINES];
				if (*line != '\0')
					break;
				y += DEBUG_LINE_HEIGHT;
				if (i == Debug_MAX_LINES) {
					Debug_BackendDrawFonts();
					return;
				}
			}
			Debug_BackendAsciiToUnicode(line, Debug_BackendUnicode());
			Debug_BackendColor(0, 0, 0, 255);
			Debug_BackendPrintString(DEBUG_X_POS, y-1, Debug_BackendUnicode());
			Debug_BackendColor(255, 128, 128, 255);
			Debug_BackendPrintString(DEBUG_X_POS+1, y, Debug_BackendUnicode());
			y += DEBUG_LINE_HEIGHT;
		} while (i != Debug_MAX_LINES);
		Debug_BackendDrawFonts();
	}
#endif
}


// custom

Debug_ScreenStr Debug_ms_aScreenStrs[Debug_MAX_SCREEN_STRS];
int Debug_ms_nScreenStrs;

void
Debug_DisplayScreenStrings(void)
{
    // Preserve the complete original DisplayScreenStrings operation and every live queue access
	int i;
	

	Debug_BackendSetPropOn();
	Debug_BackendSetBackgroundOff();
	Debug_BackendSetScale(1.0f, 1.0f);
	Debug_BackendSetCentreOff();
	Debug_BackendSetRightJustifyOff();
	Debug_BackendSetJustifyOff();
	Debug_BackendSetRightJustifyWrap(0.0f);
	Debug_BackendSetWrapx(9999.0f);
	Debug_BackendSetBackGroundOnlyTextOff();
	Debug_BackendSetFontStyle(FONT_STANDARD);

	for(i = 0; i < Debug_ms_nScreenStrs; i++){
/*
		Debug_BackendAsciiToUnicode(Debug_ms_aScreenStrs[i].str, Debug_BackendUnicode());
		Debug_BackendColor(0, 0, 0, 255);
		Debug_BackendPrintString(Debug_ms_aScreenStrs[i].x, Debug_ms_aScreenStrs[i].y, Debug_BackendUnicode());
		Debug_BackendColor(255, 255, 255, 255);
		Debug_BackendPrintString(Debug_ms_aScreenStrs[i].x+1, Debug_ms_aScreenStrs[i].y+1, Debug_BackendUnicode());
*/
		Debug_BackendObrsPrintfString(Debug_ms_aScreenStrs[i].str, Debug_ms_aScreenStrs[i].x, Debug_ms_aScreenStrs[i].y);
	}
	Debug_BackendDrawFonts();

	Debug_ms_nScreenStrs = 0;
}

void
Debug_PrintAt(const char *str, int x, int y)
{
    // Preserve the complete original PrintAt operation and every live queue access
	if(Debug_ms_nScreenStrs >= Debug_MAX_SCREEN_STRS)
		return;
	strncpy(Debug_ms_aScreenStrs[Debug_ms_nScreenStrs].str, str, 256);
	Debug_ms_aScreenStrs[Debug_ms_nScreenStrs].x = x;//*12;
	Debug_ms_aScreenStrs[Debug_ms_nScreenStrs].y = y;//*22;
	Debug_ms_nScreenStrs++;
}

Debug_Line Debug_ms_aLines[Debug_MAX_DEBUG_LINES];
int Debug_ms_nLines;

void
Debug_AddLine(Debug_Vector p1, Debug_Vector p2, uint32_t c1, uint32_t c2)
{
    // Preserve the complete original AddLine operation and every live queue access
	if(Debug_ms_nLines >= Debug_MAX_DEBUG_LINES)
		return;
	Debug_ms_aLines[Debug_ms_nLines].p1 = p1;
	Debug_ms_aLines[Debug_ms_nLines].p2 = p2;
	Debug_ms_aLines[Debug_ms_nLines].c1 = c1;
	Debug_ms_aLines[Debug_ms_nLines].c2 = c2;
	Debug_ms_nLines++;
}

void
Debug_DrawLines(void)
{
    // Preserve the complete original DrawLines operation and every live queue access
	int i;
	Debug_BackendTextureNone();
	Debug_BackendDepthTestOff();
	for(i = 0; i < Debug_ms_nLines; i++){
		Debug_Line *l = &Debug_ms_aLines[i];
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(l->p1.x, l->p1.y, l->p1.z, l->p2.x, l->p2.y, l->p2.z, l->c1, l->c2);
	}
	Debug_ms_nLines = 0;
}
//- rouz edit (ChatGPT)
