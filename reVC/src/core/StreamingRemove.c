//+ rouz edit (ChatGPT)
#include "config.h"
#include "StreamingData.h"
#include "StreamingRemove_backend.h"
#include "CdStream_constants.h"
#include "TxdStore.h"
#include "ClumpRead.h"
#include "AnimManager.h"
#include "Gangs.h"
#include "PopulationGroups.h"
#ifndef MASTER
void re3_assert(const char *expression, const char *filename, unsigned int line, const char *function);
#define StreamingRemove_Assert(expr) ((void)((!!(expr)) || (re3_assert(#expr, __FILE__, __LINE__, __func__), 0)))
#else
#define StreamingRemove_Assert(expr) (expr)
#endif

void Streaming_RemoveModel(int32_t id)
{
    // Preserve the original state-specific cleanup and memory accounting
    int i;
    if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_NOTLOADED)
        return;
    if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_LOADED) {
        // Release the loaded resource through its original owner
        if (id < STREAM_OFFSET_TXD)
            StreamingRemove_DeleteModel(id);
        else if (id >= STREAM_OFFSET_TXD && id < STREAM_OFFSET_COL)
            TxdStore_RemoveTxd(id - STREAM_OFFSET_TXD);
        else if (id >= STREAM_OFFSET_COL && id < STREAM_OFFSET_ANIM)
            StreamingRemove_RemoveCol(id - STREAM_OFFSET_COL);
        else if (id >= STREAM_OFFSET_ANIM) {
            // Check the same animation range before releasing it
            StreamingRemove_Assert(id < NUMSTREAMINFO);
            AnimManager_RemoveAnimBlock(id - STREAM_OFFSET_ANIM);
        }
        Streaming_memoryUsed -= StreamingInfo_GetCdSize(&Streaming_modelInfo[id]) * CDSTREAM_SECTOR_SIZE;
    }
    if (Streaming_modelInfo[id].m_next) {
        // Remove linked entries and update queued counters first
        if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_INQUEUE)
            Streaming_DecrementRef(id);
        StreamingInfo_RemoveFromList(&Streaming_modelInfo[id]);
    } else if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_READING) {
        // Clear matching entries from both active channels
        for (i = 0; i < 4; i++) {
            // Preserve both original channel comparisons and writes
            if (Streaming_channel[0].streamIds[i] == id)
                Streaming_channel[0].streamIds[i] = -1;
            if (Streaming_channel[1].streamIds[i] == id)
                Streaming_channel[1].streamIds[i] = -1;
        }
    }
    if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_STARTED) {
        // Cancel the partially loaded resource using its original owner
        if (id < STREAM_OFFSET_TXD)
            ClumpRead_Cancel();
        else if (id >= STREAM_OFFSET_TXD && id < STREAM_OFFSET_COL)
            TxdStore_RemoveTxd(id - STREAM_OFFSET_TXD);
        else if (id >= STREAM_OFFSET_COL && id < STREAM_OFFSET_ANIM)
            StreamingRemove_RemoveCol(id - STREAM_OFFSET_COL);
        else if (id >= STREAM_OFFSET_ANIM) {
            // Check the same animation range before releasing it
            StreamingRemove_Assert(id < NUMSTREAMINFO);
            AnimManager_RemoveAnimBlock(id - STREAM_OFFSET_ANIM);
        }
    }
    Streaming_modelInfo[id].m_loadState = STREAMSTATE_NOTLOADED;
}
bool Streaming_RemoveLoadedZoneModel(void)
{
    // Scan the active group in its original order
    int i;
    if (Streaming_currentPedGroup == -1)
        return false;
    for (i = 0; i < NUMMODELSPERPEDGROUP; i++) {
        // Preserve the ordered model, state, flag, and reference checks
        int mi = Population_pedGroups[Streaming_currentPedGroup].models[i];
        if (mi != -1 && Streaming_pedGroupLoaded[i] &&
            Streaming_HasModelLoaded(mi) && Streaming_CanRemoveModel(mi) &&
            StreamingRemove_ModelRefs(mi) == 0) {
            // Re-read the group entry before removal and update group state
            Streaming_RemoveModel(Population_pedGroups[Streaming_currentPedGroup].models[i]);
            Streaming_numPedsLoaded--;
            Streaming_pedGroupLoaded[i] = false;
            return true;
        }
    }
    return false;
}

void Streaming_RemoveCurrentZonesModels(void)
{
    // Honor the original unstreaming preference before changing state
    int i;
    if (StreamingRemove_NoUnstreaming())
        return;
    if (Streaming_currentPedGroup != -1) {
        // Mark each current group entry and its texture as deletable
        for (i = 0; i < NUMMODELSPERPEDGROUP; i++) {
            // Preserve each original live group lookup
            Streaming_pedGroupLoaded[i] = false;
            if (Population_pedGroups[Streaming_currentPedGroup].models[i] != -1) {
                // Release the model and texture ownership separately
                Streaming_SetModelIsDeletable(Population_pedGroups[Streaming_currentPedGroup].models[i]);
                Streaming_SetModelTxdIsDeletable(Population_pedGroups[Streaming_currentPedGroup].models[i]);
            }
        }
    }
    // Retain the default pedestrian requests before releasing gang resources
    Streaming_RequestModel(MI_MALE01, STREAMFLAGS_DONT_REMOVE);
    Streaming_RequestModel(MI_TAXI_D, STREAMFLAGS_DONT_REMOVE);
    for (i = 0; i < NUM_GANGS; i++) {
        // Preserve the separate live lookups for each gang resource
        if (Gangs_GetGangPedModel1(i) != -1) {
            // Release the first gang pedestrian and its texture
            Streaming_SetModelIsDeletable(Gangs_GetGangPedModel1(i));
            Streaming_SetModelTxdIsDeletable(Gangs_GetGangPedModel1(i));
        }
        if (Gangs_GetGangPedModel2(i) != -1) {
            // Release the second gang pedestrian and its texture
            Streaming_SetModelIsDeletable(Gangs_GetGangPedModel2(i));
            Streaming_SetModelTxdIsDeletable(Gangs_GetGangPedModel2(i));
        }
        if (Gangs_GetGangVehicleModel(i) != -1) {
            // Release the gang vehicle and its texture
            Streaming_SetModelIsDeletable(Gangs_GetGangVehicleModel(i));
            Streaming_SetModelTxdIsDeletable(Gangs_GetGangVehicleModel(i));
        }
    }
    // Clear the active group and gang masks after all resource operations
    Streaming_currentPedGroup = -1;
    Streaming_loadedGangs = 0;
    Streaming_loadedGangCars = 0;
}

bool Streaming_RemoveLoadedVehicle(void)
{
    // Search the rotating slots in the original order
    int i, id;
    for (i = 0; i < MAXVEHICLESLOADED; i++) {
        // Advance before examining each candidate
        Streaming_lastVehicleDeleted++;
        if (Streaming_lastVehicleDeleted == MAXVEHICLESLOADED)
            Streaming_lastVehicleDeleted = 0;
        id = Streaming_vehiclesLoaded[Streaming_lastVehicleDeleted];
        if (id != -1 && Streaming_CanRemoveModel(id) && StreamingRemove_ModelRefs(id) == 0 &&
            Streaming_modelInfo[id].m_loadState == STREAMSTATE_LOADED) {
            // Remove the selected model and update the slot before vehicle bookkeeping
            Streaming_RemoveModel(Streaming_vehiclesLoaded[Streaming_lastVehicleDeleted]);
            Streaming_numVehiclesLoaded--;
            Streaming_vehiclesLoaded[Streaming_lastVehicleDeleted] = -1;
            StreamingRemove_RemoveVehicleClass(id);
            return true;
        }
    }
    return false;
}

int32_t Streaming_GetAvailableVehicleSlot(void)
{
    // Return the first unoccupied vehicle slot
    int i;
    for (i = 0; i < MAXVEHICLESLOADED; i++)
        if (Streaming_vehiclesLoaded[i] == -1)
            return i;
    return -1;
}

bool Streaming_AddToLoadedVehiclesList(int32_t modelId)
{
    // Choose a free slot or the next removable vehicle using the original rotation
    int i, id;
    if (Streaming_numVehiclesLoaded < desiredNumVehiclesLoaded) {
        // Find an empty slot while retaining the original assertion
        for (i = 0; i < MAXVEHICLESLOADED; i++) {
            // Stop at the first empty rotating slot
            if (Streaming_vehiclesLoaded[Streaming_lastVehicleDeleted] == -1)
                break;
            Streaming_lastVehicleDeleted++;
            if (Streaming_lastVehicleDeleted == MAXVEHICLESLOADED)
                Streaming_lastVehicleDeleted = 0;
        }
        StreamingRemove_Assert(Streaming_vehiclesLoaded[Streaming_lastVehicleDeleted] == -1);
        Streaming_numVehiclesLoaded++;
    } else {
        // Locate a removable vehicle without requiring a loaded-state check
        for (i = 0; i < MAXVEHICLESLOADED; i++) {
            // Check the current slot before advancing
            id = Streaming_vehiclesLoaded[Streaming_lastVehicleDeleted];
            if (id != -1 && Streaming_CanRemoveModel(id) && StreamingRemove_ModelRefs(id) == 0)
                goto found;
            Streaming_lastVehicleDeleted++;
            if (Streaming_lastVehicleDeleted == MAXVEHICLESLOADED)
                Streaming_lastVehicleDeleted = 0;
        }
        id = -1;
found:
        if (id == -1) {
            // Use a free slot above the target count if one remains
            id = Streaming_GetAvailableVehicleSlot();
            if (id == -1)
                return false;
            Streaming_lastVehicleDeleted = id;
            Streaming_numVehiclesLoaded++;
        } else {
            // Replace the existing model and its vehicle class entry
            Streaming_RemoveModel(id);
            StreamingRemove_RemoveVehicleClass(id);
        }
    }
    // Install the new model and advance the rotating cursor
    Streaming_vehiclesLoaded[Streaming_lastVehicleDeleted++] = modelId;
    if (Streaming_lastVehicleDeleted == MAXVEHICLESLOADED)
        Streaming_lastVehicleDeleted = 0;
    StreamingRemove_AddVehicleClass(modelId);
    return true;
}

bool Streaming_RemoveLeastUsedModel(uint32_t excludeMask)
{
    // Scan from the least recently used end while preserving exclusion checks
    CStreamingInfo *si;
    int streamId;
    for (si = Streaming_endLoadedList.m_prev; si != &Streaming_startLoadedList; si = si->m_prev) {
        // Skip protected entries before interpreting the resource index
        if (si->m_flags & excludeMask)
            continue;
        streamId = (int)(si - Streaming_modelInfo);
        if (streamId < STREAM_OFFSET_TXD) {
            // Remove a model only when its native reference count is zero
            if (StreamingRemove_ModelRefs(streamId) == 0) {
                // Evict the first eligible resource
                Streaming_RemoveModel(streamId);
                return true;
            }
        } else if (streamId >= STREAM_OFFSET_TXD && streamId < STREAM_OFFSET_COL) {
            // Keep requested texture dependencies resident
            if (TxdStore_GetNumRefs(streamId - STREAM_OFFSET_TXD) == 0 &&
                !Streaming_IsTxdUsedByRequestedModels(streamId - STREAM_OFFSET_TXD)) {
                // Evict the first eligible resource
                Streaming_RemoveModel(streamId);
                return true;
            }
        } else if (streamId >= STREAM_OFFSET_ANIM) {
            // Check the original animation range before testing dependencies
            StreamingRemove_Assert(streamId < NUMSTREAMINFO);
            if (AnimManager_GetNumRefsToAnimBlock(streamId - STREAM_OFFSET_ANIM) == 0 &&
                !Streaming_AreAnimsUsedByRequestedModels(streamId - STREAM_OFFSET_ANIM)) {
                // Evict the first eligible resource
                Streaming_RemoveModel(streamId);
                return true;
            }
        }
    }
    // Attempt vehicle eviction only after the loaded list has no eligible resource
    return StreamingRemove_TryVehicleFallback();
}

void Streaming_RemoveAllUnusedModels(void)
{
    // Preserve the original unstreaming guard and ordered vehicle attempts
    int i;
    if (StreamingRemove_NoUnstreaming())
        return;
    for (i = 0; i < MAXVEHICLESLOADED; i++)
        Streaming_RemoveLoadedVehicle();

    // Remove unreferenced loaded models in their original index range
    for (i = NUM_DEFAULT_MODELS; i < MODELINFOSIZE; i++) {
        // Check load state before reading the native model reference count
        if (Streaming_modelInfo[i].m_loadState == STREAMSTATE_LOADED &&
            StreamingRemove_ModelRefs(i) == 0) {
            // Retain the original explicit state reset after removal
            Streaming_RemoveModel(i);
            Streaming_modelInfo[i].m_loadState = STREAMSTATE_NOTLOADED;
        }
    }
}
//- rouz edit (ChatGPT)
