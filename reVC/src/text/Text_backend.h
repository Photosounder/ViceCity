//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "TextLanguages.h"
#ifdef __cplusplus
extern "C" {
#endif
int32_t Text_BackendLanguage(void);
bool Text_BackendRussianGame(void);
bool Text_BackendJapaneseGame(void);
void *Text_BackendAllocate(size_t size);
void Text_BackendFree(void *pointer);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
