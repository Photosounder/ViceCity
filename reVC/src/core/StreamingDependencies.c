//+ rouz edit (ChatGPT)
#include "StreamingData.h"
#include "StreamingRequest_backend.h"
#include "ModelRegistry.h"

bool
Streaming_IsTxdUsedByRequestedModels(int32_t txdId)
{
    // Preserve list-first traversal followed by interleaved channel checks
	CStreamingInfo *si;
	int streamId;
	int i;

	for(si = Streaming_startRequestedList.m_next; si != &Streaming_endRequestedList; si = si->m_next){ // rouz edit (ChatGPT)
		streamId = si - Streaming_modelInfo; // rouz edit (ChatGPT)
		if(streamId < STREAM_OFFSET_TXD &&
		   StreamingOwnership_TxdSlot(ModelRegistry_Get(streamId)) == txdId) // rouz edit (ChatGPT)
			return true;
	}

	for(i = 0; i < 4; i++){
		streamId = Streaming_channel[0].streamIds[i]; // rouz edit (ChatGPT)
		if(streamId != -1 && streamId < STREAM_OFFSET_TXD &&
		   StreamingOwnership_TxdSlot(ModelRegistry_Get(streamId)) == txdId) // rouz edit (ChatGPT)
			return true;
		streamId = Streaming_channel[1].streamIds[i]; // rouz edit (ChatGPT)
		if(streamId != -1 && streamId < STREAM_OFFSET_TXD &&
		   StreamingOwnership_TxdSlot(ModelRegistry_Get(streamId)) == txdId) // rouz edit (ChatGPT)
			return true;
	}

	return false;
}

bool
Streaming_AreAnimsUsedByRequestedModels(int32_t animId)
{
    // Preserve list-first traversal followed by interleaved channel checks
	CStreamingInfo *si;
	int streamId;
	int i;

	for(si = Streaming_startRequestedList.m_next; si != &Streaming_endRequestedList; si = si->m_next){ // rouz edit (ChatGPT)
		streamId = si - Streaming_modelInfo; // rouz edit (ChatGPT)
		if(streamId < STREAM_OFFSET_TXD &&
		   StreamingDependencies_AnimIndex(ModelRegistry_Get(streamId)) == animId) // rouz edit (ChatGPT)
			return true;
	}

	for(i = 0; i < 4; i++){
		streamId = Streaming_channel[0].streamIds[i]; // rouz edit (ChatGPT)
		if(streamId != -1 && streamId < STREAM_OFFSET_TXD &&
		   StreamingDependencies_AnimIndex(ModelRegistry_Get(streamId)) == animId) // rouz edit (ChatGPT)
			return true;
		streamId = Streaming_channel[1].streamIds[i]; // rouz edit (ChatGPT)
		if(streamId != -1 && streamId < STREAM_OFFSET_TXD &&
		   StreamingDependencies_AnimIndex(ModelRegistry_Get(streamId)) == animId) // rouz edit (ChatGPT)
			return true;
	}

	return false;
}
//- rouz edit (ChatGPT)
