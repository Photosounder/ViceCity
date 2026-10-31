"""Replace the static clock class with C11 storage, complete operations and pointer accessors."""
import re
import timer_migration as timer
from audio_controls_migration import function
FIELDS={
    'ms_nGameClockHours':('uint8_t','Clock_hours'),
    'ms_nGameClockMinutes':('uint8_t','Clock_minutes'),
    'ms_nGameClockSeconds':('uint16_t','Clock_seconds'),
    'ms_Stored_nGameClockHours':('uint8_t','Clock_storedHours'),
    'ms_Stored_nGameClockMinutes':('uint8_t','Clock_storedMinutes'),
    'ms_Stored_nGameClockSeconds':('uint16_t','Clock_storedSeconds'),
    'ms_nMillisecondsPerGameMinute':('uint32_t','Clock_millisecondsPerMinute'),
    'ms_nLastClockTick':('uint32_t','Clock_lastTick'),
    'ms_bClockHasBeenStored':('bool','Clock_hasBeenStored'),
}
METHODS=['Initialise','Update','SetGameClock','GetGameClockMinutesUntil','GetIsTimeInRange','StoreClock','RestoreClock']
HOST='''//+ rouz edit (ChatGPT)
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
    return CTimer::GetTimeInMilliseconds();
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
    return &CStats::DaysPassed;
}

#ifndef MASTER
extern "C" void
ClockHost_RegisterHour(uint8_t *hour)
{
    // Register the original live hour byte with unchanged console bounds
    VarConsole.Add("Time (hour of day)", hour, 1, 0, 23, true);
}

extern "C" void
ClockHost_RegisterFreeze(bool *freeze)
{
    // Register the original boolean freeze flag without changing its console entry type
    VarConsole.Add("Freeze time", freeze, true);
}
#endif
//- rouz edit (ChatGPT)
'''

def calls(source):
    # Replace all class-qualified accesses while preserving reference expressions as live C lvalues
    for name,(_,target) in FIELDS.items():source=source.replace('CClock::'+name,target)
    for name in ['GetHoursRef','GetMinutesRef']:source=source.replace('CClock::'+name+'()', '(*Clock_'+name+'())')
    for name in METHODS+['GetHours','GetMinutes','GetSeconds']:source=re.sub(r'CClock::'+name+r'\b','Clock_'+name,source)
    if 'extern bool gbFastTime;' in source:
        source=source.replace('extern bool gbFastTime;','')
        if '#include "Clock.h"' not in source:source=source.replace('#include "Timer.h"','#include "Timer.h"\n#include "Clock.h"',1)
    return timer.calls(source)

def body(source):
    # Retain complete original control flow, repeated timer reads and integer promotion/narrowing
    source=source[source.index('{'):]
    for name,(_,target) in FIELDS.items():source=re.sub(r'\b'+name+r'\b',target,source)
    source=source.replace('CTimer::GetTimeInMilliseconds()', 'ClockHost_Time()').replace('CPad::GetPad(1)->GetRightShoulder1()', 'ClockHost_RightShoulder()')
    source=source.replace('CStats::DaysPassed++','(*ClockHost_DaysPassed())++')
    source=source.replace('VarConsole.Add("Time (hour of day)", &Clock_hours, 1, 0, 23, true)', 'ClockHost_RegisterHour(&Clock_hours)')
    source=source.replace('VarConsole.Add("Freeze time", &gbFreezeTime, true)', 'ClockHost_RegisterFreeze(&gbFreezeTime)')
    source=source.replace('debug("', 're3_debug("[DBG]: ')
    source=source.replace('int32 ', 'int32_t ')
    return source

def header(source):
    # Expose scalar C storage and pointer accessors without a compatibility C++ class
    fields=''.join('extern '+kind+' '+target+';\n' for kind,target in FIELDS.values())
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif
'''+fields+'''extern bool gbFastTime;
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
uint32_t ClockHost_Time(void);
bool ClockHost_RightShoulder(void);
int32_t *ClockHost_DaysPassed(void);
#ifndef MASTER
void ClockHost_RegisterHour(uint8_t *hour);
void ClockHost_RegisterFreeze(bool *freeze);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Own the original zero-initialized clock globals and all former class operations in C11
    result='//+ rouz edit (ChatGPT)\n#include "Clock.h"\n\nvoid re3_debug(const char *format, ...);\n\n'
    result+=''.join(kind+' '+target+';\n' for kind,target in FIELDS.values())
    result+='bool gbFastTime;\n#ifndef MASTER\nbool gbFreezeTime;\n#endif\n\n'
    for name in METHODS:
        original=function(source,'CClock::',name);signature=original[:original.index('{')].replace('CClock::','Clock_')
        for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('int32','int32_t')]:signature=re.sub(r'\b'+a+r'\b',b,signature)
        value=body(original).replace('{\n','{\n\t// Preserve the original clock operation and live scalar access order\n',1)
        result+=signature+value+'\n\n'
    for name,kind,target in [('GetHours','uint8_t','Clock_hours'),('GetMinutes','uint8_t','Clock_minutes'),('GetSeconds','int16_t','Clock_seconds'),('GetHoursRef','uint8_t *','&Clock_hours'),('GetMinutesRef','uint8_t *','&Clock_minutes')]:
        result+=kind+'\nClock_'+name+'(void)\n{\n    // Access the original clock scalar without copying reference-backed storage\n    return '+target+';\n}\n\n'
    return result+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Keep typed game readers in C++ and register the complete clock implementation as C11
    if 'Clock.c"' in source:return timer.project(source)
    if '<ClCompile Include="..\\src\\core\\Clock.cpp" />' in source:
        return timer.project(source.replace('<ClCompile Include="..\\src\\core\\Clock.cpp" />','<ClCompile Include="..\\src\\core\\Clock.cpp" />\n    <ClCompile Include="..\\src\\core\\Clock.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'))
    a=source.index('    <ClCompile Include="..\\src\\core\\Clock.cpp"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return timer.project(source[:b]+'\n'+source[a:b].replace('Clock.cpp','Clock.c')+source[b:])
