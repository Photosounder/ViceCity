//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void MemoryMgr_HostHeapInit(uint32_t size);
void *MemoryMgr_HostHeapMalloc(uint32_t size);
void *MemoryMgr_HostHeapRealloc(void *ptr,uint32_t size);
void MemoryMgr_HostHeapFree(void *ptr);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
