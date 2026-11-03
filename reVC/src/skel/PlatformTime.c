//+ rouz edit (ChatGPT)
#include "config.h"
#include "PlatformTime.h"
#include <time.h>
#ifndef _WIN32
#include <langinfo.h>
#include <stddef.h>

static void tmToSystemTime(const struct tm *tm, SYSTEMTIME *out)
{
    // Copy the original calendar fields without changing milliseconds
    out->wYear = tm->tm_year + 1900;
    out->wMonth = tm->tm_mon + 1;
    out->wDayOfWeek = tm->tm_wday;
    out->wDay = tm->tm_mday;
    out->wHour = tm->tm_hour;
    out->wMinute = tm->tm_min;
    out->wSecond = tm->tm_sec;
}
void GetLocalTime_CP(SYSTEMTIME *out)
{
    // Convert the current local calendar time using the original provider
    time_t timestamp = time(NULL);
    struct tm *localTm = localtime(&timestamp);
    tmToSystemTime(localTm, out);
}
void GetDateFormat(int unused1, int unused2, SYSTEMTIME *in, int unused3, char *out, int size)
{
    // Format the same calendar fields using the current locale date format
    struct tm linuxTime;
    (void)unused1; (void)unused2; (void)unused3;
    linuxTime.tm_year = in->wYear - 1900;
    linuxTime.tm_mon = in->wMonth - 1;
    linuxTime.tm_wday = in->wDayOfWeek;
    linuxTime.tm_mday = in->wDay;
    linuxTime.tm_hour = in->wHour;
    linuxTime.tm_min = in->wMinute;
    linuxTime.tm_sec = in->wSecond;
    strftime(out, size, nl_langinfo(D_FMT), &linuxTime);
}
void FileTimeToSystemTime(time_t *writeTime, SYSTEMTIME *out)
{
    // Convert the original file timestamp through UTC calendar time
    struct tm *ptm = gmtime(writeTime);
    tmToSystemTime(ptm, out);
}
#endif
#if !defined(_MSC_VER) && !defined(__CWCC__)
char *Platform_StrDate(char *buffer)
{
    // Format the current local date with the original fixed pattern
    time_t timestamp;
    time(&timestamp);
    struct tm *localTm = localtime(&timestamp);
    strftime(buffer, 10, "%m/%d/%y", localTm);
    return buffer;
}
#endif
//- rouz edit (ChatGPT)
