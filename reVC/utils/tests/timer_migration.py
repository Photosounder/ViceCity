"""Convert the static timer class and reference accessors to actual C11 state and operations."""
import re
import cull_zones_migration as cull
FIELDS={
 'm_snTimeInMilliseconds':('uint32_t','Timer_timeMs'),
 'm_snTimeInMillisecondsPauseMode':('uint32_t','Timer_pauseTimeMs'),
 'm_snTimeInMillisecondsNonClipped':('uint32_t','Timer_nonClippedTimeMs'),
 'm_snPreviousTimeInMilliseconds':('uint32_t','Timer_previousTimeMs'),
 'm_FrameCounter':('uint32_t','Timer_frameCounter'),
 'ms_fTimeScale':('float','Timer_timeScale'),
 'ms_fTimeStep':('float','Timer_timeStep'),
 'ms_fTimeStepNonClipped':('float','Timer_nonClippedTimeStep'),
 'm_UserPause':('bool','Timer_userPause'),
 'm_CodePause':('bool','Timer_codePause'),
 'm_LogicalFrameCounter':('uint32_t','Timer_logicalFrameCounter'),
 'm_LogicalFramesPassed':('uint32_t','Timer_logicalFramesPassed'),
}
REFS=['GetTimeStep','GetTimeStepNonClipped','GetFrameCounter','GetTimeInMilliseconds','GetTimeScale']
METHODS=['Initialise','Shutdown','Update','Suspend','Resume','GetCyclesPerMillisecond','GetCurrentTimeInCycles','GetIsSlowMotionActive','Stop','StartUserPause','EndUserPause','GetCyclesPerFrame']
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Timer.h"
#include "Record.h"
#include "SpecialFX.h"

extern "C" TimerPlatformTime
TimerHost_PlatformTime(void)
{
    // Read the original platform timer with its platform-specific scalar type
    return RsTimer();
}

extern "C" bool
TimerHost_SnapshotActive(void)
{
    // Read the original snapshot flag after the time-step and pause gates
    return CSpecialFX::bSnapShotActive;
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
'''

def calls(source):
    # Keep each former reference expression as a live const lvalue backed by its C pointer getter
    for name in REFS:source=source.replace('CTimer::'+name+'()', '(*Timer_'+name+'())')
    for name,(_,target) in FIELDS.items():source=re.sub(r'CTimer::'+name+r'\b',target,source)
    return cull.calls(re.sub(r'CTimer::(\w+)\b',r'Timer_\1',source))

def local(source):
    # Rewrite timer members and nested pause calls without changing their evaluation points
    for name,(_,target) in FIELDS.items():source=re.sub(r'\b'+name+r'\b',target,source)
    source=source.replace('GetIsPaused()', 'Timer_GetIsPaused()')
    source=source.replace('CSpecialFX::bSnapShotActive','TimerHost_SnapshotActive()').replace('SpecialFX_bSnapShotActive','TimerHost_SnapshotActive()')
    source=source.replace('CRecordDataForGame::IsPlayingBack()','TimerHost_PlayingBack()').replace('CRecordDataForChase::IsRecording()','TimerHost_ChaseRecording()')
    source=source.replace('RsTimer()', 'TimerHost_PlatformTime()')
    source=source.replace('debug("','re3_debug("[DBG]: ')
    source=re.sub(r'uint32\(([^()]*)\)',r'((uint32_t)(\1))',source)
    for a,b in [('uint32','uint32_t'),('int32','int32_t'),('RsTimerType','TimerPlatformTime')]:source=re.sub(r'\b'+a+r'\b',b,source)
    return source

def header(source):
    # Generate C inline accessors with the exact original scalar types and conditional availability
    result='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif
'''
    for name,(kind,target) in FIELDS.items():
        if name=='m_LogicalFrameCounter':result+='#ifdef FIX_BUGS\n'
        result+='extern '+kind+' '+target+';\n'
    result+='#endif\n\n'
    for name in METHODS:
        result+=('bool' if name=='GetIsSlowMotionActive' else 'uint32_t' if name.startswith('Get') else 'void')+' Timer_'+name+'(void);\n'
    for match in re.finditer(r'static\s+([^\n{};]+?)\b(\w+)\(([^)]*)\)\s*\{([^}]*)\}',source):
        kind,name,args,body=match.groups();kind=kind.strip()
        if name=='GetDefaultTimeStep':result+='#ifdef FIX_BUGS\n'
        kind=kind.replace('&','*')
        if name in REFS:body=body.replace('return ', 'return &')
        body=local(body)
        if name=='GetTimeStepFix':body=body.replace('GetTimeStep()', '(*Timer_GetTimeStep())').replace('GetDefaultTimeStep()', 'Timer_GetDefaultTimeStep()')
        for a,b in [('uint32','uint32_t')]:kind=re.sub(r'\b'+a+r'\b',b,kind);args=re.sub(r'\b'+a+r'\b',b,args)
        result+='static inline '+kind+'\nTimer_'+name+'('+('void' if not args else args)+')\n{\n    // Preserve the original scalar access and reference-backed storage\n    '+body.strip()+'\n}\n\n'
    result+='''#endif
#ifdef _WIN32
typedef uint32_t TimerPlatformTime;
#else
typedef double TimerPlatformTime;
#endif
TimerPlatformTime TimerHost_PlatformTime(void);
bool TimerHost_SnapshotActive(void);
bool TimerHost_PlayingBack(void);
bool TimerHost_ChaseRecording(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return result

def kernel(source):
    # Retain the complete original function bodies and shared fractional state in a C11 translation unit
    a=source.index('uint32 CTimer::m_snTimeInMilliseconds;');b=source.index('//- rouz edit (ChatGPT)',a)
    source=source[a:b]
    source=source.replace('#define RsTimerType uint32\n','').replace('#define RsTimerType double\n','')
    source=source.replace('#else\n#endif\n','\n#endif\n',1)
    source=local(calls(source))
    source=source.replace('Min(3.0f, Timer_timeStep)','TIMER_MIN(3.0f, Timer_timeStep)')
    source=source.replace('GetCyclesPerFrame()', 'GetCyclesPerFrame(void)')
    source=re.sub(r'(\bTimer_\w+\([^\n]*\)\n\{[^\n]*\n)',r'\1\t// Preserve the original timing operation, arithmetic and live state access order\n',source)
    return '''//+ rouz edit (ChatGPT)
#include "Timer.h"
#include "../audio/DMAudio.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define OutputDebugString(s) re3_debug("[DBG-2]: %s\\n",s)
#endif
void re3_debug(const char *format, ...);
#define TIMER_MIN(a,b) ((a) < (b) ? (a) : (b))

'''+source+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register the timer kernel as C11 while retaining the four typed C++ game readers
    if 'Timer.c"' in source:return cull.project(source)
    entry='<ClCompile Include="..\\src\\core\\Timer.cpp" />'
    if entry in source:return cull.project(source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\core\\Timer.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'))
    a=source.index('    <ClCompile Include="..\\src\\core\\Timer.cpp"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return cull.project(source[:b]+'\n'+source[a:b].replace('Timer.cpp','Timer.c')+source[b:])
