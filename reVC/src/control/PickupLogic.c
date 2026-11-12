//+ rouz edit (Claude)
#include <math.h>
#include "PickupLogic.h"

bool Pickups_TestForPickupsInBubble(const CPickupData *pickups, int count, float x, float y, float z, float range)
{
	// Any pickup closer than the range to the point
	int i;
	for (i = 0; i < count; i++) {
		float dx = pickups[i].m_vecPos.x - x;
		float dy = pickups[i].m_vecPos.y - y;
		float dz = pickups[i].m_vecPos.z - z;
		if (sqrtf(dx * dx + dy * dy + dz * dz) < range)
			return true;
	}
	return false;
}

bool Pickups_TryToMerge(CPickupData *pickups, int count, float x, float y, float z, uint8_t type, int16_t modelIndex, uint32_t quantity)
{
	// Add the quantity to a nearby pickup of the same type and model
	int i;
	for (i = 0; i < count; i++) {
		if (pickups[i].m_eType == type && pickups[i].m_eModelIndex == modelIndex) {
			float dx = pickups[i].m_vecPos.x - x;
			float dy = pickups[i].m_vecPos.y - y;
			float dz = pickups[i].m_vecPos.z - z;
			if (sqrtf(dx * dx + dy * dy + dz * dz) < 7.5f) {
				pickups[i].m_nQuantity += quantity;
				return true;
			}
		}
	}
	return false;
}

bool Pickups_IsPickUpPickedUp(int32_t *collected, int count, int32_t pickupId)
{
	// Consume the collected entry for the pickup
	int i;
	for (i = 0; i < count; i++) {
		if (pickupId == collected[i]) {
			collected[i] = 0;
			return true;
		}
	}
	return false;
}

void Pickups_PassTime(CPickupData *pickups, int count, uint32_t time)
{
	// Count the timers down, asset revenue pickups keep theirs
	int i;
	for (i = 0; i < count; i++) {
		if (pickups[i].m_eType != PICKUP_NONE && pickups[i].m_eType != PICKUP_ASSET_REVENUE) {
			if (pickups[i].m_nTimer <= time)
				pickups[i].m_nTimer = 0;
			else
				pickups[i].m_nTimer -= time;
		}
	}
}

int32_t Pickups_GetActualPickupIndex(const CPickupData *pickups, int32_t index)
{
	if (index == -1) return -1;

	// the upper half is a generation number that must match the slot
	if ((uint16_t)((index & 0xFFFF0000) >> 16) != pickups[(uint16_t)index].m_nIndex) return -1;
	return (uint16_t)index;
}

int Pickups_SelectUnnecessary(const CPickupData *pickups, int count, float x, float y, float z, float radius, int *out, int maxOut)
{
	int n = 0;
	int i;
	for (i = 0; i < count; i++) {
		if (pickups[i].m_eType == PICKUP_ONCE_TIMEOUT || pickups[i].m_eType == PICKUP_MONEY) {
			float dx = x - pickups[i].m_vecPos.x;
			float dy = y - pickups[i].m_vecPos.y;
			float dz = z - pickups[i].m_vecPos.z;
			if (sqrtf(dx * dx + dy * dy + dz * dz) < radius) {
				if (n < maxOut)
					out[n] = i;
				n++;
			}
		}
	}
	return n;
}
//- rouz edit (Claude)
