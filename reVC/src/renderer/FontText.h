//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
uint16_t *AllocUnicode(const char *src);
void AsciiToUnicode(const char *src, uint16_t *dst);
void UnicodeStrcpy(uint16_t *dst, const uint16_t *src);
void UnicodeStrcat(uint16_t *dst, uint16_t *append);
int UnicodeStrlen(const uint16_t *str);
void UnicodeMakeUpperCase(uint16_t *dst, const uint16_t *src);
// Map a character code to the one used by the half size font texture
uint16_t Font_FindNewCharacter(uint16_t c);
// Remove the ~tokens~ from a string in place
void Font_FilterOutTokensFromString(uint16_t *str);
// End of the word that starts at s, stepping over ~tokens~
uint16_t *Font_GetNextSpace(uint16_t *s);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
