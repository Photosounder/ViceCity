//+ rouz edit (ChatGPT)
#include "common.h"
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
    debug("%s", message);
}

void PedData_BackendUnknownType(void)
{
    // Report the original unknown-type diagnostic before returning its sentinel
    Error("Unknown ped type, Pedtype.cpp");
}
//- rouz edit (ChatGPT)
