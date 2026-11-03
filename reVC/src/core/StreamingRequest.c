//+ rouz edit (ChatGPT)
#include "StreamingData.h"
#include "StreamingRequest_backend.h"
#include "ModelRegistry.h"

void Streaming_RequestModel(int32_t id, int32_t flags)
{
    // Preserve the original priority upgrade and flag update order
    void *model;
    if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_INQUEUE) {
        // Upgrade an already queued model only once
        if ((flags & STREAMFLAGS_PRIORITY) && !StreamingInfo_IsPriority(&Streaming_modelInfo[id])) {
            Streaming_numPriorityRequests++;
            Streaming_modelInfo[id].m_flags |= STREAMFLAGS_PRIORITY;
        }
    } else if (Streaming_modelInfo[id].m_loadState != STREAMSTATE_NOTLOADED) {
        // Ignore new priority for models already being loaded or loaded
        flags &= ~STREAMFLAGS_PRIORITY;
    }
    Streaming_modelInfo[id].m_flags |= flags;
    if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_LOADED) {
        // Apply the original loaded-model alpha update before relinking
        if ((Streaming_modelInfo[id].m_flags & STREAMFLAGS_NOFADE) && id < STREAM_OFFSET_TXD) {
            model = ModelRegistry_Get(id);
            StreamingRequest_SetOpaqueIfSimple(model);
        }
        if (Streaming_modelInfo[id].m_next) {
            // Reinsert removable models into the loaded list
            StreamingInfo_RemoveFromList(&Streaming_modelInfo[id]);
            if (Streaming_CanRemoveModel(id))
                StreamingInfo_AddToList(&Streaming_modelInfo[id], &Streaming_startLoadedList);
        }
    } else if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_NOTLOADED ||
               Streaming_modelInfo[id].m_loadState == STREAMSTATE_LOADED) {
        // Request dependencies before publishing the new queued entry
        if (Streaming_modelInfo[id].m_loadState == STREAMSTATE_NOTLOADED) {
            if (id < STREAM_OFFSET_TXD) {
                // Preserve native model capture and dependent request order
                model = ModelRegistry_Get(id);
                Streaming_RequestTxd(StreamingRequest_TxdSlot(model), flags);
                int anim = StreamingRequest_AnimIndex(model);
                if (anim != -1)
                    Streaming_RequestAnim(anim, STREAMFLAGS_DEPENDENCY);
            }
            StreamingInfo_AddToList(&Streaming_modelInfo[id], &Streaming_startRequestedList);
            Streaming_numModelsRequested++;
            if (flags & STREAMFLAGS_PRIORITY)
                Streaming_numPriorityRequests++;
        }
        Streaming_modelInfo[id].m_loadState = STREAMSTATE_INQUEUE;
        Streaming_modelInfo[id].m_flags = flags;
    }
}
//- rouz edit (ChatGPT)
