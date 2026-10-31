//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
size_t MemoryHeap_HostMemoryUsed(void);
void MemoryHeap_HostMakeSpaceFor(int32_t size);
void MemoryHeap_HostTidyGame(bool a,bool b);
bool MemoryHeap_HostPlayingIntro(void);
void MemoryHeap_HostRemoveOtherCollision(void);
int32_t MemoryHeap_HostCollisionLevel(void);
void MemoryHeap_HostLoadCollision(int32_t level);
int32_t MemoryHeap_HostCurrentLevel(void);
void MemoryHeap_HostRequestBuildings(int32_t level);
void MemoryHeap_HostLoadRequested(bool priority);
void MemoryHeap_HostLoadingScreen(const char *a,const char *b,const char *c);
void MemoryHeap_HostRemoveIslandsIndustrial(void);
void MemoryHeap_HostRemoveIslandsCommercial(void);
void MemoryHeap_HostRemoveIslandsSuburban(void);
void MemoryHeap_HostRemoveBuildingsIndustrial(void);
void MemoryHeap_HostRemoveBuildingsCommercial(void);
void MemoryHeap_HostRemoveBuildingsSuburban(void);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
