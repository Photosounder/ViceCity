//+ rouz edit (ChatGPT)
#include "config.h"
#ifndef MASTER
#include "timebars_state.h"
#define FRAMETIME
#include "Timer.h"
#include "timebars_backend.h"
#include "FontText.h"
#include <stdio.h>
#include <string.h>
void re3_assert(const char *expression, const char *filename, unsigned int line, const char *function);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))

TimeBarState TimerBar;
float MaxTimes[MAX_TIMERS];
float MaxFrameTime;
uint32_t curMS;
uint32_t msCollected[MAX_MS_COLLECTED];
#ifdef FRAMETIME
float FrameInitTime;
#endif

void tbInit(void)
{
    // Preserve the original timing calculation and state updates
	TimerBar.count = 0;
	uint32_t i = (*Timer_GetFrameCounter()) & 0x7F;
	if (i == 0) {
		do
			MaxTimes[i++] = 0.0f;
		while (i != MAX_TIMERS);
#ifdef FRAMETIME
		MaxFrameTime = 0.0f;
#endif
	}
#ifdef FRAMETIME
	FrameInitTime = (float)Timer_GetCurrentTimeInCycles() / (float)Timer_GetCyclesPerFrame();
#endif
}

void tbStartTimer(int32_t unk, const char *name)
{
    // Preserve the original timing calculation and state updates
	strcpy(TimerBar.Timers[TimerBar.count].name, name);
	TimerBar.Timers[TimerBar.count].unk = unk;
	TimerBar.Timers[TimerBar.count].startTime = (float)Timer_GetCurrentTimeInCycles() / (float)Timer_GetCyclesPerFrame();
	TimerBar.count++;
}

void tbEndTimer(const char* name)
{
    // Preserve the original timing calculation and state updates
	uint32_t n = 1500;
	for (uint32_t i = 0; i < TimerBar.count; i++) {
		if (strcmp(name, TimerBar.Timers[i].name) == 0)
			n = i;
	}
	assert(n != 1500);
	TimerBar.Timers[n].endTime = (float)Timer_GetCurrentTimeInCycles() / (float)Timer_GetCyclesPerFrame();
}

float Diag_GetFPS(void)
{
    // Preserve the original timing calculation and state updates
	return 39000.0f / (msCollected[(curMS - 1) % MAX_MS_COLLECTED] - msCollected[curMS % MAX_MS_COLLECTED]);
}

void tbDisplay(void)
{
    // Preserve the original timing sample, formatting and ordered display operations
    char temp[200];
    uint16_t wtemp[200];
#ifdef FRAMETIME
    float FrameEndTime = (float)Timer_GetCurrentTimeInCycles() / (float)Timer_GetCyclesPerFrame();
#endif
    msCollected[(curMS++) % MAX_MS_COLLECTED] = RsTimer();
    TimeBars_PrepareFont();
    sprintf(temp, "FPS: %.2f", Diag_GetFPS());
    AsciiToUnicode(temp, wtemp);
    TimeBars_SetTextColor();
    if (TimeBars_IsVisible()) {
        // Draw the FPS and enabled timing rows
        TimeBars_Print(4.0f, wtemp);
#ifndef FINAL
        for (uint32_t i = 0; i < TimerBar.count; i++) {
            // Update and display each timer maximum
            MaxTimes[i] = MaxTimes[i] > TimerBar.Timers[i].endTime - TimerBar.Timers[i].startTime ? MaxTimes[i] : TimerBar.Timers[i].endTime - TimerBar.Timers[i].startTime;
            sprintf(temp, "%s: %.2f", &TimerBar.Timers[i].name[0], MaxTimes[i]);
            AsciiToUnicode(temp, wtemp);
            TimeBars_Print(8.0f * (i + 2), wtemp);
        }
#ifdef FRAMETIME
        // Update and display the frame-time maximum
        MaxFrameTime = MaxFrameTime > FrameEndTime - FrameInitTime ? MaxFrameTime : FrameEndTime - FrameInitTime;
        sprintf(temp, "Frame Time: %.2f", MaxFrameTime);
        AsciiToUnicode(temp, wtemp);
        TimeBars_Print(8.0f * (TimerBar.count + 4), wtemp);
#endif
#endif
    }
}

#endif
//- rouz edit (ChatGPT)
