//+ rouz edit (Claude)
#include <math.h>
#include "PickupLogic.h"

bool PickupLogic_TestForPickupsInBubble(const CPickupData *pickups, int count, float x, float y, float z, float range)
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

bool PickupLogic_IsPickUpPickedUp(int32_t *collected, int count, int32_t pickupId)
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

void PickupLogic_PassTime(CPickupData *pickups, int count, uint32_t time)
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

int32_t PickupLogic_GetActualPickupIndex(const CPickupData *pickups, int32_t index)
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
float Pickups_PulseSin(uint32_t timePlusEntity, uint32_t mask)
{
	const float pi = (float)3.14159265358979323846;

	return sinf((float)(timePlusEntity & mask) * ((360.0f / (float)(mask + 1)) * pi / 180.0f));
}

float Pickups_SpinAngle(uint32_t time, uint32_t mask)
{
	const float pi = (float)3.14159265358979323846;

	return (float)(time & mask) * ((360.0f / (float)(mask + 1)) * pi / 180.0f);
}

int32_t Pickups_GlowChannel(float maxDist, float dist, float pulse, float scale, float offset, float multiplier)
{
	return (int32_t)((maxDist - dist) * (scale * pulse + offset) / maxDist * multiplier);
}
int Pickups_CoronaPhase(int phase)
{
	if (phase < 3)
		return 0;
	if (phase == 3)
		return 1;
	return 2;
}

float Pickups_LightRadius(int random)
{
	return (random & 0xF) * 0.1f + 3.0f;
}

float Pickups_ModelScale(float maxDimension)
{
	float v = 1.2f / maxDimension;

	return ((1.0f > v ? 1.0f : v) - 1.0f) * 0.6f + 1.0f;
}

int Pickups_MessageAlpha(float dist, float maxDist)
{
	return (int)((1.0f - dist / maxDist) * 128.0f);
}
int Pickups_MoneyPickupCount(int money)
{
	int count = money / 20 + 1;

	return count < 7 ? count : 7;
}

float Pickups_MoneyRingOffset(int random, bool isY)
{
	const float pi = (float)3.14159265358979323846;
	float angle = (random % 256) * pi / 128;

	return 1.5f * (isY ? cosf(angle) : sinf(angle));
}

float Pickups_DeadPedRingOffset(int random, bool isY)
{
	const float twoPi = (float)3.14159265358979323846 * 2;
	float angle = (random % 256) / 256.0f * twoPi;

	return 1.5f * (isY ? cosf(angle) : sinf(angle));
}

float Pickups_MessageScale(float spriteSize, float maxScale)
{
	float scale = spriteSize / 30.0f;

	return scale > maxScale ? maxScale : scale;
}
void Pickups_SpinCosSin(float angle, float scale, float *c, float *s)
{
	*c = cosf(angle) * scale;
	*s = sinf(angle) * scale;
}
bool Pickups_OnFootTouches(float playerX, float playerY, float playerZ, float objectX, float objectY, float objectZ)
{
	if (fabsf(playerZ - objectZ) < 2.0f) {
		if ((playerX - objectX) * (playerX - objectX) +
			(playerY - objectY) * (playerY - objectY) < 1.8f)
			return true;
	}
	return false;
}
//- rouz edit (Claude)
