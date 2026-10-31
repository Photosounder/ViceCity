//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t *pMemoryTop;
void InitMemoryMgr(void);
void *MemoryMgrMalloc(size_t size);
void *MemoryMgrRealloc(void *ptr,size_t size);
void *MemoryMgrCalloc(size_t num,size_t size);
void MemoryMgrFree(void *ptr);
void *RwMallocAlign(uint32_t size,uint32_t align);
void RwFreeAlign(void *mem);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
