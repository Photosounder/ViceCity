//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "OnscreenTimer.h"
#include "OnscreenTimer_backend.h"
#include "Timer.h"
#include "DMAudio.h"
#include <stdio.h>
#include <string.h>
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
_Static_assert(sizeof(COnscreenTimerEntry)==56,"Actual original clock layout");
_Static_assert(sizeof(COnscreenCounterEntry)==60,"Actual original counter layout");
_Static_assert(sizeof(COnscreenTimer)==240,"Actual original display controller layout");
COnscreenTimer OnscreenTimer_state;

void
OnscreenTimer_Init(COnscreenTimer *self)
{
    // Preserve the complete original Init operation with explicit record storage
	self->m_bDisabled = false;
	for(uint32_t i = 0; i < NUMONSCREENCOUNTERS; i++) {
		self->m_sCounters[i].m_nCounterOffset = 0;

		for(uint32_t j = 0; j < ARRAY_SIZE(self->m_sCounters[0].m_aCounterText); j++)
			self->m_sCounters[i].m_aCounterText[j] = '\0';

		self->m_sCounters[i].m_nType = COUNTER_DISPLAY_NUMBER;
		self->m_sCounters[i].m_bCounterProcessed = false;
	}
	for(uint32_t i = 0; i < NUMONSCREENCLOCKS; i++) {
		self->m_sClocks[i].m_nClockOffset = 0;

		for(uint32_t j = 0; j < ARRAY_SIZE(self->m_sClocks[0].m_aClockText); j++)
			self->m_sClocks[i].m_aClockText[j] = '\0';

		self->m_sClocks[i].m_bClockProcessed = false;
		self->m_sClocks[i].m_bClockGoingDown = true;
	}
}

void
OnscreenTimer_Process(COnscreenTimer *self)
{
    // Preserve the complete original Process operation with explicit record storage
	if(!OnscreenTimer_BackendPlayingBack() && !self->m_bDisabled)
		for(uint32_t i = 0; i < NUMONSCREENCLOCKS; i++)
			OnscreenClock_Process(&self->m_sClocks[i]);
}

void
OnscreenTimer_ProcessForDisplay(COnscreenTimer *self)
{
    // Preserve the complete original ProcessForDisplay operation with explicit record storage
	if(OnscreenTimer_BackendDrawHud()) {
		self->m_bProcessed = false;
		for(uint32_t i = 0; i < NUMONSCREENCLOCKS; i++) {
			self->m_sClocks[i].m_bClockProcessed = false;
			if (self->m_sClocks[i].m_nClockOffset != 0) {
				OnscreenClock_ProcessForDisplayClock(&self->m_sClocks[i]);
				self->m_sClocks[i].m_bClockProcessed = true;
				self->m_bProcessed = true;
			}
		}
		for(uint32_t i = 0; i < NUMONSCREENCOUNTERS; i++) {
			self->m_sCounters[i].m_bCounterProcessed = false;
			if (self->m_sCounters[i].m_nCounterOffset != 0) {
				OnscreenCounter_ProcessForDisplayCounter(&self->m_sCounters[i]);
				self->m_sCounters[i].m_bCounterProcessed = true;
				self->m_bProcessed = true;
			}
		}
	}
}

void
OnscreenTimer_ClearCounter(COnscreenTimer *self,uint32_t offset)
{
    // Preserve the complete original ClearCounter operation with explicit record storage
	for(uint32_t i = 0; i < NUMONSCREENCOUNTERS; i++) {
		if(offset == self->m_sCounters[i].m_nCounterOffset) {
			self->m_sCounters[i].m_nCounterOffset = 0;
			self->m_sCounters[i].m_aCounterText[0] = '\0';
			self->m_sCounters[i].m_nType = COUNTER_DISPLAY_NUMBER;
			self->m_sCounters[i].m_bCounterProcessed = false;
		}
	}
}

void
OnscreenTimer_ClearClock(COnscreenTimer *self,uint32_t offset)
{
    // Preserve the complete original ClearClock operation with explicit record storage
	for(uint32_t i = 0; i < NUMONSCREENCLOCKS; i++)
		if(offset == self->m_sClocks[i].m_nClockOffset) {
			self->m_sClocks[i].m_nClockOffset = 0;
			self->m_sClocks[i].m_aClockText[0] = '\0';
			self->m_sClocks[i].m_bClockProcessed = false;
			self->m_sClocks[i].m_bClockGoingDown = true;
		}
}

void
OnscreenTimer_AddCounter(COnscreenTimer *self,uint32_t offset, uint16_t type, char* text, uint16_t pos)
{
    // Preserve the complete original AddCounter operation with explicit record storage
	if (self->m_sCounters[pos].m_aCounterText[0] != '\0')
		return;

	self->m_sCounters[pos].m_nCounterOffset = offset;
	if(text)
		strncpy(self->m_sCounters[pos].m_aCounterText, text, ARRAY_SIZE(self->m_sCounters[0].m_aCounterText));
	else
		self->m_sCounters[pos].m_aCounterText[0] = '\0';

	self->m_sCounters[pos].m_nType = type;
}

void
OnscreenTimer_AddClock(COnscreenTimer *self,uint32_t offset, char* text, bool bGoingDown)
{
    // Preserve the complete original AddClock operation with explicit record storage
	for(uint32_t i = 0; i < NUMONSCREENCLOCKS; i++) {
		if(self->m_sClocks[i].m_nClockOffset == 0) {
			self->m_sClocks[i].m_nClockOffset = offset;
			self->m_sClocks[i].m_bClockGoingDown = bGoingDown;
			if(text)
				strncpy(self->m_sClocks[i].m_aClockText, text, ARRAY_SIZE(self->m_sClocks[0].m_aClockText));
			else
				self->m_sClocks[i].m_aClockText[0] = '\0';
			break;
		}
	}
}

void
OnscreenClock_Process(COnscreenTimerEntry *self)
{
    // Preserve the complete original Process operation with explicit record storage
	if(self->m_nClockOffset == 0)
		return;

	int32_t* timerPtr = OnscreenTimer_BackendVariable(self->m_nClockOffset);
	int32_t oldTime = *timerPtr;
	if (self->m_bClockGoingDown) {
		int32_t newTime = oldTime - (int32_t)Timer_GetTimeStepInMilliseconds();
		*timerPtr = newTime;
		if (newTime < 0) {
			*timerPtr = 0;
			self->m_bClockProcessed = 0;
			self->m_nClockOffset = 0;
			self->m_aClockText[0] = 0;
		}
		else {
			int32_t oldTimeSeconds = oldTime / 1000;
			if (oldTimeSeconds < 12 && newTime / 1000 != oldTimeSeconds && !OnscreenTimer_BackendWidescreen()) {
				DMAudio_PlayFrontEndSound(SOUND_CLOCK_TICK, newTime / 1000);
			}
		}
	}
	else
		*timerPtr = oldTime + (int32_t)Timer_GetTimeStepInMilliseconds();
}

void
OnscreenClock_ProcessForDisplayClock(COnscreenTimerEntry *self)
{
    // Preserve the complete original ProcessForDisplayClock operation with explicit record storage
	uint32_t time = *OnscreenTimer_BackendVariable(self->m_nClockOffset);
	sprintf(self->m_aClockBuffer, "%02d:%02d", time / 1000 / 60 % 100,
				   time / 1000 % 60);
}

void
OnscreenCounter_ProcessForDisplayCounter(COnscreenCounterEntry *self)
{
    // Preserve the complete original ProcessForDisplayCounter operation with explicit record storage
	uint32_t counter = *OnscreenTimer_BackendVariable(self->m_nCounterOffset);
	sprintf(self->m_aCounterBuffer, "%d", counter);
}
//- rouz edit (ChatGPT)
