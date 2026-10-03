#pragma once

//+ rouz edit (ChatGPT)
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool AudioStream_IsCutscene(void);
bool IsFXSupported(void);
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);
#ifndef _WIN32
char *AudioStream_CasePath(const char *filename);
#endif
void re3_debug(const char *format, ...);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
