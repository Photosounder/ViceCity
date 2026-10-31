//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "Clock.h"
#include "Timer.h"
#include "Pad.h"
#include "Stats.h"
#include "VarConsole.h"

_TODO("gbFastTime");

extern "C" uint32_t
ClockHost_Time(void)
{
    // Read the live game timer at each original clock access
    return (*Timer_GetTimeInMilliseconds());
}

extern "C" bool
ClockHost_RightShoulder(void)
{
    // Preserve the original second-pad lookup and shoulder-button query
    return CPad::GetPad(1)->GetRightShoulder1();
}

extern "C" int32_t *
ClockHost_DaysPassed(void)
{
    // Expose the existing signed statistics counter for the original C increment
    return &Stats_DaysPassed;
}

#ifndef MASTER
extern "C" void
ClockHost_RegisterHour(uint8_t *hour)
{
    // Register the original live hour byte with unchanged console bounds
    VarConsole_AddUInt8(&VarConsole,"Time (hour of day)", hour, 1, 0, 23, true); // rouz edit (ChatGPT)
}

extern "C" void
ClockHost_RegisterFreeze(bool *freeze)
{
    // Register the original boolean freeze flag without changing its console entry type
    VarConsole_AddBool(&VarConsole,"Freeze time", freeze, true); // rouz edit (ChatGPT)
}
#endif
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
