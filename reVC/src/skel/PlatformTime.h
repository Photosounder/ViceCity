//+ rouz edit (ChatGPT)
#pragma once
#if !defined(_MSC_VER) && !defined(__CWCC__)
#ifdef __cplusplus
extern "C" {
#endif
char *Platform_StrDate(char *buffer);
#ifdef __cplusplus
}
#endif
#endif
#ifndef _WIN32
#include <stdint.h>
#include <time.h>
typedef struct SYSTEMTIME {
    uint16_t wYear, wMonth, wDayOfWeek, wDay;
    uint16_t wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME;
#ifdef __cplusplus
extern "C" {
#endif
void GetLocalTime_CP(SYSTEMTIME *out);
void FileTimeToSystemTime(time_t *writeTime, SYSTEMTIME *out);
void GetDateFormat(int unused1, int unused2, SYSTEMTIME *in, int unused3, char *out, int size);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
