//+ rouz edit (ChatGPT)
#pragma once
#include <stdio.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
const char *FileMgr_BackendUserFilesFolder(void);
#ifndef _WIN32
char *FileMgr_BackendCasePath(const char *path, bool checkPathFirst);
FILE *FileMgr_BackendCaseOpen(const char *filename, const char *mode);
#endif
void re3_assert(const char *expr, const char *filename, unsigned int lineno, const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
