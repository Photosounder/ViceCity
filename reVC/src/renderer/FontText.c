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

//- rouz edit (ChatGPT)
