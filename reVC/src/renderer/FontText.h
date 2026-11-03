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
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
