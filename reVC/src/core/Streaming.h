//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma once

#include "Game.h"

#include "Streaming_constants.h"

#include "StreamingInfo.h"
#include "StreamingData.h"

struct CDirectory;
struct CPtrList;

//+ rouz edit (Claude)
// Formerly the static class CStreaming, now C globals and functions shared with C
#ifdef __cplusplus
class CDirectory;
#else
typedef struct CDirectory CDirectory;
#endif
#ifdef __cplusplus
extern "C" {
#endif
extern bool Streaming_ms_disableStreaming;
extern bool Streaming_ms_bLoadingBigModel;
extern int32 Streaming_ms_oldSectorX;
extern int32 Streaming_ms_oldSectorY;
extern int32 Streaming_ms_streamingBufferSize;
#ifndef ONE_THREAD_PER_CHANNEL
extern int8 * Streaming_ms_pStreamingBuffer[2];
#else
extern int8 * Streaming_ms_pStreamingBuffer[4];
#endif
extern int32 Streaming_ms_channelError;
extern CDirectory * Streaming_ms_pExtraObjectsDir;
extern int32 Streaming_ms_lastCullZone;
extern int32 Streaming_ms_currentPedLoading;
extern size_t Streaming_ms_memoryAvailable;
void Streaming_Init(void);
void Streaming_Init2(void);
void Streaming_ReInit(void);
void Streaming_Shutdown(void);
void Streaming_Update(void);
bool Streaming_ConvertBufferToObject(int8 *buf, int32 streamId);
bool Streaming_FinishLoadingLargeFile(int8 *buf, int32 streamId);
void Streaming_RequestIslands(eLevelName level);
void Streaming_RequestSpecialModel(int32 modelId, const char *modelName, int32 flags);
void Streaming_RequestSpecialChar(int32 charId, const char *modelName, int32 flags);
void Streaming_RemoveUnusedBuildings(eLevelName level);
void Streaming_RemoveBuildings(eLevelName level);
void Streaming_RemoveBuildingsNotInArea(int32 area);
void Streaming_RemoveUnusedBigBuildings(eLevelName level);
void Streaming_RemoveIslandsNotUsed(eLevelName level);
void Streaming_RemoveBigBuildings(eLevelName level);
void Streaming_RemoveUnusedModelsInLoadedList(void);
void Streaming_LoadInitialPeds(void);
void Streaming_LoadInitialWeapons(void);
void Streaming_LoadInitialVehicles(void);
void Streaming_StreamVehiclesAndPeds(void);
void Streaming_LoadBigBuildingsWhenNeeded(void);
void Streaming_RequestModelStream(int32 ch);
bool Streaming_ProcessLoadingChannel(int32 ch);
void Streaming_RetryLoadFile(int32 ch);
void Streaming_LoadRequestedModels(void);
void Streaming_LoadAllRequestedModels(bool priority);
void Streaming_MakeSpaceFor(int32 size);
void Streaming_ImGonnaUseStreamingMemory(void);
void Streaming_IHaveUsedStreamingMemory(void);
void Streaming_UpdateMemoryUsed(void);
void Streaming_DeleteAllRwObjects(void);
void Streaming_DeleteRwObjectsBehindCamera(size_t mem); // originally signed
void Streaming_MemoryCardSave(uint8 *buffer, uint32 *length);
void Streaming_MemoryCardLoad(uint8 *buffer, uint32 length);
void Streaming_UpdateForAnimViewer(void);
void Streaming_PrintStreamingBufferState();
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
#ifndef ONE_THREAD_PER_CHANNEL
#else
#endif
void Streaming_LoadCdDirectory(void);
void Streaming_LoadCdDirectory(const char *dirname, int32 n);
void Streaming_RequestBigBuildings(eLevelName level);
void Streaming_RequestBigBuildings(eLevelName level, const CVector &pos);
void Streaming_InstanceBigBuildings(eLevelName level, const CVector &pos);
void Streaming_InstanceLoadedModelsInSectorList(CPtrList &list);
void Streaming_InstanceLoadedModels(const CVector &pos);
void Streaming_StreamZoneModels(const CVector &pos);
void Streaming_AddModelsToRequestList(const CVector &pos, int32 flags);
void Streaming_ProcessEntitiesInSectorList(CPtrList &list, float x, float y, float xmin, float ymin, float xmax, float ymax, int32 flags);
void Streaming_ProcessEntitiesInSectorList(CPtrList &list, int32 flags);
void Streaming_DeleteFarAwayRwObjects(const CVector &pos);
void Streaming_DeleteRwObjectsAfterDeath(const CVector &pos);
void Streaming_DeleteRwObjectsInSectorList(CPtrList &list);
void Streaming_DeleteRwObjectsInOverlapSectorList(CPtrList &list, int32 x, int32 y);
bool Streaming_DeleteRwObjectsBehindCameraInSectorList(CPtrList &list, size_t mem); // originally signed
bool Streaming_DeleteRwObjectsNotInFrustumInSectorList(CPtrList &list, size_t mem); // originally signed
void Streaming_LoadScene(const CVector &pos);
void Streaming_LoadSceneCollision(const CVector &pos);
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
