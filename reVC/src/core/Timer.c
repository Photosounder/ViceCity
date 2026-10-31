//+ rouz edit (ChatGPT)
#include "Timer.h"
#include "../audio/DMAudio.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define OutputDebugString(s) re3_debug("[DBG-2]: %s\n",s)
#endif
void re3_debug(const char *format, ...);
#define TIMER_MIN(a,b) ((a) < (b) ? (a) : (b))

uint32_t Timer_timeMs;
uint32_t Timer_pauseTimeMs = 1;

uint32_t Timer_nonClippedTimeMs;
uint32_t Timer_previousTimeMs;
uint32_t Timer_frameCounter;
float Timer_timeScale;
float Timer_timeStep;
float Timer_nonClippedTimeStep;
bool  Timer_userPause;
bool  Timer_codePause; 
#ifdef FIX_BUGS
uint32_t Timer_logicalFrameCounter;
uint32_t Timer_logicalFramesPassed;
#endif

uint32_t _nCyclesPerMS = 1;

#ifdef _WIN32
LARGE_INTEGER _oldPerfCounter;
LARGE_INTEGER perfSuspendCounter;

#endif

TimerPlatformTime oldPcTimer;

TimerPlatformTime suspendPcTimer;

uint32_t suspendDepth;

void Timer_Initialise(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	re3_debug("[DBG]: Initialising CTimer...\n");
	
	Timer_timeScale = 1.0f;
	Timer_timeStep = 1.0f;
	suspendDepth = 0;
	Timer_userPause = false;
	Timer_codePause = false;
	Timer_nonClippedTimeMs = 0;
	Timer_previousTimeMs = 0;
	Timer_timeMs = 1;
#ifdef FIX_BUGS
	Timer_logicalFrameCounter = 0;
	Timer_logicalFramesPassed = 0;
#endif
	
#ifdef _WIN32
	LARGE_INTEGER perfFreq;
	if ( QueryPerformanceFrequency(&perfFreq) )
	{
		OutputDebugString("Performance counter available\n");
		_nCyclesPerMS = ((uint32_t)(perfFreq.QuadPart / 1000));
		QueryPerformanceCounter(&_oldPerfCounter);
	}
	else
#endif
	{
		OutputDebugString("Performance counter not available, using millesecond timer\n");
		_nCyclesPerMS = 0;
		oldPcTimer = TimerHost_PlatformTime();
	}
	
	Timer_timeMs = Timer_previousTimeMs;
	
	Timer_frameCounter = 0;
	
	DMAudio_ResetTimers(Timer_previousTimeMs);
	
	re3_debug("[DBG]: CTimer ready\n");
}

void Timer_Shutdown(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	;
}

#ifdef FIX_BUGS
void Timer_Update(void)
{ 
	// Preserve the original timing operation, arithmetic and live state access order
	static double frameTimeLogical = 0.0;
	static double frameTimeFraction = 0.0;
	static double frameTimeFractionScaled = 0.0;
	double frameTime;
	double dblUpdInMs;

	Timer_previousTimeMs = Timer_timeMs;
	
#ifdef _WIN32
	if ( (double)_nCyclesPerMS != 0.0 )
	{
		LARGE_INTEGER pc;
		QueryPerformanceCounter(&pc);
		
		int32_t updInCycles = (pc.LowPart - _oldPerfCounter.LowPart); // & 0x7FFFFFFF; pointless
		
		_oldPerfCounter = pc;
		
		float updInCyclesScaled = Timer_GetIsPaused() ? updInCycles : updInCycles * Timer_timeScale;
		
		frameTime = updInCyclesScaled / (double)_nCyclesPerMS;

		dblUpdInMs = (double)updInCycles / (double)_nCyclesPerMS;
	}
	else
#endif
	{
		TimerPlatformTime timer = TimerHost_PlatformTime();
		
		TimerPlatformTime updInMs = timer - oldPcTimer;
		
		frameTime = (double)updInMs * Timer_timeScale;
		
		oldPcTimer = timer;

		dblUpdInMs = (double)updInMs;
	}

	// count frames as if we're running at 30 fps
	Timer_logicalFramesPassed = 0;
	frameTimeLogical += dblUpdInMs;
	while (frameTimeLogical >= 1000.0 / 30.0) {
		frameTimeLogical -= 1000.0 / 30.0;
		Timer_logicalFramesPassed++;
	}
	Timer_logicalFrameCounter += Timer_logicalFramesPassed;

	frameTimeFraction += dblUpdInMs;
	frameTimeFractionScaled += frameTime;

	Timer_pauseTimeMs += ((uint32_t)(frameTimeFraction));
		
	if ( Timer_GetIsPaused() )
		Timer_timeStep = 0.0f;
	else
	{
		Timer_timeMs += ((uint32_t)(frameTimeFractionScaled));
		Timer_nonClippedTimeMs += ((uint32_t)(frameTimeFractionScaled));
		Timer_timeStep = frameTime / 1000.0f * 50.0f;
	}
	frameTimeFraction -= ((uint32_t)(frameTimeFraction));
	frameTimeFractionScaled -= ((uint32_t)(frameTimeFractionScaled));
	
	if ( Timer_timeStep < 0.01f && !Timer_GetIsPaused() && !TimerHost_SnapshotActive())
		Timer_timeStep = 0.01f;

	Timer_nonClippedTimeStep = Timer_timeStep;
	
	if ( !TimerHost_PlayingBack() )
	{
		Timer_timeStep = TIMER_MIN(3.0f, Timer_timeStep);

		if ( (Timer_timeMs - Timer_previousTimeMs) > 60 )
			Timer_timeMs = Timer_previousTimeMs + 60;
	}
  
	if ( TimerHost_ChaseRecording() )
	{
		Timer_timeStep = 1.0f;
		Timer_timeMs = Timer_previousTimeMs + 16;
	}
  
	Timer_frameCounter++;
}
#else
void Timer_Update(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	Timer_previousTimeMs = Timer_timeMs;
	
#ifdef _WIN32
	if ( (double)_nCyclesPerMS != 0.0 )
	{
		LARGE_INTEGER pc;
		QueryPerformanceCounter(&pc);
		
		int32_t updInCycles = (pc.LowPart - _oldPerfCounter.LowPart); // & 0x7FFFFFFF; pointless
		
		_oldPerfCounter = pc;
		
		float updInCyclesScaled = Timer_GetIsPaused() ? updInCycles : updInCycles * Timer_timeScale;
		
		double frameTime = updInCyclesScaled / (double)_nCyclesPerMS;

		Timer_pauseTimeMs = Timer_pauseTimeMs + frameTime;
		
		if ( Timer_GetIsPaused() )
			Timer_timeStep = 0.0f;
		else
		{
			Timer_timeMs = Timer_timeMs + frameTime;
			Timer_nonClippedTimeMs = Timer_nonClippedTimeMs + frameTime;
			Timer_timeStep = frameTime / 1000.0f * 50.0f;
		}
	}
	else
#endif
	{
		TimerPlatformTime timer = TimerHost_PlatformTime();
		
		TimerPlatformTime updInMs = timer - oldPcTimer;
		
		double frameTime = (double)updInMs * Timer_timeScale;
		
		oldPcTimer = timer;

		Timer_pauseTimeMs = Timer_pauseTimeMs + frameTime;

		if ( Timer_GetIsPaused() )
			Timer_timeStep = 0.0f;
		else
		{
			Timer_timeMs = Timer_timeMs + frameTime;
			Timer_nonClippedTimeMs = Timer_nonClippedTimeMs + frameTime;
			Timer_timeStep = frameTime / 1000.0f * 50.0f;
		}
	}
	
	if ( Timer_timeStep < 0.01f && !Timer_GetIsPaused() && !TimerHost_SnapshotActive())
		Timer_timeStep = 0.01f;

	Timer_nonClippedTimeStep = Timer_timeStep;
	
	if ( !TimerHost_PlayingBack() )
	{
		Timer_timeStep = TIMER_MIN(3.0f, Timer_timeStep);

		if ( (Timer_timeMs - Timer_previousTimeMs) > 60 )
			Timer_timeMs = Timer_previousTimeMs + 60;
	}
  
	if ( TimerHost_ChaseRecording() )
	{
		Timer_timeStep = 1.0f;
		Timer_timeMs = Timer_previousTimeMs + 16;
	}
  
	Timer_frameCounter++;
}
#endif

void Timer_Suspend(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	if ( ++suspendDepth > 1 )
		return;
	
#ifdef _WIN32
	if ( (double)_nCyclesPerMS != 0.0 )
		QueryPerformanceCounter(&perfSuspendCounter);
	else
#endif
		suspendPcTimer = TimerHost_PlatformTime();
}

void Timer_Resume(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	if ( --suspendDepth != 0 )
		return;

#ifdef _WIN32
	if ( (double)_nCyclesPerMS != 0.0 )
	{
		LARGE_INTEGER pc;
		QueryPerformanceCounter(&pc);

		_oldPerfCounter.LowPart += pc.LowPart - perfSuspendCounter.LowPart;
	}
	else
#endif
		oldPcTimer += TimerHost_PlatformTime() - suspendPcTimer;
}

uint32_t Timer_GetCyclesPerMillisecond(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
#ifdef _WIN32
	if (_nCyclesPerMS != 0)
		return _nCyclesPerMS;
	else 
#endif
		return 1;
}

uint32_t Timer_GetCurrentTimeInCycles(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
#ifdef _WIN32
	if ( _nCyclesPerMS != 0 )
	{
		LARGE_INTEGER pc;
		QueryPerformanceCounter(&pc);
		return (pc.LowPart - _oldPerfCounter.LowPart); // & 0x7FFFFFFF; pointless
	}
	else
#endif
		return TimerHost_PlatformTime() - oldPcTimer;
}

bool Timer_GetIsSlowMotionActive(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	return Timer_timeScale < 1.0f;
}

void Timer_Stop(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	Timer_previousTimeMs = Timer_timeMs;
}

void Timer_StartUserPause(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	Timer_userPause = true;
}

void Timer_EndUserPause(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	Timer_userPause = false;
}

uint32_t Timer_GetCyclesPerFrame(void)
{
	// Preserve the original timing operation, arithmetic and live state access order
	return 20;
}



//- rouz edit (ChatGPT)
