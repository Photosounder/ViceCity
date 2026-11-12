//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include "ZoneRecords.h"

#ifdef __cplusplus
extern "C" {
#endif
// Car class whose vehicles were asked for the most since the last load and that the zone can spawn, the requests are per car and boat class and the thresholds are those of the zone, 0 when there is none
// The thresholds are read as in the original code, including the entry before the first car threshold for class 0 (the zone data must be laid out as the zone record)
int32_t Streaming_MostRequestedRating(const int32_t *numRequests, const int16_t *carThreshold, const int16_t *boatThreshold);
// Bit masks of the gangs whose peds and whose cars are present in a zone, bit n is gang n, a gang is present when its threshold differs from the previous one
void Streaming_GangLoadMasks(const CZoneInfo *info, uint16_t *gangsToLoad, uint16_t *gangCarsToLoad);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
