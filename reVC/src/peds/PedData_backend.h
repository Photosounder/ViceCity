//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void *PedData_BackendAllocate(size_t size);
void PedData_BackendFree(void *memory);
void PedData_BackendDebug(const char *message);
void PedData_BackendUnknownType(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
