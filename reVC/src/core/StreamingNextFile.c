//+ rouz edit (ChatGPT)
#include "StreamingData.h"
#include "CutsceneState.h"
#include "StreamingRequest_backend.h"
#include "ModelRegistry.h"
#include <stdint.h>

int32_t
Streaming_GetNextFileOnCd(int32_t lastPosn, bool priority)
{
    // Preserve traversal, dependency requests and file selection order
	CStreamingInfo *si, *next;
	int streamId;
	uint32_t posn, size;
	int streamIdFirst, streamIdNext;
	uint32_t posnFirst, posnNext;

	streamIdFirst = -1;
	streamIdNext = -1;
	posnFirst = UINT32_MAX;
	posnNext = UINT32_MAX;

	for(si = Streaming_startRequestedList.m_next; si != &Streaming_endRequestedList; si = next){ // rouz edit (ChatGPT)
		next = si->m_next;
		streamId = si - Streaming_modelInfo; // rouz edit (ChatGPT)

		// only priority requests if there are any
		if(priority && Streaming_numPriorityRequests != 0 && !StreamingInfo_IsPriority(si)) // rouz edit (ChatGPT)
			continue;

		// request Txds or anims if necessary
		if(streamId < STREAM_OFFSET_TXD){
			int txdId = StreamingOwnership_TxdSlot(ModelRegistry_Get(streamId)); // rouz edit (ChatGPT)
			if(Streaming_TxdNotLoaded(txdId)){
				Streaming_ReRequestTxd(txdId); // rouz edit (ChatGPT)
				continue;
			}
			int animId = StreamingDependencies_AnimIndex(ModelRegistry_Get(streamId)); // rouz edit (ChatGPT)
			if(Streaming_AnimNotLoaded(animId)){
				Streaming_ReRequestAnim(animId); // rouz edit (ChatGPT)
				continue;
			}
		}else if(streamId >= STREAM_OFFSET_ANIM && Cutscene_IsProcessing())
			continue;

		if(StreamingInfo_GetCdPosnAndSize(&Streaming_modelInfo[streamId], &posn, &size)){ // rouz edit (ChatGPT)
			if(posn < posnFirst){
				// find first requested file in image
				streamIdFirst = streamId;
				posnFirst = posn;
			}
			if(posn < posnNext && posn >= (uint32_t)lastPosn){
				// find first requested file after last read position
				streamIdNext = streamId;
				posnNext = posn;
			}
		}else{
			// empty file
			Streaming_DecrementRef(streamId); // rouz edit (ChatGPT)
			StreamingInfo_RemoveFromList(si);
			si->m_loadState = STREAMSTATE_LOADED;
		}
	}

	// wrap around
	if(streamIdNext == -1)
		streamIdNext = streamIdFirst;

	if(streamIdNext == -1 && Streaming_numPriorityRequests != 0){ // rouz edit (ChatGPT)
		// try non-priority files
		Streaming_numPriorityRequests = 0; // rouz edit (ChatGPT)
		streamIdNext = Streaming_GetNextFileOnCd(lastPosn, false);
	}

	return streamIdNext;
}


//- rouz edit (ChatGPT)
