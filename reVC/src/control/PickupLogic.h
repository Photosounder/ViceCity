//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "PickupData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Pickup table helpers, the table has count entries
bool Pickups_TestForPickupsInBubble(const CPickupData *pickups, int count, float x, float y, float z, float range);
bool Pickups_TryToMerge(CPickupData *pickups, int count, float x, float y, float z, uint8_t type, int16_t modelIndex, uint32_t quantity);
bool Pickups_IsPickUpPickedUp(int32_t *collected, int count, int32_t pickupId);
void Pickups_PassTime(CPickupData *pickups, int count, uint32_t time);
int32_t Pickups_GetActualPickupIndex(const CPickupData *pickups, int32_t index);
// Pickups to get rid of around a point: money and timed pickups within the radius, in table order
int Pickups_SelectUnnecessary(const CPickupData *pickups, int count, float x, float y, float z, float radius, int *out, int maxOut);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
