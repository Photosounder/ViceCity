//+ rouz edit (ChatGPT)
#include <stdint.h>
#include <stdlib.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#ifdef __MWERKS__
void debug(char *format, ...);
void Error(char *format, ...);
#else
void re3_debug(const char *format, ...);
#endif
#include "PedData_backend.h"

void *PedData_BackendAllocate(size_t size)
{
    // Retain the existing native allocation provider for records and parser buffers
    return malloc(size);
}

void PedData_BackendFree(void *memory)
{
    // Release each allocation through the matching native provider
    free(memory);
}

void PedData_BackendDebug(const char *message)
{
    // Preserve the native debug prefix and message text
#ifdef __MWERKS__
    debug("%s", message);
#else
    re3_debug("[DBG]: %s", message);
#endif
}

void PedData_BackendUnknownType(void)
{
    // Report the original unknown-type diagnostic before returning its sentinel
#ifdef __MWERKS__
    Error("Unknown ped type, Pedtype.cpp");
#else
    re3_debug("[ERROR]: Unknown ped type, Pedtype.cpp");
#endif
}
//- rouz edit (ChatGPT)
