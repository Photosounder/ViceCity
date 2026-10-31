//+ rouz edit (ChatGPT)
#include "Console_backend.h"
#include "Timer.h"
#include "FontStyles.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define CONSOLE_X_POS (30.0f)
#define CONSOLE_Y_POS (10.0f)
#define CONSOLE_LINE_HEIGHT (12.0f)

Console TheConsole;

void
Console_AddLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b)
{
    // Split the live input with the original forty-byte copy and thirty-nine-byte advance
	char tempstr[CONSOLE_MAX_STR_LEN+1];

	while (strlen(s) > CONSOLE_MAX_STR_LEN) {
		// Preserve the original loop reads, writes and iteration order
		strncpy(tempstr, s, CONSOLE_MAX_STR_LEN);
		tempstr[CONSOLE_MAX_STR_LEN-1] = '\0';
		s += CONSOLE_MAX_STR_LEN - 1;
		Console_AddOneLine(console,tempstr, r, g, b);
	}
	Console_AddOneLine(console,s, r, g, b);
}

void
Console_AddOneLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b)
{
    // Copy each input byte directly into the live Unicode row before publishing metadata
	int32_t StrIndex = (console->m_nLineCount + console->m_nCurrentLine) % CONSOLE_MAX_LINES;

	for (int32_t i = 0; i < CONSOLE_MAX_STR_LEN; i++) {
		// Preserve the original loop reads, writes and iteration order
		console->Buffers[StrIndex][i] = s[i];
		if (s[i] == '\0') break;
	}

	uint8_t _strNum1 = console->m_nLineCount;
	if (_strNum1 < CONSOLE_MAX_LINES)
		_strNum1++;

	console->m_aTimer[StrIndex] = (*Timer_GetTimeInMilliseconds());
	console->Buffers[StrIndex][CONSOLE_MAX_STR_LEN-1] = '\0';
	console->m_aRed[StrIndex] = r;
	console->m_aGreen[StrIndex] = g;
	console->m_aBlue[StrIndex] = b;

	if (_strNum1 >= CONSOLE_MAX_LINES)
		console->m_nCurrentLine = (console->m_nCurrentLine + 1) % CONSOLE_MAX_LINES;
	else
		console->m_nLineCount = _strNum1;

}

void
Console_Display(Console *console)
{
    // Set original font state before expiring and drawing the live ring contents
	Console_BackendFontPropOn();
	Console_BackendFontBackgroundOff();
	Console_BackendFontScale(0.6f, 0.6f); 
	Console_BackendFontCentreOff();
	Console_BackendFontRightJustifyOff();
	Console_BackendFontJustifyOn();
	Console_BackendFontRightJustifyWrap(0.0f);
	Console_BackendFontBackGroundOnlyTextOff();
	Console_BackendFontStyle(FONT_STANDARD);
#ifndef FIX_BUGS
	Console_BackendFontPropOff(); // not sure why this is here anyway
#endif
	Console_BackendFontWrapx(Console_BackendScreenWidth());

	while (console->m_nLineCount != 0 && (*Timer_GetTimeInMilliseconds()) - console->m_aTimer[console->m_nCurrentLine] > 20000) {
		// Preserve the original loop reads, writes and iteration order
		console->m_nLineCount--;
		console->m_nCurrentLine = (console->m_nCurrentLine + 1) % CONSOLE_MAX_LINES;
	}

	for (int16_t i = 0; i < console->m_nLineCount; i++) {
		// Preserve the original loop reads, writes and iteration order
		int16_t line = (i + console->m_nCurrentLine) % CONSOLE_MAX_LINES;
		Console_BackendFontColor(0, 0, 0, 200);
		Console_BackendPrintString(CONSOLE_X_POS + 1.0f, CONSOLE_Y_POS + 1.0f + i * CONSOLE_LINE_HEIGHT, console->Buffers[line]);
		Console_BackendFontColor(console->m_aRed[line], console->m_aGreen[line], console->m_aBlue[line], 200);
		Console_BackendPrintString(CONSOLE_X_POS, CONSOLE_Y_POS + i * CONSOLE_LINE_HEIGHT, console->Buffers[line]);
	}
}

void
cprintf(char* format, ...)
{
    // Retain the original variadic formatting and singleton insertion sequence
	char s[256];
	va_list vl1, vl2;

	va_start(vl1, format);
	va_copy(vl2, vl1);
	vsprintf(s, format, vl1);
	Console_AddLine(&TheConsole,s, 255, 255, 128);
}

void Console_Init(Console *console)
{
    // Reset only the two original counters and retain all row, timer and color bytes
    console->m_nCurrentLine = 0; console->m_nLineCount = 0;
}
//- rouz edit (ChatGPT)
