//+ rouz edit (ChatGPT)
#include "StreamingData.h"
#include "StreamingFlush_backend.h"
#include "CdStreamSync.h"

void
Streaming_FlushChannels(void)
{
    // Preserve the original ordered channel and request-list operations
	if(Streaming_channel[1].state == CHANNELSTATE_STARTED) // rouz edit (ChatGPT)
		StreamingFlush_ProcessLoadingChannel(1);

	if(Streaming_channel[0].state == CHANNELSTATE_READING){ // rouz edit (ChatGPT)
		CdStreamSync(0);
		StreamingFlush_ProcessLoadingChannel(0);
	}
	if(Streaming_channel[0].state == CHANNELSTATE_STARTED) // rouz edit (ChatGPT)
		StreamingFlush_ProcessLoadingChannel(0);

	if(Streaming_channel[1].state == CHANNELSTATE_READING){ // rouz edit (ChatGPT)
		CdStreamSync(1);
		StreamingFlush_ProcessLoadingChannel(1);
	}
	if(Streaming_channel[1].state == CHANNELSTATE_STARTED) // rouz edit (ChatGPT)
		StreamingFlush_ProcessLoadingChannel(1);
}

void
Streaming_FlushRequestList(void)
{
    // Preserve the original ordered channel and request-list operations
	CStreamingInfo *si, *next;

	for(si = Streaming_startRequestedList.m_next; si != &Streaming_endRequestedList; si = next){ // rouz edit (ChatGPT)
		next = si->m_next;
		Streaming_RemoveModel(si - Streaming_modelInfo); // rouz edit (ChatGPT)
	}
#ifdef FLUSHABLE_STREAMING
	if(Streaming_channel[0].state == CHANNELSTATE_READING) { // rouz edit (ChatGPT)
		flushStream[0] = 1;
	}
	if(Streaming_channel[1].state == CHANNELSTATE_READING) { // rouz edit (ChatGPT)
		flushStream[1] = 1;
	}
#endif
	Streaming_FlushChannels();
}


//- rouz edit (ChatGPT)
