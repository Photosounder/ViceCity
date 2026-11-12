//+ rouz edit (Claude)
#include "StreamingVehicles.h"
#include "config.h"
#include "CarClasses.h"

int32_t Streaming_MostRequestedRating(const int32_t *numRequests, const int16_t *carThreshold, const int16_t *boatThreshold)
{
	int32_t i;
	int32_t maxReq = -1;
	int32_t mostRequestedRating = 0;

	// Pick the class with the most requests among those that have a vehicle in the zone
	for (i = 0; i < CARCLASS_TOTAL_CUSTOM_CLASSES; i++) {
		if (numRequests[i] > maxReq &&
			((i == 0 && carThreshold[0] != 0) ||
#ifdef FIX_BUGS
			(i < CARCLASS_NUM_CAR_CLASSES && carThreshold[i] != carThreshold[i-1]) ||
			(i == CARCLASS_NUM_CAR_CLASSES && boatThreshold[i - CARCLASS_NUM_CAR_CLASSES] != 0) ||
			(i > CARCLASS_NUM_CAR_CLASSES && i < CARCLASS_TOTAL_CUSTOM_CLASSES && boatThreshold[i - CARCLASS_NUM_CAR_CLASSES] != boatThreshold[i - CARCLASS_NUM_CAR_CLASSES - 1]))) {
#else
			(i != 0 && carThreshold[i] != carThreshold[i-1]))) {
#endif
			maxReq = numRequests[i];
			mostRequestedRating = i;
		}
	}

	return mostRequestedRating;
}
void Streaming_GangLoadMasks(const CZoneInfo *info, uint16_t *gangsToLoad, uint16_t *gangCarsToLoad)
{
	int i;

	*gangsToLoad = 0;
	*gangCarsToLoad = 0;

	// The first gang is compared to the cops, the others to the gang before them
	if (info->gangPedThreshold[0] != info->copPedThreshold)
		*gangsToLoad = 1;
	for (i = 1; i < NUM_GANGS; i++)
		if (info->gangPedThreshold[i] != info->gangPedThreshold[i-1])
			*gangsToLoad |= 1 << i;
	if (info->gangThreshold[0] != info->copThreshold)
		*gangCarsToLoad = 1;
	for (i = 1; i < NUM_GANGS; i++)
		if (info->gangThreshold[i] != info->gangThreshold[i-1])
			*gangCarsToLoad |= 1 << i;
}

//- rouz edit (Claude)
