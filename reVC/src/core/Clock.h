//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t Clock_hours;
extern uint8_t Clock_minutes;
extern uint16_t Clock_seconds;
extern uint8_t Clock_storedHours;
extern uint8_t Clock_storedMinutes;
extern uint16_t Clock_storedSeconds;
extern uint32_t Clock_millisecondsPerMinute;
extern uint32_t Clock_lastTick;
extern bool Clock_hasBeenStored;
extern bool gbFastTime;
#ifndef MASTER
extern bool gbFreezeTime;
#endif
void Clock_Initialise(uint32_t scale);
void Clock_Update(void);
void Clock_SetGameClock(uint8_t h, uint8_t m);
int32_t Clock_GetGameClockMinutesUntil(uint8_t h, uint8_t m);
bool Clock_GetIsTimeInRange(uint8_t h1, uint8_t h2);
void Clock_StoreClock(void);
void Clock_RestoreClock(void);
uint8_t Clock_GetHours(void);
uint8_t Clock_GetMinutes(void);
int16_t Clock_GetSeconds(void);
uint8_t *Clock_GetHoursRef(void);
uint8_t *Clock_GetMinutesRef(void);
bool ClockHost_RightShoulder(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
