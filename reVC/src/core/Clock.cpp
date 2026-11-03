//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "Clock.h"
#include "Timer.h"
#include "Pad.h"
#include "Stats.h"
#include "VarConsole.h"

_TODO("gbFastTime");

extern "C" bool
ClockHost_RightShoulder(void)
{
    // Preserve the original second-pad lookup and shoulder-button query
    return CPad::GetPad(1)->GetRightShoulder1();
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
