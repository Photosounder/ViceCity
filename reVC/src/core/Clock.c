//+ rouz edit (ChatGPT)
#include "Clock.h"
#include "Timer.h"
#include "Stats.h"
#include "VarConsole.h"

void re3_debug(const char *format, ...);

uint8_t Clock_hours;
uint8_t Clock_minutes;
uint16_t Clock_seconds;
uint8_t Clock_storedHours;
uint8_t Clock_storedMinutes;
uint16_t Clock_storedSeconds;
uint32_t Clock_millisecondsPerMinute;
uint32_t Clock_lastTick;
bool Clock_hasBeenStored;
bool gbFastTime;
#ifndef MASTER
bool gbFreezeTime;
#endif

void
Clock_Initialise(uint32_t scale)
{
	// Preserve the original clock operation and live scalar access order
	re3_debug("[DBG]: Initialising CClock...\n");
	Clock_hours = 12;
	Clock_minutes = 0;
	Clock_seconds = 0;
	Clock_millisecondsPerMinute = scale;
	Clock_lastTick = (*Timer_GetTimeInMilliseconds());
	Clock_hasBeenStored = false;
	re3_debug("[DBG]: CClock ready\n");
#ifndef MASTER
	VarConsole_AddUInt8(&VarConsole,"Time (hour of day)", &Clock_hours, 1, 0, 23, true);
	VarConsole_AddBool(&VarConsole,"Freeze time", &gbFreezeTime, true);
#endif
}

void
Clock_Update(void)
{
	// Preserve the original clock operation and live scalar access order
	if(ClockHost_RightShoulder())
	{
		Clock_minutes += 8;
		Clock_lastTick = (*Timer_GetTimeInMilliseconds());
		
		if(Clock_minutes >= 60)
		{
			Clock_hours++;
			Clock_minutes = 0;
			if(Clock_hours >= 24)
				Clock_hours = 0;
		}
		
	}
#ifndef MASTER
	else if (gbFreezeTime)
		Clock_lastTick = (*Timer_GetTimeInMilliseconds());
#endif
	else if((*Timer_GetTimeInMilliseconds()) - Clock_lastTick > Clock_millisecondsPerMinute || gbFastTime)
	{
		Clock_minutes++;
		Clock_lastTick += Clock_millisecondsPerMinute;
		
		if ( gbFastTime )
			Clock_lastTick = (*Timer_GetTimeInMilliseconds());
		
		if(Clock_minutes >= 60)
		{
			Clock_hours++;
			Clock_minutes = 0;
			if(Clock_hours >= 24)
			{
				Stats_DaysPassed++;
				Clock_hours = 0;
			}
		}
	}
	Clock_seconds = 60 * ((*Timer_GetTimeInMilliseconds()) - Clock_lastTick) / Clock_millisecondsPerMinute;
}

void
Clock_SetGameClock(uint8_t h, uint8_t m)
{
	// Preserve the original clock operation and live scalar access order
	while (m >= 60) {
		m -= 60;
		h++;
	}
	Clock_minutes = m;
	while (h >= 24)
		h -= 24;
	Clock_hours = h;
	Clock_seconds = 0;
	Clock_lastTick = (*Timer_GetTimeInMilliseconds());
}

int32_t
Clock_GetGameClockMinutesUntil(uint8_t h, uint8_t m)
{
	// Preserve the original clock operation and live scalar access order
	int32_t now, then;
	now = Clock_hours*60 + Clock_minutes;
	then = h*60 + m;
	if(then < now)
		then += 24*60;
	return then-now;
}

bool
Clock_GetIsTimeInRange(uint8_t h1, uint8_t h2)
{
	// Preserve the original clock operation and live scalar access order
	if(h1 > h2)
		return Clock_hours >= h1 || Clock_hours < h2;
	else
		return Clock_hours >= h1 && Clock_hours < h2;
}

void
Clock_StoreClock(void)
{
	// Preserve the original clock operation and live scalar access order
	Clock_storedHours = Clock_hours;
	Clock_storedMinutes = Clock_minutes;
	Clock_storedSeconds = Clock_seconds;
	Clock_hasBeenStored = true;
}

void
Clock_RestoreClock(void)
{
	// Preserve the original clock operation and live scalar access order
	Clock_hours = Clock_storedHours;
	Clock_minutes = Clock_storedMinutes;
	Clock_seconds = Clock_storedSeconds;
}

uint8_t
Clock_GetHours(void)
{
    // Access the original clock scalar without copying reference-backed storage
    return Clock_hours;
}

uint8_t
Clock_GetMinutes(void)
{
    // Access the original clock scalar without copying reference-backed storage
    return Clock_minutes;
}

int16_t
Clock_GetSeconds(void)
{
    // Access the original clock scalar without copying reference-backed storage
    return Clock_seconds;
}

uint8_t *
Clock_GetHoursRef(void)
{
    // Access the original clock scalar without copying reference-backed storage
    return &Clock_hours;
}

uint8_t *
Clock_GetMinutesRef(void)
{
    // Access the original clock scalar without copying reference-backed storage
    return &Clock_minutes;
}

//- rouz edit (ChatGPT)
