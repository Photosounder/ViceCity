//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "RsTimer.h"
#ifdef __cplusplus
extern "C" {
#endif
extern uint32_t Timer_timeMs;
extern uint32_t Timer_pauseTimeMs;
extern uint32_t Timer_nonClippedTimeMs;
extern uint32_t Timer_previousTimeMs;
extern uint32_t Timer_frameCounter;
extern float Timer_timeScale;
extern float Timer_timeStep;
extern float Timer_nonClippedTimeStep;
extern bool Timer_userPause;
extern bool Timer_codePause;
#ifdef FIX_BUGS
extern uint32_t Timer_logicalFrameCounter;
extern uint32_t Timer_logicalFramesPassed;
#endif

void Timer_Initialise(void);
void Timer_Shutdown(void);
void Timer_Update(void);
void Timer_Suspend(void);
void Timer_Resume(void);
uint32_t Timer_GetCyclesPerMillisecond(void);
uint32_t Timer_GetCurrentTimeInCycles(void);
bool Timer_GetIsSlowMotionActive(void);
void Timer_Stop(void);
void Timer_StartUserPause(void);
void Timer_EndUserPause(void);
uint32_t Timer_GetCyclesPerFrame(void);
static inline const float *
Timer_GetTimeStep(void)
{
    // Preserve the original scalar access and reference-backed storage
    return &Timer_timeStep;
}

static inline void
Timer_SetTimeStep(float ts)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_timeStep = ts;
}

static inline float
Timer_GetTimeStepInSeconds(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_timeStep / 50.0f;
}

static inline uint32_t
Timer_GetTimeStepInMilliseconds(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_timeStep / 50.0f * 1000.0f;
}

static inline const float *
Timer_GetTimeStepNonClipped(void)
{
    // Preserve the original scalar access and reference-backed storage
    return &Timer_nonClippedTimeStep;
}

static inline float
Timer_GetTimeStepNonClippedInSeconds(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_nonClippedTimeStep / 50.0f;
}

static inline float
Timer_GetTimeStepNonClippedInMilliseconds(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_nonClippedTimeStep / 50.0f * 1000.0f;
}

static inline void
Timer_SetTimeStepNonClipped(float ts)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_nonClippedTimeStep = ts;
}

static inline const uint32_t *
Timer_GetFrameCounter(void)
{
    // Preserve the original scalar access and reference-backed storage
    return &Timer_frameCounter;
}

static inline void
Timer_SetFrameCounter(uint32_t fc)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_frameCounter = fc;
}

static inline const uint32_t *
Timer_GetTimeInMilliseconds(void)
{
    // Preserve the original scalar access and reference-backed storage
    return &Timer_timeMs;
}

static inline void
Timer_SetTimeInMilliseconds(uint32_t t)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_timeMs = t;
}

static inline uint32_t
Timer_GetTimeInMillisecondsNonClipped(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_nonClippedTimeMs;
}

static inline void
Timer_SetTimeInMillisecondsNonClipped(uint32_t t)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_nonClippedTimeMs = t;
}

static inline uint32_t
Timer_GetTimeInMillisecondsPauseMode(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_pauseTimeMs;
}

static inline void
Timer_SetTimeInMillisecondsPauseMode(uint32_t t)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_pauseTimeMs = t;
}

static inline uint32_t
Timer_GetPreviousTimeInMilliseconds(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_previousTimeMs;
}

static inline void
Timer_SetPreviousTimeInMilliseconds(uint32_t t)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_previousTimeMs = t;
}

static inline const float *
Timer_GetTimeScale(void)
{
    // Preserve the original scalar access and reference-backed storage
    return &Timer_timeScale;
}

static inline void
Timer_SetTimeScale(float ts)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_timeScale = ts;
}

static inline bool
Timer_GetIsPaused(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_userPause || Timer_codePause;
}

static inline bool
Timer_GetIsUserPaused(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_userPause;
}

static inline bool
Timer_GetIsCodePaused(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_codePause;
}

static inline void
Timer_SetCodePause(bool pause)
{
    // Preserve the original scalar access and reference-backed storage
    Timer_codePause = pause;
}

#ifdef FIX_BUGS
static inline float
Timer_GetDefaultTimeStep(void)
{
    // Preserve the original scalar access and reference-backed storage
    return 50.0f / 30.0f;
}

static inline float
Timer_GetTimeStepFix(void)
{
    // Preserve the original scalar access and reference-backed storage
    return (*Timer_GetTimeStep()) / Timer_GetDefaultTimeStep();
}

static inline uint32_t
Timer_GetLogicalFrameCounter(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_logicalFrameCounter;
}

static inline uint32_t
Timer_GetLogicalFramesPassed(void)
{
    // Preserve the original scalar access and reference-backed storage
    return Timer_logicalFramesPassed;
}

#endif
typedef RsTimerValue TimerPlatformTime;
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
