//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)

#include "General.h"
#include "Pad.h"
#include "Hud.h"
#include "Text.h"
#include "Clock.h"
#include "Renderer.h"
#include "ModelInfo.h"
#include "SimpleModelPolicy.h" // rouz edit (ChatGPT)
#include "TxdStore.h"
#include "ModelIndices.h"
#include "Pools.h"
#include "Wanted.h"
#include "Directory.h"
#include "RwHelper.h"
#include "World.h"
#include "WorldLogic.h"
#include "StreamingVehicles.h"
#include "Entity.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "Zones.h"
#include "Radar.h"
#include "Camera.h"
#include "Record.h"
#include "CarCtrl.h"
#include "Population.h"
#include "Gangs.h"
#include "CutsceneMgr.h"
#include "CdStream.h"
#include "Streaming.h"
#include "StreamingRequest_backend.h"
#include "StreamingRemove_backend.h"
#include "StreamingFlush_backend.h"
#include "Replay.h"
#include "main.h"
#include "ColStore.h"
#include "DMAudio.h"
#include "Script.h"
#include "MemoryMgr_native.h"
#include "MemoryHeap.h"
#include "Font.h"
#include "Frontend.h"
#include "VarConsole.h"

//+ rouz edit (ChatGPT)
void StreamingFlush_ProcessLoadingChannel(int32_t channel)
{
    // Preserve the original native channel processing call
    Streaming_ProcessLoadingChannel(channel);
}
int StreamingRemove_NoUnstreaming(void)
{
    // Read the original live unstreaming preference
    return rouz.no_unstreaming;
}
bool StreamingRemove_TryVehicleFallback(void)
{
    // Preserve the original vehicle thresholds and short-circuit order
    return (Streaming_numVehiclesLoaded > 7 || Game_currArea != AREA_MAIN_MAP && Streaming_numVehiclesLoaded > 4) && Streaming_RemoveLoadedVehicle();
}
void StreamingRemove_RemoveVehicleClass(int32_t id)
{
    // Remove the original vehicle class entry after streaming removal
    CVehicleModelInfo *info = (CVehicleModelInfo *)((CBaseModelInfo *)ModelRegistry_Get(id));
    if (info->m_vehicleClass != -1)
        CarCtrl_RemoveFromLoadedVehicleArray(id, info->m_vehicleClass);
}
void StreamingRemove_AddVehicleClass(int32_t id)
{
    // Register the original vehicle class and frequency
    CVehicleModelInfo *info = (CVehicleModelInfo *)((CBaseModelInfo *)ModelRegistry_Get(id));
    if (info->m_vehicleClass != -1)
        CarCtrl_AddToLoadedVehicleArray(id, info->m_vehicleClass, info->m_frequency);
}
int StreamingRemove_ModelRefs(int32_t id)
{
    // Read the original native model reference count
    return ((CBaseModelInfo *)ModelRegistry_Get(id))->GetNumRefs();
}
void StreamingRemove_DeleteModel(int32_t id)
{
    // Preserve the original virtual render-object deletion
    ((CBaseModelInfo *)ModelRegistry_Get(id))->DeleteRwObject();
}
void StreamingRemove_RemoveCol(int32_t id)
{
    // Preserve the original collision-store removal
    ColStore_RemoveCol(id);
}
int32_t StreamingDependencies_AnimIndex(void *model)
{
    // Preserve the base-model virtual animation-file lookup
    return ((CBaseModelInfo *)model)->GetAnimFileIndex();
}
int16_t StreamingOwnership_TxdSlot(void *model)
{
    // Read the original base-model texture slot
    return ((CBaseModelInfo *)model)->GetTxdSlot();
}
int16_t StreamingRequest_TxdSlot(void *model)
{
    // Read the texture slot from the captured native model
    return ((CSimpleModelInfo *)((CBaseModelInfo *)model))->GetTxdSlot();
}
int32_t StreamingRequest_AnimIndex(void *model)
{
    // Preserve the original virtual animation-file lookup
    return ((CSimpleModelInfo *)((CBaseModelInfo *)model))->GetAnimFileIndex();
}
void StreamingRequest_SetOpaqueIfSimple(void *model)
{
    // Preserve the original simple-model test and alpha write
    CSimpleModelInfo *info = (CSimpleModelInfo *)((CBaseModelInfo *)model);
    if (info->IsSimple())
        info->m_alpha = 255;
}
//- rouz edit (ChatGPT)

bool Streaming_ms_disableStreaming;
bool Streaming_ms_bLoadingBigModel;
int32 Streaming_ms_oldSectorX;
int32 Streaming_ms_oldSectorY;
int32 Streaming_ms_streamingBufferSize;
#ifndef ONE_THREAD_PER_CHANNEL
int8 *Streaming_ms_pStreamingBuffer[2];
#else
int8 *Streaming_ms_pStreamingBuffer[4];
#endif
int32 Streaming_ms_channelError; // rouz edit (ChatGPT)
CDirectory *Streaming_ms_pExtraObjectsDir;
int32 Streaming_ms_currentPedLoading;
int32 Streaming_ms_lastCullZone;
size_t Streaming_ms_memoryAvailable;

int32 desiredNumVehiclesLoaded = 12;

CEntity *pIslandLODmainlandEntity;
CEntity *pIslandLODbeachEntity;
int32 islandLODmainland;
int32 islandLODbeach;

#ifndef MASTER
bool gbPrintStats;
bool gbPrintVehiclesInMemory;  // TODO
bool gbPrintStreamingBuffer; // TODO
#endif





void
Streaming_Init2(void)
{
	int i;

	for(i = 0; i < NUMSTREAMINFO; i++){
		Streaming_modelInfo[i].m_loadState = STREAMSTATE_NOTLOADED; // rouz edit (ChatGPT)
		Streaming_modelInfo[i].m_next = nil; // rouz edit (ChatGPT)
		Streaming_modelInfo[i].m_prev = nil; // rouz edit (ChatGPT)
		Streaming_modelInfo[i].m_nextID = -1; // rouz edit (ChatGPT)
		Streaming_modelInfo[i].m_size = 0; // rouz edit (ChatGPT)
		Streaming_modelInfo[i].m_position = 0; // rouz edit (ChatGPT)
	}

	Streaming_ms_channelError = -1;

	// init lists

	Streaming_startLoadedList.m_next = &Streaming_endLoadedList; // rouz edit (ChatGPT)
	Streaming_startLoadedList.m_prev = nil; // rouz edit (ChatGPT)
	Streaming_endLoadedList.m_prev = &Streaming_startLoadedList; // rouz edit (ChatGPT)
	Streaming_endLoadedList.m_next = nil; // rouz edit (ChatGPT)

	Streaming_startRequestedList.m_next = &Streaming_endRequestedList; // rouz edit (ChatGPT)
	Streaming_startRequestedList.m_prev = nil; // rouz edit (ChatGPT)
	Streaming_endRequestedList.m_prev = &Streaming_startRequestedList; // rouz edit (ChatGPT)
	Streaming_endRequestedList.m_next = nil; // rouz edit (ChatGPT)

	// init misc

	Streaming_ms_oldSectorX = 0;
	Streaming_ms_oldSectorY = 0;
	Streaming_ms_streamingBufferSize = 0;
	Streaming_ms_disableStreaming = false;
	Streaming_memoryUsed = 0; // rouz edit (ChatGPT)
	Streaming_ms_bLoadingBigModel = false;

	// init channels

	Streaming_channel[0].state = CHANNELSTATE_IDLE; // rouz edit (ChatGPT)
	Streaming_channel[1].state = CHANNELSTATE_IDLE; // rouz edit (ChatGPT)
	for(i = 0; i < 4; i++){
		Streaming_channel[0].streamIds[i] = -1; // rouz edit (ChatGPT)
		Streaming_channel[0].offsets[i] = -1; // rouz edit (ChatGPT)
		Streaming_channel[1].streamIds[i] = -1; // rouz edit (ChatGPT)
		Streaming_channel[1].offsets[i] = -1; // rouz edit (ChatGPT)
	}

	// init stream info, mark things that are already loaded

	for(i = 0; i < MODELINFOSIZE; i++){
		CBaseModelInfo *mi = ((CBaseModelInfo*)ModelRegistry_Get(i)); // rouz edit (ChatGPT)
		if(mi && mi->GetRwObject()){
			Streaming_modelInfo[i].m_loadState = STREAMSTATE_LOADED; // rouz edit (ChatGPT)
			Streaming_modelInfo[i].m_flags = STREAMFLAGS_DONT_REMOVE; // rouz edit (ChatGPT)
			if(mi->IsSimple())
				((CSimpleModelInfo*)mi)->m_alpha = 255;
		}
	}

	for(i = 0; i < TXDSTORESIZE; i++)
		// Use the complete C texture dictionary owner with the original live arguments
		if(TxdStore_GetSlot(i) && ((RwTexDictionary*)TxdStore_GetSlot(i)->texDict))
			Streaming_modelInfo[i + STREAM_OFFSET_TXD].m_loadState = STREAMSTATE_LOADED; // rouz edit (ChatGPT)


	for(i = 0; i < MAXVEHICLESLOADED; i++)
		Streaming_vehiclesLoaded[i] = -1; // rouz edit (ChatGPT)
	Streaming_numVehiclesLoaded = 0; // rouz edit (ChatGPT)
	Streaming_numPedsLoaded = 8; // rouz edit (ChatGPT)

	for(i = 0; i < ARRAY_SIZE(Streaming_pedGroupLoaded); i++) // rouz edit (ChatGPT)
		Streaming_pedGroupLoaded[i] = false; // rouz edit (ChatGPT)

	// Allocate the extra-object directory without invoking C++ new.
	Streaming_ms_pExtraObjectsDir = (CDirectory*)malloc(sizeof(CDirectory));
	assert(Streaming_ms_pExtraObjectsDir);
	Directory_Initialize(Streaming_ms_pExtraObjectsDir, EXTRADIRSIZE);
	Streaming_numPriorityRequests = 0; // rouz edit (ChatGPT)
	Streaming_currentPedGroup = -1; // rouz edit (ChatGPT)
	Streaming_ms_lastCullZone = -1;		// unused because RemoveModelsNotVisibleFromCullzone is gone
	Streaming_loadedGangs = 0; // rouz edit (ChatGPT)
	Streaming_ms_currentPedLoading = NUMMODELSPERPEDGROUP;	// unused, whatever it is

	Streaming_LoadCdDirectory();

	// allocate streaming buffers
	if(Streaming_ms_streamingBufferSize & 1) Streaming_ms_streamingBufferSize++;
#ifndef ONE_THREAD_PER_CHANNEL
	Streaming_ms_pStreamingBuffer[0] = (int8*)RwMallocAlign(Streaming_ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE, CDSTREAM_SECTOR_SIZE);
	Streaming_ms_streamingBufferSize /= 2;
	Streaming_ms_pStreamingBuffer[1] = Streaming_ms_pStreamingBuffer[0] + Streaming_ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
#else
	Streaming_ms_pStreamingBuffer[0] = (int8*)RwMallocAlign(Streaming_ms_streamingBufferSize*2*CDSTREAM_SECTOR_SIZE, CDSTREAM_SECTOR_SIZE);
	Streaming_ms_streamingBufferSize /= 2;
	Streaming_ms_pStreamingBuffer[1] = Streaming_ms_pStreamingBuffer[0] + Streaming_ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
	Streaming_ms_pStreamingBuffer[2] = Streaming_ms_pStreamingBuffer[1] + Streaming_ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
	Streaming_ms_pStreamingBuffer[3] = Streaming_ms_pStreamingBuffer[2] + Streaming_ms_streamingBufferSize*CDSTREAM_SECTOR_SIZE;
#endif
	debug("Streaming buffer size is %d sectors", Streaming_ms_streamingBufferSize);

	// PC only, figure out how much memory we got
#ifdef GTA_PC
#define MB (1024*1024)
#ifdef FIX_BUGS
	// do what gta3 does
	extern size_t _dwMemAvailPhys;
	Streaming_ms_memoryAvailable = (_dwMemAvailPhys - 10*MB)/2;
	if(Streaming_ms_memoryAvailable < 65*MB)
		Streaming_ms_memoryAvailable = 65*MB;
	desiredNumVehiclesLoaded = (int32)((Streaming_ms_memoryAvailable / MB - 65) / 3 + 12);
	if(desiredNumVehiclesLoaded > MAXVEHICLESLOADED)
		desiredNumVehiclesLoaded = MAXVEHICLESLOADED;
#else
	Streaming_ms_memoryAvailable = 65 * MB;
	desiredNumVehiclesLoaded = 25;
	debug("Memory allocated to Streaming is %zuMB", Streaming_ms_memoryAvailable/MB); // original modifier was %d
#endif
#undef MB
#endif

	// find island LODs

	pIslandLODmainlandEntity = nil;
	pIslandLODbeachEntity = nil;
	islandLODmainland = -1;
	islandLODbeach = -1;
	(CBaseModelInfo*)ModelRegistry_Find("IslandLODmainland", 0, MODELINFOSIZE-1, &islandLODmainland); // rouz edit (ChatGPT)
	(CBaseModelInfo*)ModelRegistry_Find("IslandLODbeach", 0, MODELINFOSIZE-1, &islandLODbeach); // rouz edit (ChatGPT)

#ifndef MASTER
	VarConsole_AddBool(&VarConsole,"Streaming Debug", &gbPrintStats, true); // rouz edit (ChatGPT)
	VarConsole_AddBool(&VarConsole,"Streaming Vehicle Debug", &gbPrintVehiclesInMemory, true); // rouz edit (ChatGPT)
	VarConsole_AddBool(&VarConsole,"Printf Streaming Buffer contents", &gbPrintStreamingBuffer, true); // rouz edit (ChatGPT)
#endif
}

void
Streaming_Init(void)
{
#ifdef USE_TXD_CDIMAGE
	// Use the complete C texture reader with the original live SDK arguments
	if(!TexRead_CanVideoCardDoDXT()){
		int txdHandle = FileMgr_OpenFile("MODELS\\TXD.IMG", "r");
		if (txdHandle)
			FileMgr_CloseFile(txdHandle);
		// Use the complete C texture reader with the original live SDK arguments
		if (!TexRead_CheckVideoCardCaps() && txdHandle) {
			CdStreamAddImage("MODELS\\TXD.IMG");
			Streaming_Init2();
		} else {
			Streaming_Init2();
			// Use the complete C texture reader with the original live SDK arguments
			if (TexRead_CreateTxdImageForVideoCard()) {
				Streaming_Shutdown();
				CdStreamAddImage("MODELS\\TXD.IMG");
				Streaming_Init2();
			}
		}
	} else
		Streaming_Init2();
#else
	Streaming_Init2();
#endif
}

void
Streaming_ReInit(void)
{
	int i;
	Streaming_FlushRequestList(); // rouz edit (ChatGPT)
	Streaming_DeleteAllRwObjects();
	Streaming_RemoveAllUnusedModels(); // rouz edit (ChatGPT)
	for(i = 0; i < MODELINFOSIZE; i++)
		if(((CBaseModelInfo*)ModelRegistry_Get(i)) && Streaming_modelInfo[i].m_flags & STREAMFLAGS_SCRIPTOWNED) // rouz edit (ChatGPT)
			Streaming_SetMissionDoesntRequireModel(i); // rouz edit (ChatGPT)
	Streaming_ms_disableStreaming = false;
}

void
Streaming_Shutdown(void)
{
	RwFreeAlign(Streaming_ms_pStreamingBuffer[0]);
	Streaming_ms_streamingBufferSize = 0;
	if(Streaming_ms_pExtraObjectsDir) {
		// Destroy and release the extra-object directory without invoking C++ delete.
		Directory_Shutdown(Streaming_ms_pExtraObjectsDir);
		free(Streaming_ms_pExtraObjectsDir);
#ifdef FIX_BUGS
		Streaming_ms_pExtraObjectsDir = nil;
#endif
	}
}

#ifndef MASTER
uint64 timeProcessingTXD;
uint64 timeProcessingDFF;
#endif

void
Streaming_Update(void)
{
	CStreamingInfo *si, *prev;
	bool requestedSubway = false;

#ifndef MASTER
	timeProcessingTXD = 0;
	timeProcessingDFF = 0;
#endif

	Streaming_UpdateMemoryUsed();

	if(Streaming_ms_channelError != -1){
		Streaming_RetryLoadFile(Streaming_ms_channelError);
		return;
	}

	if(Timer_GetIsPaused())
		return;

	Streaming_LoadBigBuildingsWhenNeeded();
	if(!Streaming_ms_disableStreaming && TheCamera.GetPosition().z < 55.0f)
		Streaming_AddModelsToRequestList(TheCamera.GetPosition(), 0);

	if (rouz.no_unstreaming)
		Streaming_DeleteFarAwayRwObjects(TheCamera.GetPosition());

	if(!Streaming_ms_disableStreaming &&
	   !Cutscene_IsProcessing() && // rouz edit (ChatGPT)
	   Streaming_numModelsRequested < 5 && // rouz edit (ChatGPT)
	   !Renderer_m_loadingPriority &&
	   Game_currArea == AREA_MAIN_MAP &&
	   !Replay_IsPlayingBack()){ // rouz edit (ChatGPT)
		Streaming_StreamVehiclesAndPeds();
		Streaming_StreamZoneModels(FindPlayerCoors());
	}

	Streaming_LoadRequestedModels();

	if(World_Players[0].m_pRemoteVehicle){
		ColStore_AddCollisionNeededAtPosn(FindPlayerCoors());
		ColStore_LoadCollision(World_Players[0].m_pRemoteVehicle->GetPosition());
		ColStore_EnsureCollisionIsInMemory(World_Players[0].m_pRemoteVehicle->GetPosition());
	}else{
		ColStore_LoadCollision(FindPlayerCoors());
		ColStore_EnsureCollisionIsInMemory(FindPlayerCoors());
	}

	// TODO: PrintRequestList
	//if (CPad::GetPad(1)->GetLeftShoulder2JustDown() && CPad::GetPad(1)->GetRightShoulder1() && CPad::GetPad(1)->GetRightShoulder2())
	//	PrintRequestList();

	if (rouz.no_unstreaming)
	for(si = Streaming_endRequestedList.m_prev; si != &Streaming_startRequestedList; si = prev){ // rouz edit (ChatGPT)
		prev = si->m_prev;
		if((si->m_flags & (STREAMFLAGS_KEEP_IN_MEMORY|STREAMFLAGS_PRIORITY)) == 0)
			Streaming_RemoveModel(si - Streaming_modelInfo); // rouz edit (ChatGPT)
	}
}

void
Streaming_LoadCdDirectory(void)
{
	char dirname[132];
	int i;

#ifdef GTA_PC
	Streaming_imageOffsets[0] = 0; // rouz edit (ChatGPT)
	Streaming_imageOffsets[1] = -1; // rouz edit (ChatGPT)
	Streaming_imageOffsets[2] = -1; // rouz edit (ChatGPT)
	Streaming_imageOffsets[3] = -1; // rouz edit (ChatGPT)
	Streaming_imageOffsets[4] = -1; // rouz edit (ChatGPT)
	Streaming_imageOffsets[5] = -1; // rouz edit (ChatGPT)
	Streaming_imageSize = GetGTA3ImgSize(); // rouz edit (ChatGPT)
	// PS2 uses CFileMgr::GetCdFile on all IMG files to fill the array
#endif

	i = CdStreamGetNumImages();
	while(i-- >= 1){
		strcpy(dirname, CdStreamGetImageName(i));
		strncpy(strrchr(dirname, '.') + 1, "DIR", 3);
		Streaming_LoadCdDirectory(dirname, i);
	}

	Streaming_lastImageRead = 0; // rouz edit (ChatGPT)
	Streaming_imageSize /= CDSTREAM_SECTOR_SIZE; // rouz edit (ChatGPT)
}

void
Streaming_LoadCdDirectory(const char *dirname, int n)
{
	int fd, lastID, imgSelector;
	int modelId;
	DirectoryInfo direntry;
	char *dot;

	lastID = -1;
	fd = FileMgr_OpenFile(dirname, "rb");
	assert(fd > 0);

	imgSelector = n<<24;
	assert(sizeof(direntry) == 32);
	while(FileMgr_Read(fd, (char*)&direntry, sizeof(direntry))){
		bool bAddToStreaming = false;

		if(direntry.size > (uint32)Streaming_ms_streamingBufferSize)
			Streaming_ms_streamingBufferSize = direntry.size;
		direntry.name[23] = '\0';
		dot = strchr(direntry.name, '.');
		if(dot == nil || dot-direntry.name > 20){
			debug("%s is too long\n", direntry.name);
			lastID = -1;
			continue;
		}

		*dot = '\0';

		if(strncasecmp(dot+1, "DFF", 3) == 0){
			if((CBaseModelInfo*)ModelRegistry_Find(direntry.name, 0, MODELINFOSIZE-1, &modelId)){ // rouz edit (ChatGPT)
				bAddToStreaming = true;
			}else{
#ifdef FIX_BUGS
				// remember which cdimage this came from
				Directory_AddItemWithImage(Streaming_ms_pExtraObjectsDir, &direntry, n);
#else
				Directory_AddItem(Streaming_ms_pExtraObjectsDir, &direntry);
#endif
				lastID = -1;
			}
		}else if(strncasecmp(dot+1, "TXD", 3) == 0){
			// Use the complete C texture dictionary owner with the original live arguments
			modelId = TxdStore_FindTxdSlot(direntry.name);
			if(modelId == -1)
				// Use the complete C texture dictionary owner with the original live arguments
				modelId = TxdStore_AddTxdSlot(direntry.name);
			modelId += STREAM_OFFSET_TXD;
			bAddToStreaming = true;
		}else if(strncasecmp(dot+1, "COL", 3) == 0){
			modelId = ColStore_FindColSlot(direntry.name);
			if(modelId == -1)
				modelId = ColStore_AddColSlot(direntry.name);
			modelId += STREAM_OFFSET_COL;
			bAddToStreaming = true;
		}else if(strncasecmp(dot+1, "IFP", 3) == 0){
			// Use the complete C animation manager at the original call boundary
			modelId = AnimManager_RegisterAnimBlock(direntry.name);
			modelId += STREAM_OFFSET_ANIM;
			bAddToStreaming = true;
		}else{
			*dot = '.';
			lastID = -1;
		}

		if(bAddToStreaming){
			if(StreamingInfo_GetCdSize(&Streaming_modelInfo[modelId])){ // rouz edit (ChatGPT)
				debug("%s.%s appears more than once in %s\n", direntry.name, dot+1, dirname);
				lastID = -1;
			}else{
				direntry.offset |= imgSelector;
				StreamingInfo_SetCdPosnAndSize(&Streaming_modelInfo[modelId], direntry.offset, direntry.size); // rouz edit (ChatGPT)
				if(lastID != -1)
					Streaming_modelInfo[lastID].m_nextID = modelId; // rouz edit (ChatGPT)
				lastID = modelId;
			}
		}
	}

	FileMgr_CloseFile(fd);
}

static char*
GetObjectName(int streamId)
{
	static char objname[32];
	if(streamId < STREAM_OFFSET_TXD)
		sprintf(objname, "%s.dff", ((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetModelName()); // rouz edit (ChatGPT)
	else if(streamId >= STREAM_OFFSET_TXD && streamId < STREAM_OFFSET_COL)
		// Use the complete C texture dictionary owner with the original live arguments
		sprintf(objname, "%s.txd", TxdStore_GetTxdName(streamId-STREAM_OFFSET_TXD));
	else if(streamId >= STREAM_OFFSET_COL && streamId < STREAM_OFFSET_ANIM)
		sprintf(objname, "%s.col", ColStore_GetColName(streamId-STREAM_OFFSET_COL));
	else{
		assert(streamId < NUMSTREAMINFO);
		// Use the complete C animation manager at the original call boundary
		sprintf(objname, "%s.ifp", AnimManager_GetAnimationBlockById(streamId-STREAM_OFFSET_ANIM)->name);
	}
	return objname;
}

#ifdef USE_CUSTOM_ALLOCATOR
RpAtomic*
RegisterAtomicMemPtrsCB(RpAtomic *atomic, void *data)
{
	// empty because we expect models to be pre-instanced
	return atomic;
}
#endif

bool
Streaming_ConvertBufferToObject(int8 *buf, int32 streamId)
{
	RwMemory mem;
	RwStream *stream;
	int cdsize;
	uint32 startTime, endTime, timeDiff;
	CBaseModelInfo *mi;
	bool success;

	startTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond();

	cdsize = StreamingInfo_GetCdSize(&Streaming_modelInfo[streamId]); // rouz edit (ChatGPT)
	mem.start = (uint8*)buf;
	mem.length = cdsize * CDSTREAM_SECTOR_SIZE;
	stream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &mem);

	if(streamId < STREAM_OFFSET_TXD){
		// Model
		mi = ((CBaseModelInfo*)ModelRegistry_Get(streamId)); // rouz edit (ChatGPT)

		// Txd and anim have to be loaded
		int animId = mi->GetAnimFileIndex();
#ifdef FIX_BUGS
		if(!Streaming_HasTxdLoaded(mi->GetTxdSlot()) || // rouz edit (ChatGPT)
#else
		// texDict will exist even if only first part has loaded
		// Use the complete C texture dictionary owner with the original live arguments
		if(((RwTexDictionary*)TxdStore_GetSlot(mi->GetTxdSlot())->texDict) == nil ||
#endif
		   // Use the complete C animation manager at the original call boundary
		   animId != -1 && !AnimManager_GetAnimationBlockById(animId)->isLoaded){
			Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
			Streaming_ReRequestModel(streamId); // rouz edit (ChatGPT)
			RwStreamClose(stream, &mem);
			return false;
		}

		// Set Txd and anims to use
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_AddRef(mi->GetTxdSlot());
#if GTA_VERSION > GTAVC_PS2
		if(animId != -1)
			// Use the complete C animation manager at the original call boundary
			AnimManager_AddAnimBlockRef(animId);
#endif

		PUSH_MEMID(MEMID_STREAM_MODELS);
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_SetCurrentTxd(mi->GetTxdSlot());
		if(mi->IsSimple()){
			success = FileLoader_LoadAtomicFile(stream, streamId);
			// TODO(MIAMI)? complain if file is not pre-instanced. we hardly are interested in that
		} else if (mi->GetModelType() == MITYPE_VEHICLE) {
			// load vehicles in two parts
			((CBaseModelInfo*)ModelRegistry_Get(streamId))->AddRef(); // rouz edit (ChatGPT)
			success = FileLoader_StartLoadClumpFile(stream, streamId);
			if(success)
				Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_STARTED; // rouz edit (ChatGPT)
		}else{
			success = FileLoader_LoadClumpFile(stream, streamId);
#ifdef USE_CUSTOM_ALLOCATOR
			if(success)
				RpClumpForAllAtomics((RpClump*)mi->GetRwObject(), RegisterAtomicMemPtrsCB, nil);
#endif
		}
		POP_MEMID();
		Streaming_UpdateMemoryUsed();

		// Txd and anims no longer needed unless we only read part of the file
		if(Streaming_modelInfo[streamId].m_loadState != STREAMSTATE_STARTED){ // rouz edit (ChatGPT)
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_RemoveRefWithoutDelete(mi->GetTxdSlot());
#if GTA_VERSION > GTAVC_PS2
			if(animId != -1)
				// Use the complete C animation manager at the original call boundary
				AnimManager_RemoveAnimBlockRefWithoutDelete(animId);
#endif
		}

		if(!success){
			debug("Failed to load %s\n", ((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetModelName()); // rouz edit (ChatGPT)
			Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
			Streaming_ReRequestModel(streamId); // rouz edit (ChatGPT)
			RwStreamClose(stream, &mem);
			return false;
		}
	}else if(streamId >= STREAM_OFFSET_TXD && streamId < STREAM_OFFSET_COL){
		// Txd
		if((Streaming_modelInfo[streamId].m_flags & STREAMFLAGS_KEEP_IN_MEMORY) == 0 && // rouz edit (ChatGPT)
		   !Streaming_IsTxdUsedByRequestedModels(streamId - STREAM_OFFSET_TXD)){ // rouz edit (ChatGPT)
			Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
			RwStreamClose(stream, &mem);
			return false;
		}

		PUSH_MEMID(MEMID_STREAM_TEXUTRES);
		if(Streaming_ms_bLoadingBigModel || cdsize > 200){
			// Use the complete C texture dictionary owner with the original live arguments
			success = TxdStore_StartLoadTxd(streamId - STREAM_OFFSET_TXD, stream);
			if(success)
				Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_STARTED; // rouz edit (ChatGPT)
		}else
		        // Use the complete C texture dictionary owner with the original live arguments
		        success = TxdStore_LoadStream(streamId - STREAM_OFFSET_TXD, stream);
		POP_MEMID();
		Streaming_UpdateMemoryUsed();

		if(!success){
			// Use the complete C texture dictionary owner with the original live arguments
			debug("Failed to load %s.txd\n", TxdStore_GetTxdName(streamId - STREAM_OFFSET_TXD));
			Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
			Streaming_ReRequestModel(streamId); // rouz edit (ChatGPT)
			RwStreamClose(stream, &mem);
			return false;
		}
	}else if(streamId >= STREAM_OFFSET_COL && streamId < STREAM_OFFSET_ANIM){
		PUSH_MEMID(MEMID_STREAM_COLLISION);
		bool success = ColStore_LoadCol(streamId-STREAM_OFFSET_COL, mem.start, mem.length);
		POP_MEMID();
		if(!success){
			debug("Failed to load %s.col\n", ColStore_GetColName(streamId - STREAM_OFFSET_COL));
			Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
			Streaming_ReRequestModel(streamId); // rouz edit (ChatGPT)
			RwStreamClose(stream, &mem);
			return false;
		}
	}else if(streamId >= STREAM_OFFSET_ANIM){
		assert(streamId < NUMSTREAMINFO);
		if((Streaming_modelInfo[streamId].m_flags & STREAMFLAGS_KEEP_IN_MEMORY) == 0 && // rouz edit (ChatGPT)
		   !Streaming_AreAnimsUsedByRequestedModels(streamId - STREAM_OFFSET_ANIM)){ // rouz edit (ChatGPT)
			Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
			RwStreamClose(stream, &mem);
			return false;
		}
		PUSH_MEMID(MEMID_STREAM_ANIMATION);
		// Use the complete C animation manager at the original call boundary
		AnimManager_LoadAnimStream(stream, true, nil);
		// Use the complete C animation manager at the original call boundary
		AnimManager_CreateAnimAssocGroups();
		POP_MEMID();
	}

	RwStreamClose(stream, &mem);

	if(streamId < STREAM_OFFSET_TXD){
		// Model
		// Vehicles and Peds not in loaded list
		if (mi->GetModelType() != MITYPE_VEHICLE && mi->GetModelType() != MITYPE_PED) {
			CSimpleModelInfo *smi = (CSimpleModelInfo*)mi;

			// Set fading for some objects
			if(mi->IsSimple() && !smi->m_isBigBuilding){
				if(Streaming_modelInfo[streamId].m_flags & STREAMFLAGS_NOFADE) // rouz edit (ChatGPT)
					smi->m_alpha = 255;
				else
					smi->m_alpha = 0;
			}

			if(Streaming_CanRemoveModel(streamId)) // rouz edit (ChatGPT)
				StreamingInfo_AddToList(&Streaming_modelInfo[streamId], &Streaming_startLoadedList); // rouz edit (ChatGPT)
		}
	}else if(streamId >= STREAM_OFFSET_TXD && streamId < STREAM_OFFSET_COL ||
	         streamId >= STREAM_OFFSET_ANIM){
		assert(streamId < NUMSTREAMINFO);
		// Txd and anims
		if(Streaming_CanRemoveModel(streamId)) // rouz edit (ChatGPT)
			StreamingInfo_AddToList(&Streaming_modelInfo[streamId], &Streaming_startLoadedList); // rouz edit (ChatGPT)
	}

	// Mark objects as loaded
	if(Streaming_modelInfo[streamId].m_loadState != STREAMSTATE_STARTED){ // rouz edit (ChatGPT)
		Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_LOADED; // rouz edit (ChatGPT)
#ifndef USE_CUSTOM_ALLOCATOR
		Streaming_memoryUsed += StreamingInfo_GetCdSize(&Streaming_modelInfo[streamId]) * CDSTREAM_SECTOR_SIZE; // rouz edit (ChatGPT)
#endif
	}

	endTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond();
	timeDiff = endTime - startTime;
	if(timeDiff > 5)
		debug("%s took %d ms\n", GetObjectName(streamId), timeDiff);

	return true;
}

bool
Streaming_FinishLoadingLargeFile(int8 *buf, int32 streamId)
{
	RwMemory mem;
	RwStream *stream;
	uint32 startTime, endTime, timeDiff;
	CBaseModelInfo *mi;
	bool success;

	startTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond();

	if(Streaming_modelInfo[streamId].m_loadState != STREAMSTATE_STARTED){ // rouz edit (ChatGPT)
		if(streamId < STREAM_OFFSET_TXD)
			((CBaseModelInfo*)ModelRegistry_Get(streamId))->RemoveRef(); // rouz edit (ChatGPT)
		return false;
	}

	mem.start = (uint8*)buf;
	mem.length = StreamingInfo_GetCdSize(&Streaming_modelInfo[streamId]) * CDSTREAM_SECTOR_SIZE; // rouz edit (ChatGPT)
	stream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &mem);

	if(streamId < STREAM_OFFSET_TXD){
		// Model
		mi = ((CBaseModelInfo*)ModelRegistry_Get(streamId)); // rouz edit (ChatGPT)
		PUSH_MEMID(MEMID_STREAM_MODELS);
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_SetCurrentTxd(mi->GetTxdSlot());
		success = FileLoader_FinishLoadClumpFile(stream, streamId);
		if(success){
#ifdef USE_CUSTOM_ALLOCATOR
			RpClumpForAllAtomics((RpClump*)mi->GetRwObject(), RegisterAtomicMemPtrsCB, nil);
#endif
			success = Streaming_AddToLoadedVehiclesList(streamId); // rouz edit (ChatGPT)
		}
		POP_MEMID();
		mi->RemoveRef();
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_RemoveRefWithoutDelete(mi->GetTxdSlot());
#if GTA_VERSION > GTAVC_PS2
		if(mi->GetAnimFileIndex() != -1)
			// Use the complete C animation manager at the original call boundary
			AnimManager_RemoveAnimBlockRefWithoutDelete(mi->GetAnimFileIndex());
#endif
	}else if(streamId >= STREAM_OFFSET_TXD && streamId < STREAM_OFFSET_COL){
		// Txd
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_AddRef(streamId - STREAM_OFFSET_TXD);
		PUSH_MEMID(MEMID_STREAM_TEXUTRES);
		// Use the complete C texture dictionary owner with the original live arguments
		success = TxdStore_FinishLoadTxd(streamId - STREAM_OFFSET_TXD, stream);
		POP_MEMID();
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_RemoveRefWithoutDelete(streamId - STREAM_OFFSET_TXD);
	}else{
		assert(0 && "invalid streamId");
	}

	RwStreamClose(stream, &mem);

	Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_LOADED; // rouz edit (ChatGPT)
#ifndef USE_CUSTOM_ALLOCATOR
	Streaming_memoryUsed += StreamingInfo_GetCdSize(&Streaming_modelInfo[streamId]) * CDSTREAM_SECTOR_SIZE; // rouz edit (ChatGPT)
#endif

	if(!success){
		Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
		Streaming_ReRequestModel(streamId); // rouz edit (ChatGPT)
		Streaming_UpdateMemoryUsed();
		return false;
	}

	Streaming_UpdateMemoryUsed();

	endTime = Timer_GetCurrentTimeInCycles() / Timer_GetCyclesPerMillisecond();
	timeDiff = endTime - startTime;
	if(timeDiff > 5)
		debug("%s took %d ms\n", GetObjectName(streamId), timeDiff);

	return true;
}

#define BIGBUILDINGFLAGS STREAMFLAGS_DONT_REMOVE

void
Streaming_RequestBigBuildings(eLevelName level)
{
	int i, n;
	CBuilding *b;

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetBuildingPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		b = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
				if(b && b->bIsBIGBuilding
#ifdef NO_ISLAND_LOADING
		   && (((FrontEndMenuManager.m_PrefsIslandLoading != CMenuManager::ISLAND_LOADING_LOW) && (b != pIslandLODmainlandEntity) &&
		        (b != pIslandLODbeachEntity)) ||
		       (b->m_level == level))
#else
		   && b->m_level == level
#endif
			)
			if(!b->bStreamBIGBuilding)
				Streaming_RequestModel(b->GetModelIndex(), BIGBUILDINGFLAGS); // rouz edit (ChatGPT)
	}
	Streaming_RequestIslands(level);
}

void
Streaming_RequestBigBuildings(eLevelName level, const CVector &pos)
{
	int i, n;
	CBuilding *b;

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetBuildingPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		b = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
				if(b && b->bIsBIGBuilding
#ifdef NO_ISLAND_LOADING
		    && (((FrontEndMenuManager.m_PrefsIslandLoading != CMenuManager::ISLAND_LOADING_LOW) && (b != pIslandLODmainlandEntity) && (b != pIslandLODbeachEntity)
				) || (b->m_level == level))
#else
		   && b->m_level == level
#endif
		)
			if(b->bStreamBIGBuilding){
				if(Renderer_ShouldModelBeStreamed(b, pos))
					Streaming_RequestModel(b->GetModelIndex(), 0); // rouz edit (ChatGPT)
			}else
				Streaming_RequestModel(b->GetModelIndex(), BIGBUILDINGFLAGS); // rouz edit (ChatGPT)
	}
	Streaming_RequestIslands(level);
}

void
Streaming_InstanceBigBuildings(eLevelName level, const CVector &pos)
{
	int i, n;
	CBuilding *b;

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetBuildingPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		b = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
				if(b && b->bIsBIGBuilding && b->m_level == level &&
		   b->bStreamBIGBuilding && b->m_rwObject == nil)
			if(Renderer_ShouldModelBeStreamed(b, pos))
				b->CreateRwObject();
	}
}

void
Streaming_InstanceLoadedModelsInSectorList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *e;
	for(node = list.first; node; node = node->next) {
		e = (CEntity *)node->item;
		if(IsAreaVisible(e->m_area) && e->m_rwObject == nil)
			e->CreateRwObject();
	}
}

void
Streaming_InstanceLoadedModels(const CVector &pos)
{
	// Delegate the range of sectors to the C implementation
	int32_t minX, minY, maxX, maxY;
	World_SectorRange(pos.x - 80.0f, pos.y - 80.0f, pos.x + 80.0f, pos.y + 80.0f, &minX, &minY, &maxX, &maxY);

	int x, y;
	for(y = minY; y <= maxY; y++){
		for(x = minX; x <= maxX; x++){
			CSector *sector = World_GetSector(x, y);
			Streaming_InstanceLoadedModelsInSectorList(sector->m_lists[ENTITYLIST_BUILDINGS]);
			Streaming_InstanceLoadedModelsInSectorList(sector->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
			Streaming_InstanceLoadedModelsInSectorList(sector->m_lists[ENTITYLIST_OBJECTS]);
			Streaming_InstanceLoadedModelsInSectorList(sector->m_lists[ENTITYLIST_DUMMIES]);
		}
	}
}

void
Streaming_RequestIslands(eLevelName level)
{
	ISLAND_LOADING_ISNT(HIGH)
	switch(level){
	case LEVEL_MAINLAND:
		if(islandLODbeach != -1)
			Streaming_RequestModel(islandLODbeach, BIGBUILDINGFLAGS); // rouz edit (ChatGPT)
		break;
	case LEVEL_BEACH:
		if(islandLODmainland != -1)
			Streaming_RequestModel(islandLODmainland, BIGBUILDINGFLAGS); // rouz edit (ChatGPT)
		break;
	default: break;
	}
}

static char *IGnames[] = {
	"player",
	"player2",
	"player3",
	"player4",
	"player5",
	"player6",
	"player7",
	"player8",
	"player9",
	"play10",
	"play11",
	"igken",
	"igcandy",
	"igsonny",
	"igbuddy",
	"igjezz",
	"ighlary",
	"igphil",
	"igmerc",
	"igdick",
	"igdiaz",
	""
};

static char *CSnames[] = {
	"csplay",
	"csplay2",
	"csplay3",
	"csplay4",
	"csplay5",
	"csplay6",
	"csplay7",
	"csplay8",
	"csplay9",
	"csplay10",
	"csplay11",
	"csken",
	"cscandy",
	"cssonny",
	"csbuddy",
	"csjezz",
	"cshlary",
	"csphil",
	"csmerc",
	"csdick",
	"csdiaz",
	""
};

void
Streaming_RequestSpecialModel(int32 modelId, const char *modelName, int32 flags)
{
	CBaseModelInfo *mi;
	int txdId;
	char oldName[48];
	uint32 pos, size;
	int i, n;

	mi = ((CBaseModelInfo*)ModelRegistry_Get(modelId)); // rouz edit (ChatGPT)
	if(strncasecmp("CSPlay", modelName, 6) == 0){
		char *curname = ((CBaseModelInfo*)ModelRegistry_Get(MI_PLAYER))->GetModelName(); // rouz edit (ChatGPT)
		for(int i = 0; CSnames[i][0]; i++){
			if(strcasecmp(curname, IGnames[i]) == 0){
				modelName = CSnames[i];
				break;
			}
		}
	}
	if(!General_faststrcmp(mi->GetModelName(), modelName)){
		// Already have the correct name, just request it
		Streaming_RequestModel(modelId, flags); // rouz edit (ChatGPT)
		return;
	}

	if(mi->GetNumRefs() > 0){
				// Access raw storage through the C store or pool API
		n = CPool_GetSize(Pools_GetPedPool())-1;
				for(i = n; i >= 0 && mi->GetNumRefs() > 0; i--){
						// Access raw storage through the C store or pool API
			CPed *ped = ((CPed*)CPool_GetSlot(Pools_GetPedPool(), i));
						if(ped && ped->GetModelIndex() == modelId &&
			   !ped->IsPlayer() && ped->CanBeDeletedEvenInVehicle())
				TheScripts_RemoveThisPed(ped);
		}
				// Access raw storage through the C store or pool API
		n = CPool_GetSize(Pools_GetObjectPool())-1;
				for(i = n; i >= 0 && mi->GetNumRefs() > 0; i--){
						// Access raw storage through the C store or pool API
			CObject *obj = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), i));
						if(obj && obj->GetModelIndex() == modelId && obj->CanBeDeleted()){
				World_Remove(obj);
				World_RemoveReferencesToDeletedObject(obj);
				// Destroy and release the swapped object without invoking C++ delete.
				obj->~CObject();
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetObjectPool(), obj);
							}
		}
	}

	strcpy(oldName, mi->GetModelName());
	mi->SetModelName(modelName);

	// What exactly is going on here?
	if((CBaseModelInfo*)ModelRegistry_Find(oldName, 0, MODELINFOSIZE-1, nil)){ // rouz edit (ChatGPT)
		// Use the complete C texture dictionary owner with the original live arguments
		txdId = TxdStore_FindTxdSlot(oldName);
		// Use the complete C texture dictionary owner with the original live arguments
		if(txdId != -1 && ((RwTexDictionary*)TxdStore_GetSlot(txdId)->texDict)){
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_AddRef(txdId);
			Streaming_RemoveModel(modelId); // rouz edit (ChatGPT)
			// Use the complete C texture dictionary owner with the original live arguments
			TxdStore_RemoveRefWithoutDelete(txdId);
		}else
			Streaming_RemoveModel(modelId); // rouz edit (ChatGPT)
	}else
		Streaming_RemoveModel(modelId); // rouz edit (ChatGPT)

	bool found = Directory_FindItem(Streaming_ms_pExtraObjectsDir, modelName, &pos, &size);
	assert(found);
	mi->ClearTexDictionary();
	// Use the complete C texture dictionary owner with the original live arguments
	if(TxdStore_FindTxdSlot(modelName) == -1)
		mi->SetTexDictionary("generic");
	else
		mi->SetTexDictionary(modelName);
	StreamingInfo_SetCdPosnAndSize(&Streaming_modelInfo[modelId], pos, size); // rouz edit (ChatGPT)
	Streaming_RequestModel(modelId, flags); // rouz edit (ChatGPT)
}

void
Streaming_RequestSpecialChar(int32 charId, const char *modelName, int32 flags)
{
	Streaming_RequestSpecialModel(charId + MI_SPECIAL01, modelName, flags);
}




void
Streaming_RemoveUnusedBuildings(eLevelName level)
{
	if (rouz.no_unstreaming)
		return;

	if(level != LEVEL_BEACH)
		Streaming_RemoveBuildings(LEVEL_BEACH);
	if(level != LEVEL_MAINLAND)
		Streaming_RemoveBuildings(LEVEL_MAINLAND);
}

void
Streaming_RemoveBuildings(eLevelName level)
{
	int i, n;
	CEntity *e;
	CBaseModelInfo *mi;

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetBuildingPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
				if(e && e->m_level == level){
			mi = ((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					Streaming_RemoveModel(e->GetModelIndex()); // rouz edit (ChatGPT)
			}
		}
	}

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetTreadablePool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CTreadable*)CPool_GetSlot(Pools_GetTreadablePool(), i));
				if(e && e->m_level == level){
			mi = ((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					Streaming_RemoveModel(e->GetModelIndex()); // rouz edit (ChatGPT)
			}
		}
	}

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetObjectPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), i));
				if(e && e->m_level == level){
			mi = ((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered && ((CObject*)e)->ObjectCreatedBy == GAME_OBJECT){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					Streaming_RemoveModel(e->GetModelIndex()); // rouz edit (ChatGPT)
			}
		}
	}

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetDummyPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CDummy*)CPool_GetSlot(Pools_GetDummyPool(), i));
				if(e && e->m_level == level){
			mi = ((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					Streaming_RemoveModel(e->GetModelIndex()); // rouz edit (ChatGPT)
			}
		}
	}
}

void
Streaming_RemoveBuildingsNotInArea(int32 area)
{
	int i, n;
	CEntity *e;

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetBuildingPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
				if(e && e->m_rwObject && !IsAreaVisible(area) &&
		   (!e->bIsBIGBuilding || e->bStreamBIGBuilding)){
			if(e->bIsBIGBuilding)
				Streaming_RequestModel(e->GetModelIndex(), 0); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered)
				e->DeleteRwObject();
		}
	}

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetTreadablePool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CTreadable*)CPool_GetSlot(Pools_GetTreadablePool(), i));
				if(e && e->m_rwObject && !IsAreaVisible(area) &&
		   (!e->bIsBIGBuilding || e->bStreamBIGBuilding)){
			if(e->bIsBIGBuilding)
				Streaming_RequestModel(e->GetModelIndex(), 0); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered)
				e->DeleteRwObject();
		}
	}

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetObjectPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), i));
				if(e && e->m_rwObject && !IsAreaVisible(area) &&
		   (!e->bIsBIGBuilding || e->bStreamBIGBuilding)){
			if(e->bIsBIGBuilding)
				Streaming_RequestModel(e->GetModelIndex(), 0); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered)
				e->DeleteRwObject();
		}
	}

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetDummyPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CDummy*)CPool_GetSlot(Pools_GetDummyPool(), i));
				if(e && e->m_rwObject && !IsAreaVisible(area) &&
		   (!e->bIsBIGBuilding || e->bStreamBIGBuilding)){
			if(e->bIsBIGBuilding)
				Streaming_RequestModel(e->GetModelIndex(), 0); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered)
				e->DeleteRwObject();
		}
	}
}

void
Streaming_RemoveUnusedBigBuildings(eLevelName level)
{
	if (rouz.no_unstreaming)
		return;

	ISLAND_LOADING_IS(LOW)
	{
	if(level != LEVEL_BEACH)
		Streaming_RemoveBigBuildings(LEVEL_BEACH);
	if(level != LEVEL_MAINLAND)
		Streaming_RemoveBigBuildings(LEVEL_MAINLAND);
	}
	Streaming_RemoveIslandsNotUsed(level);
}

void
DeleteIsland(CEntity *island)
{
	if(island == nil)
		return;
	if(island->bImBeingRendered)
		debug("Didn't delete island because it was being rendered\n");
	else{
		island->DeleteRwObject();
		Streaming_RemoveModel(island->GetModelIndex()); // rouz edit (ChatGPT)
	}
}

void
Streaming_RemoveIslandsNotUsed(eLevelName level)
{
	int i;
	if(pIslandLODmainlandEntity == nil)
		// Access raw storage through the C store or pool API
	for(i = CPool_GetSize(Pools_GetBuildingPool())-1; i >= 0; i--){
		// Access raw storage through the C store or pool API
		CBuilding *building = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
			if(building == nil)
			continue;
		if(building->GetModelIndex() == islandLODmainland)
			pIslandLODmainlandEntity = building;
		if(building->GetModelIndex() == islandLODbeach)
			pIslandLODbeachEntity = building;
	}
#ifdef NO_ISLAND_LOADING
	if(FrontEndMenuManager.m_PrefsIslandLoading == CMenuManager::ISLAND_LOADING_HIGH) {
		DeleteIsland(pIslandLODmainlandEntity);
		DeleteIsland(pIslandLODbeachEntity);
	} else
#endif
	switch(level){
	case LEVEL_MAINLAND:
		DeleteIsland(pIslandLODmainlandEntity);
		break;
	case LEVEL_BEACH:
		DeleteIsland(pIslandLODbeachEntity);

		break;
	}
}

void
Streaming_RemoveBigBuildings(eLevelName level)
{
	int i, n;
	CEntity *e;
	CBaseModelInfo *mi;

		// Access raw storage through the C store or pool API
	n = CPool_GetSize(Pools_GetBuildingPool())-1;
		for(i = n; i >= 0; i--){
				// Access raw storage through the C store or pool API
		e = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
				if(e && e->bIsBIGBuilding && e->m_level == level){
			mi = ((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())); // rouz edit (ChatGPT)
			if(!e->bImBeingRendered){
				e->DeleteRwObject();
				if (mi->GetNumRefs() == 0)
					Streaming_RemoveModel(e->GetModelIndex()); // rouz edit (ChatGPT)
			}
		}
	}
}

void
Streaming_RemoveUnusedModelsInLoadedList(void)
{
	// empty
}

void
Streaming_LoadInitialPeds(void)
{
	Streaming_RequestModel(MI_COP, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_MALE01, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_TAXI_D, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
}

void
Streaming_LoadInitialWeapons(void)
{
	Streaming_RequestModel(MI_NIGHTSTICK, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_MISSILE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
}

void
Streaming_LoadInitialVehicles(void)
{
	Streaming_numVehiclesLoaded = 0; // rouz edit (ChatGPT)
	Streaming_lastVehicleDeleted = 0; // rouz edit (ChatGPT)

	Streaming_RequestModel(MI_POLICE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
}

void
Streaming_StreamVehiclesAndPeds(void)
{
	int model;
	static int timeBeforeNextLoad = 0;
	static int modelQualityClass = 0;

	if(RecordGame_IsRecording() || // rouz edit (ChatGPT)
	   RecordGame_IsPlayingBack() // rouz edit (ChatGPT)
#ifdef FIX_BUGS
	   || Replay_IsPlayingBack() // rouz edit (ChatGPT)
#endif
		)
		return;

	// rouz edit
	if(FindPlayerPed()==NULL)
		return;

	if(Wanted_AreSwatRequired(FindPlayerPed()->m_pWanted)){
		Streaming_RequestModel(MI_ENFORCER, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
		Streaming_RequestModel(MI_SWAT, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	}else{
		Streaming_SetModelIsDeletable(MI_ENFORCER); // rouz edit (ChatGPT)
		if(!Streaming_HasModelLoaded(MI_ENFORCER)) // rouz edit (ChatGPT)
			Streaming_SetModelIsDeletable(MI_SWAT); // rouz edit (ChatGPT)
	}

	if(Wanted_AreFbiRequired(FindPlayerPed()->m_pWanted)){
		Streaming_RequestModel(MI_FBIRANCH, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
		Streaming_RequestModel(MI_FBI, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	}else{
		Streaming_SetModelIsDeletable(MI_FBIRANCH); // rouz edit (ChatGPT)
		if(!Streaming_HasModelLoaded(MI_FBIRANCH)) // rouz edit (ChatGPT)
			Streaming_SetModelIsDeletable(MI_FBI); // rouz edit (ChatGPT)
	}

	if(Wanted_AreArmyRequired(FindPlayerPed()->m_pWanted)){
		Streaming_RequestModel(MI_RHINO, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
		Streaming_RequestModel(MI_BARRACKS, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
		Streaming_RequestModel(MI_ARMY, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	}else{
		Streaming_SetModelIsDeletable(MI_RHINO); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_BARRACKS); // rouz edit (ChatGPT)
		if(!Streaming_HasModelLoaded(MI_RHINO) && !Streaming_HasModelLoaded(MI_BARRACKS)) // rouz edit (ChatGPT)
			Streaming_SetModelIsDeletable(MI_ARMY); // rouz edit (ChatGPT)
	}

	if(Wanted_NumOfHelisRequired(FindPlayerPed()->m_pWanted) > 0)
		Streaming_RequestModel(MI_CHOPPER, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	else
		Streaming_SetModelIsDeletable(MI_CHOPPER); // rouz edit (ChatGPT)

	if (Wanted_AreMiamiViceRequired(FindPlayerPed()->m_pWanted)) {
		Streaming_SetModelIsDeletable(MI_VICE1); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE2); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE3); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE4); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE5); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE6); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE7); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE8); // rouz edit (ChatGPT)
		Streaming_RequestModel(MI_VICECHEE, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
		if(Population_numMiamiViceCops == 0) // rouz edit (ChatGPT)
			switch (CarCtrl_MiamiViceCycle) {
			case 0:
				Streaming_RequestModel(MI_VICE1, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				Streaming_RequestModel(MI_VICE2, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				break;
			case 1:
				Streaming_RequestModel(MI_VICE3, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				Streaming_RequestModel(MI_VICE4, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				break;
			case 2:
				Streaming_RequestModel(MI_VICE5, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				Streaming_RequestModel(MI_VICE6, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				break;
			case 3:
				Streaming_RequestModel(MI_VICE7, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				Streaming_RequestModel(MI_VICE8, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				break;
			}
	}
	else {
		Streaming_SetModelIsDeletable(MI_VICECHEE); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE1); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE2); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE3); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE4); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE5); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE6); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE7); // rouz edit (ChatGPT)
		Streaming_SetModelIsDeletable(MI_VICE8); // rouz edit (ChatGPT)
	}

	if(timeBeforeNextLoad >= 0)
		timeBeforeNextLoad--;
	else if(Streaming_numVehiclesLoaded <= desiredNumVehiclesLoaded){ // rouz edit (ChatGPT)
		CZoneInfo zone;
		CVector coors = FindPlayerCoors();
		Zones_GetZoneInfoForTimeOfDay(ZonesPosition_FromVector(*(&coors)), &zone);
		// Delegate to the C implementation
		int32 mostRequestedRating = Streaming_MostRequestedRating(CarCtrl_NumRequestsOfCarRating, zone.carThreshold, zone.boatThreshold);
		model = CarCtrl_ChooseCarModelToLoad(mostRequestedRating);
		if(!Streaming_HasModelLoaded(model)){ // rouz edit (ChatGPT)
			Streaming_RequestModel(model, STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
			timeBeforeNextLoad = 350;
		}
		CarCtrl_NumRequestsOfCarRating[mostRequestedRating] = 0;
	}
}

void
Streaming_StreamZoneModels(const CVector &pos)
{
	int i, j;
	uint16 gangsToLoad, gangCarsToLoad, bit;
	CZoneInfo info;
	static int timeBeforeNextLoad = 0;

	Zones_GetZoneInfoForTimeOfDay(ZonesPosition_FromVector(*(&pos)), &info);

	if(info.pedGroup != Streaming_currentPedGroup){ // rouz edit (ChatGPT)

		// unload pevious group
		if(Streaming_currentPedGroup != -1) // rouz edit (ChatGPT)
			for(i = 0; i < NUMMODELSPERPEDGROUP; i++){
				Streaming_pedGroupLoaded[i] = false; // rouz edit (ChatGPT)
				if(Population_pedGroups[Streaming_currentPedGroup].models[i] != -1){ // rouz edit (ChatGPT)
					Streaming_SetModelIsDeletable(Population_pedGroups[Streaming_currentPedGroup].models[i]); // rouz edit (ChatGPT)
					Streaming_SetModelTxdIsDeletable(Population_pedGroups[Streaming_currentPedGroup].models[i]); // rouz edit (ChatGPT)
				}
			}

		Streaming_currentPedGroup = info.pedGroup; // rouz edit (ChatGPT)

		for(i = 0; i < MAXZONEPEDSLOADED; i++){
			do
				j = General_GetRandomNumberInRangeInt(0, NUMMODELSPERPEDGROUP);
			while(Streaming_pedGroupLoaded[j]); // rouz edit (ChatGPT)
			Streaming_pedGroupLoaded[j] = true; // rouz edit (ChatGPT)
			if(Population_pedGroups[Streaming_currentPedGroup].models[j] != -1) // rouz edit (ChatGPT)
				Streaming_RequestModel(Population_pedGroups[Streaming_currentPedGroup].models[j], STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
		}
		Streaming_numPedsLoaded = MAXZONEPEDSLOADED; // rouz edit (ChatGPT)
		timeBeforeNextLoad = 300;
	}

	if(timeBeforeNextLoad >= 0)
		timeBeforeNextLoad--;
	else{
		// Switch a ped
		int oldMI;
		// Find a ped to unload
		for(i = 0; i < NUMMODELSPERPEDGROUP; i++)
			if(Streaming_pedGroupLoaded[i]){ // rouz edit (ChatGPT)
				oldMI = Population_pedGroups[Streaming_currentPedGroup].models[i]; // rouz edit (ChatGPT)
				if(oldMI != -1 && ((CBaseModelInfo*)ModelRegistry_Get(oldMI))->GetNumRefs() == 0) // rouz edit (ChatGPT)
					break;
			}
		// And load a new one
		if(i != NUMMODELSPERPEDGROUP || Streaming_numPedsLoaded < MAXZONEPEDSLOADED){ // rouz edit (ChatGPT)
			do
				j = General_GetRandomNumberInRangeInt(0, NUMMODELSPERPEDGROUP);
			while(Streaming_pedGroupLoaded[j]); // rouz edit (ChatGPT)
			if(Streaming_numPedsLoaded == MAXZONEPEDSLOADED) // rouz edit (ChatGPT)
				Streaming_pedGroupLoaded[i] = false; // rouz edit (ChatGPT)
			Streaming_pedGroupLoaded[j] = true; // rouz edit (ChatGPT)
			int newMI = Population_pedGroups[Streaming_currentPedGroup].models[j]; // rouz edit (ChatGPT)
			if(newMI != oldMI){
				Streaming_RequestModel(newMI, STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
				debug("Request Ped %s\n", ((CBaseModelInfo*)ModelRegistry_Get(newMI))->GetModelName()); // rouz edit (ChatGPT)
				if(Streaming_numPedsLoaded == MAXZONEPEDSLOADED){ // rouz edit (ChatGPT)
					Streaming_SetModelIsDeletable(oldMI); // rouz edit (ChatGPT)
					Streaming_SetModelTxdIsDeletable(oldMI); // rouz edit (ChatGPT)
					debug("Remove Ped %s\n", ((CBaseModelInfo*)ModelRegistry_Get(oldMI))->GetModelName()); // rouz edit (ChatGPT)
				}else
					Streaming_numPedsLoaded++; // rouz edit (ChatGPT)
				timeBeforeNextLoad = 300;
			}
		}
	}

	Streaming_RequestModel(MI_MALE01, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
	Streaming_RequestModel(MI_TAXI_D, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)

	// Delegate to the C implementation
	Streaming_GangLoadMasks(&info, &gangsToLoad, &gangCarsToLoad);

	if(gangsToLoad == Streaming_loadedGangs && gangCarsToLoad == Streaming_loadedGangCars) // rouz edit (ChatGPT)
		return;

	int gangModelsToload = gangsToLoad | gangCarsToLoad;

	if(gangsToLoad != Streaming_loadedGangs || gangCarsToLoad != Streaming_loadedGangCars){ // rouz edit (ChatGPT)
		for(i = 0; i < NUM_GANGS; i++){
			bit = 1<<i;

			if(gangModelsToload & bit && (Streaming_loadedGangs & bit) == 0){ // rouz edit (ChatGPT)
				Streaming_RequestModel(Gangs_GetGangPedModel1(i), STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
				Streaming_RequestModel(Gangs_GetGangPedModel2(i), STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
				Streaming_loadedGangs |= bit; // rouz edit (ChatGPT)
			}else if((gangModelsToload & bit) == 0 && Streaming_loadedGangs & bit){ // rouz edit (ChatGPT)
				Streaming_SetModelIsDeletable(Gangs_GetGangPedModel1(i)); // rouz edit (ChatGPT)
				Streaming_SetModelIsDeletable(Gangs_GetGangPedModel2(i)); // rouz edit (ChatGPT)
				Streaming_SetModelTxdIsDeletable(Gangs_GetGangPedModel1(i)); // rouz edit (ChatGPT)
				Streaming_SetModelTxdIsDeletable(Gangs_GetGangPedModel2(i)); // rouz edit (ChatGPT)
				Streaming_loadedGangs &= ~bit; // rouz edit (ChatGPT)
			}

			if(Gangs_GetGangVehicleModel(i) != -1){
				if((gangCarsToLoad & bit) && (Streaming_loadedGangCars & bit) == 0){ // rouz edit (ChatGPT)
					Streaming_RequestModel(Gangs_GetGangVehicleModel(i), STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
				}else if((gangCarsToLoad & bit) == 0 && Streaming_loadedGangCars & bit){ // rouz edit (ChatGPT)
					Streaming_SetModelIsDeletable(Gangs_GetGangVehicleModel(i)); // rouz edit (ChatGPT)
					Streaming_SetModelTxdIsDeletable(Gangs_GetGangVehicleModel(i)); // rouz edit (ChatGPT)
				}
			}
		}
		Streaming_loadedGangCars = gangCarsToLoad; // rouz edit (ChatGPT)
	}
}

void
Streaming_LoadBigBuildingsWhenNeeded(void)
{
	// Very much like CCollision::Update and CCollision::LoadCollisionWhenINeedIt
	if(Cutscene_IsProcessing()) // rouz edit (ChatGPT)
		return;

	if(Zones_currentLevel == LEVEL_GENERIC || 
	   Zones_currentLevel == Game_currLevel)
		return;

	Timer_Suspend();
	Game_currLevel = Zones_currentLevel;
	ISLAND_LOADING_IS(LOW)
	{
		DMAudio_SetEffectsFadeVol(0);
		CPad::StopPadsShaking();
		Collision_LoadCollisionScreen(Game_currLevel);
		DMAudio_Service();

		Streaming_RemoveUnusedBigBuildings(Game_currLevel);
		Streaming_RemoveUnusedBuildings(Game_currLevel);
		Streaming_RemoveUnusedModelsInLoadedList();
		Game_TidyUpMemory(true, true);
	}
	CReplay::EmptyReplayBuffer();
	if(Game_currLevel != LEVEL_GENERIC)
		LoadSplash(GetLevelSplashScreen(Game_currLevel));

	ISLAND_LOADING_IS(LOW)
		Streaming_RequestBigBuildings(Game_currLevel, TheCamera.GetPosition());
#ifdef NO_ISLAND_LOADING
	else if(FrontEndMenuManager.m_PrefsIslandLoading == CMenuManager::ISLAND_LOADING_MEDIUM) {
		Streaming_RemoveIslandsNotUsed(Game_currLevel);
		Streaming_RequestIslands(Game_currLevel);
	}
#endif

	Streaming_LoadAllRequestedModels(false);

	Game_TidyUpMemory(true, true);
	Timer_Resume();

	ISLAND_LOADING_IS(LOW)
		DMAudio_SetEffectsFadeVol(127);
}


// Find starting offset of the cdimage we next want to read
// Not useful at all on PC...

/*
 * Streaming buffer size is half of the largest file.
 * Files larger than the buffer size can only be loaded by channel 0,
 * which then uses both buffers, while channel 1 is idle.
 * ms_bLoadingBigModel is set to true to indicate this state.
 */

// Make channel read from disc
void
Streaming_RequestModelStream(int32 ch)
{
	int lastPosn, imgOffset, streamId;
	int totalSize;
	uint32 posn, size, unused;
	int i;
	int haveBigFile, havePed;

	lastPosn = CdStreamGetLastPosn();
	imgOffset = Streaming_GetCdImageOffset(lastPosn); // rouz edit (ChatGPT)
	streamId = Streaming_GetNextFileOnCd(lastPosn - imgOffset, true); // rouz edit (ChatGPT)

	// remove Txds and Anims that aren't requested anymore
	while(streamId != -1){
		if(Streaming_modelInfo[streamId].m_flags & STREAMFLAGS_KEEP_IN_MEMORY) // rouz edit (ChatGPT)
			break;
		if(streamId >= STREAM_OFFSET_TXD && streamId < STREAM_OFFSET_COL){
			if(Streaming_IsTxdUsedByRequestedModels(streamId - STREAM_OFFSET_TXD)) // rouz edit (ChatGPT)
				break;
		}else if(streamId >= STREAM_OFFSET_ANIM){
			assert(streamId < NUMSTREAMINFO);
			if(Streaming_AreAnimsUsedByRequestedModels(streamId - STREAM_OFFSET_ANIM)) // rouz edit (ChatGPT)
				break;
		}else
			break;
		Streaming_RemoveModel(streamId); // rouz edit (ChatGPT)
		// so try next file
		StreamingInfo_GetCdPosnAndSize(&Streaming_modelInfo[streamId], &posn, &size); // rouz edit (ChatGPT)
		streamId = Streaming_GetNextFileOnCd(posn + size, true); // rouz edit (ChatGPT)
	}

	if(streamId == -1)
		return;

	StreamingInfo_GetCdPosnAndSize(&Streaming_modelInfo[streamId], &posn, &size); // rouz edit (ChatGPT)
	if(size > (uint32)Streaming_ms_streamingBufferSize){
		// Can only load big models on channel 0, and 1 has to be idle
		if(ch == 1 || Streaming_channel[1].state != CHANNELSTATE_IDLE) // rouz edit (ChatGPT)
			return;
		Streaming_ms_bLoadingBigModel = true;
	}

	// Load up to 4 adjacent files
	haveBigFile = 0;
	havePed = 0;
	totalSize = 0;
	for(i = 0; i < 4; i++){
		// no more files we can read
		if(streamId == -1 || Streaming_modelInfo[streamId].m_loadState != STREAMSTATE_INQUEUE) // rouz edit (ChatGPT)
			break;

		// also stop at non-priority files
		StreamingInfo_GetCdPosnAndSize(&Streaming_modelInfo[streamId], &unused, &size); // rouz edit (ChatGPT)
		if(Streaming_numPriorityRequests != 0 && !StreamingInfo_IsPriority(&Streaming_modelInfo[streamId])) // rouz edit (ChatGPT)
			break;

		// Can't load certain combinations of files together
		if(streamId < STREAM_OFFSET_TXD){
			if (havePed && ((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetModelType() == MITYPE_PED || // rouz edit (ChatGPT)
			    haveBigFile && ((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetModelType() == MITYPE_VEHICLE || // rouz edit (ChatGPT)
			    Streaming_TxdNotLoaded(((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetTxdSlot()) || // rouz edit (ChatGPT)
			    Streaming_AnimNotLoaded(((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetAnimFileIndex())) // rouz edit (ChatGPT)
				break;
		}else{
			if(haveBigFile && size > 200)
				break;
		}

		// Now add the file
		Streaming_channel[ch].streamIds[i] = streamId; // rouz edit (ChatGPT)
		Streaming_channel[ch].offsets[i] = totalSize; // rouz edit (ChatGPT)
		totalSize += size;

		// To big for buffer, remove again
		if(totalSize > Streaming_ms_streamingBufferSize && i > 0){
			totalSize -= size;
			break;
		}
		if(streamId < STREAM_OFFSET_TXD){
			if (((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetModelType() == MITYPE_PED) // rouz edit (ChatGPT)
				havePed = 1;
			if (((CBaseModelInfo*)ModelRegistry_Get(streamId))->GetModelType() == MITYPE_VEHICLE) // rouz edit (ChatGPT)
				haveBigFile = 1;
		}else{
			if(size > 200)
				haveBigFile = 1;
		}
		Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_READING; // rouz edit (ChatGPT)
		StreamingInfo_RemoveFromList(&Streaming_modelInfo[streamId]); // rouz edit (ChatGPT)
		Streaming_DecrementRef(streamId); // rouz edit (ChatGPT)

		streamId = Streaming_modelInfo[streamId].m_nextID; // rouz edit (ChatGPT)
	}

	// clear remaining slots
	for(; i < 4; i++)
		Streaming_channel[ch].streamIds[i] = -1; // rouz edit (ChatGPT)
	// Now read the data
	assert(!(Streaming_ms_bLoadingBigModel && ch == 1));	// this would clobber the buffer
	if(CdStreamRead(ch, Streaming_ms_pStreamingBuffer[ch], imgOffset+posn, totalSize) == STREAM_NONE)
		debug("FUCKFUCKFUCK\n");
	Streaming_channel[ch].state = CHANNELSTATE_READING; // rouz edit (ChatGPT)
	Streaming_channel[ch].field24 = 0; // rouz edit (ChatGPT)
	Streaming_channel[ch].size = totalSize; // rouz edit (ChatGPT)
	Streaming_channel[ch].position = imgOffset+posn; // rouz edit (ChatGPT)
	Streaming_channel[ch].numTries = 0; // rouz edit (ChatGPT)
}

// Load data previously read from disc
bool
Streaming_ProcessLoadingChannel(int32 ch)
{
	int status;
	int i, id, cdsize;

	status = CdStreamGetStatus(ch);
	if(status != STREAM_NONE){
		// busy
		if(status != STREAM_READING && status != STREAM_WAITING){
			Streaming_ms_channelError = ch;
			Streaming_channel[ch].state = CHANNELSTATE_ERROR; // rouz edit (ChatGPT)
			Streaming_channel[ch].status = status; // rouz edit (ChatGPT)
		}
		return false;
	}

	if(Streaming_channel[ch].state == CHANNELSTATE_STARTED){ // rouz edit (ChatGPT)
		Streaming_channel[ch].state = CHANNELSTATE_IDLE; // rouz edit (ChatGPT)
		Streaming_FinishLoadingLargeFile(&Streaming_ms_pStreamingBuffer[ch][Streaming_channel[ch].offsets[0]*CDSTREAM_SECTOR_SIZE], // rouz edit (ChatGPT)
			Streaming_channel[ch].streamIds[0]); // rouz edit (ChatGPT)
		Streaming_channel[ch].streamIds[0] = -1; // rouz edit (ChatGPT)
	}else{
		Streaming_channel[ch].state = CHANNELSTATE_IDLE; // rouz edit (ChatGPT)
		for(i = 0; i < 4; i++){
			id = Streaming_channel[ch].streamIds[i]; // rouz edit (ChatGPT)
			if(id == -1)
				continue;

			cdsize = StreamingInfo_GetCdSize(&Streaming_modelInfo[id]); // rouz edit (ChatGPT)
			if(id < STREAM_OFFSET_TXD && ((CBaseModelInfo*)ModelRegistry_Get(id))->GetModelType() == MITYPE_VEHICLE && // rouz edit (ChatGPT)
			   Streaming_numVehiclesLoaded >= desiredNumVehiclesLoaded && // rouz edit (ChatGPT)
			   !Streaming_RemoveLoadedVehicle() && // rouz edit (ChatGPT)
			   (Streaming_CanRemoveModel(id) || Streaming_GetAvailableVehicleSlot() == -1)){ // rouz edit (ChatGPT)
				// can't load vehicle
				Streaming_RemoveModel(id); // rouz edit (ChatGPT)
				if(!Streaming_CanRemoveModel(id)) // rouz edit (ChatGPT)
					Streaming_ReRequestModel(id); // rouz edit (ChatGPT)
				// Use the complete C texture dictionary owner with the original live arguments
				else if(TxdStore_GetNumRefs(((CBaseModelInfo*)ModelRegistry_Get(id))->GetTxdSlot()) == 0) // rouz edit (ChatGPT)
					Streaming_RemoveTxd(((CBaseModelInfo*)ModelRegistry_Get(id))->GetTxdSlot()); // rouz edit (ChatGPT)
			}else{
				Streaming_MakeSpaceFor(cdsize * CDSTREAM_SECTOR_SIZE);
				Streaming_ConvertBufferToObject(&Streaming_ms_pStreamingBuffer[ch][Streaming_channel[ch].offsets[i]*CDSTREAM_SECTOR_SIZE], // rouz edit (ChatGPT)
					id);
				if(Streaming_modelInfo[id].m_loadState == STREAMSTATE_STARTED){ // rouz edit (ChatGPT)
					// queue for second part
					Streaming_channel[ch].state = CHANNELSTATE_STARTED; // rouz edit (ChatGPT)
					Streaming_channel[ch].offsets[0] = Streaming_channel[ch].offsets[i]; // rouz edit (ChatGPT)
					Streaming_channel[ch].streamIds[0] = id; // rouz edit (ChatGPT)
					if(i != 0)
						Streaming_channel[ch].streamIds[i] = -1; // rouz edit (ChatGPT)
				}else
					Streaming_channel[ch].streamIds[i] = -1; // rouz edit (ChatGPT)
			}
		}
	}

	if(Streaming_ms_bLoadingBigModel && Streaming_channel[ch].state != CHANNELSTATE_STARTED){ // rouz edit (ChatGPT)
		Streaming_ms_bLoadingBigModel = false;
		// reset channel 1 after loading a big model
		for(i = 0; i < 4; i++)
			Streaming_channel[1].streamIds[i] = -1; // rouz edit (ChatGPT)
		Streaming_channel[1].state = CHANNELSTATE_IDLE; // rouz edit (ChatGPT)
	}

	return true;
}

void
Streaming_RetryLoadFile(int32 ch)
{
	Const char *key;

	CPad::StopPadsShaking();

	if(Streaming_channel[ch].numTries >= 3){ // rouz edit (ChatGPT)
		switch(Streaming_channel[ch].status){ // rouz edit (ChatGPT)
		case STREAM_ERROR_NOCD: key = "NOCD"; break;
		case STREAM_ERROR_OPENCD: key = "OPENCD"; break;
		case STREAM_ERROR_WRONGCD: key = "WRONGCD"; break;
		default: key = "CDERROR"; break;
		}
		Hud_SetMessage(Text_Get(&TheText,key));
		Timer_SetCodePause(true);
	}

	switch(Streaming_channel[ch].state){ // rouz edit (ChatGPT)
	case CHANNELSTATE_ERROR:
		Streaming_channel[ch].numTries++; // rouz edit (ChatGPT)
		if (CdStreamGetStatus(ch) == STREAM_READING || CdStreamGetStatus(ch) == STREAM_WAITING) break;
	case CHANNELSTATE_IDLE:
		CdStreamRead(ch, Streaming_ms_pStreamingBuffer[ch], Streaming_channel[ch].position, Streaming_channel[ch].size); // rouz edit (ChatGPT)
		Streaming_channel[ch].state = CHANNELSTATE_READING; // rouz edit (ChatGPT)
		Streaming_channel[ch].field24 = -600; // rouz edit (ChatGPT)
		break;
	case CHANNELSTATE_READING:
		if(Streaming_ProcessLoadingChannel(ch)){
			Streaming_ms_channelError = -1;
			Timer_SetCodePause(false);
		}
		break;
	}
}

void
Streaming_LoadRequestedModels(void)
{
	static int currentChannel = 0;

	// We can't read with channel 1 while channel 0 is using its buffer
	if(Streaming_ms_bLoadingBigModel)
		currentChannel = 0;

	// We have data, load
	if(Streaming_channel[currentChannel].state == CHANNELSTATE_READING || // rouz edit (ChatGPT)
	   Streaming_channel[currentChannel].state == CHANNELSTATE_STARTED) // rouz edit (ChatGPT)
		Streaming_ProcessLoadingChannel(currentChannel);

	if(Streaming_ms_channelError == -1){
		// Channel is idle, read more data
		if(Streaming_channel[currentChannel].state == CHANNELSTATE_IDLE) // rouz edit (ChatGPT)
			Streaming_RequestModelStream(currentChannel);
		// Switch channel
		if(Streaming_channel[currentChannel].state != CHANNELSTATE_STARTED) // rouz edit (ChatGPT)
			currentChannel = 1 - currentChannel;
	}
}


// Let's load models in 4 threads; when one of them becomes idle, process the file, and fill thread with another file. Unfortunately processing models are still single-threaded.
// Currently only supported on POSIX streamer.
// WIP - some files are loaded swapped (CdStreamPosix problem?)
#if 0 //def ONE_THREAD_PER_CHANNEL
void
Streaming_LoadAllRequestedModels(bool priority)
{
	static bool bInsideLoadAll = false;
	int imgOffset, streamId, status;
	int i;
	uint32 posn, size;

	if(bInsideLoadAll)
		return;
	bInsideLoadAll = true;

	Streaming_FlushChannels(); // rouz edit (ChatGPT)
	imgOffset = Streaming_GetCdImageOffset(CdStreamGetLastPosn()); // rouz edit (ChatGPT)

	int streamIds[ARRAY_SIZE(Streaming_ms_pStreamingBuffer)];
	int streamSizes[ARRAY_SIZE(Streaming_ms_pStreamingBuffer)];
	int streamPoses[ARRAY_SIZE(Streaming_ms_pStreamingBuffer)];
	int readOrder[4] = {-1}; // Channel IDs ordered by read time
	int readI = 0;
	int processI = 0;
	bool first = true;

	// All those "first" checks are because of variables aren't initialized in first pass.

	while (true) {
		for (int i=0; i<ARRAY_SIZE(Streaming_ms_pStreamingBuffer); i++) {

			// Channel has file to load
			if (!first && streamIds[i] != -1) {
				continue;
			}

			if(Streaming_endRequestedList.m_prev != &Streaming_startRequestedList){ // rouz edit (ChatGPT)
				streamId = Streaming_GetNextFileOnCd(0, priority); // rouz edit (ChatGPT)
				if(streamId == -1){
					streamIds[i] = -1;
					break;
				}

				if (StreamingInfo_GetCdPosnAndSize(&Streaming_modelInfo[streamId], &posn, &size)) { // rouz edit (ChatGPT)
					streamIds[i] = -1;

					// Big file, needs 2 buffer
					if (size > (uint32)Streaming_ms_streamingBufferSize) {
						if (i + 1 == ARRAY_SIZE(Streaming_ms_pStreamingBuffer))
							break;
						else if (!first && streamIds[i+1] != -1)
							continue;

					} else {
						// Buffer of current channel is part of a "big file", pass
						if (i != 0 && streamIds[i-1] != -1 && streamSizes[i-1] > (uint32)Streaming_ms_streamingBufferSize)
							continue;
					}
					StreamingInfo_RemoveFromList(&Streaming_modelInfo[streamId]); // rouz edit (ChatGPT)
					Streaming_DecrementRef(streamId); // rouz edit (ChatGPT)

					streamIds[i] = streamId;
					streamSizes[i] = size;
					streamPoses[i] = posn;

					if (!first)
						assert(readOrder[readI] == -1);

					//printf("read: order %d, ch %d, id %d, size %d\n", readI, i, streamId, size);

					CdStreamRead(i, Streaming_ms_pStreamingBuffer[i], imgOffset+posn, size);
					readOrder[readI] = i;
					if (first && readI+1 != ARRAY_SIZE(readOrder))
						readOrder[readI+1] = -1;

					readI = (readI + 1) % ARRAY_SIZE(readOrder);
				} else {
					StreamingInfo_RemoveFromList(&Streaming_modelInfo[streamId]); // rouz edit (ChatGPT)
					Streaming_DecrementRef(streamId); // rouz edit (ChatGPT)

					Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_LOADED; // rouz edit (ChatGPT)
					streamIds[i] = -1;
				}
			} else {
				streamIds[i] = -1;
				break;
			}
		}

		first = false;
		int nextChannel = readOrder[processI];

		// Now start processing
		if (nextChannel == -1 || streamIds[nextChannel] == -1)
			break;

		//printf("process: order %d, ch %d, id %d\n", processI, nextChannel, streamIds[nextChannel]);

		// Try again on error
		while (CdStreamSync(nextChannel) != STREAM_NONE) {
			CdStreamRead(nextChannel, Streaming_ms_pStreamingBuffer[nextChannel], imgOffset+streamPoses[nextChannel], streamSizes[nextChannel]);
		}
		Streaming_modelInfo[streamIds[nextChannel]].m_loadState = STREAMSTATE_READING; // rouz edit (ChatGPT)

		Streaming_MakeSpaceFor(streamSizes[nextChannel] * CDSTREAM_SECTOR_SIZE);
		Streaming_ConvertBufferToObject(Streaming_ms_pStreamingBuffer[nextChannel], streamIds[nextChannel]);
		if(Streaming_modelInfo[streamIds[nextChannel]].m_loadState == STREAMSTATE_STARTED) // rouz edit (ChatGPT)
			Streaming_FinishLoadingLargeFile(Streaming_ms_pStreamingBuffer[nextChannel], streamIds[nextChannel]);

		if(streamIds[nextChannel] < STREAM_OFFSET_TXD){
			CSimpleModelInfo *mi = (CSimpleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(streamIds[nextChannel])); // rouz edit (ChatGPT)
			if(mi->IsSimple())
				mi->m_alpha = 255;
		}
		streamIds[nextChannel] = -1;
		readOrder[processI] = -1;
		processI = (processI + 1) % ARRAY_SIZE(readOrder);
	}

	Streaming_ms_bLoadingBigModel = false;
	for(i = 0; i < 4; i++){
		Streaming_channel[1].streamIds[i] = -1; // rouz edit (ChatGPT)
		Streaming_channel[1].offsets[i] = -1; // rouz edit (ChatGPT)
	}
	Streaming_channel[1].state = CHANNELSTATE_IDLE; // rouz edit (ChatGPT)
	bInsideLoadAll = false;
}
#else
void
Streaming_LoadAllRequestedModels(bool priority)
{
	static bool bInsideLoadAll = false;
	int imgOffset, streamId, status;
	int i;
	uint32 posn, size;

	int numRequests = 4*Streaming_numModelsRequested; // rouz edit (ChatGPT)

	if(bInsideLoadAll)
		return;
	bInsideLoadAll = true;

	if(priority)
		numRequests = Streaming_numPriorityRequests; // rouz edit (ChatGPT)

	Streaming_FlushChannels(); // rouz edit (ChatGPT)
	imgOffset = Streaming_GetCdImageOffset(CdStreamGetLastPosn()); // rouz edit (ChatGPT)

	while(Streaming_endRequestedList.m_prev != &Streaming_startRequestedList && numRequests > 0){ // rouz edit (ChatGPT)
		numRequests--;
		streamId = Streaming_GetNextFileOnCd(0, priority); // rouz edit (ChatGPT)
		if(streamId == -1)
			break;

		StreamingInfo_RemoveFromList(&Streaming_modelInfo[streamId]); // rouz edit (ChatGPT)
		Streaming_channel[0].streamIds[0] = streamId; // rouz edit (ChatGPT)
		Streaming_DecrementRef(streamId); // rouz edit (ChatGPT)

		if(StreamingInfo_GetCdPosnAndSize(&Streaming_modelInfo[streamId], &posn, &size)){ // rouz edit (ChatGPT)
			do
				status = CdStreamRead(0, Streaming_ms_pStreamingBuffer[0], imgOffset+posn, size);
			while(CdStreamSync(0) || status == STREAM_NONE);
			Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_READING; // rouz edit (ChatGPT)

			Streaming_MakeSpaceFor(size * CDSTREAM_SECTOR_SIZE);
			Streaming_ConvertBufferToObject(Streaming_ms_pStreamingBuffer[0], streamId);
			if(Streaming_modelInfo[streamId].m_loadState == STREAMSTATE_STARTED) // rouz edit (ChatGPT)
				Streaming_FinishLoadingLargeFile(Streaming_ms_pStreamingBuffer[0], streamId);

			if(streamId < STREAM_OFFSET_TXD){
				CSimpleModelInfo *mi = (CSimpleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(streamId)); // rouz edit (ChatGPT)
				if(mi->IsSimple())
					mi->m_alpha = 255;
			}
		}else{
			// empty
			Streaming_modelInfo[streamId].m_loadState = STREAMSTATE_LOADED; // rouz edit (ChatGPT)
		}
	}

	Streaming_ms_bLoadingBigModel = false;
	for(i = 0; i < 4; i++){
		Streaming_channel[1].streamIds[i] = -1; // rouz edit (ChatGPT)
		Streaming_channel[1].offsets[i] = -1; // rouz edit (ChatGPT)
	}
	Streaming_channel[1].state = CHANNELSTATE_IDLE; // rouz edit (ChatGPT)
	bInsideLoadAll = false;
}
#endif

void
Streaming_ImGonnaUseStreamingMemory(void)
{
	PUSH_MEMID(MEMID_STREAM);
}

void
Streaming_IHaveUsedStreamingMemory(void)
{
	POP_MEMID();
	Streaming_UpdateMemoryUsed();
}

void
Streaming_UpdateMemoryUsed(void)
{
#ifdef USE_CUSTOM_ALLOCATOR
	// Read streaming categories through the complete C heap owner
	Streaming_memoryUsed = // rouz edit (ChatGPT)
		MemoryHeap_GetMemoryUsed(&gMainHeap,MEMID_STREAM) +
		MemoryHeap_GetMemoryUsed(&gMainHeap,MEMID_STREAM_MODELS) +
		MemoryHeap_GetMemoryUsed(&gMainHeap,MEMID_STREAM_TEXUTRES) +
		MemoryHeap_GetMemoryUsed(&gMainHeap,MEMID_STREAM_COLLISION) +
		MemoryHeap_GetMemoryUsed(&gMainHeap,MEMID_STREAM_ANIMATION);
#endif
}

#define STREAM_DIST 80.0f

void
Streaming_AddModelsToRequestList(const CVector &pos, int32 flags)
{
	float xmin, xmax, ymin, ymax;
	int ixmin, ixmax, iymin, iymax;
	int ix, iy;
	int dx, dy, d;
	CSector *sect;

	xmin = pos.x - STREAM_DIST;
	ymin = pos.y - STREAM_DIST;
	xmax = pos.x + STREAM_DIST;
	ymax = pos.y + STREAM_DIST;

	// Delegate to the C implementation
	World_SectorRangeClamped(xmin, ymin, xmax, ymax, &ixmin, &iymin, &ixmax, &iymax);

	World_AdvanceCurrentScanCode();

	for(iy = iymin; iy <= iymax; iy++){
		dy = iy - World_GetSectorIndexY(pos.y);
		for(ix = ixmin; ix <= ixmax; ix++){

			if(Renderer_m_loadingPriority && Streaming_numModelsRequested > 5) // rouz edit (ChatGPT)
				return;

			dx = ix - World_GetSectorIndexX(pos.x);
			d = World_SectorStreamRing(dx, dy);
			sect = World_GetSector(ix, iy);
			if(d == 0){
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], flags);
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], flags);
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], flags);
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], flags);
			}else if(d == 1){
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], pos.x, pos.y, xmin, ymin, xmax, ymax, flags);
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], pos.x, pos.y, xmin, ymin, xmax, ymax, flags);
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], pos.x, pos.y, xmin, ymin, xmax, ymax, flags);
				Streaming_ProcessEntitiesInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], pos.x, pos.y, xmin, ymin, xmax, ymax, flags);
			}
		}
	}
}

void
Streaming_ProcessEntitiesInSectorList(CPtrList &list, float x, float y, float xmin, float ymin, float xmax, float ymax, int32 flags)
{
	CPtrNode *node;
	CEntity *e;
	float lodDistSq;
	CVector2D pos;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;

		if(e->m_scanCode == World_GetCurrentScanCode())
			continue;

		e->m_scanCode = World_GetCurrentScanCode();
		if(!e->bStreamingDontDelete && IsAreaVisible(e->m_area) && !e->bDontStream && e->bIsVisible){
			CTimeModelInfo *mi = (CTimeModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())); // rouz edit (ChatGPT)
			if (mi->GetModelType() != MITYPE_TIME || Clock_GetIsTimeInRange(mi->m_timeData.timeOn, mi->m_timeData.timeOff)) { // rouz edit (ChatGPT)
				lodDistSq = sq(SimpleModel_LargestDistance(mi->m_lodDistances, mi->m_numAtomics, mi->m_firstDamaged, mi->m_isDamaged, TheCamera.LODDistMultiplier)); // rouz edit (ChatGPT)
				pos = CVector2D(e->GetPosition());
				// Delegate the check that the entity should be requested to the C implementation
				if(Streaming_ShouldRequestEntity(pos.x, pos.y, xmin, ymin, xmax, ymax, x, y, lodDistSq, sq(STREAM_DIST)))
					Streaming_RequestModel(e->GetModelIndex(), flags); // rouz edit (ChatGPT)
			}
		}
	}
}

void
Streaming_ProcessEntitiesInSectorList(CPtrList &list, int32 flags)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;

		if(e->m_scanCode == World_GetCurrentScanCode())
			continue;

		e->m_scanCode = World_GetCurrentScanCode();
		if(!e->bStreamingDontDelete && IsAreaVisible(e->m_area) && !e->bDontStream && e->bIsVisible){
			CTimeModelInfo *mi = (CTimeModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex())); // rouz edit (ChatGPT)
			if (mi->GetModelType() != MITYPE_TIME || Clock_GetIsTimeInRange(mi->m_timeData.timeOn, mi->m_timeData.timeOff)) // rouz edit (ChatGPT)
				Streaming_RequestModel(e->GetModelIndex(), flags); // rouz edit (ChatGPT)
		}
	}
}

void
Streaming_DeleteFarAwayRwObjects(const CVector &pos)
{
	int posx, posy;
	int x, y;
	int r, i;
	CSector *sect;

	posx = World_GetSectorIndexX(pos.x);
	posy = World_GetSectorIndexY(pos.y);

	// Move oldSectorX/Y to new sector and delete RW objects in its "wake" for every step:
	// O is the old sector, <- is the direction in which we move it,
	// X are the sectors we delete RW objects from (except we go up to 10)
	//            X
	//          X X
	//        X X X
	//        X X X
	// <- O   X X X
	//        X X X
	//        X X X
	//          X X
	//            X

	while(posx != Streaming_ms_oldSectorX){
		if(posx < Streaming_ms_oldSectorX){
			for(r = 2; r <= 10; r++){
				x = Streaming_ms_oldSectorX + r;
				if(x < 0)
					continue;
				if(x >= NUMSECTORS_X)
					break;

				for(i = -r; i <= r; i++){
					y = Streaming_ms_oldSectorY + i;
					if(y < 0)
						continue;
					if(y >= NUMSECTORS_Y)
						break;

					sect = World_GetSector(x, y);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					Streaming_DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], Streaming_ms_oldSectorX, Streaming_ms_oldSectorY);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			Streaming_ms_oldSectorX--;
		}else{
			for(r = 2; r <= 10; r++){
				x = Streaming_ms_oldSectorX - r;
				if(x < 0)
					break;
				if(x >= NUMSECTORS_X)
					continue;

				for(i = -r; i <= r; i++){
					y = Streaming_ms_oldSectorY + i;
					if(y < 0)
						continue;
					if(y >= NUMSECTORS_Y)
						break;

					sect = World_GetSector(x, y);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					Streaming_DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], Streaming_ms_oldSectorX, Streaming_ms_oldSectorY);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			Streaming_ms_oldSectorX++;
		}
	}

	while(posy != Streaming_ms_oldSectorY){
		if(posy < Streaming_ms_oldSectorY){
			for(r = 2; r <= 10; r++){
				y = Streaming_ms_oldSectorY + r;
				if(y < 0)
					continue;
				if(y >= NUMSECTORS_Y)
					break;

				for(i = -r; i <= r; i++){
					x = Streaming_ms_oldSectorX + i;
					if(x < 0)
						continue;
					if(x >= NUMSECTORS_X)
						break;

					sect = World_GetSector(x, y);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					Streaming_DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], Streaming_ms_oldSectorX, Streaming_ms_oldSectorY);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			Streaming_ms_oldSectorY--;
		}else{
			for(r = 2; r <= 10; r++){
				y = Streaming_ms_oldSectorY - r;
				if(y < 0)
					break;
				if(y >= NUMSECTORS_Y)
					continue;

				for(i = -r; i <= r; i++){
					x = Streaming_ms_oldSectorX + i;
					if(x < 0)
						continue;
					if(x >= NUMSECTORS_X)
						break;

					sect = World_GetSector(x, y);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
					Streaming_DeleteRwObjectsInOverlapSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP], Streaming_ms_oldSectorX, Streaming_ms_oldSectorY);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
					Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				}
			}
			Streaming_ms_oldSectorY++;
		}
	}
}

void
Streaming_DeleteAllRwObjects(void)
{
	int x, y;
	CSector *sect;

	if (rouz.no_unstreaming)
		return;

	for(x = 0; x < NUMSECTORS_X; x++)
		for(y = 0; y < NUMSECTORS_Y; y++){
			sect = World_GetSector(x, y);
			Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
			Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
			Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
			Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
			Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
			Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
		}
}

void
Streaming_DeleteRwObjectsAfterDeath(const CVector &pos)
{
	int ix, iy;
	int x, y;
	CSector *sect;

	ix = World_GetSectorIndexX(pos.x);
	iy = World_GetSectorIndexY(pos.y);

	for(x = 0; x < NUMSECTORS_X; x++)
		for(y = 0; y < NUMSECTORS_Y; y++)
			// Delegate the check that the sector is far from the player to the C implementation
			if(Streaming_SectorIsFar(ix, iy, x, y)){
				sect = World_GetSector(x, y);
				Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS]);
				Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS_OVERLAP]);
				Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS]);
				Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_OBJECTS_OVERLAP]);
				Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES]);
				Streaming_DeleteRwObjectsInSectorList(sect->m_lists[ENTITYLIST_DUMMIES_OVERLAP]);
			}
}

void
Streaming_DeleteRwObjectsBehindCamera(size_t mem)
{
	int ix, iy;
	int x, y;
	int xmin, xmax, ymin, ymax;
	int inc;
	CSector *sect;

	if (rouz.no_unstreaming)
		return;

	if(Streaming_memoryUsed < mem) // rouz edit (ChatGPT)
		return;

	ix = World_GetSectorIndexX(TheCamera.GetPosition().x);
	iy = World_GetSectorIndexY(TheCamera.GetPosition().y);

	if(General_AbsGreater(TheCamera.GetForward().x, TheCamera.GetForward().y)){
		// looking west/east

		// Delegate the span of sectors to the C implementation
		Streaming_SectorSpan(iy, NUMSECTORS_Y, &ymin, &ymax);
		assert(ymin <= ymax);

		// Delete a block of sectors that we know is behind the camera
		// Delegate the range of sectors to scan to the C implementation
		Streaming_SectorRange(ix, TheCamera.GetForward().x > 0.0f, true, NUMSECTORS_X, &xmin, &xmax, &inc);
		for(y = ymin; y <= ymax; y++){
			for(x = xmin; x != xmax; x += inc){
				sect = World_GetSector(x, y);
				if(Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

		
		while(Streaming_RemoveLoadedZoneModel()) // rouz edit (ChatGPT)
			if(Streaming_memoryUsed < mem) // rouz edit (ChatGPT)
				return;

		// Now a block that intersects with the camera's frustum
		// Delegate the range of sectors to scan to the C implementation
		Streaming_SectorRange(ix, TheCamera.GetForward().x > 0.0f, false, NUMSECTORS_X, &xmin, &xmax, &inc);
		for(y = ymin; y <= ymax; y++){
			for(x = xmin; x != xmax; x += inc){
				sect = World_GetSector(x, y);
				if(Streaming_DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   Streaming_DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   Streaming_DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

		// As last resort, delete objects from the last step more aggressively
		for(y = ymin; y <= ymax; y++){
			for(x = xmax; x != xmin; x -= inc){
				sect = World_GetSector(x, y);
				if(Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}
	}else{
		// looking north/south

		// Delegate the span of sectors to the C implementation
		Streaming_SectorSpan(ix, NUMSECTORS_X, &xmin, &xmax);
		assert(xmin <= xmax);

		// Delete a block of sectors that we know is behind the camera
		// Delegate the range of sectors to scan to the C implementation
		Streaming_SectorRange(iy, TheCamera.GetForward().y > 0.0f, true, NUMSECTORS_Y, &ymin, &ymax, &inc);
		for(x = xmin; x <= xmax; x++){
			for(y = ymin; y != ymax; y += inc){
				sect = World_GetSector(x, y);
				if(Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

		while(Streaming_RemoveLoadedZoneModel()) // rouz edit (ChatGPT)
			if(Streaming_memoryUsed < mem) // rouz edit (ChatGPT)
				return;

		// Now a block that intersects with the camera's frustum
		// Delegate the range of sectors to scan to the C implementation
		Streaming_SectorRange(iy, TheCamera.GetForward().y > 0.0f, false, NUMSECTORS_Y, &ymin, &ymax, &inc);
		for(x = xmin; x <= xmax; x++){
			for(y = ymin; y != ymax; y += inc){
				sect = World_GetSector(x, y);
				if(Streaming_DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   Streaming_DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   Streaming_DeleteRwObjectsNotInFrustumInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}

// this is gone in mobile together with RemoveReferencedTxds
//		if(RemoveReferencedTxds(mem))
//			return;

		// As last resort, delete objects from the last step more aggressively
		for(x = xmin; x <= xmax; x++){
			for(y = ymax; y != ymin; y -= inc){
				sect = World_GetSector(x, y);
				if(Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_BUILDINGS], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_DUMMIES], mem) ||
				   Streaming_DeleteRwObjectsBehindCameraInSectorList(sect->m_lists[ENTITYLIST_OBJECTS], mem))
					return;
			}
		}
	}

	while(Streaming_memoryUsed >= mem && Streaming_RemoveLeastUsedModel(0)); // rouz edit (ChatGPT)
}

void
Streaming_DeleteRwObjectsInSectorList(CPtrList &list)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(!e->bStreamingDontDelete && !e->bImBeingRendered)
			e->DeleteRwObject();
	}
}

void
Streaming_DeleteRwObjectsInOverlapSectorList(CPtrList &list, int32 x, int32 y)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(e->m_rwObject && !e->bStreamingDontDelete && !e->bImBeingRendered){
			// Now this is pretty weird...
			if(Abs(World_GetSectorIndexX(e->GetPosition().x) - x) >= 1.6f)
//			{
				e->DeleteRwObject();
//				return;		// BUG?
//			}
			else	// FIX?
			if(Abs(World_GetSectorIndexY(e->GetPosition().y) - y) >= 1.6f)
				e->DeleteRwObject();
		}
	}
}

bool
Streaming_DeleteRwObjectsBehindCameraInSectorList(CPtrList &list, size_t mem)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(!e->bStreamingDontDelete && !e->bImBeingRendered &&
		   e->m_rwObject && Streaming_modelInfo[e->GetModelIndex()].m_next && // rouz edit (ChatGPT)
		   FindPlayerPed()->m_pCurSurface != e){
			e->DeleteRwObject();
			if (((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex()))->GetNumRefs() == 0) { // rouz edit (ChatGPT)
				Streaming_RemoveModel(e->GetModelIndex()); // rouz edit (ChatGPT)
				if(Streaming_memoryUsed < mem) // rouz edit (ChatGPT)
					return true;
			}
		}
	}
	return false;
}

bool
Streaming_DeleteRwObjectsNotInFrustumInSectorList(CPtrList &list, size_t mem)
{
	CPtrNode *node;
	CEntity *e;

	for(node = list.first; node; node = node->next){
		e = (CEntity*)node->item;
		if(!e->bStreamingDontDelete && !e->bImBeingRendered &&
		   e->m_rwObject && (!e->IsVisible() || e->bOffscreen) && Streaming_modelInfo[e->GetModelIndex()].m_next){ // rouz edit (ChatGPT)
			e->DeleteRwObject();
			if (((CBaseModelInfo*)ModelRegistry_Get(e->GetModelIndex()))->GetNumRefs() == 0) { // rouz edit (ChatGPT)
				Streaming_RemoveModel(e->GetModelIndex()); // rouz edit (ChatGPT)
				if(Streaming_memoryUsed < mem) // rouz edit (ChatGPT)
					return true;
			}
		}
	}
	return false;
}

void
Streaming_MakeSpaceFor(int32 size)
{
	if (rouz.no_unstreaming)
		return;

#ifdef FIX_BUGS
#define MB (1024 * 1024)
	if(Streaming_ms_memoryAvailable == 0) {
		extern size_t _dwMemAvailPhys;
		Streaming_ms_memoryAvailable = (_dwMemAvailPhys - 10 * MB) / 2;
		if(Streaming_ms_memoryAvailable < 65 * MB) Streaming_ms_memoryAvailable = 65 * MB;
	}
#undef MB
#endif
	while(Streaming_memoryUsed >= Streaming_ms_memoryAvailable - size) // rouz edit (ChatGPT)
		if(!Streaming_RemoveLeastUsedModel(STREAMFLAGS_20)){ // rouz edit (ChatGPT)
			Streaming_DeleteRwObjectsBehindCamera(Streaming_ms_memoryAvailable - size);
			return;
		}
}

void
Streaming_LoadScene(const CVector &pos)
{
	CStreamingInfo *si, *prev;
	eLevelName level;

	level = Zones_GetLevelFromPosition(ZonesPosition_FromVector(*(&pos)));
	debug("Start load scene\n");
	for(si = Streaming_endRequestedList.m_prev; si != &Streaming_startRequestedList; si = prev){ // rouz edit (ChatGPT)
		prev = si->m_prev;
		if((si->m_flags & (STREAMFLAGS_KEEP_IN_MEMORY|STREAMFLAGS_PRIORITY)) == 0)
			Streaming_RemoveModel(si - Streaming_modelInfo); // rouz edit (ChatGPT)
	}
	Renderer_m_loadingPriority = false;
	Streaming_DeleteAllRwObjects();
	if(level == LEVEL_GENERIC)
		level = Game_currLevel;
	Game_currLevel = level;
	Streaming_RemoveUnusedBigBuildings(level);
	Streaming_RequestBigBuildings(level, pos);
	Streaming_RequestBigBuildings(LEVEL_GENERIC, pos);
	Streaming_RemoveIslandsNotUsed(level);
	Streaming_LoadAllRequestedModels(false);
	Streaming_InstanceBigBuildings(level, pos);
	Streaming_InstanceBigBuildings(LEVEL_GENERIC, pos);
	Streaming_AddModelsToRequestList(pos, STREAMFLAGS_20);
	Radar_StreamRadarSections(pos);

	if (!Game_IsInInterior()) {
		for (int i = 0; i < 5; i++) {
			CZoneInfo zone;
			Zones_GetZoneInfoForTimeOfDay(ZonesPosition_FromVector(*(&pos)), &zone);
			int32 model = CarCtrl_ChooseCarModelToLoad(CarCtrl_ChooseCarRating(&zone));
			Streaming_RequestModel(model, STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
		}
	}
	Streaming_LoadAllRequestedModels(false);
	Streaming_InstanceLoadedModels(pos);

	for(int i = 0; i < NUMSTREAMINFO; i++)
		Streaming_modelInfo[i].m_flags &= ~STREAMFLAGS_20; // rouz edit (ChatGPT)
	debug("End load scene\n");
}

void
Streaming_LoadSceneCollision(const CVector &pos)
{
	ColStore_LoadCollision(pos);
	Streaming_LoadAllRequestedModels(false);
}

void
Streaming_MemoryCardSave(uint8 *buf, uint32 *size)
{
	int i;

	*size = NUM_DEFAULT_MODELS;
	for(i = 0; i < NUM_DEFAULT_MODELS; i++)
		if(Streaming_modelInfo[i].m_loadState == STREAMSTATE_LOADED) // rouz edit (ChatGPT)
			buf[i] = Streaming_modelInfo[i].m_flags; // rouz edit (ChatGPT)
		else
			buf[i] = 0xFF;
}

void 
Streaming_MemoryCardLoad(uint8 *buf, uint32 size)
{
	uint32 i;

	assert(size == NUM_DEFAULT_MODELS);
	for(i = 0; i < size; i++)
		if(Streaming_modelInfo[i].m_loadState == STREAMSTATE_LOADED) // rouz edit (ChatGPT)
			if(buf[i] != 0xFF)
				Streaming_modelInfo[i].m_flags = buf[i]; // rouz edit (ChatGPT)
}

void
Streaming_UpdateForAnimViewer(void)
{
	if (Streaming_ms_channelError == -1) { // rouz edit (ChatGPT)
		Streaming_AddModelsToRequestList(CVector(0.0f, 0.0f, 0.0f), 0);
		Streaming_LoadRequestedModels();
		// original modifier was %d
		sprintf(gString, "Requested %d, memory size %zuK\n", Streaming_numModelsRequested, 2 * Streaming_memoryUsed); // rouz edit (ChatGPT)
	}
	else {
		Streaming_RetryLoadFile(Streaming_ms_channelError); // rouz edit (ChatGPT)
	}
}


void
Streaming_PrintStreamingBufferState()
{
	char str[128];
	wchar wstr[128];
	uint32 offset, size;

	Timer_Stop();
	int i = 0;
	while (i < NUMSTREAMINFO) {
		while (true) {
			int j = 0;
			DoRWStuffStartOfFrame(50, 50, 50, 0, 0, 0, 255);
			CPad::UpdatePads();
			// Preserve the original operation through the C sprite interface
			Sprite2d_InitPerFrame();
			Font_InitPerFrame();
			DefinedState();

			CRect unusedRect = Rect_Make(0, 0, RsGlobal.maximumWidth, RsGlobal.maximumHeight);
			CRGBA unusedColor(255, 255, 255, 255);
			Font_SetFontStyle(FONT_BANK);
			Font_SetBackgroundOff();
			Font_SetWrapx(DEFAULT_SCREEN_WIDTH);
			Font_SetScale(0.5f, 0.75f);
			Font_SetCentreOff();
			Font_SetCentreSize(DEFAULT_SCREEN_WIDTH);
			Font_SetJustifyOff();
			Font_SetColor(CRGBA(200, 200, 200, 200));
			Font_SetBackGroundOnlyTextOff();
			int modelIndex = i;
			if (modelIndex < NUMSTREAMINFO) {
				int y = 24;
				for ( ; j < 34 && modelIndex < NUMSTREAMINFO; modelIndex++) {
					CStreamingInfo *streamingInfo = &Streaming_modelInfo[modelIndex]; // rouz edit (ChatGPT)
					CBaseModelInfo *modelInfo = ((CBaseModelInfo*)ModelRegistry_Get(modelIndex)); // rouz edit (ChatGPT)
					if (streamingInfo->m_loadState != STREAMSTATE_LOADED || !StreamingInfo_GetCdPosnAndSize(streamingInfo, &offset, &size))
						continue;

					if (modelIndex >= STREAM_OFFSET_TXD)
						// Use the complete C texture dictionary owner with the original live arguments
						sprintf(str, "txd %s, refs %d, size %dK, flags 0x%x", TxdStore_GetTxdName(modelIndex - STREAM_OFFSET_TXD),
						        // Use the complete C texture dictionary owner with the original live arguments
						        TxdStore_GetNumRefs(modelIndex - STREAM_OFFSET_TXD), 2 * size, streamingInfo->m_flags);
					else
						sprintf(str, "model %d,%s, refs%d, size%dK, flags%x", modelIndex, modelInfo->GetModelName(), modelInfo->GetNumRefs(), 2 * size,
						        streamingInfo->m_flags);
					AsciiToUnicode(str, wstr);
					Font_PrintString(24.0f, y, wstr);
					y += 12;
					j++;
				}
			}

			if (CPad::GetPad(1)->GetCrossJustDown())
				i = modelIndex;

			if (!CPad::GetPad(1)->GetTriangleJustDown())
				break;

			i = 0;
			Font_DrawFonts();
			DoRWStuffEndOfFrame();
		}
		Font_DrawFonts();
		DoRWStuffEndOfFrame();
	}
	Timer_Update();
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
