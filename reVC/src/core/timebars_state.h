//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#define MAX_TIMERS 50
#define MAX_MS_COLLECTED 40

typedef struct sTimeBar {
    char name[20];
    float startTime;
    float endTime;
    int32_t unk;
} sTimeBar;
typedef struct TimeBarState {
    sTimeBar Timers[MAX_TIMERS];
    uint32_t count;
} TimeBarState;
#ifdef __cplusplus
extern "C" {
#endif
extern TimeBarState TimerBar;
extern float MaxTimes[MAX_TIMERS];
extern float MaxFrameTime;
extern uint32_t curMS;
extern uint32_t msCollected[MAX_MS_COLLECTED];
extern float FrameInitTime;
void tbInit(void);
void tbStartTimer(int32_t value, const char *name);
void tbEndTimer(const char *name);
float Diag_GetFPS(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
