//+ rouz edit (ChatGPT)
#include "StreamingData.h"
#include "StreamingRequest_backend.h"
#include "ModelRegistry.h"
#include "ModelInfoType.h"

void Streaming_SetModelIsDeletable(int32_t id)
{
    // Clear the original ownership flag before checking resource eligibility
    Streaming_modelInfo[id].m_flags &= ~STREAMFLAGS_DONT_REMOVE;
    if (((id >= STREAM_OFFSET_TXD && id < STREAM_OFFSET_COL) || ModelRegistry_Type(ModelRegistry_Get(id)) != MITYPE_VEHICLE) &&
        (Streaming_modelInfo[id].m_flags & STREAMFLAGS_SCRIPTOWNED) == 0) {
        // Remove unloaded entries or relink loaded entries at the original points
        if (Streaming_modelInfo[id].m_loadState != STREAMSTATE_LOADED)
            Streaming_RemoveModel(id);
        else if (Streaming_modelInfo[id].m_next == NULL)
            StreamingInfo_AddToList(&Streaming_modelInfo[id], &Streaming_startLoadedList);
    }
}
void Streaming_SetModelTxdIsDeletable(int32_t id)
{
    // Resolve the captured model texture slot before clearing its ownership flag
    Streaming_SetModelIsDeletable(StreamingOwnership_TxdSlot(ModelRegistry_Get(id)) + STREAM_OFFSET_TXD);
}
void Streaming_SetMissionDoesntRequireModel(int32_t id)
{
    // Clear the original mission flag before checking resource eligibility
    Streaming_modelInfo[id].m_flags &= ~STREAMFLAGS_SCRIPTOWNED;
    if ((id >= STREAM_OFFSET_TXD || ModelRegistry_Type(ModelRegistry_Get(id)) != MITYPE_VEHICLE) &&
        (Streaming_modelInfo[id].m_flags & STREAMFLAGS_DONT_REMOVE) == 0) {
        // Remove unloaded entries or relink loaded entries at the original points
        if (Streaming_modelInfo[id].m_loadState != STREAMSTATE_LOADED)
            Streaming_RemoveModel(id);
        else if (Streaming_modelInfo[id].m_next == NULL)
            StreamingInfo_AddToList(&Streaming_modelInfo[id], &Streaming_startLoadedList);
    }
}
//- rouz edit (ChatGPT)
