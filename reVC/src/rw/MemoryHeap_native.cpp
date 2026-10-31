//+ rouz edit (ChatGPT)
#include "common.h"
#include "MemoryHeap.h"
#include "MemoryHeap_backend.h"
#include "Game.h"
#include "Streaming.h"
#include "ModelInfo.h"
#include "FileLoader.h"
#include "Collision.h"
#include "main.h"
#ifdef USE_CUSTOM_ALLOCATOR
size_t MemoryHeap_HostMemoryUsed(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    return CStreaming::ms_memoryUsed;
}
void MemoryHeap_HostMakeSpaceFor(int32_t size)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::MakeSpaceFor(size);
}
void MemoryHeap_HostTidyGame(bool a,bool b)
{
    // Forward the original external owner primitive at its exact operation boundary
    CGame::TidyUpMemory(a,b);
}
bool MemoryHeap_HostPlayingIntro(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    return CGame::playingIntro;
}
void MemoryHeap_HostRemoveOtherCollision(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    CModelInfo::RemoveColModelsFromOtherLevels(LEVEL_GENERIC);
}
int32_t MemoryHeap_HostCollisionLevel(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    return CCollision::ms_collisionInMemory;
}
void MemoryHeap_HostLoadCollision(int32_t level)
{
    // Forward the original external owner primitive at its exact operation boundary
    CFileLoader::LoadCollisionFromDatFile(static_cast<eLevelName>(level));
}
int32_t MemoryHeap_HostCurrentLevel(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    return CGame::currLevel;
}
void MemoryHeap_HostRequestBuildings(int32_t level)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::RequestBigBuildings(static_cast<eLevelName>(level));
}
void MemoryHeap_HostLoadRequested(bool priority)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::LoadAllRequestedModels(priority);
}
void MemoryHeap_HostLoadingScreen(const char *a,const char *b,const char *c)
{
    // Forward the original external owner primitive at its exact operation boundary
    LoadingScreen(a,b,c);
}
void MemoryHeap_HostRemoveIslandsIndustrial(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
}
void MemoryHeap_HostRemoveIslandsCommercial(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
}
void MemoryHeap_HostRemoveIslandsSuburban(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
}
void MemoryHeap_HostRemoveBuildingsIndustrial(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
}
void MemoryHeap_HostRemoveBuildingsCommercial(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
}
void MemoryHeap_HostRemoveBuildingsSuburban(void)
{
    // Forward the original external owner primitive at its exact operation boundary
    CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
}
#endif
//- rouz edit (ChatGPT)
