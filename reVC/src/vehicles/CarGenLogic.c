//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "CarGenLogic.h"

void CarGen_SwitchOff(CCarGeneratorData *self, uint32_t *activeCount)
{
	// Only a generator that still had uses counts as active
	if (self->m_nUsesRemaining != 0) {
		self->m_nUsesRemaining = 0;
		--*activeCount;
	}
}

uint32_t CarGen_CalcNextGen(uint32_t now)
{
	// Almost immediately
	return now + 4;
}

void CarGen_SwitchOn(CCarGeneratorData *self, uint32_t *activeCount, uint32_t now)
{
	// Unlimited uses
	self->m_nUsesRemaining = UINT16_MAX;
	self->m_nTimer = CarGen_CalcNextGen(now);
	++*activeCount;
}

void CarGen_Setup(CCarGeneratorData *self, float x, float y, float z, float angle, int32_t mi, int16_t color1, int16_t color2, uint8_t force, uint8_t alarm, uint8_t lock, uint16_t minDelay, uint16_t maxDelay, uint32_t now)
{
	// Store the placement and reset the spawn state
	self->m_vecPos.x = x;
	self->m_vecPos.y = y;
	self->m_vecPos.z = z;
	self->m_fAngle = angle;
	self->m_nModelIndex = mi;
	self->m_nColor1 = color1;
	self->m_nColor2 = color2;
	self->m_bForceSpawn = force;
	self->m_nAlarm = alarm;
	self->m_nDoorlock = lock;
	self->m_nMinDelay = minDelay;
	self->m_nMaxDelay = maxDelay;
	self->m_nVehicleHandle = -1;
	self->m_nTimer = now + 1;
	self->m_nUsesRemaining = 0;
	self->m_bIsBlocking = false;
}

bool CarGen_IsDueToSpawn(const CCarGeneratorData *self, bool generateEvenIfPlayerIsClose, uint32_t now)
{
	// No car out yet, the timer ran out and uses remain
	return self->m_nVehicleHandle == -1 &&
		(generateEvenIfPlayerIsClose || now >= self->m_nTimer) &&
		self->m_nUsesRemaining != 0;
}

void CarGen_AfterSpawn(CCarGeneratorData *self, uint32_t *activeCount, uint32_t now)
{
	// Use up one spawn unless unlimited
	if (self->m_nUsesRemaining < UINT16_MAX)
		--self->m_nUsesRemaining;
	self->m_nTimer = CarGen_CalcNextGen(now);
	if (self->m_nUsesRemaining == 0)
		--*activeCount;
}

void CarGen_ReleaseAfterPlayerTookOver(CCarGeneratorData *self)
{
	// Give the car up and block this spot for a minute
	self->m_nTimer += 60000;
	self->m_nVehicleHandle = -1;
	self->m_bIsBlocking = true;
	if (self->m_nModelIndex < 0)
		self->m_nModelIndex = -1;
}

bool CarGen_CheckRange(CCarGeneratorData *self, float dirX, float dirY, float speedX, float speedY, float genDistMult, bool isBoat, CarGenVisibleFn visible, void *context, bool generateEvenIfPlayerIsClose)
{
	float distance = sqrtf(dirX * dirX + dirY * dirY);
	float farclip = 110.0f * genDistMult;
	float nearclip = farclip - 20.0f;

	// Boats in view can be spawned further out
	bool canBeRemoved = (self->m_nModelIndex > 0 && isBoat && 165.0f * genDistMult > distance && visible(context));

	// Too far away
	if (distance >= farclip && !canBeRemoved) {
		if (self->m_bIsBlocking)
			self->m_bIsBlocking = false;
		return false;
	}
	if (generateEvenIfPlayerIsClose)
		return true;
	if (self->m_bIsBlocking)
		return false;

	// Too close unless forced, then only ahead of the player
	if (distance < nearclip && !self->m_bForceSpawn)
		return false;
	return dirX * speedX + dirY * speedY <= 0;
}
//- rouz edit (Claude)
