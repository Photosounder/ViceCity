//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma once

#include "Game.h"

#include "Streaming_constants.h"

#include "StreamingInfo.h"
#include "StreamingData.h"

struct CDirectory;
struct CPtrList;

class CStreaming
{
public:
	static bool ms_disableStreaming;
	static bool ms_bLoadingBigModel;
	static int32 ms_oldSectorX;
	static int32 ms_oldSectorY;
	static int32 ms_streamingBufferSize;
#ifndef ONE_THREAD_PER_CHANNEL
	static int8 *ms_pStreamingBuffer[2];
#else
	static int8 *ms_pStreamingBuffer[4];
#endif
	static int32 ms_channelError;


	static CDirectory *ms_pExtraObjectsDir;

	static int32 ms_lastCullZone;


	static int32 ms_currentPedLoading;
	static size_t ms_memoryAvailable;

	static void Init(void);
	static void Init2(void);
	static void ReInit(void);
	static void Shutdown(void);
	static void Update(void);
	static void LoadCdDirectory(void);
	static void LoadCdDirectory(const char *dirname, int32 n);
	static bool ConvertBufferToObject(int8 *buf, int32 streamId);
	static bool FinishLoadingLargeFile(int8 *buf, int32 streamId);
	static void RequestBigBuildings(eLevelName level);
	static void RequestBigBuildings(eLevelName level, const CVector &pos);
	static void InstanceBigBuildings(eLevelName level, const CVector &pos);
	static void InstanceLoadedModelsInSectorList(CPtrList &list);
	static void InstanceLoadedModels(const CVector &pos);
	static void RequestIslands(eLevelName level);
	static void RequestSpecialModel(int32 modelId, const char *modelName, int32 flags);
	static void RequestSpecialChar(int32 charId, const char *modelName, int32 flags);
	static void RemoveUnusedBuildings(eLevelName level);
	static void RemoveBuildings(eLevelName level);
	static void RemoveBuildingsNotInArea(int32 area);
	static void RemoveUnusedBigBuildings(eLevelName level);
	static void RemoveIslandsNotUsed(eLevelName level);
	static void RemoveBigBuildings(eLevelName level);

	static void RemoveUnusedModelsInLoadedList(void);



	static void LoadInitialPeds(void);
	static void LoadInitialWeapons(void);
	static void LoadInitialVehicles(void);
	static void StreamVehiclesAndPeds(void);
	static void StreamZoneModels(const CVector &pos);

	static void LoadBigBuildingsWhenNeeded(void);

	static void RequestModelStream(int32 ch);
	static bool ProcessLoadingChannel(int32 ch);
	static void RetryLoadFile(int32 ch);
	static void LoadRequestedModels(void);
	static void LoadAllRequestedModels(bool priority);

	static void MakeSpaceFor(int32 size);
	static void ImGonnaUseStreamingMemory(void);
	static void IHaveUsedStreamingMemory(void);
	static void UpdateMemoryUsed(void);

	static void AddModelsToRequestList(const CVector &pos, int32 flags);
	static void ProcessEntitiesInSectorList(CPtrList &list, float x, float y, float xmin, float ymin, float xmax, float ymax, int32 flags);
	static void ProcessEntitiesInSectorList(CPtrList &list, int32 flags);
	static void DeleteFarAwayRwObjects(const CVector &pos);
	static void DeleteAllRwObjects(void);
	static void DeleteRwObjectsAfterDeath(const CVector &pos);
	static void DeleteRwObjectsBehindCamera(size_t mem); // originally signed
	static void DeleteRwObjectsInSectorList(CPtrList &list);
	static void DeleteRwObjectsInOverlapSectorList(CPtrList &list, int32 x, int32 y);
	static bool DeleteRwObjectsBehindCameraInSectorList(CPtrList &list, size_t mem); // originally signed
	static bool DeleteRwObjectsNotInFrustumInSectorList(CPtrList &list, size_t mem); // originally signed

	static void LoadScene(const CVector &pos);
	static void LoadSceneCollision(const CVector &pos);

	static void MemoryCardSave(uint8 *buffer, uint32 *length);
	static void MemoryCardLoad(uint8 *buffer, uint32 length);

	static void UpdateForAnimViewer(void);

	static void PrintStreamingBufferState();
};
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
