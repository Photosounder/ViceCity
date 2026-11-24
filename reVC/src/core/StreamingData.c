//+ rouz edit (ChatGPT)
#include <stdlib.h>
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
//+ rouz edit (Claude)
void Streaming_SectorRange(int cameraIndex, bool lookingPositive, bool behind, int numSectors, int *minIndex, int *maxIndex, int *inc)
{
	int i = cameraIndex;
	int last = numSectors - 1;
	int a, b;

	if (lookingPositive) {
		if (behind) {
			*maxIndex = (i - 2 > 0) ? i - 2 : 0;
			*minIndex = (i - 10 > 0) ? i - 10 : 0;
		} else {
			*maxIndex = (i + 10 > 0) ? i + 10 : 0;
			*minIndex = (i - 2 > 0) ? i - 2 : 0;
		}
		*inc = 1;
	} else {
		if (behind) {
			a = i + 2; b = i + 10;
			*maxIndex = (a < last) ? a : last;
			*minIndex = (b < last) ? b : last;
		} else {
			a = i - 10; b = i + 2;
			*maxIndex = (a < last) ? a : last;
			*minIndex = (b < last) ? b : last;
		}
		*inc = -1;
	}
}

void Streaming_SectorSpan(int cameraIndex, int numSectors, int *minIndex, int *maxIndex)
{
	*minIndex = (cameraIndex - 10 > 0) ? cameraIndex - 10 : 0;
	*maxIndex = (cameraIndex + 10 < numSectors - 1) ? cameraIndex + 10 : numSectors - 1;
}
bool Streaming_ShouldRequestEntity(float posX, float posY, float xmin, float ymin, float xmax, float ymax, float x, float y, float lodDistSq, float streamDistSq)
{
	float dx, dy;

	lodDistSq = lodDistSq < streamDistSq ? lodDistSq : streamDistSq;
	dx = x - posX;
	dy = y - posY;
	return xmin < posX && posX < xmax &&
	       ymin < posY && posY < ymax &&
	       dx * dx + dy * dy < lodDistSq;
}

bool Streaming_SectorIsFar(int playerX, int playerY, int sectorX, int sectorY)
{
	return abs(playerX - sectorX) > 3 && abs(playerY - sectorY) > 3;
}
//- rouz edit (Claude)
