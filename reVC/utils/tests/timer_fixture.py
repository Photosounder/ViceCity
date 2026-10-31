"""Compare complete timer operations and C pointer accessors against the original class."""
from pathlib import Path
import re
import timer_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/timer-c-tests'
old=(out/'before/src/core/Timer.cpp').read_text();oldh=(out/'before/src/core/Timer.h').read_text()
inline=list(re.finditer(r'static\s+([^\n{};]+?)\b(\w+)\(([^)]*)\)\s*\{([^}]*)\}',oldh))
ops=[(name,'bool' if name=='GetIsSlowMotionActive' else 'uint32' if name.startswith('Get') else 'void','void') for name in m.METHODS]
ops += [(a.group(2),a.group(1).strip(),a.group(3)) for a in inline]
assert len(ops)==40

def dispatch(before):
    # Generate calls from the original signatures while retaining all reference identities
    result='switch(op){\n'
    for index,(name,kind,args) in enumerate(ops):
        fix=name in ['GetDefaultTimeStep','GetTimeStepFix','GetLogicalFrameCounter','GetLogicalFramesPassed']
        if fix:result+='#ifdef FIX_BUGS\n'
        argument='' if args in ['void',''] else 'f' if 'float' in args else 'b' if 'bool' in args else 'arg'
        call=('CTimer::' if before else 'Timer_')+name+'('+argument+')'
        if name in m.REFS:
            field=re.search(r'return\s+(\w+);',next(a.group(4) for a in inline if a.group(2)==name)).group(1)
            if before:statement='assert(&'+call+'==&CTimer::'+field+');return '+('bits('+call+')' if 'float' in kind else call)+';'
            else:statement='assert('+call+'==&'+m.FIELDS[field][1]+');return '+('bits(*'+call+')' if 'float' in kind else '*'+call)+';'
        elif kind=='void':statement=call+';return 0;'
        elif 'float' in kind:statement='return bits('+call+');'
        else:statement='return '+call+';'
        result+='case '+str(index)+':'+statement+'\n'
        if fix:result+='#else\ncase '+str(index)+':return 0;\n#endif\n'
    return result+'default:assert(0);return 0;\n}'

force='''#include <windows.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI TestTimerFrequency(LARGE_INTEGER *value);
BOOL WINAPI TestTimerCounter(LARGE_INTEGER *value);
void TestTimerOutput(const char *text);
#ifdef __cplusplus
}
#endif
#undef QueryPerformanceFrequency
#undef QueryPerformanceCounter
#undef OutputDebugString
#define QueryPerformanceFrequency TestTimerFrequency
#define QueryPerformanceCounter TestTimerCounter
#define OutputDebugString TestTimerOutput
'''
portable='#undef _WIN32\n#undef OutputDebugString\n#define OutputDebugString(s) re3_debug("[DBG-2]: %s\\n",s)\n'
for shape,extra in {'default':'','vanilla':'#undef FIX_BUGS\n','master':'#define MASTER\n','portable':portable,'portable-vanilla':portable+'#undef FIX_BUGS\n'}.items():(out/(shape+'.h')).write_text(force+extra,newline='\n')
for variant in ['before','after']:
    # Compile the preserved class and whole source with the same deterministic external inputs
    source=(root/'utils/tests/timer.cpp.in').read_text()
    source=source.replace('@HEADER@',oldh.replace('class CTimer\n{','class CTimer\n{\npublic:') if variant=='before' else '#include "Timer.h"')
    if variant=='before':body=re.sub(r'(?m)^#include[^\n]*\n|^#define WITHWINDOWS\n','',old)
    else:body=re.sub(r'(?m)^#include[^\n]*\n','',m.HOST)
    source=source.replace('@BODY@',body).replace('@RUN@',dispatch(True) if variant=='before' else 'return TestTimerC(op,arg,f,b);')
    for name,(_,target) in m.FIELDS.items():source=source.replace('@'+name+'@','CTimer::'+name if variant=='before' else target)
    (out/(variant+'.cpp')).write_text(source,newline='\n')
caller='''#include "Timer.h"
#include <assert.h>
#include <string.h>
static uint32_t bits(float value)
{
    // Observe floating-point return values without changing their representation
    uint32_t result;memcpy(&result,&value,4);return result;
}
uint32_t TestTimerC(unsigned op,uint32_t arg,float f,bool b)
{
    // Call every operation and confirm reference-backed getter identity from actual C
'''+dispatch(False)+'\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
print('Complete original timer class and source, 40 operation/accessor entries and five live aliases prepared')
