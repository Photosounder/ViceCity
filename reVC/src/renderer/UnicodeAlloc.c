//+ rouz edit (ChatGPT)
#include "FontText.h"
#include <stdlib.h>
#include <string.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>

uint16_t *AllocUnicode(const char *src)
{
    // Allocate through the original tracked allocator and copy the terminating byte
    uint16_t *dst = (uint16_t *)malloc(strlen(src) * 2 + 2);
    uint16_t *i = dst;
    while ((*i++ = (unsigned char)*src++) != '\0');
    return dst;
}
//- rouz edit (ChatGPT)
