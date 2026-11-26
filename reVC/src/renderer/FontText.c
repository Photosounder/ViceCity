//+ rouz edit (ChatGPT)
#include "FontText.h"

void
AsciiToUnicode(const char *src, uint16_t *dst)
{
    // Preserve the original text conversion and copying behavior
	while((*dst++ = (unsigned char)*src++) != '\0');
}

void
UnicodeStrcat(uint16_t *dst, uint16_t *append)
{
    // Preserve the original text conversion and copying behavior
	UnicodeStrcpy(&dst[UnicodeStrlen(dst)], append);
}

void
UnicodeStrcpy(uint16_t *dst, const uint16_t *src)
{
    // Preserve the original text conversion and copying behavior
	while((*dst++ = *src++) != '\0');
}

int
UnicodeStrlen(const uint16_t *str)
{
    // Preserve the original text conversion and copying behavior
	int len;
	for(len = 0; *str != '\0'; len++, str++);
	return len;
}

void
UnicodeMakeUpperCase(uint16_t *dst, const uint16_t *src) //idk what to do with it, seems to be incorrect implementation by R*
{
    // Preserve the original text conversion and copying behavior
	while (*src != '\0') {
		if (*src < 'a' || *src > 'z')
			*dst = *src;
		else
			*dst = *src - 32;
		dst++;
		src++;
	}
	*dst = '\0';
}

uint16_t FontText_FindNewCharacter(uint16_t c)
{
	// Symbols and letters live at different places in the half size texture
	if (c >= 16 && c <= 26) return c + 128;
	if (c >= 8 && c <= 9) return c + 86;
	if (c == 4) return c + 89;
	if (c == 7) return 206;
	if (c == 14) return 207;
	if (c >= 33 && c <= 58) return c + 122;
	if (c >= 65 && c <= 90) return c + 90;
	if (c >= 96 && c <= 118) return c + 85;
	if (c >= 119 && c <= 140) return c + 62;
	if (c >= 141 && c <= 142) return 204;
	if (c == 143) return 205;
	if (c == 1) return 208;
	return c;
}

void FontText_FilterOutTokensFromString(uint16_t *str)
{
	int newIdx = 0;
	uint16_t copy[256], *c;
	UnicodeStrcpy(copy, str);

	// Drop everything between a pair of tildes, tildes included
	for (c = copy; *c != '\0'; c++) {
		if (*c == '~') {
			c++;
			while (*c != '~') c++;
		} else {
			str[newIdx++] = *c;
		}
	}
	str[newIdx] = '\0';
}

uint16_t *FontText_GetNextSpace(uint16_t *s)
{
	// Up to the next space or the end, skipping tokens
	for (; *s != ' ' && *s != '\0'; s++)
		if (*s == '~') {
			s++;
			while (*s != '~') s++;
		}
	return s;
}
//- rouz edit (ChatGPT)
