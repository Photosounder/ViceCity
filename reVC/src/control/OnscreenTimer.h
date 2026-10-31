//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

enum { COUNTER_DISPLAY_NUMBER, COUNTER_DISPLAY_BAR };
typedef struct COnscreenTimerEntry {
    uint32_t m_nClockOffset;
    char m_aClockText[10];
    char m_aClockBuffer[40];
    bool m_bClockProcessed;
    bool m_bClockGoingDown;
} COnscreenTimerEntry;
typedef struct COnscreenCounterEntry {
    uint32_t m_nCounterOffset;
    char m_aCounterText[10];
    uint16_t m_nType;
    char m_aCounterBuffer[40];
    bool m_bCounterProcessed;
} COnscreenCounterEntry;
typedef struct COnscreenTimer {
    COnscreenTimerEntry m_sClocks[NUMONSCREENCLOCKS];
    COnscreenCounterEntry m_sCounters[NUMONSCREENCOUNTERS];
    bool m_bProcessed;
    bool m_bDisabled;
} COnscreenTimer;
#ifdef __cplusplus
extern "C" {
#endif
extern COnscreenTimer OnscreenTimer_state;
void OnscreenTimer_Init(COnscreenTimer *self);
void OnscreenTimer_Process(COnscreenTimer *self);
void OnscreenTimer_ProcessForDisplay(COnscreenTimer *self);
void OnscreenTimer_ClearCounter(COnscreenTimer *self,uint32_t offset);
void OnscreenTimer_ClearClock(COnscreenTimer *self,uint32_t offset);
void OnscreenTimer_AddCounter(COnscreenTimer *self,uint32_t offset,uint16_t type,char *text,uint16_t pos);
void OnscreenTimer_AddClock(COnscreenTimer *self,uint32_t offset,char *text,bool bGoingDown);
void OnscreenClock_Process(COnscreenTimerEntry *self);
void OnscreenClock_ProcessForDisplayClock(COnscreenTimerEntry *self);
void OnscreenCounter_ProcessForDisplayCounter(COnscreenCounterEntry *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
