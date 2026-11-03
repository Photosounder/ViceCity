//+ rouz edit (ChatGPT)
#pragma once
#include "Streaming_constants.h"
#include "StreamingInfo.h"
#include "ModelIds.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
enum ChannelState {
    CHANNELSTATE_IDLE = 0,
    CHANNELSTATE_READING = 1,
    CHANNELSTATE_STARTED = 2,
    CHANNELSTATE_ERROR = 3
};
typedef struct CStreamingChannel {
    int32_t streamIds[4];
    int32_t offsets[4];
    int32_t state, field24, position, size, numTries, status;
} CStreamingChannel;
extern CStreamingInfo Streaming_modelInfo[NUMSTREAMINFO];
extern CStreamingChannel Streaming_channel[2];
extern size_t Streaming_memoryUsed;
extern int32_t Streaming_imageOffsets[NUMCDIMAGES];
extern int32_t Streaming_lastImageRead;
extern int32_t Streaming_imageSize;
int32_t Streaming_GetCdImageOffset(int32_t lastPosn);
extern CStreamingInfo Streaming_startLoadedList;
extern CStreamingInfo Streaming_endLoadedList;
extern CStreamingInfo Streaming_startRequestedList;
extern CStreamingInfo Streaming_endRequestedList;
extern int32_t Streaming_numPedsLoaded;
extern bool Streaming_pedGroupLoaded[NUMMODELSPERPEDGROUP];
extern int32_t Streaming_currentPedGroup;
extern uint16_t Streaming_loadedGangs, Streaming_loadedGangCars;
bool Streaming_RemoveLoadedZoneModel(void);
void Streaming_RemoveCurrentZonesModels(void);
extern int32_t Streaming_numVehiclesLoaded;
extern int32_t Streaming_vehiclesLoaded[MAXVEHICLESLOADED];
extern int32_t Streaming_lastVehicleDeleted;
extern int32_t desiredNumVehiclesLoaded;
bool Streaming_RemoveLoadedVehicle(void);
int32_t Streaming_GetAvailableVehicleSlot(void);
bool Streaming_AddToLoadedVehiclesList(int32_t modelId);
extern int32_t Streaming_numModelsRequested;
extern int32_t Streaming_numPriorityRequests;
void Streaming_RequestModel(int32_t model, int32_t flags);
void Streaming_FlushChannels(void);
void Streaming_FlushRequestList(void);
void Streaming_RemoveModel(int32_t id);
void Streaming_RemoveAllUnusedModels(void);
bool Streaming_RemoveLeastUsedModel(uint32_t excludeMask);
void Streaming_SetModelIsDeletable(int32_t id);
void Streaming_SetModelTxdIsDeletable(int32_t id);
void Streaming_SetMissionDoesntRequireModel(int32_t id);
static inline void Streaming_SetMissionDoesntRequireSpecialChar(int32_t id)
{
    // Apply the original special-character model offset
    Streaming_SetMissionDoesntRequireModel(id + MI_SPECIAL01);
}
static inline void Streaming_RemoveTxd(int32_t id)
{
    // Remove the original texture resource offset
    Streaming_RemoveModel(id + STREAM_OFFSET_TXD);
}
static inline void Streaming_RemoveCol(int32_t id)
{
    // Remove the original collision resource offset
    Streaming_RemoveModel(id + STREAM_OFFSET_COL);
}
static inline void Streaming_RemoveAnim(int32_t id)
{
    // Remove the original animation resource offset
    Streaming_RemoveModel(id + STREAM_OFFSET_ANIM);
}
void Streaming_DecrementRef(int32_t id);
bool Streaming_IsTxdUsedByRequestedModels(int32_t txdId);
bool Streaming_AreAnimsUsedByRequestedModels(int32_t animId);
bool Streaming_IsObjectInCdImage(int32_t id);
int32_t Streaming_GetNextFileOnCd(int32_t lastPosn, bool priority);
static inline bool Streaming_ModelNotLoaded(int32_t modelId)
{
    // Preserve both original states that count as available dependencies
    CStreamingInfo *info = &Streaming_modelInfo[modelId];
    return info->m_loadState != STREAMSTATE_LOADED && info->m_loadState != STREAMSTATE_READING;
}
static inline bool Streaming_TxdNotLoaded(int32_t txdId)
{
    // Check the original texture resource offset
    return Streaming_ModelNotLoaded(txdId + STREAM_OFFSET_TXD);
}
static inline bool Streaming_AnimNotLoaded(int32_t animId)
{
    // Keep the original sentinel check before the animation lookup
    return animId != -1 && Streaming_ModelNotLoaded(animId + STREAM_OFFSET_ANIM);
}
static inline bool Streaming_HasSpecialCharLoaded(int32_t id)
{
    // Preserve the original special-character model offset
    return Streaming_modelInfo[id + MI_SPECIAL01].m_loadState == STREAMSTATE_LOADED;
}
static inline void Streaming_ReRequestModel(int32_t model)
{
    // Reuse the current flags at the original request point
    Streaming_RequestModel(model, Streaming_modelInfo[model].m_flags);
}

static inline void Streaming_RequestTxd(int32_t id, int32_t flags)
{
    // Preserve the original resource-offset request
    Streaming_RequestModel(id + STREAM_OFFSET_TXD, flags);
}
static inline void Streaming_ReRequestTxd(int32_t id)
{
    // Preserve the original resource-offset request and live flags
    Streaming_ReRequestModel(id + STREAM_OFFSET_TXD);
}
static inline void Streaming_RequestCol(int32_t id, int32_t flags)
{
    // Preserve the original resource-offset request
    Streaming_RequestModel(id + STREAM_OFFSET_COL, flags);
}
static inline void Streaming_ReRequestCol(int32_t id)
{
    // Preserve the original resource-offset request and live flags
    Streaming_ReRequestModel(id + STREAM_OFFSET_COL);
}
static inline void Streaming_RequestAnim(int32_t id, int32_t flags)
{
    // Preserve the original resource-offset request
    Streaming_RequestModel(id + STREAM_OFFSET_ANIM, flags);
}
static inline void Streaming_ReRequestAnim(int32_t id)
{
    // Preserve the original resource-offset request and live flags
    Streaming_ReRequestModel(id + STREAM_OFFSET_ANIM);
}
static inline bool Streaming_HasModelLoaded(int32_t id)
{
    // Read the original live model load state
    return Streaming_modelInfo[id].m_loadState == STREAMSTATE_LOADED;
}
static inline bool Streaming_HasTxdLoaded(int32_t id)
{
    // Query the original resource-offset load state
    return Streaming_HasModelLoaded(id + STREAM_OFFSET_TXD);
}
static inline bool Streaming_HasColLoaded(int32_t id)
{
    // Query the original resource-offset load state
    return Streaming_HasModelLoaded(id + STREAM_OFFSET_COL);
}
static inline bool Streaming_HasAnimLoaded(int32_t id)
{
    // Query the original resource-offset load state
    return Streaming_HasModelLoaded(id + STREAM_OFFSET_ANIM);
}
static inline bool Streaming_CanRemoveModel(int32_t id)
{
    // Preserve the original live removal-flag test
    return (Streaming_modelInfo[id].m_flags & STREAMFLAGS_CANT_REMOVE) == 0;
}
static inline bool Streaming_CanRemoveTxd(int32_t id)
{
    // Apply the original resource offset before testing removal flags
    return Streaming_CanRemoveModel(id + STREAM_OFFSET_TXD);
}
static inline bool Streaming_CanRemoveCol(int32_t id)
{
    // Apply the original resource offset before testing removal flags
    return Streaming_CanRemoveModel(id + STREAM_OFFSET_COL);
}
static inline bool Streaming_CanRemoveAnim(int32_t id)
{
    // Apply the original resource offset before testing removal flags
    return Streaming_CanRemoveModel(id + STREAM_OFFSET_ANIM);
}
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
