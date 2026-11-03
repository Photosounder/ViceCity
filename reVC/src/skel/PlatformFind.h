//+ rouz edit (ChatGPT)
#pragma once
#ifndef _WIN32
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <time.h>
#include <dirent.h>
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif
typedef void *HANDLE;
#define INVALID_HANDLE_VALUE NULL
#define FindClose(h) do { if ((h) != NULL) closedir((DIR *)(h)); } while (0)
typedef struct WIN32_FIND_DATA {
    char extension[32];
    char folder[MAX_PATH];
    char cFileName[256];
    time_t ftLastWriteTime;
} WIN32_FIND_DATA;
#ifdef __cplusplus
extern "C" {
#endif
HANDLE FindFirstFile(const char *pathname, WIN32_FIND_DATA *firstfile);
bool FindNextFile(HANDLE handle, WIN32_FIND_DATA *finddata);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
