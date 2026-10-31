//+ rouz edit (ChatGPT)
#include "common.h"
#include "Timer.h"
#include "Record.h"

extern "C" TimerPlatformTime
TimerHost_PlatformTime(void)
{
    // Read the original platform timer with its platform-specific scalar type
    return RsTimer();
}


extern "C" bool
TimerHost_PlayingBack(void)
{
    // Query the original game-record playback state before clipping
    return CRecordDataForGame::IsPlayingBack();
}

extern "C" bool
TimerHost_ChaseRecording(void)
{
    // Query the original chase-record state before its fixed-step override
    return CRecordDataForChase::IsRecording();
}
//- rouz edit (ChatGPT)
