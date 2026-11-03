//+ rouz edit (ChatGPT)
#include "StreamingData.h"
CStreamingInfo Streaming_modelInfo[NUMSTREAMINFO];
CStreamingChannel Streaming_channel[2];
size_t Streaming_memoryUsed;
int32_t Streaming_imageOffsets[NUMCDIMAGES];
int32_t Streaming_lastImageRead;
int32_t Streaming_imageSize;
CStreamingInfo Streaming_startLoadedList;
CStreamingInfo Streaming_endLoadedList;
CStreamingInfo Streaming_startRequestedList;
CStreamingInfo Streaming_endRequestedList;
int32_t Streaming_numPedsLoaded;
bool Streaming_pedGroupLoaded[NUMMODELSPERPEDGROUP];
int32_t Streaming_currentPedGroup;
uint16_t Streaming_loadedGangs, Streaming_loadedGangCars;
int32_t Streaming_numVehiclesLoaded;
int32_t Streaming_vehiclesLoaded[MAXVEHICLESLOADED];
int32_t Streaming_lastVehicleDeleted;
int32_t Streaming_numModelsRequested;
int32_t Streaming_numPriorityRequests;
void Streaming_DecrementRef(int32_t id)
{
    // Preserve the original request and priority counter updates
    Streaming_numModelsRequested--;
    if (StreamingInfo_IsPriority(&Streaming_modelInfo[id])) {
        // Clear priority before reducing the corresponding counter
        Streaming_modelInfo[id].m_flags &= ~STREAMFLAGS_PRIORITY;
        Streaming_numPriorityRequests--;
    }
}
bool Streaming_IsObjectInCdImage(int32_t id)
{
    // Query the original model CD position and size record
    uint32_t position, size;
    return StreamingInfo_GetCdPosnAndSize(&Streaming_modelInfo[id], &position, &size);
}
//- rouz edit (ChatGPT)
