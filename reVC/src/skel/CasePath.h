//+ rouz edit (ChatGPT)
#pragma once
#ifndef _WIN32
#include <stdbool.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
char *strupr(char *str);
char *strlwr(char *str);
char *casepath(const char *path, bool checkPathFirst);
FILE *_fcaseopen(const char *filename, const char *mode);
int _caserename(const char *old_filename, const char *new_filename);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
